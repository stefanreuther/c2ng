/**
  *  \file client/map/circleoverlay.hpp
  *  \brief Class client::map::CircleOverlay
  */
#ifndef C2NG_CLIENT_MAP_CIRCLEOVERLAY_HPP
#define C2NG_CLIENT_MAP_CIRCLEOVERLAY_HPP

#include "client/map/overlay.hpp"
#include "game/map/point.hpp"
#include "ui/colorscheme.hpp"

namespace client { namespace map {

    /** Map overlay: circle.
        Displays a circle with a given center position, radius and color. */
    class CircleOverlay : public Overlay {
     public:
        /** Constructor.
            \param colorScheme Color scheme */
        explicit CircleOverlay(ui::ColorScheme& colorScheme);
        ~CircleOverlay();

        /** Set position.
            \param center Center position (game coordinates)
            \param radius Radius (game coordinates) */
        void setPosition(game::map::Point center, int radius);

        /** Set color.
            \param color (ui::Color_XXX) */
        void setColor(uint8_t color);

        // Overlay:
        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

     private:
        ui::ColorScheme& m_colorScheme;
        game::map::Point m_center;
        int m_radius;
        uint8_t m_color;
    };

} }

#endif
