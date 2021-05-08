/**
  *  \file client/map/ionstormforecastoverlay.hpp
  */
#ifndef C2NG_CLIENT_MAP_IONSTORMFORECASTOVERLAY_HPP
#define C2NG_CLIENT_MAP_IONSTORMFORECASTOVERLAY_HPP

#include "client/map/overlay.hpp"
#include "game/map/ionstorm.hpp"
#include "ui/colorscheme.hpp"

namespace client { namespace map {

    class IonStormForecastOverlay : public Overlay {
     public:
        explicit IonStormForecastOverlay(ui::ColorScheme& colorScheme);

        void setForecast(int voltage, const game::map::IonStorm::Forecast_t& pred);

        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

     private:
        game::map::IonStorm::Forecast_t m_forecasts;
        int m_voltage;
        ui::ColorScheme& m_colorScheme;
    };

} }

#endif
