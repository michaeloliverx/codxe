#pragma once

#include <cfloat>
#include <cmath>

namespace iw3
{
namespace mp
{
struct usercmd_s;
namespace cj_strafe
{
const float DEGREES = 57.29577951308232f;
const float SHORT_ANGLE = 360.0f / 65536.0f;

struct Input
{
    float velocityX, velocityY;
    float viewYaw, deltaYaw;
    int forward, right;
    float wishspeed, frametime, stopspeed;
    int preferredSide;
    bool lockSide;
};

struct Result
{
    bool valid, canGain;
    int side, commandYaw;
    float yaw, bandLow, bandHigh;
    float gain, currentGain;
};

// The evaluator receives a decoded world yaw and returns horizontal speed squared.
typedef float (*Evaluator)(void *context, float yaw);

inline float Clamp(float value, float low, float high)
{
    return value < low ? low : value > high ? high : value;
}

inline bool Finite(float value)
{
    return value == value && value > -FLT_MAX && value < FLT_MAX;
}

inline float Normalize(float angle)
{
    angle = fmodf(angle, 360.0f);
    if (angle > 180.0f)
        angle -= 360.0f;
    else if (angle < -180.0f)
        angle += 360.0f;
    return angle;
}

inline int CommandYaw(float yaw, float delta)
{
    float relative = fmodf(yaw - delta, 360.0f);
    if (relative < 0.0f)
        relative += 360.0f;
    return static_cast<int>(relative / SHORT_ANGLE) & 65535;
}

inline float DecodedYaw(float yaw, float delta)
{
    return Normalize(CommandYaw(yaw, delta) * SHORT_ANGLE + delta);
}

inline int NearestCommandYaw(float yaw, float delta)
{
    float relative = fmodf(yaw - delta, 360.0f);
    if (relative < 0.0f)
        relative += 360.0f;
    return static_cast<int>(floorf(relative / SHORT_ANGLE + 0.5f)) & 65535;
}

inline float LimitedYaw(float from, float target, float maximum, float delta)
{
    maximum = Clamp(maximum, 0.0f, 180.0f);
    const float wanted = Normalize(target - from);
    int word = NearestCommandYaw(from + Clamp(wanted, -maximum, maximum), delta);
    float result = Normalize(word * SHORT_ANGLE + delta);
    // Quantization must not exceed the turn budget by one encoded angle.
    if (fabsf(Normalize(result - from)) > maximum + 0.0001f)
    {
        word = (word + (Normalize(result - from) > 0.0f ? -1 : 1)) & 65535;
        result = Normalize(word * SHORT_ANGLE + delta);
    }
    return result;
}

namespace detail
{
struct Candidate
{
    float yaw, angle, score;
};

inline Candidate Sample(const Input &input, float velocityYaw, float offset, int side, float angle, Evaluator evaluate,
                        void *context)
{
    Candidate result;
    result.angle = Clamp(angle, 0.0f, 90.0f);
    result.yaw = DecodedYaw(velocityYaw + side * result.angle - offset, input.deltaYaw);
    result.score = evaluate(context, result.yaw);
    return result;
}

inline void Consider(Candidate &best, const Candidate &candidate, float seed)
{
    if (Finite(candidate.score) &&
        (candidate.score > best.score ||
         (candidate.score == best.score && fabsf(candidate.angle - seed) < fabsf(best.angle - seed))))
        best = candidate;
}

inline Candidate Search(const Input &input, float velocityYaw, float offset, int side, float seed, Evaluator evaluate,
                        void *context)
{
    Candidate best = Sample(input, velocityYaw, offset, side, seed, evaluate, context);
    for (int angle = 0; angle <= 90; ++angle)
        Consider(best, Sample(input, velocityYaw, offset, side, static_cast<float>(angle), evaluate, context), seed);

    const float coarse = best.angle;
    for (int step = -20; step <= 20; ++step)
        Consider(best, Sample(input, velocityYaw, offset, side, coarse + step * 0.05f, evaluate, context), seed);

    const float fine = best.angle;
    for (int step = -9; step <= 9; ++step)
        Consider(best, Sample(input, velocityYaw, offset, side, fine + step * SHORT_ANGLE, evaluate, context), seed);
    return best;
}
} // namespace detail

inline Result Solve(const Input &input, Evaluator evaluate, void *context)
{
    Result result = {};
    const float speedSquared = input.velocityX * input.velocityX + input.velocityY * input.velocityY;
    if (!evaluate || !Finite(speedSquared) || speedSquared < 1.0f || !Finite(input.viewYaw) ||
        !Finite(input.deltaYaw) || !Finite(input.wishspeed) || !Finite(input.frametime) || !Finite(input.stopspeed) ||
        input.wishspeed <= 0.0f || input.frametime <= 0.0f || input.frametime > 0.2f ||
        (!input.forward && !input.right))
        return result;

    const float speed = sqrtf(speedSquared);
    const float velocityYaw = atan2f(input.velocityY, input.velocityX) * DEGREES;
    const float offset = atan2f(-static_cast<float>(input.right), static_cast<float>(input.forward)) * DEGREES;
    const float budget = input.frametime * (input.stopspeed > input.wishspeed ? input.stopspeed : input.wishspeed);
    const float seed = acosf(Clamp((input.wishspeed - budget) / speed, 0.0f, 1.0f)) * DEGREES;
    const detail::Candidate left = detail::Search(input, velocityYaw, offset, 1, seed, evaluate, context);
    const detail::Candidate right = detail::Search(input, velocityYaw, offset, -1, seed, evaluate, context);
    if (!Finite(left.score) || !Finite(right.score) || left.score < 0.0f || right.score < 0.0f)
        return result;

    const float leftError = fabsf(Normalize(left.yaw - input.viewYaw));
    const float rightError = fabsf(Normalize(right.yaw - input.viewYaw));
    int side = leftError <= rightError ? 1 : -1;
    // Keep the branch through small stick/view changes; a deliberate turn can select the other side.
    if (input.preferredSide == 1 && leftError <= rightError + 12.0f)
        side = 1;
    else if (input.preferredSide == -1 && rightError <= leftError + 12.0f)
        side = -1;
    if (input.lockSide)
    {
        if (input.preferredSide == 1 || input.preferredSide == -1)
            side = input.preferredSide;
    }
    else
    {
        if (left.score <= speedSquared && right.score > speedSquared)
            side = -1;
        else if (right.score <= speedSquared && left.score > speedSquared)
            side = 1;
    }

    const detail::Candidate &best = side == 1 ? left : right;
    result.valid = true;
    result.canGain = best.score > speedSquared;
    result.side = side;
    result.yaw = best.yaw;
    // best.yaw was already decoded; rounding avoids encoding it one short below the selected value.
    float relative = fmodf(best.yaw - input.deltaYaw + 360.0f, 360.0f);
    if (relative < 0.0f)
        relative += 360.0f;
    result.commandYaw = static_cast<int>(relative / SHORT_ANGLE + 0.5f) & 65535;
    result.gain = sqrtf(best.score) - speed;
    const float current = evaluate(context, DecodedYaw(input.viewYaw, input.deltaYaw));
    result.currentGain = Finite(current) && current >= 0.0f ? sqrtf(current) - speed : 0.0f;
    result.bandLow = result.bandHigh = best.yaw;

    if (result.canGain)
    {
        const float threshold = speed + result.gain * 0.9f;
        for (int direction = -1; direction <= 1; direction += 2)
        {
            for (int step = 1; step <= 60; ++step)
            {
                const float yaw = DecodedYaw(best.yaw + direction * step * 0.25f, input.deltaYaw);
                const float angle = Normalize(yaw + offset - velocityYaw) * side;
                if (angle < 0.0f || angle > 90.0f || evaluate(context, yaw) < threshold * threshold)
                    break;
                if (direction < 0)
                    result.bandLow = yaw;
                else
                    result.bandHigh = yaw;
            }
        }
    }
    return result;
}

void RegisterCommands();
void RegisterDvars();
void Reset();
void ResetClient(int localClientNum, bool clearTarget = false);
void UpdateCommand(int localClientNum, usercmd_s *cmd, bool playback, bool aimingRpg);
void Draw(int localClientNum);
} // namespace cj_strafe
} // namespace mp
} // namespace iw3
