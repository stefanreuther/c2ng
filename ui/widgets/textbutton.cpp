/**
  *  \file ui/widgets/textbutton.cpp
  */

#include "ui/widgets/textbutton.hpp"

ui::widgets::TextButton::TextButton(const String_t& text, util::Key_t key, Root& root)
    : BaseButton(root, key),
      m_icon(text, root)
{
    // ex UIMinimalButton, WShipCargoButton
    setIcon(m_icon);
}

ui::widgets::TextButton::~TextButton()
{ }

void
ui::widgets::TextButton::setText(const String_t& text)
{
    if (m_icon.setText(text)) {
        requestRedraw();
    }
}

void
ui::widgets::TextButton::setColor(uint8_t color)
{
    if (m_icon.setColor(color)) {
        requestRedraw();
    }
}

void
ui::widgets::TextButton::setHoverColor(uint8_t color)
{
    if (m_icon.setHoverColor(color)) {
        requestRedraw();
    }
}

void
ui::widgets::TextButton::setFont(gfx::FontRequest font)
{
    if (m_icon.setFont(font)) {
        requestRedraw();
    }
}

void
ui::widgets::TextButton::setTextAlign(gfx::HorizontalAlignment x, gfx::VerticalAlignment y)
{
    if (m_icon.setTextAlign(x, y)) {
        requestRedraw();
    }
}
