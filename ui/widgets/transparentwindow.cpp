/**
  *  \file ui/widgets/transparentwindow.cpp
  *  \brief Class ui::widgets::TransparentWindow
  */

#include "ui/widgets/transparentwindow.hpp"
#include "gfx/context.hpp"
#include "gfx/rgbapixmap.hpp"
#include "gfx/complex.hpp"
#include "gfx/basecontext.hpp"

namespace {
    // Padding, in pixels
    const int PAD = 10;

    // Transparency
    const int TRANSPARENCY = 180;
}

ui::widgets::TransparentWindow::ColorScheme::ColorScheme(TransparentWindow& parent)
    : m_parent(parent)
{ }

gfx::Color_t
ui::widgets::TransparentWindow::ColorScheme::getColor(util::SkinColor::Color index)
{
    return m_parent.m_parentColors.getColor(index);
}

void
ui::widgets::TransparentWindow::ColorScheme::drawBackground(gfx::Canvas& can, const gfx::Rectangle& area)
{
    // We cannot draw beyond (0,0)
    gfx::Rectangle adjustedArea(area);
    if (adjustedArea.getLeftX() < 0) {
        adjustedArea.consumeX(-adjustedArea.getLeftX());
    }
    if (adjustedArea.getTopY() < 0) {
        adjustedArea.consumeY(-adjustedArea.getTopY());
    }

    // Check cached pixmap
    if (m_cachedBackground.get() == 0 || !m_cachedSize.contains(adjustedArea)) {
        // We have not cached anything, or the request asks for pixels outside our cache.
        // Create a new pixmap.
        // The origin must always be (0,0), so include that point.
        m_cachedSize.include(adjustedArea);
        m_cachedSize.include(gfx::Point(0, 0));
        afl::base::Ref<gfx::RGBAPixmap> pix(gfx::RGBAPixmap::create(m_cachedSize.getWidth(), m_cachedSize.getHeight()));
        m_cachedBackground = pix->makeCanvas().asPtr();

        // Draw original background.
        m_parent.m_parentColors.drawBackground(*m_cachedBackground, m_cachedSize);

        // Draw a transparent black bar atop.
        m_cachedBackground->drawBar(m_cachedSize, COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, TRANSPARENCY);
    }

    // Draw
    gfx::blitTiledAnchored(gfx::BaseContext(can), area, *m_cachedBackground, m_cachedSize.getTopLeft(), 0);
}



ui::widgets::TransparentWindow::TransparentWindow(gfx::ColorScheme<util::SkinColor::Color>& parentColors, ui::layout::Manager& manager)
    : LayoutableGroup(manager),
      m_colorScheme(*this),
      m_parentColors(parentColors)
{
    setColorScheme(m_colorScheme);
    setState(ModalState, true);
}

void
ui::widgets::TransparentWindow::draw(gfx::Canvas& can)
{
    m_colorScheme.drawBackground(can, getExtent());
    defaultDrawChildren(can);
}

gfx::Rectangle
ui::widgets::TransparentWindow::transformSize(gfx::Rectangle size, Transformation kind) const
{
    switch (kind) {
     case InnerToOuter: size.grow(PAD, PAD);   break;
     case OuterToInner: size.grow(-PAD, -PAD); break;
    }
    return size;
}

bool
ui::widgets::TransparentWindow::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::widgets::TransparentWindow::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}


