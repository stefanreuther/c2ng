/**
  *  \file ui/reshack/fontcolorselector.cpp
  *  \brief Class ui::reshack::FontColorSelector
  */

#include "ui/reshack/fontcolorselector.hpp"

#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/reshack/painter.hpp"
#include "ui/reshack/palette.hpp"
#include "ui/reshack/session.hpp"

namespace {
    const int NUM_FONT_COLORS = 3;
    const uint8_t FONT_COLORS[NUM_FONT_COLORS] = {
        ui::reshack::Palette::FC_Black,
        ui::reshack::Palette::FC_Half,
        ui::reshack::Palette::FC_White
    };
}

ui::reshack::FontColorSelector::FontColorSelector(Session& session, Painter& painter)
    : m_session(session),
      m_painter(painter),
      conn_colorChange(painter.sig_colorChange.add(this, (void (Widget::*)()) &Widget::requestRedraw))
{ }

ui::reshack::FontColorSelector::~FontColorSelector()
{ }

void
ui::reshack::FontColorSelector::draw(gfx::Canvas& can)
{
    // RHFontColorSelector::drawContent(GfxCanvas& can)

    // >[nnnn]<
    // |  |   `- background color indicator
    // |  `- color block
    // `- foreground color indicator

    gfx::Context<uint8_t> ctx(can, m_session.root().colorScheme());
    ctx.useFont(*m_session.root().provider().getFont("f"));

    int lineHeight = ctx.getFont()->getLineHeight();
    int emWidth = ctx.getFont()->getEmWidth();
    const gfx::Rectangle& extent = getExtent();

    drawBackground(ctx, getExtent());
    ctx.setColor(Color_White);
    for (int i = 0; i < NUM_FONT_COLORS; ++i) {
        uint8_t color = static_cast<uint8_t>(FONT_COLORS[i]);
        int y = extent.getTopY() + lineHeight*i;
        if (color == m_painter.getColor(false)) {
            outText(ctx, gfx::Point(extent.getLeftX(), y), ">");
        }
        if (color == m_painter.getColor(true)) {
            outText(ctx, gfx::Point(extent.getRightX() - emWidth, y), "<");
        }

        drawSolidBar(ctx, gfx::Rectangle(extent.getLeftX() + emWidth, y, extent.getWidth() - 2*emWidth, lineHeight), color);
    }
}

void
ui::reshack::FontColorSelector::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::reshack::FontColorSelector::handlePositionChange()
{ }

ui::layout::Info
ui::reshack::FontColorSelector::getLayoutInfo() const
{
    // RHFontColorSelector::getLayoutInfo(LayoutInfo& info)
    return ui::layout::Info(m_session.root().provider().getFont("f")->getCellSize().scaledBy(5, NUM_FONT_COLORS));
}

bool
ui::reshack::FontColorSelector::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
ui::reshack::FontColorSelector::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // RHFontColorSelector::handleEvent(const UIEvent& e, bool second_pass)
    if (getExtent().contains(pt)) {
        // Mouse is inside this widget, so consume this event
        afl::base::Ref<gfx::Font> font = m_session.root().provider().getFont("f");
        int lineHeight = std::max(1, font->getLineHeight());

        int y = (pt.getY() - getExtent().getTopY()) / lineHeight;
        if (pressedButtons.contains(LeftButton)) {
            // Left mouse button pressed
            requestActive();
            if (y >= 0 && y < NUM_FONT_COLORS) {
                m_painter.setColor(false, FONT_COLORS[y]);
            }
            return true;
        }
        if (pressedButtons.contains(RightButton)) {
            // Right mouse button pressed
            requestActive();
            if (y >= 0 && y < NUM_FONT_COLORS) {
                m_painter.setColor(true, FONT_COLORS[y]);
            }
            return true;
        }
    }
    return false;
}
