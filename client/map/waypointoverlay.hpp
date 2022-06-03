/**
  *  \file client/map/waypointoverlay.hpp
  *  \brief Class client::map::WaypointOverlay
  */
#ifndef C2NG_CLIENT_MAP_WAYPOINTOVERLAY_HPP
#define C2NG_CLIENT_MAP_WAYPOINTOVERLAY_HPP

#include "client/map/overlay.hpp"
#include "game/map/shipinfo.hpp"
#include "game/proxy/objectobserver.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace map {

    class WaypointOverlay : public Overlay {
     public:
        WaypointOverlay(ui::Root& root, bool isFleet);
        ~WaypointOverlay();

        void setData(const game::map::ShipMovementInfos_t& infos);

        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

        void attach(game::proxy::ObjectObserver& oop);

     private:
        ui::Root& m_root;
        util::RequestReceiver<WaypointOverlay> m_reply;
        game::map::ShipMovementInfos_t m_infos;
        bool m_isFleet;
    };

} }

#endif
