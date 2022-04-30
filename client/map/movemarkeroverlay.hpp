/**
  *  \file client/map/movemarkeroverlay.hpp
  */
#ifndef C2NG_CLIENT_MAP_MOVEMARKEROVERLAY_HPP
#define C2NG_CLIENT_MAP_MOVEMARKEROVERLAY_HPP

#include "client/map/markeroverlaybase.hpp"
#include "ui/root.hpp"
#include "afl/string/translator.hpp"

namespace client { namespace map {

    class Location;
    class Screen;

    class MoveMarkerOverlay : public MarkerOverlayBase {
     public:
        MoveMarkerOverlay(ui::Root& root,
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
        void editComment();

        client::map::Location& m_location;

        afl::base::SignalConnection conn_positionChange;
    };

    void editMarkerComment(ui::Root& root,
                           const game::map::Drawing& marker,
                           game::proxy::DrawingProxy& proxy,
                           afl::string::Translator& tx);

} }

#endif
