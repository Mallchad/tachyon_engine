
#include "math.h"

#define  VECTOR3_BASE vector3<t_calculable>

template <typename t_calculable> VECTOR3_BASE
VECTOR3_BASE::operator+ (const vector3<t_calculable> rhs) const
{
    vector3 out = vector3(mx + rhs.mx,
                          my + rhs.my,
                          mz + rhs.mz);
    return out;
}
template <class t_calculable> VECTOR3_BASE
VECTOR3_BASE::operator- (const vector3<t_calculable> rhs) const
{
    vector3 out = vector3(mx - rhs.mx,
                          my - rhs.my,
                          mz - rhs.mz);
    return out;
}
template <class t_calculable> VECTOR3_BASE
VECTOR3_BASE::operator* (const vector3<t_calculable> rhs) const
{
    vector3 out = vector3(mx * rhs.mx,
                          my * rhs.my,
                          mz * rhs.mz);
    return out;
}
template <class t_calculable> VECTOR3_BASE
VECTOR3_BASE::operator/ (const vector3<t_calculable> rhs) const
{
    vector3 out = vector3(mx / rhs.mx,
                          my / rhs.my,
                          mz / rhs.mz);
    return out;
}

template <class t_calculable> VECTOR3_BASE
VECTOR3_BASE::operator= (t_calculable all)
{
    mx = all;
    my = all;
    mz = all;
}

#define  VECTOR4_BASE vector4<t_calculable>

