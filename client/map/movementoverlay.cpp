/**
  *  \file client/map/movementoverlay.cpp
  */

#include <cstdlib>
#include "client/map/movementoverlay.hpp"
#include "client/map/renderer.hpp"
#include "client/map/widget.hpp"
#include "game/map/renderoptions.hpp"
#include "game/proxy/drawingproxy.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/icons/skintext.hpp"
#include "ui/icons/vbox.hpp"
#include "ui/prefixargument.hpp"

using game::map::RenderOptions;

client::map::MovementOverlay::MovementOverlay(util::RequestDispatcher& disp, util::RequestSender<game::Session> gameSender, Widget& parent, afl::string::Translator& tx)
    : m_gameSender(gameSender),
      m_lockProxy(gameSender, disp),
      m_parent(parent),
      m_translator(tx),
      m_modes(),
      m_toolTip(parent.root()),
      m_keyboardMode(false),
      m_keyboardAdviceOnTop(true),
      m_valid(false),
      m_position(),
      m_hoveredPoint()
{
    m_lockProxy.sig_result.add(this, &MovementOverlay::onLockResult);
    m_lockProxy.sig_unitNameResult.add(this, &MovementOverlay::onUnitNameResult);
    m_toolTip.sig_hover.add(this, &MovementOverlay::onHover);
}

client::map::MovementOverlay::~MovementOverlay()
{ }

void
client::map::MovementOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

void
client::map::MovementOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    if (m_keyboardMode) {
        // Text
        String_t message = m_translator("Keyboard Mode");

        // Compute dimensions
        afl::base::Ref<gfx::Font> font(m_parent.root().provider().getFont("+"));
        int height = font->getTextHeight(message) + 4;
        int width = font->getTextWidth(message) + 10;

        // Determine position
        gfx::Rectangle extent(ren.getExtent());
        gfx::Rectangle area(0, 0, width, height);
        gfx::Point cursorPos(ren.scale(m_position));
        if (m_keyboardAdviceOnTop) {
            if (cursorPos.getY() < extent.getTopY() + extent.getHeight()/3) {
                m_keyboardAdviceOnTop = false;
            }
        } else {
            if (cursorPos.getY() > extent.getTopY() + 2*extent.getHeight()/3) {
                m_keyboardAdviceOnTop = true;
            }
        }
        area.moveToEdge(extent, gfx::CenterAlign, m_keyboardAdviceOnTop ? gfx::TopAlign : gfx::BottomAlign, 5);

        // Draw
        gfx::Context<uint8_t> ctx(can, m_parent.root().colorScheme());
        drawSolidBar(ctx, area, ui::Color_Shield+2);
        ctx.useFont(*font);
        ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
        ctx.setColor(ui::Color_White);
        outTextF(ctx, area, message);
    }
}

