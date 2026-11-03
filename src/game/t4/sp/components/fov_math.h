#pragma once

namespace t4
{
namespace sp
{
namespace fov
{
inline float Clamp(float value, float minimum, float maximum)
{
    return value < minimum ? minimum : (value > maximum ? maximum : value);
}

// Preferences scale the script-controlled values instead of replacing them.
inline float ApplyBase(float stock, float preference, float minimum)
{
    return Clamp(stock * (preference / 65.0f), minimum, 160.0f);
}

inline float ApplyScale(float stock, float preference, float minimum, float maximum)
{
    return Clamp(stock * preference, minimum, maximum);
}
} // namespace fov
} // namespace sp
} // namespace t4
