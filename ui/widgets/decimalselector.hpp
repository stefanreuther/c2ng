/**
  *  \file ui/widgets/decimalselector.hpp
  *  \brief Class ui::widgets::DecimalSelector
  */
#ifndef C2NG_UI_WIDGETS_DECIMALSELECTOR_HPP
#define C2NG_UI_WIDGETS_DECIMALSELECTOR_HPP

#include "afl/base/deletable.hpp"
#include "afl/bits/smallset.hpp"
#include "ui/root.hpp"
#include "ui/widgets/numberselector.hpp"

namespace ui { namespace widgets {

    /** "Move-or-Type" number selector.

        This widget provides behaviour close to a normal input line (i.e., digits "echo" and backspace deletes),
        but also accepts '+' or '-' resp. cursor keys like all number selectors.

        Internal consistency model: this maintains a consistent value all the time.
        Special states (Mode) affect display only.
        For example, assume low=3, and value is 5.
        If the user deletes that using backspace, the internal stored value will still be 3
        but the widget displays an empty line.
        If the user enters a '9' now, the value will be 9, not 39.
        There still is the problem that the user cannot enter a value such as 10, because 1 would be below our minimum. */
    class DecimalSelector : public NumberSelector {
     public:
        enum Flag {
            RightJustified,     // ex pds_RightJust
            ShowMaximum         // ex pds_ShowMax
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;

        enum Mode {
            TypeErase,          // ex sTypeErase
            Zeroed,             // ex sZeroed
            Normal              // ex sNormal
        };

        /** Peer.
            The optional Peer can modify the DecimalSelector's behaviour. */
        class Peer : public afl::base::Deletable {
         public:
            virtual String_t toString(DecimalSelector& sel, int32_t value) = 0;
            virtual bool handleKey(DecimalSelector& sel, util::Key_t key, int prefix) = 0;
        };

        /** Constructor.
            \param root Root
            \param value Storage for the value
            \param min Lower limit
            \param max Upper limit
            \param step Default step size */
        DecimalSelector(Root& root, afl::base::Observable<int32_t>& value, int32_t min, int32_t max, int32_t step);

        /** Destructor. */
        ~DecimalSelector();

        /** Set flag.
            Flags affect appearance or behaviour.
            \param flag Flag to modify
            \param enable set or clear the flag */
        void setFlag(Flag flag, bool enable);

        /** Set mode.
            \param m New mode */
        void setMode(Mode m);

        /** Set peer.
            The optional Peer can modify the DecimalSelector's behaviour.
            DecimalSelector starts with no peer.
            \param peer Peer */
        void setPeer(Peer& peer);

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        Root& m_root;
        Mode m_mode;            // ex state
        Flags_t m_flags;
        Peer* m_pPeer;

        void onChange();
    };

} }

#endif
