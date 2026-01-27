
#pragma once

namespace tyon
{

FORWARD struct v3f;
FORWARD template <typename T> struct vector3_t;
FORWARD template <typename T> struct vector4_t;

/// 4x4 matrix
struct matrix
{
    union
    {
        f32 access[4][4];
        f32 xy[4][4];
        f32 data[16];
        f32 d[16];
    };

    static matrix
    one() { return matrix
        { 1., 0., 0., 0.,
          0., 1., 0., 0.,
          0., 0., 1., 0.,
          0., 0., 0., 1. };
    }

    static matrix
    sequence() { return matrix
        { 1.,  2.,  3.,  4.,
          5.,  6.,  7.,  8.,
          9.,  10., 11., 12.,
          13., 14., 15., 16. };
    }

    CONSTRUCTOR matrix()
        : data() {}
    COPY_CONSTRUCTOR matrix(matrix& rhs)
    {
        this->d[0] = rhs.d[0];
        this->d[1] = rhs.d[1];
        this->d[2] = rhs.d[2];
        this->d[3] = rhs.d[3];
        this->d[4] = rhs.d[4];
        this->d[5] = rhs.d[5];
        this->d[6] = rhs.d[6];
        this->d[7] = rhs.d[7];
        this->d[8] = rhs.d[8];
        this->d[9] = rhs.d[9];
        this->d[10] = rhs.d[10];
        this->d[11] = rhs.d[11];
        this->d[12] = rhs.d[12];
        this->d[13] = rhs.d[13];
        this->d[14] = rhs.d[14];
        this->d[15] = rhs.d[15];
    }
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
    matrix operator* ( matrix rhs );

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
    operator*= ( matrix& rhs );
    matrix&
    operator*= ( matrix&& rhs );

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
    create_translation( v3f target );
    template<typename T>
    static matrix
    create_translation( vector4_t<T> target )
    {
        return matrix(
            { 1.0, 0.0, 0.0, target.x,
              0.0, 1.0, 0.0, target.y,
              0.0, 0.0, 1.0, target.z,
              0.0, 0.0, 0.0, 1.0 });
    }

    static matrix
    create_rotation( v3f r );

    // matrix
    // to_right_handed();
    // matrix
    // to_left_handed();

    matrix
    unreal_to_opengl();
};

}
