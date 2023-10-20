#pragma once

#include "code_helpers.h"

#include <cstdint>

template <typename t_calculable>
class vector3
{
public:
    t_calculable mx, my, mz = 0;

// Public Interface
    CONSTRUCTOR vector3() = default;
    CONSTRUCTOR vector3(t_calculable all);
    CONSTRUCTOR vector3(t_calculable x, t_calculable y, t_calculable z);
    vector3 operator+ (const vector3<t_calculable> rhs) const;
    vector3 operator- (const vector3<t_calculable> rhs) const;
    vector3 operator* (const vector3<t_calculable> rhs) const;
    vector3 operator/ (const vector3<t_calculable> rhs) const;
    vector3 operator= (t_calculable all);

    private:
};

typedef vector3<float> float3;
typedef vector3<int32_t> int3;
typedef vector3<int64_t> long3;
