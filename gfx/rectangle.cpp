/**
  *  \file gfx/rectangle.cpp
  */

#include <ostream>
#include "gfx/rectangle.hpp"

// Construct empty rectangle.
gfx::Rectangle::Rectangle() throw()
    : m_left(0),
      m_top(0),
      m_width(0),
      m_height(0)
{ }

// FIXME: delete
// /** Construct Rectangle from SDL_Rect. */
// inline
// Rectangle::Rectangle(const SDL_Rect& r) throw()
//     : SDL_Rect(r)
// { }

// Construct Rectangle from explicit parameters.
gfx::Rectangle::Rectangle(int x, int y, int w, int h) throw()
    : m_left(x),
      m_top(y),
      m_width(w),
      m_height(h)
{ }

// FIXME: delete. This makes all sorts of problems with overloaded functions
// where 'foo(pt)' is different from 'foo(Rectangle(pt))'.
// // Construct rectangle with given size.
// gfx::Rectangle::Rectangle(Point extent) throw()
//     : m_left(0),
//       m_top(0),
//       m_width(extent.getX()),
//       m_height(extent.getY())
// { }

// FIXME: delete
// /** Construct rectangle with given size at position.
//     \param x,y Origin
//     \param pt  Specifies size of rectangle */
// gfx::Rectangle::Rectangle(int x, int y, Point pt) throw()
// {
//     this->x = x;
//     this->y = y;
//     this->w = pt.x;
//     this->h = pt.y;
// }

// FIXME: delete
// /** Construct rectangle at given position.
//     \param pt Origin
//     \param w,h Size */
// inline
// Rectangle::Rectangle(Point pt, int w, int h) throw()
// {
//     this->x = pt.x;
//     this->y = pt.y;
//     this->w = w;
//     this->h = h;
// }

// Construct rectangle with position and size.
gfx::Rectangle::Rectangle(Point origin, Point extent) throw()
    : m_left(origin.getX()),
      m_top(origin.getY()),
      m_width(extent.getX()),
      m_height(extent.getY())
{ }

// FIXME: delete
// /** Construct bounding rectangle for GfxCanvas.
//     Constructs a rectangle that covers the whole area of \c can. */
// Rectangle::Rectangle(GfxCanvas& can)
// {
//     Point pt = can.getSize();
//     x = y = 0;
//     w = pt.x;
//     h = pt.y;
// }

// FIXME: delete
// /** Clip at size. Modify this rectangle so that it lies entirely
//     within \c Rectangle(0, 0, cw, ch). This is slightly more efficient
//     than clipRect(). */
// void
// Rectangle::clipSize(int cw, int ch) throw()
// {
//     register long sw = w, sh = h;
//     if (x < 0)
//         sw += x, x = 0;
//     if (y < 0)
//         sh += y, y = 0;
//     if (x + sw > cw)
//         sw = cw - x;
//     if (y + sh > ch)
//         sh = ch - y;
//     w = (sw < 0) ? 0 : sw;
//     h = (sh < 0) ? 0 : sh;
// }


// Intersect (clip) at rectangle.
void
gfx::Rectangle::intersect(const Rectangle& r) throw()
{
    // ex GfxRect::clipSize
    // FIXME: this algorithm sucks
    register long sw = m_width, sh = m_height;
    if (m_left < r.m_left) {
        sw += m_left - r.m_left, m_left = r.m_left;
    }
    if (m_top < r.m_top) {
        sh += m_top - r.m_top, m_top = r.m_top;
    }

    if (m_left + sw > r.m_width + r.m_left) {
        sw = r.m_width - m_left + r.m_left;
    }
    if (m_top + sh > r.m_height + r.m_top) {
        sh = r.m_height - m_top + r.m_top;
    }
    m_width = (sw < 0) ? 0 : sw;
    m_height = (sh < 0) ? 0 : sh;
}

// Include rectangle.
void
gfx::Rectangle::include(const Rectangle& r) throw()
{
    // ex GfxRect::include
    if (r.m_width <= 0 || r.m_height <= 0) {
        // Other does not exist
        return;
    }
    if (!exists()) {
        // This does not exist
        *this = r;
        return;
    }

    if (r.m_left < m_left) {
        m_width += m_left - r.m_left;
        m_left = r.m_left;
    }
    if (r.m_top < m_top) {
        m_height += m_top - r.m_top;
        m_top = r.m_top;
    }
    if (r.m_left + r.m_width - m_left > m_width)
        m_width = r.m_left + r.m_width - m_left;
    if (r.m_top + r.m_height - m_top > m_height)
        m_height = r.m_top + r.m_height - m_top;
}

