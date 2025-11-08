/**
  *  \file client/widgets/busyindicator.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_BUSYINDICATOR_HPP
#define C2NG_CLIENT_WIDGETS_BUSYINDICATOR_HPP

#include <vector>
#include "afl/base/signal.hpp"
#include "afl/string/string.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace client { namespace widgets {

    // FIXME: make this look nicer
    // FIXME: give this some sort of debouncing (pop up after ~500ms only, but block UI all the time)
    // FIXME: give this a start/stop method
    class BusyIndicator : public ui::SimpleWidget {
     public:
        BusyIndicator(ui::Root& root, String_t text);

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        void replayEvents();

        afl::base::Signal<void()> sig_interrupt;
        afl::base::Signal<void()> sig_quit;

     private:
        ui::Root& m_root;
        String_t m_text;

        std::vector<util::Key_t> m_keys;
        bool m_quit;
    };

} }

#endif
