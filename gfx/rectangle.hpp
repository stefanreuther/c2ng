/**
  *  \file gfx/rectangle.hpp
  *  \brief Class gfx::Rectangle
  */
#ifndef C2NG_GFX_RECTANGLE_HPP
#define C2NG_GFX_RECTANGLE_HPP

#include <iosfwd>
#include "gfx/point.hpp"

namespace gfx {

    /** Rectangle.
        Rectangles are represented as position and extent.
        This corresponds to a half-open interval representation, where the top/left coordinate is part of the covered area,
        but the bottom/right one is not.

        Degenerate (zero-size, i.e. zero-width or zero-height) rectangles can be represented and are not normalized. */
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

        /** Construct rectangle with position and size.
            \param origin origin
            \param extent extent */
        Rectangle(Point origin, Point extent) throw();

        /** Get top coordinate.
            \return top Y */
        int getTopY() const
            { return m_top; }

        /** Get left coordinate.
            \return left X */
        int getLeftX() const
            { return m_left; }

        /** Get bottom coordinate.
            \return bottom Y */
        int getBottomY() const
            { return m_top + m_height; }

        /** Get right coordinate.
            \return right X */
        int getRightX() const
            { return m_left + m_width; }

        /** Get width.
            \return width */
        int getWidth() const
            { return m_width; }

        /** Get height.
            \return height */
        int getHeight() const
            { return m_height; }

        /** Get top-left point.
            \return point */
        Point getTopLeft() const
            { return Point(m_left, m_top); }

        /** Get top-right point.
            If the rectangle represents a pixel area, this point is just outside the rectangle;
            the nearest point inside the rectangle (if exists) is (-1,0) from that.
            \return point */
        Point getTopRight() const
            { return Point(getRightX(), m_top); }

        /** Get bottom-left point.
            If the rectangle represents a pixel area, this point is just outside the rectangle;
            the nearest point inside the rectangle (if exists) is (0,-1) from that.
            \return point */
        Point getBottomLeft() const
            { return Point(m_left, getBottomY()); }

        /** Get bottom-right point.
            If the rectangle represents a pixel area, this point is just outside the rectangle;
            the nearest point inside the rectangle (if exists) is (-1,-1) from that.
            \return point */
        Point getBottomRight() const
            { return Point(getRightX(), getBottomY()); }

        /** Get size of rectangle.
            \return size */
        Point getSize() const
            { return Point(m_width, m_height); }

        /** Set left X coordinate.
            \param left new value */
        void setLeftX(int left)
            { m_left = left; }

        /** Set top Y coordinate.
            \param top new value */
        void setTopY(int top)
            { m_top = top; }

        /** Set width.
            \param width new value */
        void setWidth(int width)
            { m_width = width; }

        /** Set height.
            \param height new value */
        void setHeight(int height)
            { m_height = height; }

        /** Intersect (clip) at rectangle.
            Modify this rectangle so that it lies entirely within the other rectangle, \c r.
            \param r Rectangle
            \post result.contains(p) iff orig.contains(p) && r.contains(p) for all points p */
        void intersect(const Rectangle& r) throw();

        /** Include rectangle.
            Computes the smallest possible rectangle which includes this one as well r,
            and modifies this rectangle to contain these bounds.
            \param r Rectangle
            \post r.contains(p) || orig.contains(p) => result.contains(p) for all points p (the converse obviously does not hold). */
        void include(const Rectangle& r) throw();

        /** Include point.
            Computes the smallest possible rectangle which includes this one as well as pt,
            and modifies this rectangle to contain these bounds.
            \param pt Point
            \post r.contains(p) => result.contains(p) for all points p
            \post result.contains(pt) */
        void include(Point pt) throw();

        /** Check whether this rectangle completely contains r.
            \param r rectangle
            \return true iff r.contains(x,y) => contains(x,y) for all x,y. */
        bool contains(const Rectangle& r) const throw();

        /** Check whether rectangle contains a given point.
            \param px,py Coordinates
            \return true if point is within rectangle */
        bool contains(int px, int py) const
            {
                // FIXME: improvement possible
                px -= m_left;
                py -= m_top;
                return px >= 0 && py >= 0 && px < m_width && py < m_height;
            }

        /** Check whether rectangle contains a given point.
            \param pt Point
            \return true if point is within rectangle */
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

        /** Check whether this rectangle has a non-zero extent.
            \return true if rectangle has a non-zero extent */
        bool exists() const throw()
            { return m_width > 0 && m_height > 0; }