// Include point.
void
gfx::Rectangle::include(Point pt) throw()
{
    include(Rectangle(pt, Point(1, 1)));
}

// Check whether this rectangle completely contains r.
bool
gfx::Rectangle::contains(const Rectangle& r) const throw()
{
    if (!exists()) {
        // empty does not contain anything
        return false;
    }
    if (!r.exists()) {
        // empty contained in anything
        return true;
    }

    // check whether r AND *this == r
    Rectangle rcopy = r;
    rcopy.intersect(*this);
    return rcopy == r;
}


// Compare rectangles for equality.
bool
gfx::Rectangle::operator==(const Rectangle& rhs) const throw()
{
    return m_left == rhs.m_left
        && m_top == rhs.m_top
        && m_width == rhs.m_width
        && m_height == rhs.m_height;
}

// Compare rectangles for inequality.
bool
gfx::Rectangle::operator!=(const Rectangle& rhs) const throw()
{
    return m_left != rhs.m_left
        || m_top != rhs.m_top
        || m_width != rhs.m_width
        || m_height != rhs.m_height;
}

// Move to point without changing the size.
gfx::Point
gfx::Rectangle::moveTo(Point where) throw()
{
    Point movedBy(where.getX() - m_left, where.getY() - m_top);
    moveBy(movedBy);
    return movedBy;
}


// /** Move this rectangle such that it is contained within \c other. 
//     Does not change our size. */
// void
// Rectangle::moveInto(const Rectangle& other) throw()
// {
//     if (m_left < other.m_left)
//         m_left = other.m_left;
//     if (m_top < other.m_top)
//         m_top = other.m_top;

//     int avail_x = other.m_left + other.m_width - m_left;
//     if (avail_x < m_width)
//         m_left = other.m_left + other.m_width - m_width;

//     int avail_y = other.m_top + other.m_height - m_top;
//     if (avail_y < m_height)
//         m_top = other.m_top + other.m_height - m_height;
// }

// Center this rectangle within \c other.
void
gfx::Rectangle::centerWithin(const Rectangle& other) throw()
{
    moveToEdge(other, 1, 1, 0);
}

// Move this rectangle to edge of \c other.
void
gfx::Rectangle::moveToEdge(const Rectangle& other, int xPos, int yPos, int offset) throw()
{
    const int virtW = other.m_width  - m_width;
    const int virtH = other.m_height - m_height;

    m_left = other.m_left + xPos * virtW / 2 - offset * (xPos-1);
    m_top  = other.m_top  + yPos * virtH / 2 - offset * (yPos-1);
}

// Reduce this rectangle's width from the left.
void
gfx::Rectangle::consumeX(int pix)
{
    if (pix < 0) {
        pix = 0;
    }
    if (pix > m_width) {
        pix = m_width;
    }
    m_width -= pix;
    m_left += pix;
}

// Split rectangle vertically.
gfx::Rectangle
gfx::Rectangle::splitX(int pix)
{
    if (pix < 0) {
        pix = 0;
    }
    if (pix > m_width) {
        pix = m_width;
    }
    const int resultX = m_left;
    m_width -= pix;
    m_left += pix;
    return Rectangle(resultX, m_top, pix, m_height);
}

// Reduce this rectangle's height from the top.
void
gfx::Rectangle::consumeY(int pix)
{
    if (pix < 0) {
        pix = 0;
    }
    if (pix > m_height) {
        pix = m_height;
    }
    m_height -= pix;
    m_top += pix;
}

// Split rectangle horizontally.
gfx::Rectangle
gfx::Rectangle::splitY(int pix)
{
    if (pix < 0) {
        pix = 0;
    }
    if (pix > m_height) {
        pix = m_height;
    }
    const int resultY = m_top;
    m_height -= pix;
    m_top += pix;
    return Rectangle(m_left, resultY, m_width, pix);
}

std::ostream& operator<<(std::ostream& os, const gfx::Rectangle& r)
{
    return os << r.getLeftX() << "x" << r.getTopY() << "+" << r.getWidth() << "+" << r.getHeight();
}
