/**
  *  \file ui/scrollablewidget.hpp
  *  \brief Interface ui::ScrollableWidget
  */
#ifndef C2NG_UI_SCROLLABLEWIDGET_HPP
#define C2NG_UI_SCROLLABLEWIDGET_HPP

#include "ui/simplewidget.hpp"
#include "afl/base/signal.hpp"

namespace ui {

    /** Base class for a scrollable widget.

        This interface provides methods to observe and control scrollable widgets.

        A scrollable widget has a <b>total size</b>.
        We display one page of that content; described by the <b>page top</b> position and the <b>page size</b>.
        The page contains a cursor which highlights part of it.
        All positions are 0-based.

        For a list box, we would have
        - getPageTop() = index of topmost item
        - getPageSize() = number of list items displayed on the widget
        - getCursorTop() = index of item the cursor is on
        - getCursorSize() = 1 (=the cursor covers one element)
        - getTotalSize() = number of items

        FIXME: do we need getCursorTop(), getCursorSize() in the interface?
        FIXME: does this need to be a widget descendant or could this be a separate interface? */
    class ScrollableWidget : public SimpleWidget {
     public:
        /** Scroll operation. */
        enum Operation {
            LineUp,             ///< Scroll up one line (item, cursor height).
            LineDown,           ///< Scroll down one line (item, cursor height).
            PageUp,             ///< Scroll up one page.
            PageDown            ///< Scroll down one page.
        };

        /** Default constructor. */
        ScrollableWidget();

        /** Destructor. */
        ~ScrollableWidget();

        /** Get position of page top.
            \return position (index, pixel position, ...) */
        virtual int getPageTop() = 0;

        /** Get size of one page.
            \return size (number of items, pixels, ...) */
        virtual int getPageSize() = 0;

        /** Get position of cursor.
            \return position (index, pixel position, ...) */
        virtual int getCursorTop() = 0;

        /** Get size of cursor.
            \return size (number of items, pixels, ...) */
        virtual int getCursorSize() = 0;

        /** Get total size of content.
            \return size (number of items, pixels, ...) */
        virtual int getTotalSize() = 0;

        /** Set position of page top.
            If this actually results in a change, the widget needs to emit sig_change.
            \param top New top */
        virtual void setPageTop(int top) = 0;

        /** Scroll operation.
            If this actually results in a change, the widget needs to emit sig_change.
            \param op Operation */
        virtual void scroll(Operation op) = 0;

        /** Change signal.
            Emit when any of the values accessible with our "get" functions changed. */
        afl::base::Signal<void()> sig_change;
    };

}

#endif
