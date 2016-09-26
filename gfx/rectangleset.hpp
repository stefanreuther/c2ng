/**
  *  \file gfx/rectangleset.hpp
  */
#ifndef C2NG_GFX_RECTANGLESET_HPP
#define C2NG_GFX_RECTANGLESET_HPP

#include <list>
#include "gfx/rectangle.hpp"

namespace gfx {

    /** Rectangle Set.
        Objects of this class contain an arbitrary set of rectangles.
        The class provides the usual set operations.
        This class is, among others, used in redraw management.

        \invariant this class contains a list of rectangle which are all disjoint.
        The disjointness is managed by all manipulator methods. */
    class RectangleSet {
     public:
        typedef std::list<Rectangle> List_t;
        typedef List_t::const_iterator Iterator_t;

        /** Construct empty set.
            \post contains(x,y) == false for any x,y */
        RectangleSet();

        /** Construct rectangular set.
            The set will contain the one rectangle r. */
        RectangleSet(const Rectangle& r);

        /** Intersect this rectangle set with r.
            \post r.contains(x,y)==false => contains(x,y)==false */
        void intersect(const Rectangle& r);

        /** Add rectangle to set.
            This does not necessarily add a copy of r to the set;
            if this set already contains parts of r, only the remaining parts are added.
            \param r rectangle
            \post r.contains(x,y) => contains(x,y)
            \post contains(r) */
        void add(const Rectangle& r);

        void add(const RectangleSet& r);

        /** Remove rectangle from set.
            \param r rectangle
            \post r.contains(x,y) => !contains(x,y) */
        void remove(const Rectangle& r);
        void clear()
            { m_list.clear(); }

        // FIXME: retire
        //    bool contains(int x, int y) const;

        /** Check if point is in set.
            \param pt point
            \returns true iff point is in this set. */
        bool contains(const Point pt) const;

        /** Check if rectangle in set.
            \param r rectangle
            \return true iff r is completely contained in *this. */
        bool contains(const Rectangle& r) const;

        /** Compute bounding rectangle.
            \returns the smallest possible rectangle which entirely contains this rectangle set */
        Rectangle getBoundingRectangle() const;

        /** Iterator interface: get iterator to first element. */
        Iterator_t begin() const
            { return m_list.begin(); }
        /** Iterator interface: get iterator to last element. */
        Iterator_t end() const
            { return m_list.end(); }
        /** Check whether this set is empty. */
        bool empty() const
            { return m_list.empty(); }

        // FIXME: retire
        // /** Get number of rectangles in this rectangle set. */
        // size_t getNumRectangles() const
        //     { return m_list.size(); }

     private:
        List_t m_list;

        static void doSubtract(const Rectangle& r, List_t& l2, List_t& other);
    };

}

#endif
