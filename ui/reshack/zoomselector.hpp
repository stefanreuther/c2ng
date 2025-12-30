/**
  *  \file ui/reshack/zoomselector.hpp
  *  \brief Class ui::reshack::ZoomSelector
  */
#ifndef C2NG_UI_RESHACK_ZOOMSELECTOR_HPP
#define C2NG_UI_RESHACK_ZOOMSELECTOR_HPP

#include "ui/reshack/modeselector.hpp"

namespace ui { namespace reshack {

    class Painter;

    /** Zoom selector widget.
        @see Painter::setZoom */
    class ZoomSelector : public ModeSelector {
     public:
        /** Constructor.
            @param root    UI root
            @param p       Painter whose zoom to control
            @param nslots  Number of slots (max zoom is 1<<nslots) */
        ZoomSelector(Root& root, Painter& p, size_t nslots);
        ~ZoomSelector();

     protected:
        virtual bool isActive(size_t slot) const;
        virtual bool isUsable(size_t slot) const;
        virtual void activate(size_t slot);
        virtual String_t getName(size_t slot) const;
        virtual size_t getNumSlots() const;
        virtual size_t getSlotFromKey(util::Key_t k) const;

     private:
        Painter& m_painter;
        const size_t m_numSlots;
    };

} }

#endif
