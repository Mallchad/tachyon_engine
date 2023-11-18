
#pragma once

#include "code_helpers.h"

#include <cstdint>

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

/** STL Format
    Presumed little endian
    80 Bytes  - Header
    4 Bytes   - Number of triangles
    84th Byte - First triangle
    12 Bytes  - float3 Triangle Normal
    12 Bytes  - float3 Vertex 1
    12 Bytes  - float3 Vertex 2
    12 Bytes  - float3 Vertex 3
    2 Bytes   - Unused attribute width
*/

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


// TO BE DELETED
typedef vector3<ffloat> vfloat3;
typedef vector3<fint32> vint3;
typedef vector3<fint64> vlong3;

typedef vector4<ffloat> vfloat4;
typedef vector4<fint32> vint4;
typedef vector4<fint64> vlong4;
//
typedef vector3<ffloat> ffloat3;
typedef vector3<fint32> fint3;
typedef vector3<fint64> flong3;

typedef vector4<ffloat> ffloat4;
typedef vector4<fint32> fint4;
typedef vector4<fint64> flong4;

typedef transform ftransform;
