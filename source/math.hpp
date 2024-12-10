
#pragma once

#include "code_helpers.h"

#include <cstdint>
#include <cmath>
#include <initializer_list>
#include <memory.hpp>

// Gurantee vectors are packed tightly because of OpenGL functions
#pragma pack(push, 1)
template <typename t_calculable>
class vector3 final
{
    using t_vector = vector3<t_calculable>;
public:
    t_calculable x, y, z;

// Public Interface
    CONSTRUCTOR vector3() = default;
    explicit
    CONSTRUCTOR vector3(t_calculable all) :
        x(all), y(all), z(all) {}
    CONSTRUCTOR vector3(t_calculable _x, t_calculable _y, t_calculable _z) :
        x(_x), y(_y), z(_z) {}

    t_vector
    operator+ (const t_vector rhs) const
    { return vector3(x + rhs.x, y + rhs.y, z + rhs.z); }

    t_vector
    operator- (const t_vector rhs) const
    { return vector3(x - rhs.x, y - rhs.y, z - rhs.z); }

    /// Cross Product
    t_vector
    operator* ( t_vector rhs )
    {
        t_vector out;
        out.x = ( y * rhs.z ) - (z * rhs.y);
        out.y = ( z * rhs.x ) - (x * rhs.z);
        out.z = ( x * rhs.y ) - (y * rhs.x);
        return out;
    }

    /// Scalar multiplication
    t_vector
    operator* (const t_calculable rhs) const
    { return vector3(x * rhs, y * rhs, z * rhs); }

    t_vector
    operator/ (const t_vector rhs) const
    { return vector3(x / rhs.x, y / rhs.y, z / rhs.z); }

    t_vector
    operator/ (const t_calculable rhs) const
    { return vector3(x / rhs, y / rhs, z / rhs); }

    t_vector
    operator= (t_calculable all)
    { x = all; y = all; z = all; }

    t_vector&
    operator- ()
    { x = -x; y = -y; z = -z; return *this; }

    /// Compound Functions
    t_vector
    operator+= (const t_vector rhs) const
    {
        this->x += rhs.x; this->y += rhs.y; this->z += rhs.y;
        return *this;
    }

    t_vector
    FUNCTION normalize() const
    {
        ffloat length = sqrtf( (x * x) + (y * y) + (z * z) );
        return *this/length;
    }

