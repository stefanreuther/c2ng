/**
  *  \file ui/widgets/basedecimalselector.hpp
  *  \brief Class ui::widgets::BaseDecimalSelector
  */
#ifndef C2NG_UI_WIDGETS_BASEDECIMALSELECTOR_HPP
#define C2NG_UI_WIDGETS_BASEDECIMALSELECTOR_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "ui/widgets/numberselector.hpp"
#include "util/key.hpp"

namespace ui { namespace widgets {

    /** "Move-or-Type" number selector.

        This widget provides behaviour close to a normal input line (i.e., digits "echo" and backspace deletes),
        but also accepts '+' or '-' resp. cursor keys like all number selectors.

        This is the base class that processes input, but does not have an appearance.

        Internal consistency model: this maintains a consistent value all the time.
        Special states (Mode) affect display only.
        For example, assume low=3, and value is 5.
        If the user deletes that using backspace, the internal stored value will still be 3
        but the widget displays an empty line.
        If the user enters a '9' now, the value will be 9, not 39.
        There still is the problem that the user cannot enter a value such as 10, because 1 would be below our minimum. */
    class BaseDecimalSelector : public NumberSelector {
     public:
        enum Mode {
            TypeErase,          // ex sTypeErase
            Zeroed,             // ex sZeroed
            Normal              // ex sNormal
        };

        /** Peer.
            The optional Peer can modify the DecimalSelector's behaviour. */
        class Peer : public afl::base::Deletable {
         public:
            virtual String_t toString(const BaseDecimalSelector& sel, int32_t value) = 0;
            virtual bool handleKey(const BaseDecimalSelector& sel, util::Key_t key, int prefix) = 0;
        };

        /** Constructor.
            \param value Storage for the value
            \param min Lower limit
            \param max Upper limit
            \param step Default step size */
        BaseDecimalSelector(afl::base::Observable<int32_t>& value, int32_t min, int32_t max, int32_t step);

        /** Destructor. */
        ~BaseDecimalSelector();

        /** Set mode.
            The mode is the user-visible state of the widget.
            \param m New mode */
        void setMode(Mode m);

        /** Get mode.
            \return mode */
        Mode getMode() const;

        /** Set peer.
            The optional Peer can modify the DecimalSelector's behaviour.
            DecimalSelector starts with no peer.
            \param peer Peer */
        void setPeer(Peer& peer);

        /** Get peer.
            \return peer, if any */
        Peer* getPeer() const;

        // SimpleWidget:
        // virtual void draw(gfx::Canvas& can); -> child
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        // virtual ui::layout::Info getLayoutInfo() const; -> child
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        Mode m_mode;            // ex state
        Peer* m_pPeer;

        void onChange();
    };

} }

#endif
