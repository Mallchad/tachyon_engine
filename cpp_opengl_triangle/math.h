#pragma once

#include <cstdint>

#define  VECTOR3_BASE vector3<t_calculable>

template <typename t_calculable>
class vector3
{
// Public Interface
public:
    vector3(t_calculable x, t_calculable y, t_calculable z);
    bool operator<=> (vector3<t_calculable> rhs) const = default;
    vector3 operator+ (const vector3<t_calculable> rhs) const;
    vector3 operator- (const vector3<t_calculable> rhs) const;
    vector3 operator* (const vector3<t_calculable> rhs) const;
    vector3 operator/ (const vector3<t_calculable> rhs) const;

    t_calculable mx, my, mz = 0;
private:
};

typedef vector3<float> float3;
typedef vector3<int32_t> int3;
typedef vector3<int64_t> long3;