    // Non-Member Functions
    friend t_calculable
    FUNCTION dot( const t_vector lhs, const t_vector rhs )
    { return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z); }

    friend t_vector
    FUNCTION cross( const t_vector lhs, const t_vector rhs )
    {
        t_vector out;
        out.x = ( lhs.y * rhs.z ) - (lhs.z * rhs.y);
        out.y = ( lhs.z * rhs.x ) - (lhs.x * rhs.z);
        out.z = ( lhs.x * rhs.y ) - (lhs.y * rhs.x);
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
    t_calculable x, y, z, w = 0;

// Public Interface
    CONSTRUCTOR vector4() = default;
    explicit
    CONSTRUCTOR vector4( t_calculable all )  :
        x(all), y(all), z(all), w(all) {}
    CONSTRUCTOR vector4( t_calculable _x, t_calculable _y, t_calculable _z, t_calculable _w ) :
        x(_x), y(_y), z(_z), w(_w) {}

    t_vector operator+ ( const t_vector& rhs ) const
    { return t_vector(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }

    t_vector
    operator- ( const t_vector& rhs ) const
    { return t_vector(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }

    t_vector
    operator* ( const t_vector& rhs ) const
    { return t_vector(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }

    t_vector
    operator* ( t_calculable rhs ) const
    { return t_vector(x * rhs, y * rhs, z * rhs, w * rhs); }

    t_vector
    operator/ ( const t_vector rhs ) const
    { return t_vector(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

    t_vector&
    operator= ( t_calculable all )
    { x = all; y = all; z = all; w = all; }

private:
};

struct matrix;
/// 4x4 matrix
struct matrix
{
    union
    {
        f32 access[4][4];
        f32 xy[4][4];
        vector4<f32> e[4];
        f32 data[16];
        f32 d[16];
    };
    static matrix one;
    static matrix sequence;

    CONSTRUCTOR matrix()
        : data() {}
    // COPY_CONSTRUCTOR matrix(matrix& rhs) {}
    CONSTRUCTOR matrix( std::initializer_list<f32> list );

    f32
    operator[] (u32 i)
    { return this->data[i]; }

    // Math Operators
    /// Scalar Multiply
    matrix
    operator* ( f32 rhs )
    {
        return { data[0] *rhs, data[1] *rhs, data[2] *rhs, data[3] *rhs,
                 data[4] *rhs, data[5] *rhs, data[6] *rhs, data[7] *rhs,
                 data[8] *rhs, data[9] *rhs, data[10] *rhs, data[11] *rhs,
                 data[12] *rhs, data[13] *rhs, data[14] *rhs, data[15] *rhs };
    }
    /// Vector Product/Multiply
    matrix
    operator* ( matrix rhs )
    {
        f32* a = this->d;
        f32* b = rhs.d;
        return matrix{ a[0] * b[0] + a[1]*b[4] + a[2]*b[8] + a[3]*b[12],
                       a[0] * b[1] + a[1]*b[5] + a[2]*b[9] + a[3]*b[13],
                       a[0] * b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14],
                       a[0] * b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15],

                       a[4] * b[0] + a[5]*b[4] + a[6]* b[8] + a[7]*b[12],
                       a[4] * b[1] + a[5]*b[5] + a[6]* b[9] + a[7]*b[13],
                       a[4] * b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14],
                       a[4] * b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15],

                       a[8] * b[0] + a[9]*b[4] + a[10]* b[8] + a[11]*b[12],
                       a[8] * b[1] + a[9]*b[5] + a[10]* b[9] + a[11]*b[13],
                       a[8] * b[2] + a[9]*b[6] + a[10]*b[10] + a[11]*b[14],
                       a[8] * b[3] + a[9]*b[7] + a[10]*b[11] + a[11]*b[15],

                       a[12] * b[0] + a[13]*b[4] + a[14]* b[8] + a[15]*b[12],
                       a[12] * b[1] + a[13]*b[5] + a[14]* b[9] + a[15]*b[13],
                       a[12] * b[2] + a[13]*b[6] + a[14]*b[10] + a[15]*b[14],
                       a[12] * b[3] + a[13]*b[7] + a[14]*b[11] + a[15]*b[15] };
    }
    matrix
    operator+ ( matrix rhs )
    {
        return { data[0]+  rhs[0],  data[1]+  rhs[1],  data[2]+  rhs[2],  data[3]+  rhs[3],
                 data[4]+  rhs[4],  data[5]+  rhs[5],  data[6]+  rhs[6],  data[7]+  rhs[7],
                 data[8]+  rhs[8],  data[9]+  rhs[9],  data[10]+ rhs[10], data[11]+ rhs[11],
                 data[12]+ rhs[12], data[13]+ rhs[13], data[14]+ rhs[14], data[15]+ rhs[15] };
    }
    matrix
    operator- ( matrix rhs )
    {
        return { data[0]*  rhs[0],  data[1]*  rhs[1],  data[2]*  rhs[2],  data[3]*  rhs[3],
                 data[4]*  rhs[4],  data[5]*  rhs[5],  data[6]*  rhs[6],  data[7]*  rhs[7],
                 data[8]*  rhs[8],  data[9]*  rhs[9],  data[10]* rhs[10], data[11]* rhs[11],
                 data[12]* rhs[12], data[13]* rhs[13], data[14]* rhs[14], data[15]* rhs[15] };
    }

    matrix&
    operator*= ( matrix rhs )
    { return (*this = (*this) * rhs); }

    matrix
    transpose()
    {
        return matrix { access[0][0], access[1][0], access[2][0], access[3][0],
                        access[0][1], access[1][1], access[2][1], access[3][1],
                        access[0][2], access[1][2], access[2][2], access[3][2],
                        access[0][3], access[1][3], access[2][3], access[3][3] };
    }

    // Compound Math Operators
    matrix
    operator+= ( matrix rhs )
    { return *this = (*this) + rhs; }

    static matrix
    create_translation( vector4<f32> target )
    {
        return matrix({ 1.0, 0.0, 0.0, target.x,
                        0.0, 1.0, 0.0, target.y,
                        0.0, 0.0, 1.0, target.z,
                        0.0, 0.0, 0.0, 1.0 });
    }
};

struct transform
{
    using v4 = vector4<f32>;
    v4 translation;
    v4 rotation;
    v4 scale;

    matrix
    translation_matrix()
    { return matrix::create_translation( translation ); }

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

using v3 = vector3<f32>;
using v4 = vector4<f32>;

using ftransform = transform;
