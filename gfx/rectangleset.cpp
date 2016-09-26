/**
  *  \file gfx/rectangleset.cpp
  */

#include "gfx/rectangleset.hpp"

// Construct empty set.
gfx::RectangleSet::RectangleSet()
    : m_list()
{ }

// Construct rectangular set.
gfx::RectangleSet::RectangleSet(const Rectangle& r)
    : m_list()
{
    m_list.push_back(r);
}

// Intersect this rectangle set with r.
void
gfx::RectangleSet::intersect(const Rectangle& r)
{
    // FIXME: this is more efficient to do in-place
    List_t other;
    for (Iterator_t i = m_list.begin(); i != m_list.end(); ++i) {
        Rectangle q(*i);
        q.intersect(r);
        if (q.exists()) {
            other.push_back(q);
        }
    }
    m_list.swap(other);
}


// Add rectangle to set.
void
gfx::RectangleSet::add(const Rectangle& r)
{
    List_t l2;
    l2.push_back(r);
    for (Iterator_t i = m_list.begin(); i != m_list.end(); ++i) {
        List_t result;
        doSubtract(*i, result, l2);
        l2.swap(result);
    }
    m_list.splice(m_list.end(), l2);
}

void
gfx::RectangleSet::add(const RectangleSet& r)
{
    if (&r != this) {
        for (Iterator_t i = r.m_list.begin(); i != r.m_list.end(); ++i) {
            add(*i);
        }
    }
}

// Remove rectangle from set.
void
gfx::RectangleSet::remove(const Rectangle& r)
{
    List_t l2;
    doSubtract(r, l2, m_list);
    m_list.swap(l2);
}

// Check if point is in set.
bool
gfx::RectangleSet::contains(Point pt) const
{
    for (Iterator_t i = begin(); i != end(); ++i) {
        if (i->contains(pt)) {
            return true;
        }
    }
    return false;
}

// Check if rectangle in set.
bool
gfx::RectangleSet::contains(const Rectangle& r) const
{
    List_t l2;
    l2.push_back(r);
    for (Iterator_t i = begin(); i != end(); ++i) {
        List_t result;
        doSubtract(*i, result, l2);
        l2.swap(result);
    }

    return !l2.empty();
}

// Compute bounding rectangle.
gfx::Rectangle
gfx::RectangleSet::getBoundingRectangle() const
{
    Rectangle br;
    for (Iterator_t i = begin(); i != end(); ++i) {
        br.include(*i);
    }
    return br;
}

/** Rectangle Set Subtraction. Constructs in \c l2 the set of rectangles
    in \c other which are outside \c r. Rectangles are split when needed. */
void
gfx::RectangleSet::doSubtract(const Rectangle& r, List_t& l2, List_t& other)
{
    for(Iterator_t i = other.begin(); i != other.end(); ++i) {
        Rectangle q(*i);
        q.intersect(r);
        if (!q.exists()) {
            l2.push_back(*i);
        } else {
            int qby = q.getTopY() + q.getHeight();
            int qbx = q.getLeftX() + q.getWidth();
            int iby = i->getTopY() + i->getHeight();
            int ibx = i->getLeftX() + i->getWidth();
            if(q.getTopY() > i->getTopY()) {
                l2.push_back(Rectangle(i->getLeftX(), i->getTopY(), i->getWidth(), q.getTopY() - i->getTopY()));
            }
            if(q.getLeftX() > i->getLeftX()) {
                l2.push_back(Rectangle(i->getLeftX(), q.getTopY(), q.getLeftX() - i->getLeftX(), q.getHeight()));
            }
            if(qbx < ibx) {
                l2.push_back(Rectangle(qbx, q.getTopY(), ibx - qbx, q.getHeight()));
            }
            if(qby < iby) {
                l2.push_back(Rectangle(i->getLeftX(), qby, i->getWidth(), iby - qby));
            }
        }
    }
}
