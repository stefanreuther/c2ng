/**
  *  \file gfx/point.hpp
  *  \brief Class gfx::Point
  */
#ifndef C2NG_GFX_POINT_HPP
#define C2NG_GFX_POINT_HPP

#include "afl/string/string.hpp"

namespace gfx {

    /** Point.
        This object contains a point in the X/Y plane on a graphics canvas. */
    class Point {
     public:
        /** Create point at origin. */
        Point()
            : m_x(0), m_y(0)
            { }

        /** Create point at specified X/Y.
            \param x,y Coordinates */
        Point(int x, int y)
            : m_x(x), m_y(y)
            { }

        /** Get X coordinate.
            \return value */
        int getX() const
            { return m_x; }

        /** Get Y coordinate.
            \return value */
        int getY() const
            { return m_y; }

        /** Set X coordinate.
            \param x new value */
        void setX(int x)
            { m_x = x; }

        /** Set Y coordinate.
            \param y new value */
        void setY(int y)
            { m_y = y; }

        /** Add to X coordinate.
            \param dx value to add */
        void addX(int dx)
            { m_x += dx; }

        /** Add to Y coordinate.
            \param dy value to add */
        void addY(int dy)
            { m_y += dy; }

        /** Compare two points for equality.
            \param rhs Point to compare
            \return true if equal */
        bool operator==(Point rhs) const
            { return m_x == rhs.m_x && m_y == rhs.m_y; }

        /** Compare two points for inequality.
            \param rhs Point to compare
            \return true if different */
        bool operator!=(Point rhs) const
            { return m_x != rhs.m_x || m_y != rhs.m_y; }

        /** Vector addition.
            \param other Point to add
            \return point with component-wise coordinate sum */
        Point operator+(Point other) const
            { return Point(m_x + other.m_x, m_y + other.m_y); }

        /** Vector subtraction.
            \param other Point to subtract
            \return point with component-wise coordinate difference */
        Point operator-(Point other) const
            { return Point(m_x - other.m_x, m_y - other.m_y); }

        /** In-place vector addition.
            Adds the other point's coordinates to this one.
            \param other Point to add
            \return *this */
        Point& operator+=(Point other)
            { m_x += other.m_x; m_y += other.m_y; return *this; }

        /** In-place vector subtraction.
            Subtracts the other point's coordinates from this one.
            \param other Point to subtract
            \return *this */
        Point& operator-=(Point other)
            { m_x -= other.m_x; m_y -= other.m_y; return *this; }

        /** Scale point coordinates.
            \param sx Multiply x by this value
            \param sy Multiply y by this value
            \return scaled point (does not change *this)! */
        Point scaledBy(int sx, int sy) const
            { return Point(m_x * sx, m_y * sy); }

        /** Scale point coordinates.
            \param other Point to to multiply with
            \return point with component-wise coordinate product */
        Point scaledBy(Point other) const
            { return Point(m_x * other.m_x, m_y * other.m_y); }

        /** Extend area to the right.
            Assuming this point and \c other describe the size of two rectangles,
            respectively, modifies this to describe the size of the bounding rectangle
            if both are laid next to each other.
            \param other Other dimension
            \return *this */
        Point& extendRight(Point other);

        /** Extend area below.
            Assuming this point and \c other describe the size of two rectangles,
            respectively, modifies this to describe the size of the bounding rectangle
            if both are laid below to each other.
            \param other Other dimension
            \return *this */
        Point& extendBelow(Point other);

     private:
        int m_x;    ///< X coordinate.
        int m_y;    ///< Y coordinate.
    };

    /** makePrintable for testing.
        \param pt Point */
    String_t makePrintable(const Point& pt);

}

#endif
