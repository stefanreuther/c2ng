/**
  *  \file client/map/ionstormforecastoverlay.cpp
  */

#include "client/map/ionstormforecastoverlay.hpp"
#include "client/map/renderer.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"

namespace {
    uint8_t getForecastColor(int voltage, int uncertainity)
    {
        // ex getPredictionColor, chartdlg.pas::SetStormColor; only _col256 branch
        uint8_t baseColor;
        if (voltage < 50) {
            baseColor = ui::Color_Shield;
        } else if (voltage < 100) {
            baseColor = ui::Color_Grayscale;
        } else if (voltage < 150) {
            baseColor = ui::Color_Grayscale+6;
        } else {
            baseColor = ui::Color_Fire;
        }

        return static_cast<uint8_t>(baseColor + 7 - std::min(uncertainity, 5));
    }
}


client::map::IonStormForecastOverlay::IonStormForecastOverlay(ui::ColorScheme& colorScheme)
    : m_forecasts(),
      m_voltage(),
      m_colorScheme(colorScheme)
{ }

void
client::map::IonStormForecastOverlay::setForecast(int voltage, const game::map::IonStorm::Forecast_t& pred)
{
    m_voltage = voltage;
    m_forecasts = pred;
    requestRedraw();
}

void
client::map::IonStormForecastOverlay::drawBefore(gfx::Canvas& can, const Renderer& ren)
{
    // ex WIonForecastChart::drawPre
    const uint8_t IONSTORM_FILL[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    gfx::Context<uint8_t> ctx(can, m_colorScheme);
    ctx.setFillPattern(IONSTORM_FILL);
    for (size_t i = 0, n = m_forecasts.size(); i < n; ++i) {
        ctx.setColor(getForecastColor(m_voltage, m_forecasts[i].uncertainity));
        drawFilledCircle(ctx, ren.scale(m_forecasts[i].center), ren.scale(m_forecasts[i].radius));
    }
}

void
client::map::IonStormForecastOverlay::drawAfter(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

bool
client::map::IonStormForecastOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::IonStormForecastOverlay::handleKey(util::Key_t /*key*/, int /*prefix*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::IonStormForecastOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}
