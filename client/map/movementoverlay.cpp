/**
  *  \file client/map/movementoverlay.cpp
  */

#include <cstdlib>
#include "client/map/movementoverlay.hpp"
#include "client/map/renderer.hpp"
#include "client/map/widget.hpp"
#include "game/map/renderoptions.hpp"
#include "game/proxy/drawingproxy.hpp"

using game::map::RenderOptions;

client::map::MovementOverlay::MovementOverlay(util::RequestDispatcher& disp, util::RequestSender<game::Session> gameSender, Widget& parent)
    : m_gameSender(gameSender),
      m_lockProxy(gameSender, disp),
      m_parent(parent),
      m_modes(),
      m_valid(false),
      m_position()
{
    m_lockProxy.sig_result.add(this, &MovementOverlay::onLockResult);
}

client::map::MovementOverlay::~MovementOverlay()
{ }

void
client::map::MovementOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

void
client::map::MovementOverlay::drawAfter(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

bool
client::map::MovementOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::MovementOverlay::handleKey(util::Key_t key, int prefix, const Renderer& ren)
{
    if (m_valid && m_modes.contains(AcceptMovementKeys)) {
        // ex WScannerChartWidget::handleKey
        switch (key) {
         case util::Key_Left:
            moveBy(prefix ? -prefix : -10, 0, ren);
            return true;
         case util::Key_Right:
            moveBy(prefix ? prefix : 10, 0, ren);
            return true;
         case util::Key_Up:
            moveBy(0, prefix ? prefix : 10, ren);
            return true;
         case util::Key_Down:
            moveBy(0, prefix ? -prefix : -10, ren);
            return true;
         case util::KeyMod_Shift + util::Key_Left:
            moveBy(-1, 0, ren);
            return true;
         case util::KeyMod_Shift + util::Key_Right:
            moveBy(1, 0, ren);
            return true;
         case util::KeyMod_Shift + util::Key_Up:
            moveBy(0, 1, ren);
            return true;
         case util::KeyMod_Shift + util::Key_Down:
            moveBy(0, -1, ren);
            return true;
         case util::KeyMod_Ctrl + util::Key_Left:
            moveBy(-100, 0, ren);
            return true;
         case util::KeyMod_Ctrl + util::Key_Right:
            moveBy(100, 0, ren);
            return true;
         case util::KeyMod_Ctrl + util::Key_Up:
            moveBy(0, 100, ren);
            return true;
         case util::KeyMod_Ctrl + util::Key_Down:
            moveBy(0, -100, ren);
            return true;
         case ' ':
         case util::Key_Return:
         case util::KeyMod_Ctrl + ' ':
         case util::KeyMod_Ctrl + util::Key_Return:
         case util::KeyMod_Shift + ' ':
         case util::KeyMod_Shift + util::Key_Return:
         case util::KeyMod_Shift + util::KeyMod_Ctrl + ' ':
         case util::KeyMod_Shift + util::KeyMod_Ctrl + util::Key_Return:
            lockItem(m_position,
                     (key & ~util::KeyMod_Shift & ~util::KeyMod_Ctrl) != ' ',
                     (key & util::KeyMod_Ctrl) != 0,
                     (key & util::KeyMod_Shift) != 0,
                     ren);
            return true;

         case util::KeyMod_Alt + 'r':
            moveBy(-5 + std::rand() % 11, -5 + std::rand() % 11, ren);
            return true;

         case 'm':
         case util::KeyMod_Ctrl + 'm':
            game::proxy::DrawingProxy(m_gameSender, m_parent.root().engine().dispatcher())
                .createCannedMarker(m_position, prefix % game::config::UserConfiguration::NUM_CANNED_MARKERS);
            return true;

            /* TODO: z [edit zoom in PCC 1.x] */
        }
    }
    if (m_valid && m_modes.contains(AcceptZoomKeys)) {
        switch (key) {
         case '+':
            m_parent.zoomIn();
            return true;
         case '-':
            m_parent.zoomOut();
            return true;
        }
    }
    if (m_valid && m_modes.contains(AcceptConfigKeys)) {
        // ex WScannerChartWidget::handleOption
        if ((key & util::KeyMod_Alt) != 0) {
            RenderOptions::Options_t opt = RenderOptions::getOptionFromKey(key & ~util::KeyMod_Alt & ~util::KeyMod_Ctrl);
            if (!opt.empty()) {
                m_parent.toggleOptions(opt);
                return true;
            }
        }
    }

    return false;
}

bool
client::map::MovementOverlay::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren)
{
    if (!pressedButtons.empty()) {
        bool dbl   = pressedButtons.contains(gfx::EventConsumer::DoubleClick);
        bool shift = pressedButtons.contains(gfx::EventConsumer::ShiftKey);
        bool ctrl  = pressedButtons.contains(gfx::EventConsumer::CtrlKey);
        pressedButtons -= gfx::EventConsumer::DoubleClick;
        pressedButtons -= gfx::EventConsumer::ShiftKey;
        pressedButtons -= gfx::EventConsumer::CtrlKey;

        if (dbl) {
            if (m_valid) {
                sig_doubleClick.raise(m_position);
            }
        } else if (pressedButtons == MouseButtons_t(gfx::EventConsumer::LeftButton)) {
            lockItem(ren.unscale(pt), true, ctrl, shift, ren);
        } else if (pressedButtons == MouseButtons_t(gfx::EventConsumer::RightButton)) {
            lockItem(ren.unscale(pt), false, ctrl, shift, ren);
        } else {
            // Middle butten, button plus Alt, ...
            moveTo(ren.unscale(pt), ren);
        }
        return true;
    } else {
        return false;
    }
}

