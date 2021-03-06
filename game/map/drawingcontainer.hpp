/**
  *  \file game/map/drawingcontainer.hpp
  *  \brief Class game::map::DrawingContainer
  */
#ifndef C2NG_GAME_MAP_DRAWINGCONTAINER_HPP
#define C2NG_GAME_MAP_DRAWINGCONTAINER_HPP

#include "afl/container/ptrmultilist.hpp"
#include "game/map/drawing.hpp"
#include "afl/base/signal.hpp"

namespace game { namespace map {

    class Configuration;

    /** Container for Drawing objects.
        This owns a list of Drawing objects. */
    class DrawingContainer {
     public:
        /** Underlying container type. */
        typedef afl::container::PtrMultiList<Drawing> List_t;

        /** Iterator.
            Iterators given out by DrawingContainer remain valid as long as the DrawingContainer is alive.
            Deletion of a drawing may cause the pointed-to object to become null. */
        typedef List_t::iterator Iterator_t;

        /** Create blank container. */
        DrawingContainer();

        /** Destructor. */
        ~DrawingContainer();

        /** Add new drawing.
            \param drawing Drawing. DrawingContainer assumes ownership
            \return iterator pointing to drawing */
        Iterator_t addNew(Drawing* drawing);

        /** Find nearest visible drawing.
            \param pt          Origin point
            \param config      Map configuration
            \param maxDistance Maximum distance to consider
            \return Iterator to closest drawing that is visible (Drawing::isVisible()) and closer than maxDistance.
                    end() if there is no applicable drawing. */
        Iterator_t findNearestVisibleDrawing(Point pt, const Configuration& config, double maxDistance) const;

        /** Find marker at a given position.
            \param pt     Position
            \param config Map configuration
            \return Iterator to first visible marker at that position, end() if none found */
        Iterator_t findMarkerAt(Point pt, const Configuration& config) const;

        /** Get iterator to first drawing.
            \return iterator */
        Iterator_t begin() const
            { return m_drawings.begin(); }

        /** Get iterator to after last drawing.
            \return iterator */
        Iterator_t end() const
            { return m_drawings.end(); }

        /** Erase drawing.
            Iterator remains valid and can be used for further iteration;
            dereferencing this iterator will return null.
            \param it Iterator */
        void erase(Iterator_t it);

        /** Erase all expired drawings.
            \param turnNumber Current turn number */
        void eraseExpiredDrawings(int turnNumber);

        /** Erase a set of lines starting at a given position.
            Looks for a continuous set of lines, directly adjacent to each other,
            starting at the given position, and erases them.
            \param pos Position
            \param config Map configuration (required for coordinate mapping) */
        void eraseAdjacentLines(Point pos, const Configuration& config);

        /** Set color for all lines adjacent to a given position.
            Looks for a continuous set of lines of a color other then \c color, directly adjacent to each other,
            starting at the given position, and changes their color to \c color.
            \param pos Position
            \param color New color
            \param config Map configuration (required for coordinate mapping)
            \see Drawing::setColor */
        void setAdjacentLinesColor(Point pos, uint8_t color, const Configuration& config);

        /** Set tag for all lines adjacent to a given position.
            Looks for a continuous set of lines of a tag other then \c tag, directly adjacent to each other,
            starting at the given position, and changes their tag to \c tag.
            \param pos Position
            \param tag New tag
            \param config Map configuration (required for coordinate mapping)
            \see Drawing::setTag */
        void setAdjacentLinesTag(Point pos, util::Atom_t tag, const Configuration& config);

        /** Signal: change.
            Raised whenever a new drawing is added or one is deleted. */
        afl::base::Signal<void()> sig_change;

     private:
        List_t m_drawings;

        /* Common logic for "process adjacent" */
        class Worker;
        void processAdjacent(Point pos, Worker& worker, const Configuration& config);
    };

} }

#endif
