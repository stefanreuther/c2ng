/**
  *  \file client/map/messageoverlay.hpp
  *  \brief Class client::map::MessageOverlay
  */
#ifndef C2NG_CLIENT_MAP_MESSAGEOVERLAY_HPP
#define C2NG_CLIENT_MAP_MESSAGEOVERLAY_HPP

#include "client/map/overlay.hpp"
#include "gfx/timer.hpp"

namespace client { namespace map {

    class Screen;

    /** Message overlay.
        Implements the same look as client::widgets::showDecayingMessage(), but for starchart.
        Using client::widgets::showDecayingMessage() would work, but this version is a little more fluent
        because it doesn't get the map out of infinite-movement mode. */
    class MessageOverlay : public Overlay {
     public:
        /** Constructor.
            \param parent     Screen
            \param message    Message to show */
        MessageOverlay(Screen& parent, String_t message);
        ~MessageOverlay();

        // Overlay:
        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

     private:
        void startTimer();
        void onTimer();

        Screen& m_parent;
        String_t m_message;
        afl::base::Ref<gfx::Timer> m_timer;
        int m_state;
    };

} }

#endif
