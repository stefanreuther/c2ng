/**
 *  \file ui/rich/statictext.cpp
 */

#include <algorithm>
#include "ui/rich/statictext.hpp"
#include "gfx/context.hpp"

ui::rich::StaticText::StaticText(const util::rich::Text& text, int width, gfx::ResourceProvider& provider)
    : m_document(provider),
      m_width(width)
{
    setText(text);
}

ui::rich::StaticText::~StaticText()
{ }

void
ui::rich::StaticText::setText(const util::rich::Text& text)
{
    // ex UIRichStatic::setText, UIMultilineStatic::setText
    m_document.clear();
    m_document.setPageWidth(std::max(getExtent().getWidth(), m_width));
    m_document.add(text);
    m_document.finish();
}

// SimpleWidget:
void
ui::rich::StaticText::draw(gfx::Canvas& can)
{
    // ex UIRichStatic::drawContent, UIMultilineStatic::drawContent
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    m_document.draw(ctx, getExtent(), 0);
}

void
ui::rich::StaticText::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::rich::StaticText::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
ui::rich::StaticText::getLayoutInfo() const
{
    return gfx::Point(m_width, m_document.getDocumentHeight());
}

bool
ui::rich::StaticText::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
ui::rich::StaticText::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}
