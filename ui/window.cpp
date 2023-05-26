/**
  *  \file ui/window.cpp
  */

#include <algorithm>
#include "ui/window.hpp"
#include "gfx/complex.hpp"
#include "ui/colorscheme.hpp"


ui::Window::WindowColorScheme::WindowColorScheme(Window& parent)
    : m_parent(parent)
{ }

gfx::Color_t
ui::Window::WindowColorScheme::getColor(SkinColor::Color index)
{
    if (size_t(index) < SkinColor::NUM_COLORS) {
        return m_parent.m_uiColorScheme.getColor(m_parent.m_style.colors[index]);
    }
    return 0;
}

void
ui::Window::WindowColorScheme::drawBackground(gfx::Canvas& can, const gfx::Rectangle& area)
{
    can.drawBar(area, getColor(SkinColor::Background), gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
}




ui::Window::Window(String_t title, gfx::ResourceProvider& provider, ColorScheme& uiColorScheme, const WindowStyle& style, const ui::layout::Manager& manager)
    : LayoutableGroup(manager),
      m_title(title),
      m_resourceProvider(provider),
      m_style(style),
      m_border(5),
      m_uiColorScheme(uiColorScheme),
      m_colorScheme(*this),
      conn_providerImageChange(provider.sig_imageChange.add(this, (void (Window::*)())&Window::requestRedraw))
{
    setColorScheme(m_colorScheme);
    setState(ModalState, true);
}

ui::Window::~Window()
{ }

void
ui::Window::draw(gfx::Canvas& can)
{
    // ex UIWindow::drawContent
    gfx::Context<uint8_t> ctx(can, m_uiColorScheme);
    drawWindow(ctx, getExtent(), m_resourceProvider, m_style, m_title);
    defaultDrawChildren(can);
}

void
ui::Window::handleStateChange(State /*st*/, bool /*enable*/)
{ }

gfx::Rectangle
ui::Window::transformSize(gfx::Rectangle size, Transformation kind) const
{
    // ex UIWindow::adjustSize
    afl::base::Ref<gfx::Font> font = m_resourceProvider.getFont(gfx::FontRequest().addSize(1));
    int height = font->getTextHeight("Tp") + 2;

    switch (kind) {
     case OuterToInner:
        return gfx::Rectangle(size.getLeftX() + 4 + m_border,
                              size.getTopY() + height + m_border,
                              size.getWidth() - 8 - 2*m_border,
                              size.getHeight() - 4 - height - 2*m_border);

     case InnerToOuter:
        return gfx::Rectangle(size.getLeftX() - 4 - m_border,
                              size.getTopY() - height - m_border,
                              std::max(size.getWidth() + 8 + 2*m_border, font->getTextWidth(m_title)) + 10,
                              size.getHeight() + height + 4 + 2*m_border);
    }
    return size;
}

bool
ui::Window::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::Window::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    if (defaultHandleMouse(pt, pressedButtons)) {
        return true;
    }
    if (getExtent().contains(pt)) {
        requestActive();
        return true;
    }
    return false;
}
