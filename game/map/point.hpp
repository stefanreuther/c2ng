/**
  *  \file game/point.hpp
  *  \brief Class game::map::Point
  */
#ifndef C2NG_GAME_POINT_HPP
#define C2NG_GAME_POINT_HPP

#include "afl/string/string.hpp"

namespace game { namespace map {

    /** Point in universe space.
        Represents a 2D vector (point or distance).

        This is a data container class.
        Methods to deal with map configuration were in this class in PCC2; they are now in game::map::Configuration. */
    class Point {
     public:
        enum Component { X, Y };

        /** Constructor.
            Makes a default-initialized point.
            \post getX()=0, getY()=0 */
        Point();

        /** Constructor.
            \param x Initial X (west..east)
            \param y Initial Y (south..north) */
        Point(int x, int y);

        /** Get X coordinate.
            \return X */
        int getX() const;

        /** Get Y coordinate.
            \return Y */
        int getY() const;

        /** Set X coordinate.
            \param x New X coordinate */
        void setX(int x);

        /** Set Y coordinate.
            \param y New Y coordinate */
        void setY(int y);

        /** Add to X coordinate.
            \param dx Value to add */
        void addX(int dx);

        /** Add to Y coordinate.
            \param dy Value to add */
        void addY(int dy);

        /** Get component.
            \param c Component */
        int get(Component c) const;

        /** Set component.
            \param c Component
            \param v Value */
        void set(Component c, int v);

        /** Compare for equality.
            \param rhs Other point
            \return true if both points are equal */
        bool operator==(Point rhs) const;

        /** Compare for inequality.
            \param rhs Other point
            \return true if both points are different */
        bool operator!=(Point rhs) const;

        /** Vector addition.
            \param other Other point
            \return result */
        Point operator+(Point other) const;

        /** Vector subtraction.
            \param other Other point
            \return result */
        Point operator-(Point other) const;

        /** In-Place vector addition.
            \param other Other point
            \return *this */
        Point& operator+=(Point other);

        /** In-Place vector subtraction.
            \param other Other point
            \return *this */
        Point& operator-=(Point other);

        /** Parse coordinates.
            \param s String containing coordinates in the form "111,222"
            \retval true String was parsed, object updated
            \retval false Syntax error, object not updated */
        bool parseCoordinates(const String_t& s);

        /** Three-way comparison.
            \param other Other point
            \return -1 if this point is lexically before \c other, 0 if equal, +1 if after */
        int compare(const Point& other) const;

        /** Get squared distance to another point.
            \param other Other point
            \return Squared distance */
        long getSquaredRawDistance(Point other) const;

        /** Check distance.
            \param other Other point
            \param distance Distance to check
            \return true if other point is strictly closer than \c distance to this one */
        bool isCloserThan(Point other, long distance) const;

     private:
        int m_x;
        int m_y;
    };

} }

// Constructor.
inline
game::map::Point::Point()
    : m_x(0), m_y(0)
{ }

// Constructor.
inline
game::map::Point::Point(int x, int y)
    : m_x(x), m_y(y)
{ }

// Get X coordinate.
inline int
game::map::Point::getX() const
{
    return m_x;
}

// Get Y coordinate.
inline int
game::map::Point::getY() const
{
    return m_y;
}

// Set X coordinate.
inline void
game::map::Point::setX(int x)
{
    m_x = x;
}

// Set Y coordinate.
inline void
game::map::Point::setY(int y)
{
    m_y = y;
}

// Add to X coordinate.
inline void
game::map::Point::addX(int dx)
{
    m_x += dx;
}

// Add to Y coordinate.
inline void
game::map::Point::addY(int dy)
{
    m_y += dy;
}

// Compare for equality.
inline bool
game::map::Point::operator==(Point rhs) const
{
    return m_x == rhs.m_x && m_y == rhs.m_y;
}

// Compare for inequality.
inline bool
game::map::Point::operator!=(Point rhs) const
{
    return m_x != rhs.m_x || m_y != rhs.m_y;
}

// Vector addition.
inline game::map::Point
game::map::Point::operator+(Point other) const
{
    return Point(m_x + other.m_x, m_y + other.m_y);
}

// Vector subtraction.
inline game::map::Point
game::map::Point::operator-(Point other) const
{
    return Point(m_x - other.m_x, m_y - other.m_y);
}

// In-Place vector addition.
inline game::map::Point&
game::map::Point::operator+=(Point other)
{
    m_x += other.m_x; m_y += other.m_y;
    return *this;
}

// In-Place vector subtraction.
inline game::map::Point&
game::map::Point::operator-=(Point other)
{
    m_x -= other.m_x;
    m_y -= other.m_y;
    return *this;
}

#endif
