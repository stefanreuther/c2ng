/**
  *  \file gfx/rectangle.hpp
  */
#ifndef C2NG_GFX_RECTANGLE_HPP
#define C2NG_GFX_RECTANGLE_HPP

#include <iosfwd>
#include "gfx/point.hpp"

namespace gfx {

// /** Rectangle. An SDL_Rect or a faster implementation. Represents
//     rectangles as position plus extent. This offers some useful
//     primitives to work with rectangles. Otherwise, this is a plain old
//     structure and you are invited to modify it directly. */
    class Rectangle {
     public:
        /** Construct empty rectangle.
            \post contains(p) == false for every p */
        Rectangle() throw();

        /** Construct Rectangle from explicit parameters.
            \param x left
            \param y top
            \param w width
            \param h height */
        Rectangle(int x, int y, int w, int h) throw();

        // /** Construct rectangle with given size.
        //     The rectangle is located at (0,0).
        //     \param extent Specifies size of rectangle */
        // explicit Rectangle(Point extent) throw();

        /** Construct rectangle with position and size.
            \param origin Origin
            \param extent extent */
        Rectangle(Point origin, Point extent) throw();

        /** Get top coordinate. */
        int getTopY() const
            { return m_top; }

        /** Get right coordinate. */
        int getLeftX() const
            { return m_left; }

        /** Get bottom coordinate. */
        int getBottomY() const
            { return m_top + m_height; }

        /** Get right coordinate. */
        int getRightX() const
            { return m_left + m_width; }

        /** Get width. */
        int getWidth() const
            { return m_width; }

        /** Get height. */
        int getHeight() const
            { return m_height; }

        /** Get top-left point. */
        Point getTopLeft() const
            { return Point(m_left, m_top); }

        /** Get bottom-right point. */
        Point getBottomRight() const
            { return Point(getRightX(), getBottomY()); }

        /** Get size of rectangle. */
        Point getSize() const
            { return Point(m_width, m_height); }


        void setLeftX(int left)
            { m_left = left; }
        void setTopY(int top)
            { m_top = top; }
        void setWidth(int width)
            { m_width = width; }
        void setHeight(int height)
            { m_height = height; }



        /** Intersect (clip) at rectangle.
            Modify this rectangle so that it lies entirely within the other rectangle, \c r.
            \post result.contains(p) iff orig.contains(p) && r.contains(p) for all points p */
        void intersect(const Rectangle& r) throw();

        /** Include rectangle.
            Computes the smallest possible rectangle which includes this one as well r,
            and modifies this rectangle to contain these bounds.

            \post r.contains(p) || orig.contains(p) => result.contains(p) for all points p (the converse obviously does not hold). */
        void include(const Rectangle& r) throw();

        /** Include point.
            Computes the smallest possible rectangle which includes this one as well as pt,
            and modifies this rectangle to contain these bounds.

            \post r.contains(p) => result.contains(p) for all points p
            \post result.contains(pt) */
        void include(Point pt) throw();

        /** Check whether this rectangle completely contains r.
            \param r rectangle
            \return true iff r.contains(x,y) => contains(x,y) for all x,y. */
        bool contains(const Rectangle& r) const throw();

        /** Check whether rectangle contains a given point. */
        bool contains(int px, int py) const
            {
                // FIXME: improvement possible
                px -= m_left;
                py -= m_top;
                return px >= 0 && py >= 0 && px < m_width && py < m_height;
            }

        /** Check whether rectangle contains a given point. */
        bool contains(Point pt) const throw()
            { return contains(pt.getX(), pt.getY()); }

        /** Compare rectangles for equality.
            \return true iff rhs completely identical to *this
            \note empty rectangles do not compare equal unless their x,y is also identical */
        bool operator==(const Rectangle& rhs) const throw();

        /** Compare rectangles for inequality.
            \return true iff rhs differs somehow from *this
            \note empty rectangles do not compare equal unless their x,y is also identical */
        bool operator!=(const Rectangle& rhs) const throw();

