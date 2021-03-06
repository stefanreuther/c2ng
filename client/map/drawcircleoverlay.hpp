/**
  *  \file client/map/drawcircleoverlay.hpp
  *  \brief Class client::map::DrawCircleOverlay
  */
#ifndef C2NG_CLIENT_MAP_DRAWCIRCLEOVERLAY_HPP
#define C2NG_CLIENT_MAP_DRAWCIRCLEOVERLAY_HPP

#include "client/map/markeroverlaybase.hpp"

namespace client { namespace map {

    class Location;

    /** Overlay for drawing a circle. */
    class DrawCircleOverlay : public MarkerOverlayBase {
     public:
        DrawCircleOverlay(ui::Root& root,
                          afl::string::Translator& tx,
                          client::map::Location& loc,
                          client::map::Screen& screen,
                          const game::map::Drawing& drawing);
        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

     private:
        void onPositionChange(game::map::Point pt);

        client::map::Location& m_location;
        afl::base::SignalConnection conn_positionChange;
    };

} }

#endif
