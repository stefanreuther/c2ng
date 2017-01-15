/**
  *  \file client/map/widget.cpp
  */

#include "client/map/widget.hpp"
#include "gfx/clipfilter.hpp"
#include "gfx/context.hpp"

client::map::Widget::Widget(util::RequestSender<game::Session> gameSender, ui::Root& root, gfx::Point preferredSize)
    : m_renderer(),
      m_proxy(gameSender, root.engine().dispatcher()),
      m_root(root),
      m_preferredSize(preferredSize),
      m_min(),
      m_max()
{
    m_proxy.sig_update.add(this, &Widget::onUpdate);
}

client::map::Widget::~Widget()
{ }

void
client::map::Widget::setCenter(game::map::Point pt)
{
    m_renderer.setCenter(pt);
    maybeRequestNewRange();
    requestRedraw();
}

void
client::map::Widget::draw(gfx::Canvas& can)
{
    // Background
    m_root.colorScheme().drawBackground(can, getExtent());

    // Map
    {
        gfx::ClipFilter clip(can, getExtent());
        m_renderer.draw(clip, m_root.colorScheme(), m_root.provider());
    }
}

void
client::map::Widget::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::map::Widget::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    m_renderer.setExtent(getExtent());
    maybeRequestNewRange();
    requestRedraw();
}

ui::layout::Info
client::map::Widget::getLayoutInfo() const
{
    return ui::layout::Info(m_preferredSize, m_preferredSize, ui::layout::Info::GrowBoth);
}

bool
client::map::Widget::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::map::Widget::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::map::Widget::onUpdate(afl::base::Ptr<game::map::RenderList> renderList)
{
    m_renderer.setRenderList(renderList);
    requestRedraw();
}

void
client::map::Widget::maybeRequestNewRange()
{
    // Get minimum range required for rendering the current view
    game::map::Point a, b;
    m_renderer.getMinimumWorldRange(a, b);

    // If we are outside the required range, fetch preferred range
    if (a.getX() < m_min.getX() || a.getY() < m_min.getY() || b.getX() > m_max.getX() || b.getY() > m_max.getY()) {
        m_renderer.getPreferredWorldRange(m_min, m_max);
        m_proxy.setRange(m_min, m_max);
    }
}
