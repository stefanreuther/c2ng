/**
  *  \file gfx/threed/vecmath.hpp
  *  \brief Vector Math
  */
#ifndef C2NG_GFX_THREED_VECMATH_HPP
#define C2NG_GFX_THREED_VECMATH_HPP

#include <cmath>
#include "afl/base/types.hpp"   // size_t

namespace gfx { namespace threed {

    /*
     *  Detail Namespace
     *
     *  For now, I'm placing the implementation here because this vector/matrix
     *  stuff is only required for graphics.
     *
     *  The original implementation of this module appeared in c2web (WebGL-based FLAK player).
     *
     *  The public components of this module are Vec3f and Mat4f.
     *  The members of the detail namespace may be changed later (e.g. to a hardwired SIMD implementation).
     */

    namespace detail {

        /** Base class for a vector of arbitrary dimension.

            \tparam ValueType Component type
            \tparam AccType   Accumulator type (can have higher precision)
            \tparam Dim       Dimension (number of components)
            \tparam UserType  User-perceived type (derived from Vec<...>) */
        template<typename ValueType, typename AccType, size_t Dim, typename UserType>
        class Vec {
         public:
            /** Access element.
                \param n Index [0,Dim) */
            ValueType& operator()(size_t n)
                { return m_value[n]; }

            /** Access element.
                \param n Index [0,Dim) */
            const ValueType& operator()(size_t n) const
                { return m_value[n]; }

            /** Vector subtraction, in-place.
                \param other Other vector
                \return *this */
            UserType& operator-=(const UserType& other)
                {
                    // ex vSub
                    for (size_t i = 0; i < Dim; ++i) {
                        m_value[i] -= other.m_value[i];
                    }
                    return static_cast<UserType&>(*this);
                }

            /** Vector subtraction.
                \param other Other vector
                \return difference */
            UserType operator-(const UserType& other) const
                {
                    UserType tmp(static_cast<const UserType&>(*this));
                    tmp -= other;
                    return tmp;
                }

            /** Vector addition, in-place.
                \param other Other vector
                \return *this */
            UserType operator+=(const UserType& other)
                {
                    // ex vAdd
                    for (size_t i = 0; i < Dim; ++i) {
                        m_value[i] += other.m_value[i];
                    }
                    return static_cast<UserType&>(*this);
                }

            /** Vector addition.
                \param other Other vector
                \return sum */
            UserType operator+(const UserType& other) const
                {
                    UserType tmp(static_cast<const UserType&>(*this));
                    tmp += other;
                    return tmp;
                }

            /** Compute length, high precision.
                \return Length of vector */
            AccType lengthHP() const
                { return std::sqrt(dotHP(static_cast<const UserType&>(*this))); }

            /** Compute length.
                \return Length of vector */
            ValueType length() const
                {
                    // ex vLen
                    return ValueType(lengthHP());
                }

            /** Normalize vector.
                \return Vector with same direction, but length()=1
                \pre length() > 0 */
            UserType norm() const
                {
                    // ex vNorm
                    AccType len = lengthHP();
                    UserType result(static_cast<const UserType&>(*this));
                    for (size_t i = 0; i < Dim; ++i) {
                        result.m_value[i] = static_cast<ValueType>(result.m_value[i] / len);
                    }
                    return result;
                }

            /** Multiply by scalar, in-place.
                \param scalar Scalar to multiply by
                \return *this */
            UserType& operator*=(ValueType scalar)
                {
                    // ex vScale
                    for (size_t i = 0; i < Dim; ++i) {
                        m_value[i] *= scalar;
                    }
                    return static_cast<UserType&>(*this);
                }

            /** Multiply by scalar.
                \param scalar Scalar to multiply by
                \return Product */
            UserType operator*(ValueType scalar) const
                {
                    UserType result(static_cast<const UserType&>(*this));
                    result *= scalar;
                    return result;
                }

            /** Dot product, high precision.
                The dot product is the sum of component-wise multiplication of the vectors.
                \param other Other vector */
            AccType dotHP(const UserType& other) const
                {
                    AccType acc = AccType();
                    for (size_t i = 0; i < Dim; ++i) {
                        acc += m_value[i] * other.m_value[i];
                    }
                    return acc;
                }

            /** Dot product.
                The dot product is the sum of component-wise multiplication of the vectors.
                \param other Other vector */
            ValueType dot(const UserType& other) const
                {
                    // ex vDot
                    return static_cast<ValueType>(dotHP(other));
                }

         protected:
            ValueType m_value[Dim];
        };

