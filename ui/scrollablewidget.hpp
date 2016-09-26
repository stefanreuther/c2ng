/**
  *  \file ui/scrollablewidget.hpp
  */
#ifndef C2NG_UI_SCROLLABLEWIDGET_HPP
#define C2NG_UI_SCROLLABLEWIDGET_HPP

#include "ui/simplewidget.hpp"
#include "afl/base/signal.hpp"

namespace ui {

    /*
     *  Position model:
     *           ___________________________________________________
     *   |     |                                                    \
     *   |     | _______________________________                     \
     *   |:::::|                                \                     \  total
     *   |:::::|                                 \ visible part        > 0,+totalSize
     *   |XXXXX| > Cursor cursorTop,+cursorSize  / pageTop,+pageSize  /
     *   |:::::| _______________________________/                    /
     *   |     | ___________________________________________________/
     */
    class ScrollableWidget : public SimpleWidget {
     public:
        ScrollableWidget();
        virtual int getPageTop() = 0;
        virtual int getPageSize() = 0;
        virtual int getCursorTop() = 0;
        virtual int getCursorSize() = 0;
        virtual int getTotalSize() = 0;

        afl::base::Signal<void()> sig_change;
    };

}

#endif
