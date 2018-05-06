/**
  *  \file ui/widgets/textbutton.cpp
  */

#include "ui/widgets/textbutton.hpp"
#include "util/updater.hpp"
#include "ui/colorscheme.hpp"

using util::Updater;

ui::widgets::TextButton::TextButton(const String_t& text, util::Key_t key, Root& root)
    : AbstractButton(root, key),
      m_text(text),
      m_color(Color_Gray),
      m_hoverColor(Color_White),
      m_font(),
      m_alignX(0),
      m_alignY(0)
{ }

ui::widgets::TextButton::~TextButton()
{ }

void
ui::widgets::TextButton::setText(const String_t& text)
{
    if (Updater().set(m_text, text)) {
        requestRedraw();
    }
}

void
ui::widgets::TextButton::setColor(uint8_t color)
{
    if (Updater().set(m_color, color)) {
        requestRedraw();
    }
}

void
ui::widgets::TextButton::setHoverColor(uint8_t color)
{
    if (Updater().set(m_hoverColor, color)) {
        requestRedraw();
    }
}

void
ui::widgets::TextButton::setFont(gfx::FontRequest font)
{
    if (Updater().set(m_font, font)) {
        requestRedraw();
    }
}

void
ui::widgets::TextButton::setTextAlign(int x, int y)
{
    if (Updater().set(m_alignX, x).set(m_alignY, y)) {
        requestRedraw();
    }
}

void
ui::widgets::TextButton::draw(gfx::Canvas& can)
{
    // ex WShipCargoButton::drawContent [sort-of]
    gfx::Context<uint8_t> ctx(can, root().colorScheme());
    ctx.useFont(*root().provider().getFont(m_font));
    ctx.setColor(getFlags().contains(ActiveButton) ? m_hoverColor : m_color);
    ctx.setTextAlign(m_alignX, m_alignY);
    outTextF(ctx, getExtent(), m_text);
}

void
ui::widgets::TextButton::handleStateChange(State st, bool enable)
{
    defaultHandleStateChange(st, enable);
}

void
ui::widgets::TextButton::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
ui::widgets::TextButton::getLayoutInfo() const
{
    afl::base::Ref<gfx::Font> font(root().provider().getFont(m_font));
    return gfx::Point(font->getTextWidth(m_text),
                      font->getTextHeight(m_text));
}

bool
ui::widgets::TextButton::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::widgets::TextButton::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}
