/**
  *  \file ui/widgets/button.cpp
  */

#include "ui/widgets/button.hpp"
#include "ui/draw.hpp"

// /** \class UIButton
//     \brief Push button

//     This class provides the look of a standard push-button. */

ui::widgets::Button::Button(String_t text, util::Key_t key, ui::Root& root)
    : AbstractButton(root, key),
      m_text(text),
      m_font(gfx::FontRequest().addSize(1))
{ }

ui::widgets::Button::~Button()
{ }

void
ui::widgets::Button::draw(gfx::Canvas& can)
{
    afl::base::Ref<gfx::Font> font = root().provider().getFont(m_font);
    gfx::Context<uint8_t> ctx(can, root().colorScheme());
    ctx.useFont(*font);
    ctx.setTextAlign(1, 1);
    drawButton(ctx, getExtent(), getFlags(), getStates(), m_text);
}

void
ui::widgets::Button::handleStateChange(State st, bool enable)
{
    defaultHandleStateChange(st, enable);
}

void
ui::widgets::Button::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    // FIXME: move into AbstractButton?
    requestRedraw();
}

ui::layout::Info
ui::widgets::Button::getLayoutInfo() const
{
    afl::base::Ref<gfx::Font> font = root().provider().getFont(gfx::FontRequest().addSize(1));
    int h = font->getTextHeight("Tp")*9/8;
    int w = (m_text.size() == 1
             ? h
             : m_text.size() == 2 && m_text[0] == 'F' && m_text[1] >= '0' && m_text[1] <= '9'
             ? h*5/4
             : h*3/5 + font->getTextWidth(m_text));
    return gfx::Point(w, h);
}

bool
ui::widgets::Button::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::widgets::Button::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
ui::widgets::Button::setFont(gfx::FontRequest font)
{
    m_font = font;
}