        template<typename ValueType, typename AccType>
        class Mat4;


        /** Base class for a 4D vector of arbitrary type.
            A 4D vector can represent a RGBA color or XYZW coordinate.
            \tparam ValueType Component type
            \tparam AccType   Accumulator type (can have higher precision) */
        template<typename ValueType, typename AccType>
        class Vec4 : public Vec<ValueType, AccType, 4, Vec4<ValueType, AccType> > {
         public:
            /** Constructor.
                \param x First component
                \param y Second component
                \param z Third component
                \param w Fourth component*/
            Vec4(ValueType x, ValueType y, ValueType z, ValueType w)
                {
                    this->m_value[0] = x;
                    this->m_value[1] = y;
                    this->m_value[2] = z;
                    this->m_value[3] = w;
                }
        };

        /** Base class for a 3D vector of arbitrary type.
            A 3D vector can represent a point in space.
            \tparam ValueType Component type
            \tparam AccType   Accumulator type (can have higher precision) */
        template<typename ValueType, typename AccType>
        class Vec3 : public Vec<ValueType, AccType, 3, Vec3<ValueType, AccType> > {
         public:
            /** Constructor.
                \param x X-coordinate (first component)
                \param y Y-coordinate (second component)
                \param z Z-coordinate (third component) */
            Vec3(ValueType x, ValueType y, ValueType z)
                {
                    // ex vMake
                    this->m_value[0] = x; this->m_value[1] = y; this->m_value[2] = z;
                }

            /** Vector product.
                Given two vectors that are neither parallel, anti-parallel, nor zero-length,
                produces a vector perpendicular to both.
                \param b Other vector
                \return result */
            Vec3 prod(const Vec3& b) const
                {
                    // ex vProd
                    return Vec3((*this)(1)*b(2) - (*this)(2)*b(1),
                                (*this)(2)*b(0) - (*this)(0)*b(2),
                                (*this)(0)*b(1) - (*this)(1)*b(0));
                }

            /** Get perpendicular vector.
                Picks an arbitrary vector that is perpendicular to this one.
                \return vector */
            Vec3 per() const
                {
                    // ex vPer
                    ValueType a0 = this->m_value[0], a1 = this->m_value[1], a2 = this->m_value[2];
                    if (a0 == a1 && a0 == a2) {
                        // This would produce a vector of magnitude 0
                        return Vec3(a1,         - a0, 0);
                    } else {
                        // Simple case
                        return Vec3(a1 - a2, a2 - a0, a0 - a1);
                    }
                }

            /** Transform using transformation matrix.
                \param m Matrix
                \return transformed vector */
            Vec3 transform(const Mat4<ValueType, AccType>& m) const;
        };


        /** Base class for a matrix of arbitrary dimension.

            \tparam ValueType Component type
            \tparam AccType   Accumulator type (can have higher precision)
            \tparam Dim       Dimension (number of components)
            \tparam UserType  User-perceived type (derived from Vec<...>) */
        template<typename ValueType, typename AccType, size_t Dim, typename UserType>
        class Mat {
         public:
            ValueType& operator()(size_t n)
                { return m_value[n]; }
            const ValueType& operator()(size_t n) const
                { return m_value[n]; }

         protected:
            ValueType m_value[Dim*Dim];
        };


        /** Base class for a 4x4 matrix of arbitrary type.
            A 4x4 matrix can represent a coordinate transformation.
            \tparam ValueType Component type
            \tparam AccType   Accumulator type (can have higher precision) */
        template<typename ValueType, typename AccType>
        class Mat4 : public Mat<ValueType, AccType, 4, Mat4<ValueType, AccType> > {
         public:
            /** Construct from 16 values. */
            Mat4(ValueType a1, ValueType a2, ValueType a3, ValueType a4,
                 ValueType b1, ValueType b2, ValueType b3, ValueType b4,
                 ValueType c1, ValueType c2, ValueType c3, ValueType c4,
                 ValueType d1, ValueType d2, ValueType d3, ValueType d4)
                {
                    // ex mMake (mClone)
                    this->m_value[0]  = a1;
                    this->m_value[1]  = a2;
                    this->m_value[2]  = a3;
                    this->m_value[3]  = a4;
                    this->m_value[4]  = b1;
                    this->m_value[5]  = b2;
                    this->m_value[6]  = b3;
                    this->m_value[7]  = b4;
                    this->m_value[8]  = c1;
                    this->m_value[9]  = c2;
                    this->m_value[10] = c3;
                    this->m_value[11] = c4;
                    this->m_value[12] = d1;
                    this->m_value[13] = d2;
                    this->m_value[14] = d3;
                    this->m_value[15] = d4;
                }

