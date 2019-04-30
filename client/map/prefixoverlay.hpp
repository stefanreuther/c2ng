/**
  *  \file client/map/prefixoverlay.hpp
  */
#ifndef C2NG_CLIENT_MAP_PREFIXOVERLAY_HPP
#define C2NG_CLIENT_MAP_PREFIXOVERLAY_HPP

#include "client/map/overlay.hpp"
#include "gfx/timer.hpp"
#include "util/prefixargument.hpp"

namespace client { namespace map {

    class Screen;

    class PrefixOverlay : public Overlay {
     public:
        PrefixOverlay(Screen& screen, int initialValue);
        ~PrefixOverlay();

        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);

        // EventConsumer:
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

     private:
        Screen& m_screen;
        util::PrefixArgument m_value;
        afl::base::Ref<gfx::Timer> m_timer;
        bool m_blink;

        void onTimer();

        static void finish(Screen& screen, util::Key_t key, int prefix);
    };

} }

#endif
