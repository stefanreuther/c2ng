/**
  *  \file ui/widgets/statictext.cpp
  */

#include "ui/widgets/statictext.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"

ui::widgets::StaticText::StaticText(const String_t& text, util::SkinColor::Color color, gfx::FontRequest font, gfx::ResourceProvider& provider, gfx::HorizontalAlignment align)
    : SimpleWidget(),
      m_text(text),
      m_color(color),
      m_font(font),
      m_provider(provider),
      m_align(align),
      m_isFlexible(false)
{
    // ex UIStaticText::UIStaticText
    setState(DisabledState, true);
}

ui::widgets::StaticText::StaticText(const char* text, util::SkinColor::Color color, gfx::FontRequest font, gfx::ResourceProvider& provider, gfx::HorizontalAlignment align)
    : SimpleWidget(),
      m_text(text),
      m_color(color),
      m_font(font),
      m_provider(provider),
      m_align(align),
      m_isFlexible(false)
{
    setState(DisabledState, true);
}

ui::widgets::StaticText::~StaticText()
{ }

ui::widgets::StaticText&
ui::widgets::StaticText::setText(const String_t& text)
{
    // ex UIStaticText::setText
    if (text != m_text) {
        m_text = text;
        requestRedraw();
    }
    return *this;
}

ui::widgets::StaticText&
ui::widgets::StaticText::setIsFlexible(bool flex)
{
    // ex UIStaticText::setFlexible
    m_isFlexible = flex;
    m_forcedWidth = afl::base::Nothing;
    return *this;
}

ui::widgets::StaticText&
ui::widgets::StaticText::setColor(util::SkinColor::Color color)
{
    m_color = color;
    requestRedraw();
    return *this;
}

ui::widgets::StaticText&
ui::widgets::StaticText::setForcedWidth(int width)
{
    m_forcedWidth = width;
    m_isFlexible = false;
    return *this;
}

// SimpleWidget:
void
ui::widgets::StaticText::draw(gfx::Canvas& can)
{
    // ex UIStaticText::drawContent
    afl::base::Ref<gfx::Font> font = m_provider.getFont(m_font);
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.setColor(m_color);
    ctx.useFont(*font);
    ctx.setTextAlign(m_align, gfx::MiddleAlign);
    drawBackground(ctx, getExtent());
    outTextF(ctx, getExtent(), m_text);
}

void
ui::widgets::StaticText::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::widgets::StaticText::handlePositionChange()
{
    requestRedraw();
}

ui::layout::Info
ui::widgets::StaticText::getLayoutInfo() const
{
    // ex UIStaticText::getLayoutInfo
    afl::base::Ref<gfx::Font> font = m_provider.getFont(m_font);
    const int *pWidth = m_forcedWidth.get();
    gfx::Point pt(pWidth != 0 ? *pWidth : font->getTextWidth(m_text), font->getTextHeight(m_text));
    return ui::layout::Info(pt, pt, m_isFlexible ? ui::layout::Info::GrowHorizontal : ui::layout::Info::Fixed);
}

bool
ui::widgets::StaticText::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
ui::widgets::StaticText::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}
