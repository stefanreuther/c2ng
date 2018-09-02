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
}

ui::widgets::TransparentWindow::TransparentWindow(gfx::ColorScheme<util::SkinColor::Color>& parentColors, ui::layout::Manager& manager)
    : LayoutableGroup(manager),
      m_colorScheme(parentColors)
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