void
client::map::MovementOverlay::setMode(Mode mode, bool enable)
{
    m_modes.set(mode, enable);
}

void
client::map::MovementOverlay::setPosition(game::map::Point pt)
{
    m_valid = true;
    m_position = pt;
}

void
client::map::MovementOverlay::clearPosition()
{
    m_valid = false;
}

bool
client::map::MovementOverlay::getPosition(game::map::Point& out) const
{
    if (m_valid) {
        out = m_position;
        return true;
    } else {
        return false;
    }
}

void
client::map::MovementOverlay::setLockOrigin(game::map::Point pt, bool isHyperdriving)
{
    m_lockProxy.setOrigin(pt, isHyperdriving);
}

void
client::map::MovementOverlay::moveBy(int dx, int dy, const Renderer& ren)
{
    // ex WScannerChartWidget::moveKey
    if (m_valid) {
        moveTo(m_position + game::map::Point(dx, dy), ren);
    }
}

void
client::map::MovementOverlay::moveTo(game::map::Point pt, const Renderer& ren)
{
    // ex WScannerChartWidget::clipScannerPosition (far relative)
    if (m_valid) {
        gfx::Rectangle area = ren.getExtent();

        // top/left on screen is minimum X, maximum Y
        game::map::Point topLeft = ren.unscale(area.getTopLeft());

        // bottom/right on screen is maximum X, minimum Y.
        // Reduce by one because it is exclusive
        game::map::Point bottomRight = ren.unscale(area.getBottomRight() - gfx::Point(1, 1));

        // Determine result
        game::map::Point result(std::max(topLeft.getX(), std::min(bottomRight.getX(), pt.getX())),
                                std::min(topLeft.getY(), std::max(bottomRight.getY(), pt.getY())));
        if (result != m_position) {
            sig_move.raise(result);
        }
    }
}

void
client::map::MovementOverlay::lockItem(game::map::Point target, bool left, bool markedOnly, bool optimizeWarp, const Renderer& ren)
{
    // ex WScannerChartWidget::doItemLock (part)
    using game::proxy::LockProxy;

    LockProxy::Flags_t flags;
    if (left) {
        flags += LockProxy::Left;
    }
    if (markedOnly) {
        flags += LockProxy::MarkedOnly;
    }
    if (optimizeWarp) {
        flags += LockProxy::ToggleOptimizeWarp;
    }

    // Range limit
    gfx::Rectangle area = ren.getExtent();
    game::map::Point topLeft = ren.unscale(area.getTopLeft());
    game::map::Point bottomRight = ren.unscale(area.getBottomRight() - gfx::Point(1, 1));
    m_lockProxy.setRangeLimit(game::map::Point(topLeft.getX(), bottomRight.getY()),
                              game::map::Point(bottomRight.getX(), topLeft.getY()));

    m_lockProxy.postQuery(target, flags);
}

void
client::map::MovementOverlay::onLockResult(game::map::Point result)
{
    if (m_valid) {
        // Do NOT go through moveTo, for now. We don't have the required Renderer object at hand.
        sig_move.raise(result);
    }
}
