/**
  *  \file client/map/widget.cpp
  */

#include "client/map/widget.hpp"
#include "gfx/clipfilter.hpp"
#include "gfx/context.hpp"
#include "client/map/overlay.hpp"

client::map::Widget::Widget(util::RequestSender<game::Session> gameSender, ui::Root& root, gfx::Point preferredSize)
    : m_renderer(),
      m_proxy(gameSender, root.engine().dispatcher()),
      m_root(root),
      m_preferredSize(preferredSize),
      m_mode(NormalMode),
      m_currentConfigurationArea(),
      m_min(),
      m_max(),
      m_overlays()
{
    m_proxy.sig_update.add(this, &Widget::onUpdate);
    updateModeConfiguration(true);
}

client::map::Widget::~Widget()
{
    std::vector<Overlay*> tmp;
    tmp.swap(m_overlays);

    for (std::vector<Overlay*>::iterator it = tmp.begin(); it != tmp.end(); ++it) {
        (*it)->setCallback(0);
    }
}

void
client::map::Widget::setCenter(game::map::Point pt)
{
    // ex WChartWidget::setCenterPosition
    // FIXME: isValid
    m_renderer.setCenter(pt);
    maybeRequestNewRange();
    requestRedraw();
}

void
client::map::Widget::draw(gfx::Canvas& can)
{
    // ex WChartWidget::drawContent
    // Background
    m_root.colorScheme().drawBackground(can, getExtent());

    // Map
    {
        gfx::ClipFilter clip(can, getExtent());

        // Overlay backgrounds
        for (std::vector<Overlay*>::iterator it = m_overlays.begin(); it != m_overlays.end(); ++it) {
            (*it)->drawBefore(clip, m_renderer);
        }

        // Map
        m_renderer.draw(clip, m_root.colorScheme(), m_root.provider());

        // Overlay foregrounds
        for (std::vector<Overlay*>::iterator it = m_overlays.begin(); it != m_overlays.end(); ++it) {
            (*it)->drawAfter(clip, m_renderer);
        }

        // Overlay cursors
        for (std::vector<Overlay*>::reverse_iterator it = m_overlays.rbegin(); it != m_overlays.rend(); ++it) {
            if ((*it)->drawCursor(clip, m_renderer)) {
                break;
            }
        }
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
    for (std::vector<Overlay*>::reverse_iterator it = m_overlays.rbegin(); it != m_overlays.rend(); ++it) {
        if ((*it)->handleKey(key, prefix, m_renderer)) {
            return true;
        }
    }
    return defaultHandleKey(key, prefix);
}

bool
client::map::Widget::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    for (std::vector<Overlay*>::reverse_iterator it = m_overlays.rbegin(); it != m_overlays.rend(); ++it) {
        if ((*it)->handleMouse(pt, pressedButtons, m_renderer)) {
            return true;
        }
    }
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

void
client::map::Widget::updateModeConfiguration(bool force)
{
    game::map::RenderOptions::Area area = game::map::RenderOptions::Normal;
    switch (m_mode) {
     case NormalMode:
        area = (m_renderer.scale(100) < 100
                ? game::map::RenderOptions::Small
                : game::map::RenderOptions::Normal);
        break;

     case ScannerMode:
        area = game::map::RenderOptions::Scanner;
        break;
    }

    if (area != m_currentConfigurationArea || force) {
        m_currentConfigurationArea = area;
        m_proxy.setConfiguration(area);
    }
}

void
client::map::Widget::removeOverlay(Overlay& over)
{
    std::vector<Overlay*>::iterator it = std::find(m_overlays.begin(), m_overlays.end(), &over);
    if (it != m_overlays.end()) {
        m_overlays.erase(it);
        over.setCallback(0);
    }
}

void
client::map::Widget::requestRedraw()
{
    SimpleWidget::requestRedraw();
}

void
client::map::Widget::requestRedraw(gfx::Rectangle& area)
{
    SimpleWidget::requestRedraw(area);
}

void
client::map::Widget::setMode(Mode mode)
{
    m_mode = mode;
    updateModeConfiguration(false);
}

void
client::map::Widget::addOverlay(Overlay& over)
{
    m_overlays.push_back(&over);
    over.setCallback(this);
}

void
client::map::Widget::setZoomToInclude(game::map::Point pt)
{
    // Simple brute-force solution
    while (!getExtent().contains(m_renderer.scale(pt)) && m_renderer.zoomOut()) {
        // nix
    }
    updateModeConfiguration(false);
    maybeRequestNewRange();
    requestRedraw();
}

void
client::map::Widget::zoomIn()
{
    // ex WChartWidget::zoomIn
    m_renderer.zoomIn();
    updateModeConfiguration(false);
    maybeRequestNewRange();
    requestRedraw();
}

void
client::map::Widget::zoomOut()
{
    // ex WChartWidget::zoomOut
    m_renderer.zoomOut();
    updateModeConfiguration(false);
    maybeRequestNewRange();
    requestRedraw();
}

void
client::map::Widget::setZoom(int mult, int divi)
{
    m_renderer.setZoom(mult, divi);
    updateModeConfiguration(false);
    maybeRequestNewRange();
    requestRedraw();
}

void
client::map::Widget::toggleOptions(game::map::RenderOptions::Options_t opts)
{
    m_proxy.toggleOptions(opts);
}

void
client::map::Widget::setDrawingTagFilter(util::Atom_t tag)
{
    m_proxy.setDrawingTagFilter(tag);
}

void
client::map::Widget::clearDrawingTagFilter()
{
    m_proxy.clearDrawingTagFilter();
}

void
client::map::Widget::setShipTrailId(game::Id_t id)
{
    m_proxy.setShipTrailId(id);
}

const client::map::Renderer&
client::map::Widget::renderer() const
{
    return m_renderer;
}

ui::Root&
client::map::Widget::root()
{
    return m_root;
}
