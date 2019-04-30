/**
  *  \file ui/widgets/tiledpanel.cpp
  */

#include "ui/widgets/tiledpanel.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"

namespace {
    const int SIZE = 5;
}

ui::widgets::TiledPanel::TiledPanel(gfx::ResourceProvider& provider,
                                    ui::ColorScheme& scheme,
                                    ui::layout::Manager& mgr)
    : LayoutableGroup(mgr),
      m_resourceProvider(provider),
      m_colorScheme(scheme),
      conn_providerImageChange(provider.sig_imageChange.add(this, &TiledPanel::onImageChange)),
      m_tile()
{
    onImageChange();
}

ui::widgets::TiledPanel::~TiledPanel()
{ }

// Widget:
void
ui::widgets::TiledPanel::draw(gfx::Canvas& can)
{
    gfx::Context<uint8_t> ctx(can, m_colorScheme);

    gfx::Rectangle r = getExtent();
    drawFrameUp(ctx, r);
    r.grow(-1, -1);

    if (m_tile.get() != 0) {
        blitTiled(ctx, r, *m_tile, m_tile->getSize().getX() / 4);
    } else {
        drawSolidBar(ctx, r, Color_Dark);
    }

    defaultDrawChildren(can);
}

void
ui::widgets::TiledPanel::handleStateChange(State /*st*/, bool /*enable*/)
{ }

bool
ui::widgets::TiledPanel::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::widgets::TiledPanel::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

// LayoutableGroup:
gfx::Rectangle
ui::widgets::TiledPanel::transformSize(gfx::Rectangle size, Transformation kind) const
{
    switch (kind) {
     case OuterToInner:
        size.grow(-SIZE, -SIZE);
        break;
     case InnerToOuter:
        size.grow(SIZE, SIZE);
        break;
    }
    return size;
}

void
ui::widgets::TiledPanel::onImageChange()
{
    if (m_tile.get() == 0) {
        m_tile = m_resourceProvider.getImage("bgtile");
        if (m_tile.get() != 0) {
            requestRedraw();
        }
    }
}