            /** Construct empty matrix. */
            Mat4()
                {
                    for (size_t i = 0; i < 16; ++i) {
                        this->m_value[i] = ValueType();
                    }
                }

            /** Make identity matrix.
                \return identity matrix */
            static Mat4 identity()
                {
                    // ex mMakeIdentity
                    return Mat4(1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1);
                }

            /** Make perspective matrix.
                \param fov     Field-of-view in radians
                \param aspect  Aspect ratio
                \param near    Near bound
                \param far     Far bound
                \return Matrix */
            static Mat4 perspective(AccType fov, AccType aspect, AccType near, AccType far)
                {
                    // ex mMakePerspective
                    ValueType f = static_cast<ValueType>(std::cos(fov/2.0)) / static_cast<ValueType>(std::sin(fov/2.0));
                    AccType dist = (near - far);
                    ValueType c3 = static_cast<ValueType>((near + far) / dist);
                    ValueType d3 = static_cast<ValueType>(near * far * 2 / dist);
                    ValueType a1 = static_cast<ValueType>(f / aspect);

                    return Mat4(a1, 0, 0,  0,
                                0,  f, 0,  0,
                                0,  0, c3, -1,
                                0,  0, d3,  0);
                }

            /** Make perspective matrix.
                \param fov     Field-of-view in radians
                \param aspect  Aspect ratio
                \param near    Near bound
                \return Matrix */
            static Mat4 perspective(AccType fov, AccType aspect, AccType near)
                {
                    // ex mMakePerspective
                    ValueType f = static_cast<ValueType>(std::cos(fov/2.0)) / static_cast<ValueType>(std::sin(fov/2.0));
                    ValueType c3 = -1;
                    ValueType d3 = static_cast<ValueType>(-2*near);
                    ValueType a1 = static_cast<ValueType>(f / aspect);

                    return Mat4(a1, 0, 0,  0,
                                0,  f, 0,  0,
                                0,  0, c3, -1,
                                0,  0, d3,  0);
                }

            /** Invert matrix in-place.
                \retval true Matrix inverted successfully
                \retval false Matrix has no inverse; object not changed */
            bool invert()
                {
                    // ex mInvert
                    const ValueType
                        a00 = this->m_value[0],  a01 = this->m_value[1],  a02 = this->m_value[2],  a03 = this->m_value[3],
                        a10 = this->m_value[4],  a11 = this->m_value[5],  a12 = this->m_value[6],  a13 = this->m_value[7],
                        a20 = this->m_value[8],  a21 = this->m_value[9],  a22 = this->m_value[10], a23 = this->m_value[11],
                        a30 = this->m_value[12], a31 = this->m_value[13], a32 = this->m_value[14], a33 = this->m_value[15];

                    const ValueType
                        b00 = a00 * a11 - a01 * a10,
                        b01 = a00 * a12 - a02 * a10,
                        b02 = a00 * a13 - a03 * a10,
                        b03 = a01 * a12 - a02 * a11,
                        b04 = a01 * a13 - a03 * a11,
                        b05 = a02 * a13 - a03 * a12,
                        b06 = a20 * a31 - a21 * a30,
                        b07 = a20 * a32 - a22 * a30,
                        b08 = a20 * a33 - a23 * a30,
                        b09 = a21 * a32 - a22 * a31,
                        b10 = a21 * a33 - a23 * a31,
                        b11 = a22 * a33 - a23 * a32;

                    // Calculate the determinant
                    AccType det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

                    if (!det) {
                        return false;
                    }
                    det = 1.0 / det;

                    this->m_value[0]  = static_cast<ValueType>((a11 * b11 - a12 * b10 + a13 * b09) * det);
                    this->m_value[1]  = static_cast<ValueType>((a02 * b10 - a01 * b11 - a03 * b09) * det);
                    this->m_value[2]  = static_cast<ValueType>((a31 * b05 - a32 * b04 + a33 * b03) * det);
                    this->m_value[3]  = static_cast<ValueType>((a22 * b04 - a21 * b05 - a23 * b03) * det);
                    this->m_value[4]  = static_cast<ValueType>((a12 * b08 - a10 * b11 - a13 * b07) * det);
                    this->m_value[5]  = static_cast<ValueType>((a00 * b11 - a02 * b08 + a03 * b07) * det);
                    this->m_value[6]  = static_cast<ValueType>((a32 * b02 - a30 * b05 - a33 * b01) * det);
                    this->m_value[7]  = static_cast<ValueType>((a20 * b05 - a22 * b02 + a23 * b01) * det);
                    this->m_value[8]  = static_cast<ValueType>((a10 * b10 - a11 * b08 + a13 * b06) * det);
                    this->m_value[9]  = static_cast<ValueType>((a01 * b08 - a00 * b10 - a03 * b06) * det);
                    this->m_value[10] = static_cast<ValueType>((a30 * b04 - a31 * b02 + a33 * b00) * det);
                    this->m_value[11] = static_cast<ValueType>((a21 * b02 - a20 * b04 - a23 * b00) * det);
                    this->m_value[12] = static_cast<ValueType>((a11 * b07 - a10 * b09 - a12 * b06) * det);
                    this->m_value[13] = static_cast<ValueType>((a00 * b09 - a01 * b07 + a02 * b06) * det);
                    this->m_value[14] = static_cast<ValueType>((a31 * b01 - a30 * b03 - a32 * b00) * det);
                    this->m_value[15] = static_cast<ValueType>((a20 * b03 - a21 * b01 + a22 * b00) * det);

                    return true;
                }

