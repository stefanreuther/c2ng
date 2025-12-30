/**
  *  \file ui/reshack/modeselector.hpp
  *  \brief Base class ui::reshack::ModeSelector
  */
#ifndef C2NG_UI_RESHACK_MODESELECTOR_HPP
#define C2NG_UI_RESHACK_MODESELECTOR_HPP

#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace ui { namespace reshack {

    /** Mode selector widget.
        Implements widget logic to display a list of modes,
        each of which can be usable and/or active. */
    class ModeSelector : public SimpleWidget {
     public:
        explicit ModeSelector(Root& root);
        ~ModeSelector();

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     protected:
        /** Check whether mode is active.
            @param slot Slot number [0,getNumSlots())
            @return true if mode is active */
        virtual bool isActive(size_t slot) const = 0;

        /** Check whether mode is usable.
            @param slot Slot number [0,getNumSlots())
            @return true if mode is usable (=can be activated) */
        virtual bool isUsable(size_t slot) const = 0;

        /** Activate a mode.
            Called as reaction to a mouse or key event.
            @param slot Slot number [0,getNumSlots()) */
        virtual void activate(size_t slot) = 0;

        /** Get name of a slot.
            @param slot Slot number [0,getNumSlots())
            @return name */
        virtual String_t getName(size_t slot) const = 0;

        /** Get number of slots.
            @return number */
        virtual size_t getNumSlots() const = 0;

        /** Get slot number, given a key.
            @param k Key to check
            @return Slot number; nil if key does not map to a slot. */
        virtual size_t getSlotFromKey(util::Key_t k) const = 0;

        static const size_t nil = -1;

     private:
        Root& m_root;
    };


} }

#endif
