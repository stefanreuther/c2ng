/**
  *  \file game/map/drawingcontainer.hpp
  */
#ifndef C2NG_GAME_MAP_DRAWINGCONTAINER_HPP
#define C2NG_GAME_MAP_DRAWINGCONTAINER_HPP

#include "afl/container/ptrmultilist.hpp"
#include "game/map/drawing.hpp"
#include "afl/base/signal.hpp"

namespace game { namespace map {

    class Configuration;

    class DrawingContainer {
     public:
        typedef afl::container::PtrMultiList<Drawing> List_t;
        typedef List_t::iterator Iterator_t;

        // /// Flags for computeAtomMap().
        // enum {
        //     IncludeNumeric   = 1, ///< Include numeric tags (default: only atoms). For filtering.
        //     IncludeInvisible = 2  ///< Include invisible markers (default: only visible). For saving.
        // };

        DrawingContainer();
        ~DrawingContainer();

        Iterator_t addNew(Drawing* drawing);

        // void computeAtomMap(GDrawingAtomMap& map, int flags) const;
        // void save(Stream& out, const GDrawingAtomMap& map) const;
        // void load(Stream& in, const GDrawingAtomMap& map, int expire);

        Iterator_t findNearestVisibleDrawing(Point pt, const Configuration& config);

        Iterator_t begin() const
            { return m_drawings.begin(); }
        Iterator_t end() const
            { return m_drawings.end(); }

        void erase(Iterator_t it);
        void eraseExpiredDrawings(int turn);

        afl::base::Signal<void()> sig_change;
        // Signal1<iterator> sig_deleted;    // <- FIXME: need this? Not used in PCC2.

     private:
        List_t m_drawings;
    };

} }

#endif