        /** Check whether this rectangle has a non-zero extent. */
        bool exists() const throw()
            { return m_width > 0 && m_height > 0; }

        /** Move to point without changing the size.
            Moves the rectangle to point \c where without touching the size.
            \param where new top-left corner for this rectangle
            \return relative movement done. E.g. if a rectangle located at (5,5) is moved to (20,30), this returns (15,25).
            \post x == where.x && y == where.y */
        Point moveTo(Point where) throw();

        // FIXME: delete, retire and replace by generic moveTo
        //     /** Move to point. \overload */
        //     Point moveTo(int px, int py) throw()
        //         { return moveTo(Point(px, py)); }

        /** Move by relative distance.
            The distance is given in the components of a point. */
        void moveBy(Point dist)
            { m_left += dist.getX(); m_top += dist.getY(); }

        // FIXME: delete, retire and replace by generic moveBy
        //     /** Move by relative distance. The distance is given by two
        //         integers. */
        //     void moveBy(int dx, int dy)
        //         { x += dx; y += dy; }
        //     /** Move by relative distance, reverse direction. */
        //     void moveByRev(Point dist)
        //         { x -= dist.x; y -= dist.y; }
        //     /** Move by relative distance, reverse direction. */
        //     void moveByRev(int dx, int dy)
        //         { x -= dx; y -= dy; }

        /** Grow rectangle.
            This enlarges the rectangle by \c dx columns to the left and right,
            and by \c dy lines at top and bottom (for a total growth of twice the given amount, of course).
            Use negative values to shrink. */
        void grow(int dx, int dy)
            { m_left -= dx; m_top -= dy; m_width += 2*dx; m_height += 2*dy; }

        // FIXME: delete, unused
        //     /** Grow rectangle in one direction. This enlarges the rectangle
        //         by \c dx columns to the right and \c dy lines to the bottom,
        //         without affecting its origin. */
        //     void growSize(int dx, int dy)
        //         { w += dx; h += dy; }
        //     /** Grow rectangle in one direction. \overload */
        //     void growSize(Point pt)
        //         { w += pt.x; h += pt.y; }

        /** Get center point of rectangle. */
        Point getCenter() const
            { return Point(m_left + m_width/2, m_top + m_height/2); }

        /** Check whether this rectangle intersects another one. */
        bool isIntersecting(Rectangle r) const
            {
                r.intersect(*this);
                return r.exists();
            }

//     void moveInto(const Rectangle& other) throw();

        /** Center this rectangle within \c other. */
        void centerWithin(const Rectangle& other) throw();

        /** Move this rectangle to edge of \c other.
            \param other  Bounding rectangle
            \param xPos   Relative X position (0: left, 1: center, 2: right)
            \param yPos   Relative Y position (0: top, 1: center, 2: bottom)
            \param offset Distance to edge. When anchored at an edge (0 or 2),
                          leave that many pixels from that edge. */
        void moveToEdge(const Rectangle& other, int xPos, int yPos, int offset) throw();

        /** Reduce this rectangle's width from the left.
            Removes \c pix pixels from the left (less if the rectangle is narrower).
            Same as splitX, but does not return a result.
            \param pix number of pixels to remove */
        void consumeX(int pix);

        /** Split rectangle vertically.
            Removes \c pix pixels from the left (less if the rectangle is narrower), like consumeX(int).
            Returns the removed rectangle.
            \param pix number of pixels to remove */
        Rectangle splitX(int pix);

        /** Reduce this rectangle's height from the top.
            Removes \c pix pixels from the top (less if the rectangle is shorter).
            Same as splitY, but does not return a result.
            \param pix number of pixels to remove */
        void consumeY(int pix);

        /** Split rectangle horizontally.
            Removes \c pix pixels from the top (less if the rectangle is shorter), like consumeY(int).
            Returns the removed rectangle.
            \param pix number of pixels to remove */
        Rectangle splitY(int pix);

     private:
        int m_left;
        int m_top;
        int m_width;
        int m_height;
    };
}

std::ostream& operator<<(std::ostream& os, const gfx::Rectangle& r);

#endif
