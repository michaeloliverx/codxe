#pragma once

#include "cj_strafe.h"

namespace iw3
{
namespace mp
{
namespace cj_strafe
{
namespace target
{
struct Motion
{
    float x, y, oldX, oldY;
};

struct Input
{
    Motion velocity;
    float x, y, z, verticalSpeed, gravity;
    float targetX, targetY, targetZ;
    float yaw, deltaYaw, yawSpeed, frametime, speed, stopspeed;
    int previousAction, previousSide;
    bool lockAction;
};

struct Result
{
    bool valid;
    int action, side, forward, right;
    float yaw, goalYaw, landingError;
    Motion velocity;
};

typedef Motion (*Evaluator)(void *context, const Motion &velocity, float yaw, int forward, int right);

inline void Action(int index, int &forward, int &right)
{
    const int moves[9][2] = {{127, 0},  {127, 127},  {127, -127},  {0, 127}, {0, -127},
                             {-127, 0}, {-127, 127}, {-127, -127}, {0, 0}};
    forward = moves[index][0];
    right = moves[index][1];
}

inline float FlightTime(float height, float verticalSpeed, float gravity)
{
    if (gravity <= 0.0f)
        return 0.5f;
    const float discriminant = verticalSpeed * verticalSpeed + 2.0f * gravity * height;
    if (discriminant < 0.0f)
        return 0.5f;
    return Clamp((verticalSpeed + sqrtf(discriminant)) / gravity, 0.0f, 1.5f);
}

inline Result Solve(const Input &input, Evaluator evaluate, void *context)
{
    Result best = {};
    if (!evaluate || !Finite(input.x) || !Finite(input.y) || !Finite(input.z) || !Finite(input.targetX) ||
        !Finite(input.targetY) || !Finite(input.targetZ) || !Finite(input.velocity.x) || !Finite(input.velocity.y) ||
        !Finite(input.yaw) || !Finite(input.verticalSpeed) || !Finite(input.gravity) || !Finite(input.deltaYaw) ||
        !Finite(input.yawSpeed) || !Finite(input.speed) || input.speed <= 0.0f || !Finite(input.frametime) ||
        input.frametime <= 0.0f || input.frametime > 0.2f || !Finite(input.stopspeed))
        return best;

    const int steps = static_cast<int>(Clamp(0.25f / input.frametime, 4.0f, 16.0f));
    float bestCost = FLT_MAX;
    for (int action = 0; action < 9; ++action)
    {
        if (input.lockAction && input.previousAction >= 0 && action != input.previousAction)
            continue;
        int forward, right;
        Action(action, forward, right);
        const float offset = atan2f(-static_cast<float>(right), static_cast<float>(forward)) * DEGREES;
        for (int side = -1; side <= 1; ++side)
        {
            if (input.lockAction && input.previousAction >= 0 && side != input.previousSide)
                continue;
            if (action == 8 && side != 0)
                continue;
            Motion motion = input.velocity;
            float x = input.x, y = input.y, z = input.z, vz = input.verticalSpeed, yaw = input.yaw;
            Result candidate = {};
            candidate.action = action;
            candidate.side = side;
            candidate.forward = forward;
            candidate.right = right;
            for (int step = 0; step < steps; ++step)
            {
                const float speed = sqrtf(motion.x * motion.x + motion.y * motion.y);
                const float heading = atan2f(input.targetY - y, input.targetX - x) * DEGREES;
                float desired = heading - offset;
                if (side && speed >= 1.0f)
                {
                    const float budget =
                        input.frametime * (input.stopspeed > input.speed ? input.stopspeed : input.speed);
                    const float theta = acosf(Clamp((input.speed - budget) / speed, 0.0f, 1.0f)) * DEGREES;
                    desired = atan2f(motion.y, motion.x) * DEGREES + side * theta - offset;
                }
                if (action == 8)
                    desired = yaw;
                yaw = LimitedYaw(yaw, desired, input.yawSpeed * input.frametime, input.deltaYaw);
                const Motion before = motion;
                motion = evaluate(context, motion, yaw, forward, right);
                if (step == 0)
                {
                    candidate.yaw = yaw;
                    candidate.goalYaw = Normalize(desired);
                    candidate.velocity = motion;
                }
                x += (before.x + motion.x) * 0.5f * input.frametime;
                y += (before.y + motion.y) * 0.5f * input.frametime;
                z += vz * input.frametime - 0.5f * input.gravity * input.frametime * input.frametime;
                vz -= input.gravity * input.frametime;
                if (z <= input.targetZ && vz < 0.0f)
                    break;
            }
            const float remaining = FlightTime(z - input.targetZ, vz, input.gravity);
            const float dx = x + motion.x * remaining - input.targetX;
            const float dy = y + motion.y * remaining - input.targetY;
            const float landingError = dx * dx + dy * dy;
            float cost = landingError;
            if (input.previousAction >= 0 && action != input.previousAction)
                cost += 16.0f;
            if (side != input.previousSide)
                cost += 4.0f;
            const float turn = Normalize(candidate.yaw - input.yaw);
            cost += turn * turn * 0.02f;
            if (Finite(cost) && cost < bestCost)
            {
                bestCost = cost;
                best = candidate;
                best.valid = true;
                best.landingError = sqrtf(landingError);
            }
        }
    }
    return best;
}
} // namespace target
} // namespace cj_strafe
} // namespace mp
} // namespace iw3
