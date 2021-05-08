/**
  *  \file client/map/drawcircleoverlay.cpp
  *  \brief Class client::map::DrawCircleOverlay
  */

#include <cmath>
#include "client/map/drawcircleoverlay.hpp"
#include "afl/string/format.hpp"
#include "client/map/screen.hpp"
#include "gfx/context.hpp"
#include "util/math.hpp"

client::map::DrawCircleOverlay::DrawCircleOverlay(ui::Root& root,
                                                  afl::string::Translator& tx,
                                                  client::map::Location& loc,
                                                  client::map::Screen& screen,
                                                  const game::map::Drawing& drawing)
    : MarkerOverlayBase(root, tx, screen, drawing),
      m_location(loc),
      conn_positionChange(loc.sig_positionChange.add(this, &DrawCircleOverlay::onPositionChange))
{ }

void
client::map::DrawCircleOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
}

void
client::map::DrawCircleOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WDrawCircleChartMode::drawOverlays
    afl::string::Translator& tx = translator();
    afl::base::Ref<gfx::Font> font = root().provider().getFont(gfx::FontRequest());
    const int lineHeight = font->getLineHeight();
    gfx::Point pos = ren.getExtent().getTopLeft();

    gfx::Context<uint8_t> ctx(can, root().colorScheme());
    ctx.useFont(*font);
    ctx.setColor(ui::Color_White);

    outText(ctx, pos, tx("Set circle size with [+]/[-], end with [ESC]."));
    pos.addY(lineHeight);
    outText(ctx, pos, afl::string::Format(tx("Radius: %d ly"), drawing().getCircleRadius()));
}

bool
client::map::DrawCircleOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::DrawCircleOverlay::handleKey(util::Key_t key, int prefix, const Renderer& ren)
{
    // ex WDrawCircleChartMode::handleEvent
    switch (key) {
     case '+':
        screen().drawingProxy().changeCircleRadius(prefix != 0 ? prefix : 10);
        return true;

     case '-':
        screen().drawingProxy().changeCircleRadius(prefix != 0 ? -prefix : -10);
        return true;

     case util::KeyMod_Shift + '+':
        screen().drawingProxy().changeCircleRadius(1);
        return true;

     case util::KeyMod_Shift + '-':
        screen().drawingProxy().changeCircleRadius(-1);
        return true;

     case '=':
        if (prefix != 0) {
            screen().drawingProxy().setCircleRadius(prefix);
        }
        return true;

     case 'p':
        screen().removeOverlay(this);
        return true;

     case 'y':
        screen().drawingProxy().setCircleRadius(350);
        return true;

     case util::Key_Backspace:
        m_location.setPosition(drawing().getPos());
        return true;

     default:
        return defaultHandleKey(key, prefix, ren);
    }
}

bool
client::map::DrawCircleOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}

void
client::map::DrawCircleOverlay::onPositionChange(game::map::Point pt)
{
    if ((root().engine().getKeyboardModifierState() & util::KeyMod_Alt) != 0) {
       screen().drawingProxy().setCircleRadius(util::roundToInt(std::sqrt(double(pt.getSquaredRawDistance(drawing().getPos())))));
    }
}
