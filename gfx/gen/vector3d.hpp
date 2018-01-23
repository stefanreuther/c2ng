/**
  *  \file gfx/gen/vector3d.hpp
  *  \brief Template class gfx::gen::Vector3D
  */
#ifndef C2NG_GFX_GEN_VECTOR3D_HPP
#define C2NG_GFX_GEN_VECTOR3D_HPP

namespace gfx { namespace gen {

    /** 3-D vector. */
    template<typename T>
    class Vector3D {
     public:
        /** Construct null vector. */
        Vector3D()
            : x(0), y(0), z(0)
            { }

        /** Construct from components. */
        Vector3D(T x, T y, T z)
            : x(x), y(y), z(z)
            { }

        /** Vector addition.
            \param other Vector to add to this one.
            \return Vector sum */
        Vector3D operator+(const Vector3D& other) const
            { return Vector3D(x + other.x, y + other.y, z + other.z); }

        /** Vector subtraction.
            \param other Vector to subtract from this one.
            \return Vector difference */
        Vector3D operator-(const Vector3D& other) const
            { return Vector3D(x - other.x, y - other.y, z - other.z); }

        /** Vector multiplication (scaling).
            \param t Scale factor
            \return Scaled vector */
        Vector3D operator*(T t) const
            { return Vector3D(x*t, y*t, z*t); }

        /** Dot product.
            \param other Other vector
            \return dot product */
        T dot(const Vector3D& other) const
            { return x*other.x + y*other.y + z*other.z; }

        /** Magnitude, squared.
            \return Magnitude (length) of this vector, squared */
        T mag2() const
            { return dot(*this); }

        /** Components. */
        T x, y, z;
    };

} }

#endif
