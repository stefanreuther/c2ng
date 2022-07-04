/**
  *  \file ui/icons/button.cpp
  *  \brief Class ui::icons::Button
  */

#include "ui/icons/button.hpp"
#include "ui/draw.hpp"

ui::icons::Button::Button(String_t text, gfx::FontRequest font, Root& root)
    : Icon(),
      m_text(text),
      m_font(font),
      m_xAlign(gfx::CenterAlign),
      m_yAlign(gfx::MiddleAlign),
      m_compact(false),
      m_root(root)
{ }

ui::icons::Button::~Button()
{ }

gfx::Point
ui::icons::Button::getSize() const
{
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_font);

    /*
     *  Height: default gives some room for decorations.
     *  - large font (22px) -> button is 24px tall (PCC1: 25px)
     *  - normal font (16px) -> button is 18px tall
     *  - small font (10px) -> button is 11px tall
     *
     *  In compact mode, remove 4px, allowing for a 2px border.
     *  The border is actually used on control screens, but buttons of this size are also used without possible frame.
     *  - large font (22px) -> button is 20px tall (PCC1: 20px)
     */

    const int h = font->getTextHeight("Tp")*9/8;
    const int delta = m_compact ? 4 : 0;

    /*
     *  Width: Do not make a single-character button taller than square, independant from the letter on it.
     *  Likewise "F5" etc. get a special handling so they are all the same size.
     */

    const int w = (m_text.size() == 1
                   ? h
                   : m_text.size() == 2 && m_text[0] == 'F' && m_text[1] >= '0' && m_text[1] <= '9'
                   ? h*5/4
                   : h*3/5 + font->getTextWidth(m_text));
    return gfx::Point(w - delta, h - delta);
}

void
ui::icons::Button::draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const
{
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_font);
    gfx::Context<uint8_t> ctx2(ctx.canvas(), m_root.colorScheme());
    ctx2.useFont(*font);
    ctx2.setTextAlign(m_xAlign, m_yAlign);
    drawButton(ctx2, area, flags, m_text);
}

void
ui::icons::Button::setFont(gfx::FontRequest font)
{
    m_font = font;
}

void
ui::icons::Button::setTextAlign(gfx::HorizontalAlignment x, gfx::VerticalAlignment y)
{
    m_xAlign = x;
    m_yAlign = y;
}

void
ui::icons::Button::setText(const String_t& text)
{
    m_text = text;
}

void
ui::icons::Button::setCompact(bool flag)
{
    m_compact = flag;
}
