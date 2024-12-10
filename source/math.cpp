
#include <math.hpp>

matrix matrix::one =
{
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 1.
};

matrix matrix::sequence =  { 1.,  2.,  3.,  4.,
                             5.,  6.,  7.,  8.,
                             9.,  10., 11., 12.,
                             13., 14., 15., 16. };

CONSTRUCTOR matrix::matrix( std::initializer_list<f32> list )
{
    int i = 0;
    for (auto& x : list) { data[i] = x; ++i; }
}
