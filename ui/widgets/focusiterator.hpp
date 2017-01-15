/**
  *  \file ui/widgets/focusiterator.hpp
  *  \brief Class ui::widgets::FocusIterator
  */
#ifndef C2NG_UI_WIDGETS_FOCUSITERATOR_HPP
#define C2NG_UI_WIDGETS_FOCUSITERATOR_HPP

#include <vector>
#include "ui/invisiblewidget.hpp"

namespace ui { namespace widgets {

    /** Cursor Focus Control.
        This invisible widget provides a means of manipulating focus using the keyboard.
        Given a list of widgets, it provides keys to move focus between these.
        Multiple flags can be specified to accept multiple keys.
        - Horizontal: left and right arrows to move a step
        - Vertical: up and down arrows to move a step
        - Tab: Tab and Shift-Tab keys to move a step
        - Home: Home and End to move to the ends
        - Page: PgUp and PgDn to move to the ends
        - Wrap: makes Horizontal and Vertical wrap around at the ends. By default, only Tab wraps.

        \change Instead of using widget Id ranges, c2ng uses an explicit list of widgets in the FocusIterator. */
    class FocusIterator : public InvisibleWidget {
     public:
        static const int Horizontal = 1;   ///< Accept horizontal movement keys. ex fi_Horiz.
        static const int Vertical = 2;     ///< Accept vertical movement keys. ex fi_Vert.
        static const int Tab = 4;          ///< Accept Tab key. ex fi_Tab.
        static const int Page = 8;         ///< Accept page up/down keys. ex fi_Page.
        static const int Home = 16;        ///< Accept home/end keys. ex fi_Home.
        static const int Wrap = 32;        ///< Accept wraparound for horizontal/vertical movement. ex fi_Wrap.

        /** Constructor.
            \param flags Combination of Horizontal/Vertical/Tab/Page/Home/Wrap to select behaviour. */
        explicit FocusIterator(int flags);

        /** Destructor. */
        ~FocusIterator();

        /** Add widget to FocusIterator.
            \param w Widget. Must live longer than the FocusIterator. */
        void add(Widget& w);

        // InvisibleWidget/Widget:
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        int m_flags;
        std::vector<Widget*> m_widgets;
    };

} }

#endif
