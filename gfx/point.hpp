/**
  *  \file gfx/point.hpp
  */
#ifndef C2NG_GFX_POINT_HPP
#define C2NG_GFX_POINT_HPP

#include <iosfwd>

namespace gfx {

    /** Point.
        This object contains a point in the X/Y plane.
        This is a plain old structure, don't hesitate to modify it directly. */
    class Point {
     public:
        /** Create point. */
        Point()
            : m_x(0), m_y(0)
            { }
        /** Create point at specified X/Y. */
        Point(int x, int y)
            : m_x(x), m_y(y)
            { }

        int getX() const
            { return m_x; }
                
        int getY() const
            { return m_y; }

        void setX(int x)
            { m_x = x; }
        void setY(int y)
            { m_y = y; }
        void addX(int dx)
            { m_x += dx; }
        void addY(int dy)
            { m_y += dy; }

        /** Compare two points for equality. */
        bool operator==(Point rhs) const
            { return m_x == rhs.m_x && m_y == rhs.m_y; }
        /** Compare two points for inequality. */
        bool operator!=(Point rhs) const
            { return m_x != rhs.m_x || m_y != rhs.m_y; }

        Point operator+(Point other) const
            { return Point(m_x + other.m_x, m_y + other.m_y); }
        Point operator-(Point other) const
            { return Point(m_x - other.m_x, m_y - other.m_y); }
        Point& operator+=(Point other)
            { m_x += other.m_x; m_y += other.m_y; return *this; }
        Point& operator-=(Point other)
            { m_x -= other.m_x; m_y -= other.m_y; return *this; }

        /** Scale point coordinates.
            \param sx Multiply x by this value
            \param sy Multiply y by this value
            \return scaled point (does not change *this)! */
        Point scaledBy(int sx, int sy) const
            { return Point(m_x * sx, m_y * sy); }

        Point scaledBy(Point other) const
            { return Point(m_x * other.m_x, m_y * other.m_y); }

     private:
        int m_x;    ///< X coordinate.
        int m_y;    ///< Y coordinate.
    };

}

std::ostream& operator<<(std::ostream& os, const gfx::Point& pt);

#endif
