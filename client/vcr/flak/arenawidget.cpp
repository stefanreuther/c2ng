/**
  *  \file client/vcr/flak/arenawidget.cpp
  *  \brief Class client::vcr::flak::ArenaWidget
  *
  *  FIXME: implement unit selection (click)
  *  FIXME: implement drag-to-rotate/move (drag)
  */

#include "client/vcr/flak/arenawidget.hpp"
#include "client/vcr/flak/flatrenderer.hpp"
#include "client/vcr/flak/renderer.hpp"
#include "client/vcr/flak/threedrenderer.hpp"


client::vcr::flak::ArenaWidget::ArenaWidget(ui::Root& root,
                                            game::vcr::flak::VisualisationState& state,
                                            game::vcr::flak::VisualisationSettings& settings)
    : SimpleWidget(),
      m_root(root),
      m_state(state),
      m_settings(settings),
      m_renderers(),
      m_currentRenderer(),
      m_grid(true)
{
    m_renderers.pushBackNew(new ThreeDRenderer(root, state, settings));
    m_renderers.pushBackNew(new FlatRenderer(root, state, settings));
}

client::vcr::flak::ArenaWidget::~ArenaWidget()
{ }

void
client::vcr::flak::ArenaWidget::init()
{
    for (size_t i = 0, n = m_renderers.size(); i < n; ++i) {
        m_renderers[i]->init();
    }
}

bool
client::vcr::flak::ArenaWidget::hasGrid() const
{
    return m_grid;
}

void
client::vcr::flak::ArenaWidget::toggleGrid()
{
    m_grid = !m_grid;
    requestRedraw();
}

void
client::vcr::flak::ArenaWidget::setMode(Mode m)
{
    if (m != m_currentRenderer) {
        m_currentRenderer = m;
        requestRedraw();
    }
}

void
client::vcr::flak::ArenaWidget::toggleMode(Mode a, Mode b)
{
    if (m_currentRenderer == a) {
        setMode(b);
    } else {
        setMode(a);
    }
}

client::vcr::flak::ArenaWidget::Mode
client::vcr::flak::ArenaWidget::getMode() const
{
    return m_currentRenderer;
}

String_t
client::vcr::flak::ArenaWidget::toString(Mode m, afl::string::Translator& tx)
{
    switch (m) {
     case FlatMode:   return tx("flat");
     case ThreeDMode: return tx("3-D");
    }
    return String_t();
}

void
client::vcr::flak::ArenaWidget::draw(gfx::Canvas& can)
{
    size_t index = m_currentRenderer;
    if (index < m_renderers.size()) {
        m_renderers[index]->draw(can, getExtent(), m_grid);
    }
}

void
client::vcr::flak::ArenaWidget::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::vcr::flak::ArenaWidget::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
client::vcr::flak::ArenaWidget::getLayoutInfo() const
{
    return ui::layout::Info(gfx::Point(400, 400),
                            gfx::Point(400, 400),
                            ui::layout::Info::GrowBoth);
}

bool
client::vcr::flak::ArenaWidget::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::vcr::flak::ArenaWidget::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}