            /** Transpose matrix in-place.
                \return *this */
            Mat4& transpose()
                {
                    // ex mTranspose
                    const ValueType             a2 = this->m_value[1],  a3 = this->m_value[2], a4 = this->m_value[3],
                        b1 = this->m_value[4],                          b3 = this->m_value[6], b4 = this->m_value[7],
                        c1 = this->m_value[8],  c2 = this->m_value[9],                         c4 = this->m_value[11],
                        d1 = this->m_value[12], d2 = this->m_value[13], d3 = this->m_value[14];

                    /* */                   this->m_value[1] = b1;  this->m_value[2]  = c1; this->m_value[3]  = d1;
                    this->m_value[4]  = a2;                         this->m_value[6]  = c2; this->m_value[7]  = d2;
                    this->m_value[8]  = a3; this->m_value[9] = b3;                          this->m_value[11] = d3;
                    this->m_value[12] = a4; this->m_value[13] = b4; this->m_value[14] = c4;
                    return *this;
                }

            /** Add translation component in-place
                Adds a translation by a given vector to the matrix.

                \param v Translation vector
                \return *this */
            Mat4& translate(const Vec3<ValueType, AccType>& v)
                {
                    // ex mTranslate
                    ValueType x = v(0), y = v(1), z = v(2);
                    this->m_value[12] += this->m_value[0] * x + this->m_value[4] * y + this->m_value[8]  * z;
                    this->m_value[13] += this->m_value[1] * x + this->m_value[5] * y + this->m_value[9]  * z;
                    this->m_value[14] += this->m_value[2] * x + this->m_value[6] * y + this->m_value[10] * z;
                    this->m_value[15] += this->m_value[3] * x + this->m_value[7] * y + this->m_value[11] * z;
                    return *this;
                }

            /** Add scaling by vector in-place.
                Scales each dimension by the corresponding vector component.

                \param v Scaling vector
                \return *this */
            Mat4& scale(const Vec3<ValueType, AccType>& v)
                {
                    // ex mScale
                    ValueType x = v(0), y = v(1), z = v(2);
                    for (size_t i = 0; i <  4; ++i) { (*this)(i) *= x; }
                    for (size_t i = 4; i <  8; ++i) { (*this)(i) *= y; }
                    for (size_t i = 8; i < 12; ++i) { (*this)(i) *= z; }
                    return *this;
                }

            /** Add scaling by scalar in-place.
                Scales each dimension by the given scalar.

                \param v Scale factor
                \return *this */
            Mat4& scale(ValueType v)
                {
                    for (size_t i = 0; i < 12; ++i) { (*this)(i) *= v; }
                    return *this;
                }

            /** Rotate around X axis, in-place.

                \param angle Angle in radians
                \return *this */
            Mat4& rotateX(AccType angle)
                {
                    // ex mRotateX
                    // Parameter is AccType; using ValueType loses too much precision
                    rotateInternal(std::sin(angle), std::cos(angle), 4, 8);
                    return *this;
                }

            /** Rotate around Y axis, in-place.

                \param angle Angle in radians
                \return *this */
            Mat4& rotateY(AccType angle)
                {
                    // ex mRotateY
                    rotateInternal(-std::sin(angle), std::cos(angle), 0, 8);
                    return *this;
                }

