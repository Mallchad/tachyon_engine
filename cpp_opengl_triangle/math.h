
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
    CONSTRUCTOR vector3(t_calculable all) :
        mx(all), my(all), mz(all) {}
    CONSTRUCTOR vector3(t_calculable x, t_calculable y, t_calculable z) :
        mx(x), my(y), mz(z) {}
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

template <typename t_calculable>
class vector4
{
public:
    t_calculable mx, my, mz, mw = 0;

// Public Interface
    CONSTRUCTOR vector4() = default;
    CONSTRUCTOR vector4(t_calculable all)  :
        mx(all), my(all), mz(all), mw(all) {}
    CONSTRUCTOR vector4(t_calculable x, t_calculable y, t_calculable z, t_calculable w) :
        mx(x), my(y), mz(z), mw(w) {}

    vector4<t_calculable> operator+ (const vector4<t_calculable>& rhs) const
    {
        vector4<t_calculable> out(mx + rhs.mx,
                                  my + rhs.my,
                                  mz + rhs.mz,
                                  mw + rhs.mw);
        return out;
    }

    vector4<t_calculable>
    operator- (const vector4<t_calculable>& rhs) const
    {
        vector4<t_calculable> out(mx - rhs.mx,
                                  my - rhs.my,
                                  mz - rhs.mz,
                                  mw - rhs.mw);
        return out;
    }
    vector4<t_calculable>
    operator* (const vector4<t_calculable>& rhs) const
    {
        vector4<t_calculable> out(mx * rhs.mx,
                                  my * rhs.my,
                                  mz * rhs.mz,
                                  mw * rhs.mw);
        return out;
    }
    vector4<t_calculable>
    operator* (t_calculable rhs) const
    {
        vector4<t_calculable> out(mx * rhs,
                                  my * rhs,
                                  mz * rhs,
                                  mw * rhs);
        return out;
    }
    vector4<t_calculable>
    operator/ (const vector4<t_calculable> rhs) const
    {
        vector4<t_calculable> out(mx / rhs.mx,
                                  my / rhs.my,
                                  mz / rhs.mz,
                                  mw / rhs.mw);
        return out;
    }

    vector4<t_calculable>&
    operator= (t_calculable all)
    {
        mx = all;
        my = all;
        mz = all;
        mw = all;
    }

private:
};

typedef vector4<float> float4;
typedef vector4<int32_t> int4;
typedef vector4<int64_t> long4;

