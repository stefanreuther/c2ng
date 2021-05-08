/**
  *  \file ui/icons/skintext.cpp
  */

#include "ui/icons/skintext.hpp"
#include "util/updater.hpp"

using util::Updater;

ui::icons::SkinText::SkinText(const String_t& text, Root& root)
    : Icon(),
      m_root(root),
      m_text(text),
      m_font(),
      m_alignX(gfx::LeftAlign),
      m_alignY(gfx::MiddleAlign)
{ }

ui::icons::SkinText::~SkinText()
{ }

bool
ui::icons::SkinText::setFont(gfx::FontRequest font)
{
    return Updater().set(m_font, font);
}

bool
ui::icons::SkinText::setTextAlign(gfx::HorizontalAlignment x, gfx::VerticalAlignment y)
{
    return Updater().set(m_alignX, x).set(m_alignY, y);
}

gfx::Point
ui::icons::SkinText::getSize() const
{
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_font);
    return gfx::Point(font->getTextWidth(m_text),
                      font->getTextHeight(m_text));
}

void
ui::icons::SkinText::draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const
{
    ctx.setColor(flags.contains(DisabledButton) ? SkinColor::Faded : SkinColor::Static);
    ctx.useFont(*m_root.provider().getFont(m_font));
    ctx.setTextAlign(m_alignX, m_alignY);
    outTextF(ctx, area, m_text);
}
