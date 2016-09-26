/**
  *  \file ui/widgets/button.cpp
  */

#include "ui/widgets/button.hpp"
#include "ui/draw.hpp"

// /** \class UIButton
//     \brief Push button

//     This class provides the look of a standard push-button. */

ui::widgets::Button::Button(String_t text, util::Key_t key, gfx::ResourceProvider& provider, ui::ColorScheme& scheme)
    : AbstractButton(key),
      m_text(text),
      m_provider(provider),
      m_scheme(scheme),
      m_font(gfx::FontRequest().addSize(1))
{ }

ui::widgets::Button::~Button()
{ }

void
ui::widgets::Button::draw(gfx::Canvas& can)
{
    afl::base::Ptr<gfx::Font> font = m_provider.getFont(m_font);
    if (font.get() != 0) {
        gfx::Context ctx(can);
        ctx.useColorScheme(m_scheme);
        ctx.useFont(*font);
        ctx.setTextAlign(1, 1);
        drawButton(ctx, getExtent(), getFlags(), getStates(), m_text);
    }
}

void
ui::widgets::Button::handleStateChange(State st, bool enable)
{
    // FIXME: move into AbstractButton
    if (st == ActiveState) {
        setFlag(ActiveButton, enable);
        if (!enable) {
            setFlag(PressedButton, enable);
        }
    }
    requestRedraw();
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
    afl::base::Ptr<gfx::Font> font = m_provider.getFont(gfx::FontRequest().addSize(1));
    if (font.get() != 0) {
        int h = font->getTextHeight("Tp")*9/8;
        int w = m_text.size() == 1 ? h : h*3/5 + font->getTextWidth(m_text);
        return gfx::Point(w, h);
    } else {
        return gfx::Point();
    }
}

void
ui::widgets::Button::setFont(gfx::FontRequest font)
{
    m_font = font;
}
