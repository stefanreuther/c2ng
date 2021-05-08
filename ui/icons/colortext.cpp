/**
  *  \file ui/icons/colortext.cpp
  */

#include "ui/icons/colortext.hpp"
#include "util/updater.hpp"

using util::Updater;

ui::icons::ColorText::ColorText(const String_t& text, Root& root)
    : m_root(root),
      m_text(text),
      m_color(Color_Gray),
      m_hoverColor(Color_White),
      m_font(),
      m_alignX(gfx::LeftAlign),
      m_alignY(gfx::MiddleAlign)
{ }

ui::icons::ColorText::~ColorText()
{ }

bool
ui::icons::ColorText::setText(const String_t& text)
{
    return Updater().set(m_text, text);
}

bool
ui::icons::ColorText::setColor(uint8_t color)
{
    return Updater().set(m_color, color);
}

bool
ui::icons::ColorText::setHoverColor(uint8_t color)
{
    return Updater().set(m_hoverColor, color);
}

bool
ui::icons::ColorText::setFont(gfx::FontRequest font)
{
    return Updater().set(m_font, font);
}

bool
ui::icons::ColorText::setTextAlign(gfx::HorizontalAlignment x, gfx::VerticalAlignment y)
{
    return Updater().set(m_alignX, x).set(m_alignY, y);
}

gfx::Point
ui::icons::ColorText::getSize() const
{
    afl::base::Ref<gfx::Font> font(m_root.provider().getFont(m_font));
    return gfx::Point(font->getTextWidth(m_text),
                      font->getTextHeight(m_text));
}

void
ui::icons::ColorText::draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const
{
    // ex WShipCargoButton::drawContent [sort-of]
    gfx::Context<uint8_t> ctx2(ctx.canvas(), m_root.colorScheme());
    ctx2.useFont(*m_root.provider().getFont(m_font));
    ctx2.setColor(flags.contains(ActiveButton) ? m_hoverColor : m_color);
    ctx2.setTextAlign(m_alignX, m_alignY);
    outTextF(ctx2, area, m_text);
}
