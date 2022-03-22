/**
  *  \file client/map/shiptaskoverlay.hpp
  *  \brief Class client::map::ShipTaskOverlay
  */
#ifndef C2NG_CLIENT_MAP_SHIPTASKOVERLAY_HPP
#define C2NG_CLIENT_MAP_SHIPTASKOVERLAY_HPP

#include "client/map/overlay.hpp"
#include "game/proxy/taskeditorproxy.hpp"
#include "ui/root.hpp"

namespace client { namespace map {

    /** Ship task overlay: displays a ship's auto-task movement on the map.

        To use, connect setStatus to a TaskEditorProxy's sig_shipChange. */
    class ShipTaskOverlay : public Overlay {
     public:
        /** Constructor.
            @param root UI root (for color scheme, resource provider) */
        explicit ShipTaskOverlay(ui::Root& root);
        ~ShipTaskOverlay();

        /** Set content to display.
            Connect this to TaskEditorProxy::sig_shipChange.
            @param status New status */
        void setStatus(const game::proxy::TaskEditorProxy::ShipStatus& status);

        // Overlay:
        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

     private:
        ui::Root& m_root;
        game::proxy::TaskEditorProxy::ShipStatus m_status;
    };

} }

#endif
