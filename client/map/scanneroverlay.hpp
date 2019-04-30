/**
  *  \file client/map/scanneroverlay.hpp
  */
#ifndef C2NG_CLIENT_MAP_SCANNEROVERLAY_HPP
#define C2NG_CLIENT_MAP_SCANNEROVERLAY_HPP

#include "client/map/overlay.hpp"
#include "game/map/point.hpp"
#include "ui/colorscheme.hpp"

namespace client { namespace map {

    class ScannerOverlay : public Overlay {
     public:
        ScannerOverlay(ui::ColorScheme& cs);
        ~ScannerOverlay();

        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);

        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

        void setPositions(game::map::Point origin, game::map::Point target);
        void clearPositions();

     private:
        ui::ColorScheme& m_colorScheme;
        bool m_valid;
        game::map::Point m_origin;
        game::map::Point m_target;
    };

} }

#endif
