#ifndef __DEFS_HPP__
#define __DEFS_HPP__

#include <dlib/matrix.h>

// The following two defines are used througout the
// code to make changing the precision easier.
#define real float
#define GL_REAL GL_FLOAT

namespace dlib
{
    // Commonly used matrix sizes.
    typedef matrix<real, 3, 3> mat3;
    typedef matrix<real, 4, 4> mat4;
    typedef matrix<real, 9, 1> mat9x1;
    typedef matrix<real, 3, 9> mat3x9;
    typedef matrix<real, 9, 9> mat9x9;
    typedef matrix<real, 3, 1> vec3;
    typedef matrix<real, 2, 1> vec2;

    /* Simple vector cross product.
     */
    inline vec3 cross(const vec3& v1, const vec3& v2)
    {
        const real arr[3] = {
            v1(1)*v2(2) - v1(2)*v2(1),
            v1(2)*v2(0) - v1(0)*v2(2),
            v1(1)*v2(2) - v1(2)*v2(1),
        };

        return vec3(arr);
    }

    /* Perform Denman–Beavers iteration to find the square root.
     */
    inline mat3 sqrt_db(const mat3& m)
    {
        mat3 m1 = m;
        mat3 m2 = identity_matrix<real>(3);
        mat3 half_mat;
        half_mat = 0.5, 0.0, 0.0,
                   0.0, 0.5, 0.0,
                   0.0, 0.0, 0.5;
        for (int i = 0; i < 100; ++i)
        {
            mat3 inv1 = inv(m1);
            mat3 inv2 = inv(m2);
            m1 = (m1 + inv2) * half_mat;
            m2 = (m2 + inv1) * half_mat;
        }

        return m1;
    }
}

#endif