bool
client::map::MovementOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::MovementOverlay::handleKey(util::Key_t key, int prefix, const Renderer& ren)
{
    m_toolTip.handleKey(key, prefix);
    if (m_valid && (m_modes.contains(AcceptMovementKeys) || m_keyboardMode)) {
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
    if (m_valid && (m_modes.contains(AcceptConfigKeys) || m_keyboardMode)) {
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
    // Drive the tooltip
    m_toolTip.handleMouse(pt, pressedButtons, ren.getExtent().contains(pt));

    // Click-to-lock
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
            // Middle button, button plus Alt, ...
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
client::map::MovementOverlay::doKeyboardMode(const Renderer& ren)
{
    // ex WScannerChartWidget::doKeyboardMode()
    // Keyboard mode not available when there's no valid position
    if (!m_valid) {
        return;
    }

    // Avoid recursion
    if (m_keyboardMode) {
        return;
    }

    // Regular keyboard mode
    class KeyboardModeHelper : public gfx::EventConsumer {
     public:
        KeyboardModeHelper(MovementOverlay& parent, const Renderer& ren)
            : m_parent(parent),
              m_renderer(ren),
              m_pendingMouseMovement(),
              m_running(true)
            {
                m_parent.m_keyboardMode = true;
                m_parent.requestRedraw();
            }
        ~KeyboardModeHelper()
            {
                m_parent.m_keyboardMode = false;
                m_parent.requestRedraw();
            }
        virtual bool handleKey(util::Key_t key, int prefix)
            {
                switch (key) {
                    /* PCC 1's exit commands:
                         1..9         prefix arg
                         BS, ESC, y   regular exit
                         x            exit + 'x'
                         C-w          exit + C-w
                         l            exit + 'l'
                       We add some more. */
                 case util::Key_Escape:
                 case util::Key_Backspace:
                 case 'y':
                    m_running = false;
                    return true;

                 case util::Key_Quit:
                 case util::Key_F1 + util::KeyMod_Ctrl:
                 case util::Key_F2 + util::KeyMod_Ctrl:
                 case util::Key_F3 + util::KeyMod_Ctrl:
                 case util::Key_F4 + util::KeyMod_Ctrl:
                 case util::Key_F5 + util::KeyMod_Ctrl:
                 case 'x':
                 case 'w' + util::KeyMod_Ctrl:
                 case 'L':
                 case 'l':
                    m_running = false;
                    m_parent.m_parent.root().ungetKeyEvent(key, prefix);
                    return true;

                 default:
                    if (key >= '1' && key <= '9') {
                        ui::PrefixArgument(m_parent.m_parent.root()).showPopup(key - '0');
                        return true;
                    } else {
                        return m_parent.handleKey(key, prefix, m_renderer);
                    }
                }

                if (key == util::Key_Escape) {
                    m_running = false;
                }
                return true;
            }
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
            {
                // Perform relative mouse movement
                m_pendingMouseMovement += pt;
                int dx = m_renderer.unscale(m_pendingMouseMovement.getX());
                int dy = m_renderer.unscale(m_pendingMouseMovement.getY());
                if (dx != 0 || dy != 0) {
                    m_parent.moveBy(dx, -dy, m_renderer);
                    m_pendingMouseMovement = gfx::Point();
                }

                // Find new position, by locking if needed
                bool dbl   = pressedButtons.contains(gfx::EventConsumer::DoubleClick);
                bool shift = pressedButtons.contains(gfx::EventConsumer::ShiftKey);
                bool ctrl  = pressedButtons.contains(gfx::EventConsumer::CtrlKey);
                pressedButtons -= gfx::EventConsumer::DoubleClick;
                pressedButtons -= gfx::EventConsumer::ShiftKey;
                pressedButtons -= gfx::EventConsumer::CtrlKey;

                if (pressedButtons.contains(LeftButton)) {
                    m_parent.lockItem(m_parent.m_position, true, ctrl, shift, m_renderer);
                } else if (pressedButtons.contains(RightButton)) {
                    m_parent.lockItem(m_parent.m_position, false, ctrl, shift, m_renderer);
                } else {
                    // nothing
                }

                // Double-click exits
                if (dbl) {
                    m_running = false;
                }
                return true;
            }

        bool isRunning() const
            { return m_running; }

     private:
        MovementOverlay& m_parent;
        const Renderer& m_renderer;
        gfx::Point m_pendingMouseMovement;
        bool m_running;
    };


    KeyboardModeHelper helper(*this, ren);
    while (helper.isRunning()) {
        m_parent.root().handleEventRelative(helper);
    }
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

    // Flags
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
    configureLockProxy(ren);

    // Request
    m_lockProxy.requestPosition(target, flags);
}

void
client::map::MovementOverlay::configureLockProxy(const Renderer& ren)
{
    gfx::Rectangle area = ren.getExtent();
    game::map::Point topLeft = ren.unscale(area.getTopLeft());
    game::map::Point bottomRight = ren.unscale(area.getBottomRight() - gfx::Point(1, 1));
    m_lockProxy.setRangeLimit(game::map::Point(topLeft.getX(), bottomRight.getY()),
                              game::map::Point(bottomRight.getX(), topLeft.getY()));
}

void
client::map::MovementOverlay::onLockResult(game::map::Point result)
{
    if (m_valid) {
        // Do NOT go through moveTo, for now. We don't have the required Renderer object at hand.
        sig_move.raise(result);
    }
}

void
client::map::MovementOverlay::onUnitNameResult(game::map::Point result, String_t names)
{
    // Discard empty result
    if (names.empty()) {
        return;
    }

    // Verify distance
    // Distance must be small enough (50 means distance of ~7 pixels; has been pulled out
    // of a hat, and it's the same as used in PCC 1.x)
    const Renderer& ren = m_parent.renderer();
    const gfx::Point resolved = ren.scale(result);
    const int dx = resolved.getX() - m_hoveredPoint.getX();
    const int dy = resolved.getY() - m_hoveredPoint.getY();
    if (dx*dx + dy*dy > 50) {
        return;
    }

    // Show it
    afl::base::Deleter del;
    ui::icons::VBox icon;
    String_t::size_type pos = 0, n;
    while ((n = names.find('\n', pos)) != String_t::npos) {
        icon.add(del.addNew(new ui::icons::SkinText(names.substr(pos, n - pos), m_parent.root())));
        pos = n+1;
    }
    icon.add(del.addNew(new ui::icons::SkinText(names.substr(pos), m_parent.root())));
    m_toolTip.showPopup(resolved, icon);
}

void
client::map::MovementOverlay::onHover(gfx::Point pt)
{
    const Renderer& ren = m_parent.renderer();
    m_hoveredPoint = pt;
    configureLockProxy(ren);
    m_lockProxy.requestUnitNames(ren.unscale(pt));
}