            /** Rotate around Z axis, in-place.

                \param angle Angle in radians
                \return *this */
            Mat4& rotateZ(AccType angle)
                {
                    // ex mRotateZ
                    rotateInternal(std::sin(angle), std::cos(angle), 0, 4);
                    return *this;
                }

            /** Matrix multiplication, in-place.
                \param other Matrix to multiply by
                \return *this */
            Mat4& operator*=(const Mat4& other)
                {
                    // ex mMultiply
                    Mat4 tmp = *this;
                    multiplyRow(tmp, other, 0);
                    multiplyRow(tmp, other, 4);
                    multiplyRow(tmp, other, 8);
                    multiplyRow(tmp, other, 12);
                    return *this;
                }

            /** Matrix multiplication.
                \param other Matrix to multiply by
                \return product */
            Mat4 operator*(const Mat4& other) const
                {
                    // ex mMultiply
                    Mat4 tmp;
                    tmp.multiplyRow(*this, other, 0);
                    tmp.multiplyRow(*this, other, 4);
                    tmp.multiplyRow(*this, other, 8);
                    tmp.multiplyRow(*this, other, 12);
                    return tmp;
                }

         private:
            /* Internal function to implement multiplication */
            void multiplyRow(const Mat4& a, const Mat4& b, size_t x)
                {
                    // Cache only the current line of the second matrix
                    ValueType b0 = b(x), b1 = b(x+1), b2 = b(x+2), b3 = b(x+3);
                    (*this)(x)   = b0*a(0) + b1*a(4) + b2*a(8)  + b3*a(12);
                    (*this)(x+1) = b0*a(1) + b1*a(5) + b2*a(9)  + b3*a(13);
                    (*this)(x+2) = b0*a(2) + b1*a(6) + b2*a(10) + b3*a(14);
                    (*this)(x+3) = b0*a(3) + b1*a(7) + b2*a(11) + b3*a(15);
                }

            /* Internal function to implement rotation */
            void rotateInternal(AccType s, AccType c, size_t x1, size_t x2)
                {
                    const AccType
                        b1 = this->m_value[x1], b2 = this->m_value[x1+1], b3 = this->m_value[x1+2], b4 = this->m_value[x1+3],
                        c1 = this->m_value[x2], c2 = this->m_value[x2+1], c3 = this->m_value[x2+2], c4 = this->m_value[x2+3];

                    this->m_value[x1]   = static_cast<ValueType>(b1 * c + c1 * s);
                    this->m_value[x1+1] = static_cast<ValueType>(b2 * c + c2 * s);
                    this->m_value[x1+2] = static_cast<ValueType>(b3 * c + c3 * s);
                    this->m_value[x1+3] = static_cast<ValueType>(b4 * c + c4 * s);

                    this->m_value[x2]   = static_cast<ValueType>(c1 * c - b1 * s);
                    this->m_value[x2+1] = static_cast<ValueType>(c2 * c - b2 * s);
                    this->m_value[x2+2] = static_cast<ValueType>(c3 * c - b3 * s);
                    this->m_value[x2+3] = static_cast<ValueType>(c4 * c - b4 * s);
                }
        };


        template<typename ValueType, typename AccType>
        Vec3<ValueType, AccType>
        Vec3<ValueType, AccType>::transform(const Mat4<ValueType, AccType>& m) const
        {
            // ex vTransform
            ValueType x = (*this)(0),
                y = (*this)(1),
                z = (*this)(2);
            ValueType w = m(3) * x + m(7) * y + m(11) * z + m(15);
            return Vec3((m(0) * x + m(4) * y + m(8)  * z + m(12)) / w,
                        (m(1) * x + m(5) * y + m(9)  * z + m(13)) / w,
                        (m(2) * x + m(6) * y + m(10) * z + m(14)) / w);
        }

    } // namespace detail

    /** Vector of 3 floats.
        Used to represent coordinates in 3D space. */
    typedef detail::Vec3<float, double> Vec3f;

    /** Vector of 4 floats. */
    typedef detail::Vec4<float, double> Vec4f;

    /** Matrix of 4x4 floats.
        Used to represent coordinate transformations.

        To implement a transformation, apply all steps in reverse order, and then use Vec3f::transform().
        For example, to move-then-scale, do
           Mat4f::identity().scale(...).translate()
        To rotate-then-scale-then-move-then-apply-perspective, do
           Mat4f::perspective().translate(...).scale(...).rotateX(...) */
    typedef detail::Mat4<float, double> Mat4f;

} }

#endif
