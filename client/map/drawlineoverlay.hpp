/**
  *  \file client/map/drawlineoverlay.hpp
  */
#ifndef C2NG_CLIENT_MAP_DRAWLINEOVERLAY_HPP
#define C2NG_CLIENT_MAP_DRAWLINEOVERLAY_HPP

#include "client/map/markeroverlaybase.hpp"
#include "game/map/locationcontroller.hpp"

namespace client { namespace map {

    /** Overlay for drawing a line or rectangle. */
    class DrawLineOverlay : public MarkerOverlayBase {
     public:
        DrawLineOverlay(ui::Root& root,
                        afl::string::Translator& tx,
                        game::map::LocationController& loc,
                        client::map::Screen& screen,
                        const game::map::Drawing& drawing);
        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

     private:
        void onPositionChange(game::map::Point pt);

        game::map::LocationController& m_location;
        afl::base::SignalConnection conn_positionChange;
    };


} }

#endif
