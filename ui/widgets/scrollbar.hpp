/**
  *  \file ui/widgets/scrollbar.hpp
  *  \brief Class ui::widgets::Scrollbar
  */
#ifndef C2NG_UI_WIDGETS_SCROLLBAR_HPP
#define C2NG_UI_WIDGETS_SCROLLBAR_HPP

#include "ui/scrollablewidget.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/base/signalconnection.hpp"
#include "ui/draw.hpp"

namespace ui { namespace widgets {

    class Scrollbar : public SimpleWidget {
     public:
        Scrollbar(ScrollableWidget& widget, Root& root);
        ~Scrollbar();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        ScrollableWidget& m_widget;
        Root& m_root;
        afl::base::Ref<gfx::Timer> m_timer;
        afl::base::SignalConnection conn_change;
        afl::base::SignalConnection conn_timer;

        enum LocalButtonFlag {
            Disabled,
            Active,
            Pressed
        };
        typedef afl::bits::SmallSet<LocalButtonFlag> LocalButtonFlags_t;

        LocalButtonFlags_t m_up;
        LocalButtonFlags_t m_down;

        void onChange();
        void onTimer();

        static States_t getStates(LocalButtonFlags_t f);
        static ButtonFlags_t getButtonFlags(LocalButtonFlags_t f);
    };

} }

#endif
