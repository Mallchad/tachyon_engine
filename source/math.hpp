
#pragma once

#include "code_helpers.h"

#include <cstdint>
#include <cmath>

// Gurantee vectors are packed tightly because of OpenGL functions
#pragma pack(push, 1)
template <typename t_calculable>
class vector3 final
{
    using t_vector = vector3<t_calculable>;
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
    operator/ (const t_calculable rhs) const
    {
        vector3 out = vector3(mx / rhs,
                              my / rhs,
                              mz / rhs);
        return out;
    }
    vector3<t_calculable>&
    operator= (t_calculable all)
    {
        mx = all;
        my = all;
        mz = all;
    }
    t_vector&
    operator- ()
    {
        mx = -mx;
        my = -my;
        mz = -mz;
        return *this;
    }
    t_vector
    normalize() const
    {
        ffloat length = sqrtf( (mx * mx) + (my * my) + (mz * mz) );

        return *this/length;
    }

    // Non-Member Functions
    friend t_calculable
    dot( const t_vector lhs, const t_vector rhs )
    {
        return (lhs.mx * rhs.mx) + (lhs.my * rhs.my) + (lhs.mz * rhs.mz);
    }
    friend t_vector
    cross( const t_vector lhs, const t_vector rhs )
    {
        t_vector out;
        out.mx = ( lhs.my * rhs.mz ) - (lhs.mz * rhs.my);
        out.my = ( lhs.mz * rhs.mx ) - (lhs.mx * rhs.mz);
        out.mz = ( lhs.mx * rhs.my ) - (lhs.my * rhs.mx);

        return out;
    }
    friend t_vector
    normalize( const t_vector v )
    {
        return v.normalize();
    }
};
template <typename t_calculable>
class vector4
{
    using t_vector = vector4<t_calculable>;
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

struct transform
{
    vector4<ffloat> position;
    vector4<ffloat> rotation;
    vector4<ffloat> scale;
};

struct stl_facet
{
    vector3<ffloat> normal;
    vector3<ffloat> v1;
    vector3<ffloat> v2;
    vector3<ffloat> v3;
    fuint16 attribute_width = 0; // Never use, not common STL format
};

struct stl_triangle
{
    float normal_x;
    float normal_y;
    float normal_z;

    float v1_x;
    float v1_y;
    float v1_z;

    float v2_x;
    float v2_y;
    float v2_z;

    float v3_x;
    float v3_y;
    float v3_z;

    fuint16 attribute_width;

};

#pragma pack(pop)



using ffloat3 = vector3<ffloat>;
using fint3   = vector3<fint32>;
using flong3  = vector3<fint64>;

using ffloat4 = vector4<ffloat>;
using fint4   = vector4<fint32>;
using flong4  = vector4<fint64>;

using ftransform = transform;
