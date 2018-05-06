/**
  *  \file gfx/anim/textsprite.cpp
  */

#include "gfx/anim/textsprite.hpp"
#include "gfx/basecontext.hpp"

gfx::anim::TextSprite::TextSprite(ResourceProvider& provider)
    : Sprite(),
      m_provider(provider),
      m_font(),
      m_position(),
      m_textAlign(),
      m_text(),
      m_color(0)
{ }

gfx::anim::TextSprite::~TextSprite()
{ }

void
gfx::anim::TextSprite::setFont(FontRequest font)
{
    if (font != m_font) {
        m_font = font;
        updatePosition();
    }
}

void
gfx::anim::TextSprite::setPosition(Point pt)
{
    if (pt != m_position) {
        m_position = pt;
        updatePosition();
    }
}

void
gfx::anim::TextSprite::setTextAlign(int x, int y)
{
    if (Point(x, y) != m_textAlign) {
        m_textAlign = Point(x, y);
        updatePosition();
    }
}

void
gfx::anim::TextSprite::setText(const String_t& text)
{
    if (text != m_text) {
        m_text = text;
        updatePosition();
    }
}

void
gfx::anim::TextSprite::setColor(Color_t color)
{
    if (color != m_color) {
        m_color = color;
        markChanged();
    }
}

void
gfx::anim::TextSprite::tick()
{ }

void
gfx::anim::TextSprite::draw(Canvas& can)
{
    BaseContext ctx(can);
    ctx.useFont(*m_provider.getFont(m_font));
    ctx.setRawColor(m_color);
    outText(ctx, getExtent().getTopLeft(), m_text);
}

void
gfx::anim::TextSprite::updatePosition()
{
    // Compute new coordinates
    afl::base::Ref<Font> font = m_provider.getFont(m_font);
    int width = font->getTextWidth(m_text);
    int height = font->getTextHeight(m_text);
    setExtent(Rectangle(m_position.getX() - m_textAlign.getX()*width/2,
                        m_position.getY() - m_textAlign.getY()*height/2,
                        width,
                        height));

    // Even if the coordinates did not change, force redraw
    markChanged();
}
