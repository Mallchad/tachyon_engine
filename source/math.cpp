
#include "include_core.h"

namespace tyon
{

vec2 operator+(const vec2& v1, const vec2& v2) {
    vec2 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    return result;
}

vec2 operator+(const vec2& v1, const f32& c) {
    vec2 result;
    result.x = v1.x + c;
    result.y = v1.y + c;
    return result;
}

vec2 operator+(const f32& c, const vec2& v1) {
    vec2 result;
    result.x = v1.x + c;
    result.y = v1.y + c;
    return result;
}


vec2 operator-(vec2 v1)
{
    return { -v1.x, -v1.y };
}

vec2 operator-(const vec2& v1, const vec2& v2) {
    vec2 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    return result;
}

vec2 operator-(const vec2& v1, const f32& c) {
    vec2 result;
    result.x = v1.x - c;
    result.y = v1.y - c;
    return result;
}

vec2 operator*(const vec2& v1, const f32& c) {
    vec2 result;
    result.x = v1.x * c;
    result.y = v1.y * c;
    return result;
}

vec2 operator*(const f32& c, const vec2& v1) {
    vec2 result;
    result.x = v1.x * c;
    result.y = v1.y * c;
    return result;
}

vec2 operator/(const vec2& v1, const f32& c) {
    vec2 result;
    result.x = v1.x / c;
    result.y = v1.y / c;
    return result;
}

f32 dot_product(const vec2& v1, const vec2& v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

f32 vector_length(const vec2& v1) {
    return std::sqrt((v1.x * v1.x) + (v1.y * v1.y));
}

vec2
pairwise_multiply( vec2 lhs, vec2 rhs )
{ return { lhs.x * rhs.x, lhs.y * rhs.y }; }

vec2
pairwise_divide( vec2 lhs, vec2 rhs )
{ return { lhs.x / rhs.x, lhs.y / rhs.y }; }

vec2 abs( vec2 arg )
{   return vec2 { std::abs( arg.x ), std::abs( arg.y) };
}

fstring
format_as( vec2 arg )
{ return fmt::format( "[{0:.5} {1:.5}]", arg.x, arg.y ); }


v3f
FUNCTION v3f::up()
{ return { 0., 0., 1. }; }
v3f
FUNCTION v3f::forward()
{ return { 0., 1., 0. }; }
v3f
FUNCTION v3f::right()
{ return { 1., 0., 0. }; }
v3f
FUNCTION v3f::down()
{ return { 0., 0., -1. }; }
v3f
FUNCTION v3f::backward()
{ return { 0., -1, 0. }; }
v3f
FUNCTION v3f::left()
{ return { -1., 0., 0. }; }

template <typename T>
vector3_t<T>
vector3_t<T>::up()
{ return { 0., 0. ,0. }; }

CONSTRUCTOR v3f::v3f( vector4_t<float> v ) :
    x(v.x), y(v.y), z(v.z) {};

/// Scalar Product
v3f
v3f::operator* (const f32 rhs) const
{ return v3f(x * rhs, y * rhs, z * rhs); }

v3f
v3f::operator* ( v3f rhs )
{
        v3f out;
        out.x = ( y * rhs.z ) - (z * rhs.y);
        out.y = ( z * rhs.x ) - (x * rhs.z);
        out.z = ( x * rhs.y ) - (y * rhs.x);
        return out;
}

// Matrix Struct
CONSTRUCTOR matrix::matrix( std::initializer_list<f32> list )
{
    int i = 0;
    for (auto& x : list) { data[i] = x; ++i; }
}

matrix
matrix::operator* ( matrix rhs )
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

matrix&
matrix::operator*= ( matrix& rhs )
{ return (*this = (*this) * rhs); }
matrix&
matrix::operator*= ( matrix&& rhs )
{ return (*this = (*this) * rhs); }

matrix
matrix::create_rotation( v3f r )
{
    matrix result = matrix
    { cosf(r.y)*cosf(r.z),
      -cosf(r.y)*sinf(r.z),
      sinf(r.y),
      0,
      // Row 2
      cosf(r.x)*sinf(r.z) + cosf(r.z)*sinf(r.x)*sinf(r.y),
      cosf(r.x)*cosf(r.z) - sinf(r.x)*sinf(r.y)*sinf(r.z),
      -cosf(r.y)*sinf(r.x),
      0,
      // Row 3
      sinf(r.x)*sinf(r.z) - cosf(r.x)*cosf(r.z)*sinf(r.y),
      cosf(r.x)*sinf(r.y)*sinf(r.z) + cosf(r.z)*sinf(r.x),
      cosf(r.x)*cosf(r.y),
      0,
      // Row 4
      0, 0, 0, 1};
    return result;
}

matrix
matrix::create_translation( v3f target )
{
    return matrix(
        { 1.0, 0.0, 0.0, target.x,
          0.0, 1.0, 0.0, target.y,
          0.0, 0.0, 1.0, target.z,
          0.0, 0.0, 0.0, 1.0 });
}

matrix
matrix::unreal_to_opengl()
{
    matrix conversion = matrix { 1., 0., 0., 0.,
                                 0., 0., 1., 0.,
                                 0., -1., 0., 0.,
                                 0., 0., 0., 1. };
    return *this * conversion;
}

}
