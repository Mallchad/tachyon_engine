
#pragma once

#include "code_helpers.h"

#include <cstdint>

typedef int8_t   fint8;
typedef int16_t fint16;
typedef int32_t fint32;
typedef int64_t fint64;

typedef uint8_t   fuint8;
typedef uint16_t fuint16;
typedef uint32_t fuint32;
typedef uint64_t fuint64;

// Fixed width floats only supported
typedef float ffloat;
typedef double fdouble;

// Gurantee vectors are packed tightly because of OpenGL functions
#pragma pack(push, 1)
template <typename t_calculable>
class vector3 final
{
public:
    t_calculable mx, my, mz = 0;

// Public Interface
    CONSTRUCTOR vector3() = default;
    CONSTRUCTOR vector3(t_calculable all) :
        mx(all), my(all), mz(all) {}
    CONSTRUCTOR vector3(t_calculable x, t_calculable y, t_calculable z) :
        mx(x), my(y), mz(z) {}

    vector3<t_calculable>&
    operator+ (const vector3<t_calculable> rhs) const
    {
        vector3 out = vector3(mx + rhs.mx,
                              my + rhs.my,
                              mz + rhs.mz);
        return out;
    }
    vector3<t_calculable>&
    operator- (const vector3<t_calculable> rhs) const
    {
        vector3 out = vector3(mx - rhs.mx,
                              my - rhs.my,
                              mz - rhs.mz);
        return out;
    }
    vector3<t_calculable>&
    // Pairwise multiplication, it is never assumed to be standard maths
    operator* (const vector3<t_calculable> rhs) const
    {
        vector3 out = vector3(mx * rhs.mx,
                              my * rhs.my,
                              mz * rhs.mz);
        return out;
    }
    vector3<t_calculable>&
    operator/ (const vector3<t_calculable> rhs) const
    {
        vector3 out = vector3(mx / rhs.mx,
                              my / rhs.my,
                              mz / rhs.mz);
        return out;
    }
    vector3<t_calculable>&
    operator= (t_calculable all)
    {
        mx = all;
        my = all;
        mz = all;
    }
};
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
#pragma pack(pop)

typedef vector3<ffloat> vfloat3;
typedef vector3<fint32> vint3;
typedef vector3<fint64> vlong3;

typedef vector4<ffloat> vfloat4;
typedef vector4<fint32> vint4;
typedef vector4<fint64> vlong4;