        /** Move to point without changing the size.
            Moves the rectangle to point \c where without touching the size.
            \param where new top-left corner for this rectangle
            \return relative movement done. E.g. if a rectangle located at (5,5) is moved to (20,30), this returns (15,25).
            \post x == where.x && y == where.y */
        Point moveTo(Point where) throw();

        /** Move this rectangle such that it is contained within \c other.
            Does not change our size.
            If this rectangle does not fit within \c other, it is aligned at the left/top border.
            \param other Other rectangle
            \return *this */
        Rectangle& moveIntoRectangle(const Rectangle& other) throw();

        /** Move by relative distance.
            The distance is given in the components of a point.
            \param dist Relative movement */
        void moveBy(Point dist)
            { m_left += dist.getX(); m_top += dist.getY(); }

        /** Grow rectangle.
            This enlarges the rectangle by \c dx columns to the left and right,
            and by \c dy lines at top and bottom (for a total growth of twice the given amount, of course).
            Use negative values to shrink.
            \param dx Width increase
            \param dy Height increase */
        void grow(int dx, int dy)
            { m_left -= dx; m_top -= dy; m_width += 2*dx; m_height += 2*dy; }

        /** Get center point of rectangle.
            \return center */
        Point getCenter() const
            { return Point(m_left + m_width/2, m_top + m_height/2); }

        /** Check whether this rectangle intersects another one.
            \param r other rectangle
            \return true if this rectangle intersects \c r */
        bool isIntersecting(Rectangle r) const
            {
                r.intersect(*this);
                return r.exists();
            }

        /** Center this rectangle within another.
            \param other other rectangle
            \return *this */
        Rectangle& centerWithin(const Rectangle& other) throw();

        /** Move this rectangle to edge of another.
            \param other  Bounding rectangle
            \param xPos   Relative X position (0: left, 1: center, 2: right)
            \param yPos   Relative Y position (0: top, 1: center, 2: bottom)
            \param offset Distance to edge. When anchored at an edge (0 or 2),
                          leave that many pixels from that edge.
            \return *this */
        Rectangle& moveToEdge(const Rectangle& other, int xPos, int yPos, int offset) throw();

        /** Reduce this rectangle's width from the left.
            Removes \c pix pixels from the left (less if the rectangle is narrower).
            Same as splitX, but does not return a result.
            \param pix number of pixels to remove */
        void consumeX(int pix);

        /** Split rectangle vertically.
            Removes \c pix pixels from the left (less if the rectangle is narrower), like consumeX(int).
            \param pix number of pixels to remove
            \return removed rectangle */
        Rectangle splitX(int pix);

        /** Reduce this rectangle's height from the top.
            Removes \c pix pixels from the top (less if the rectangle is shorter).
            Same as splitY, but does not return a result.
            \param pix number of pixels to remove */
        void consumeY(int pix);

        /** Split rectangle horizontally.
            Removes \c pix pixels from the top (less if the rectangle is shorter), like consumeY(int).
            \param pix number of pixels to remove
            \return removed rectangle */
        Rectangle splitY(int pix);

        /** Reduce this rectangle's width from the right.
            Removes \c pix pixels from the right (less if the rectangle is narrower).
            Same as splitRightX, but does not return a result.
            \param pix number of pixels to remove */
        void consumeRightX(int pix);

        /** Split rectangle vertically.
            Removes \c pix pixels from the right (less if the rectangle is narrower), like consumeRightX(int).
            \param pix number of pixels to remove
            \return removed rectangle */
        Rectangle splitRightX(int pix);

        /** Reduce this rectangle's height from the bottom.
            Removes \c pix pixels from the bottom (less if the rectangle is shorter).
            Same as splitBottomY, but does not return a result.
            \param pix number of pixels to remove */
        void consumeBottomY(int pix);

        /** Split rectangle horizontally.
            Removes \c pix pixels from the bottom (less if the rectangle is shorter), like consumeBottomY(int).
            \param pix number of pixels to remove
            \return removed rectangle */
        Rectangle splitBottomY(int pix);

     private:
        int m_left;
        int m_top;
        int m_width;
        int m_height;
    };
}

/** Output rectangle.
    Generates the X11 geometry format (WxH+X+Y).
    \param os Output stream
    \param r Rectangle to output
    \return os */
std::ostream& operator<<(std::ostream& os, const gfx::Rectangle& r);

#endif
