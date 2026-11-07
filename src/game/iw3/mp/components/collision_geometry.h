#pragma once

#include <cmath>

namespace iw3
{
namespace mp
{
namespace collision_geometry
{
const int MAX_POINTS = 128;
const int MAX_PLANES = 128;

struct Point
{
    float xyz[3];
};

struct Plane
{
    float normal[3];
    float dist;
};

struct Transform
{
    float origin[3];
    float axis[3][3];
};

inline void AnglesTransform(const float *origin, const float *angles, Transform &transform)
{
    const float radians = 0.017453292519943295f;
    const float sp = sinf(angles[0] * radians), cp = cosf(angles[0] * radians);
    const float sy = sinf(angles[1] * radians), cy = cosf(angles[1] * radians);
    const float sr = sinf(angles[2] * radians), cr = cosf(angles[2] * radians);
    for (int i = 0; i < 3; ++i)
        transform.origin[i] = origin[i];
    transform.axis[0][0] = cp * cy;
    transform.axis[0][1] = cp * sy;
    transform.axis[0][2] = -sp;
    transform.axis[1][0] = sr * sp * cy - cr * sy;
    transform.axis[1][1] = sr * sp * sy + cr * cy;
    transform.axis[1][2] = sr * cp;
    transform.axis[2][0] = cr * sp * cy + sr * sy;
    transform.axis[2][1] = cr * sp * sy - sr * cy;
    transform.axis[2][2] = cr * cp;
}

inline bool QuaternionTransform(const float *origin, const float *quat, Transform &transform)
{
    const float lengthSquared = quat[0] * quat[0] + quat[1] * quat[1] + quat[2] * quat[2] + quat[3] * quat[3];
    if (!(lengthSquared > 0.000001f))
        return false;
    const float scale = 2.0f / lengthSquared;
    const float xx = quat[0] * quat[0] * scale, yy = quat[1] * quat[1] * scale;
    const float zz = quat[2] * quat[2] * scale, xy = quat[0] * quat[1] * scale;
    const float xz = quat[0] * quat[2] * scale, yz = quat[1] * quat[2] * scale;
    const float wx = quat[3] * quat[0] * scale, wy = quat[3] * quat[1] * scale;
    const float wz = quat[3] * quat[2] * scale;
    for (int i = 0; i < 3; ++i)
        transform.origin[i] = origin[i];
    transform.axis[0][0] = 1.0f - yy - zz;
    transform.axis[0][1] = xy + wz;
    transform.axis[0][2] = xz - wy;
    transform.axis[1][0] = xy - wz;
    transform.axis[1][1] = 1.0f - xx - zz;
    transform.axis[1][2] = yz + wx;
    transform.axis[2][0] = xz + wy;
    transform.axis[2][1] = yz - wx;
    transform.axis[2][2] = 1.0f - xx - yy;
    return true;
}

inline void TransformNormal(const Transform &transform, const float *normal, float *out)
{
    const float x = normal[0], y = normal[1], z = normal[2];
    for (int i = 0; i < 3; ++i)
        out[i] = x * transform.axis[0][i] + y * transform.axis[1][i] + z * transform.axis[2][i];
}

inline void TransformPoint(const Transform &transform, const float *point, float *out)
{
    TransformNormal(transform, point, out);
    for (int i = 0; i < 3; ++i)
        out[i] += transform.origin[i];
}

inline void TransformBounds(const Transform &transform, const float *mins, const float *maxs, float *outMins,
                            float *outMaxs)
{
    float center[3], halfSize[3], worldCenter[3];
    for (int i = 0; i < 3; ++i)
    {
        center[i] = 0.5f * (mins[i] + maxs[i]);
        halfSize[i] = 0.5f * (maxs[i] - mins[i]);
    }
    TransformPoint(transform, center, worldCenter);
    for (int i = 0; i < 3; ++i)
    {
        const float radius = fabsf(transform.axis[0][i]) * halfSize[0] + fabsf(transform.axis[1][i]) * halfSize[1] +
                             fabsf(transform.axis[2][i]) * halfSize[2];
        outMins[i] = worldCenter[i] - radius;
        outMaxs[i] = worldCenter[i] + radius;
    }
}

inline float Dot(const float *a, const float *b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline void Cross(const float *a, const float *b, float *out)
{
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

inline bool Normalize(float *normal)
{
    const float length = sqrtf(Dot(normal, normal));
    if (!(length > 0.000001f))
        return false;
    for (int i = 0; i < 3; ++i)
        normal[i] /= length;
    return true;
}

inline bool IsBounce(float normalZ)
{
    // Same candidate-slope range as iw3mptool; speed and approach still determine an actual bounce.
    return normalZ >= 0.3f && normalZ <= 0.7f;
}

inline float BoundsDistanceSquared(const float *origin, const float *mins, const float *maxs)
{
    float distance = 0.0f;
    for (int i = 0; i < 3; ++i)
    {
        const float delta = origin[i] < mins[i]   ? mins[i] - origin[i]
                            : origin[i] > maxs[i] ? origin[i] - maxs[i]
                                                  : 0.0f;
        distance += delta * delta;
    }
    return distance;
}

inline int Clip(const Point *in, int count, const Plane &plane, Point *out)
{
    int written = 0;
    for (int i = 0; i < count; ++i)
    {
        const Point &a = in[i];
        const Point &b = in[(i + 1) % count];
        const float da = Dot(a.xyz, plane.normal) - plane.dist;
        const float db = Dot(b.xyz, plane.normal) - plane.dist;
        if (da <= 0.01f)
        {
            if (written == MAX_POINTS)
                return 0;
            out[written++] = a;
        }
        if ((da > 0.01f && db < -0.01f) || (da < -0.01f && db > 0.01f))
        {
            if (written == MAX_POINTS)
                return 0;
            const float fraction = da / (da - db);
            for (int axis = 0; axis < 3; ++axis)
                out[written].xyz[axis] = a.xyz[axis] + fraction * (b.xyz[axis] - a.xyz[axis]);
            ++written;
        }
    }
    return written;
}

inline int BrushFace(const Plane *planes, int planeCount, int face, const float *mins, const float *maxs, Point *out)
{
    if (planeCount < 6 || planeCount > MAX_PLANES || face < 0 || face >= planeCount)
        return 0;
    const Plane &plane = planes[face];
    float tangent[3] = {1.0f, 0.0f, 0.0f};
    if (fabsf(plane.normal[0]) > 0.9f)
    {
        tangent[0] = 0.0f;
        tangent[1] = 1.0f;
    }
    float side[3];
    Cross(plane.normal, tangent, side);
    if (!Normalize(side))
        return 0;
    Cross(side, plane.normal, tangent);

    float center[3];
    float extentSquared = 0.0f;
    for (int i = 0; i < 3; ++i)
    {
        center[i] = 0.5f * (mins[i] + maxs[i]);
        const float extent = maxs[i] - mins[i];
        extentSquared += extent * extent;
    }
    const float distance = Dot(center, plane.normal) - plane.dist;
    const float radius = sqrtf(extentSquared) + 1.0f;
    Point scratch[MAX_POINTS];
    const float signs[4][2] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
    for (int i = 0; i < 4; ++i)
        for (int axis = 0; axis < 3; ++axis)
            out[i].xyz[axis] = center[axis] - distance * plane.normal[axis] +
                               radius * (signs[i][0] * tangent[axis] + signs[i][1] * side[axis]);

    int count = 4;
    for (int i = 0; i < planeCount && count >= 3; ++i)
    {
        if (i == face)
            continue;
        count = Clip(out, count, planes[i], scratch);
        for (int j = 0; j < count; ++j)
            out[j] = scratch[j];
    }
    return count >= 3 ? count : 0;
}
} // namespace collision_geometry
} // namespace mp
} // namespace iw3
