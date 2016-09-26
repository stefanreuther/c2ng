/**
  *  \file ui/window.cpp
  */

#include <algorithm>
#include "ui/window.hpp"
#include "gfx/complex.hpp"


ui::Window::WindowColorScheme::WindowColorScheme(Window& parent)
    : m_parent(parent)
{ }

gfx::Color_t
ui::Window::WindowColorScheme::getColor(uint32_t index)
{
    if (index < SkinColor::NUM_COLORS) {
        if (Widget* p = m_parent.getParent()) {
            return p->getColorScheme().getColor(m_parent.m_style.colors[SkinColor::Color(index)]);
        }
    }
    return 0;
}

void
ui::Window::WindowColorScheme::drawBackground(gfx::Context& ctx, const gfx::Rectangle& area)
{
    gfx::drawSolidBar(ctx, area, SkinColor::Background);
}




ui::Window::Window(String_t title, gfx::ResourceProvider& provider, const WindowStyle& style, ui::layout::Manager& manager)
    : LayoutableGroup(manager),
      m_title(title),
      m_resourceProvider(provider),
      m_style(style),
      m_border(5),
      m_colorScheme(*this),
      conn_providerImageChange(provider.sig_imageChange.add(this, (void (Window::*)())&Window::requestRedraw))
{
    setColorScheme(m_colorScheme);
    setState(ModalState, true);
}

void
ui::Window::draw(gfx::Canvas& can)
{
    // ex UIWindow::drawContent
    gfx::Context ctx(can);
    ctx.useColorScheme(getParent()->getColorScheme());
    drawWindow(ctx, getExtent(), m_resourceProvider, m_style, m_title);
    defaultDrawChildren(can);
}

gfx::Rectangle
ui::Window::transformSize(gfx::Rectangle size, Transformation kind) const
{
    // ex UIWindow::adjustSize
    afl::base::Ptr<gfx::Font> font = m_resourceProvider.getFont(gfx::FontRequest().addSize(1));
    if (font.get() == 0) {
        // Error, bail out.
        return size;
    }
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
                              std::max(size.getWidth() + 8 + 2*m_border, font->getTextWidth(m_title)),
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
