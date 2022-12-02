/**
  *  \file ui/widgets/scrollbar.cpp
  *  \brief Class ui::widgets::Scrollbar
  *
  *  FIXME: For now, this is the good old simple PCC2 scrollbar.
  *  It cannot handle clicks into the scroll area or dragging the scroll box.
  *  It would also make sense to allow a horizontal mode.
  */

#include "ui/widgets/scrollbar.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "util/unicodechars.hpp"
#include "util/updater.hpp"

const afl::sys::Timeout_t FIRE_INTERVAL_MS = 50;


ui::widgets::Scrollbar::Scrollbar(ScrollableWidget& widget, Root& root)
    : m_widget(widget),
      m_root(root),
      m_timer(root.engine().createTimer()),
      conn_change(widget.sig_change.add(this, &Scrollbar::onChange)),
      conn_timer(m_timer->sig_fire.add(this, &Scrollbar::onTimer)),
      m_up(),
      m_down()
{
    // ex UIScrollbar::UIScrollbar
    onChange();
    m_timer->setInterval(FIRE_INTERVAL_MS);
}

ui::widgets::Scrollbar::~Scrollbar()
{ }

void
ui::widgets::Scrollbar::draw(gfx::Canvas& can)
{
    // ex UIScrollbar::drawContent
    // Prepare
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addSize(1)));
    ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
    gfx::Rectangle r = getExtent();

    // Buttons
    if (r.getHeight() >= 40) {
        gfx::Rectangle top = r.splitY(20);
        gfx::Rectangle mid = r.splitY(r.getHeight()-20);
        gfx::Rectangle bot = r;
        drawButton(ctx, top, getButtonFlags(m_up),   UTF_UP_ARROW);
        drawButton(ctx, bot, getButtonFlags(m_down), UTF_DOWN_ARROW);
        r = mid;
    }

    // Scrollbox
    if (r.getHeight() >= 4) {
        drawFrameUp(ctx, r);
        r.grow(-1, -1);

        if (m_widget.getTotalSize() > m_widget.getPageSize()) {
            // Draw scroll box
            int top = m_widget.getPageTop();
            if (top + m_widget.getPageSize() > m_widget.getTotalSize()) {
                top = m_widget.getTotalSize() - m_widget.getPageSize();
            }
            int divi = m_widget.getTotalSize();

            int yy1 = (r.getHeight()-2) * int32_t(top) / divi;
            int yy2 = (r.getHeight()-2) * int32_t(top + m_widget.getPageSize()) / divi + 2;

            if (yy1 > 0) {
                drawSolidBar(ctx, gfx::Rectangle(r.getLeftX(), r.getTopY(), r.getWidth(), yy1), Color_Shield + 3);
            }
            drawFrameUp(ctx, gfx::Rectangle(r.getLeftX(), r.getTopY() + yy1, r.getWidth(), yy2 - yy1));
            if (yy1 < yy2-2) {
                drawSolidBar(ctx, gfx::Rectangle(r.getLeftX()+1, r.getTopY() + yy1+1, r.getWidth()-2, yy2-yy1-2), Color_Shield + 7);
            }
            if (yy2 < r.getHeight()) {
                drawSolidBar(ctx, gfx::Rectangle(r.getLeftX(), r.getTopY()+yy2, r.getWidth(), r.getHeight()-yy2), Color_Shield + 3);
            }
        } else {
            // Not scrollable
            drawSolidBar(ctx, r, Color_Shield+5);
        }
    }
}

void
ui::widgets::Scrollbar::handleStateChange(State st, bool enable)
{
    // ex UIScrollbar::onStateChange
    if (st == ActiveState && !enable) {
        // Remove focus
        util::Updater up;
        up.set(m_up,   m_up   - Active - Pressed);
        up.set(m_down, m_down - Active - Pressed);
        if (up) {
            requestRedraw();
        }
    }
}

void
ui::widgets::Scrollbar::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    // Get rid of pressed/hovered buttons
    m_up.clear();
    m_down.clear();

    // Update 'Disabled' state and redraw
    onChange();
}

ui::layout::Info
ui::widgets::Scrollbar::getLayoutInfo() const
{
    // ex UIScrollbar::getLayoutInfo
    return ui::layout::Info(gfx::Point(20, 60), gfx::Point(20, 80), ui::layout::Info::GrowVertical);
}

bool
ui::widgets::Scrollbar::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::widgets::Scrollbar::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    LocalButtonFlags_t newUp   = m_up   - Active - Pressed;
    LocalButtonFlags_t newDown = m_down - Active - Pressed;

    bool result;
    if (getExtent().contains(pt)) {
        // Where did we click?
        LocalButtonFlags_t* where;
        if (pt.getY() < getExtent().getTopY() + 20) {
            where = &newUp;
        } else if (pt.getY() >= getExtent().getBottomY() - 20) {
            where = &newDown;
        } else {
            where = 0;
        }

        // Process click
        if (where != 0) {
            requestActive();
            *where += Active;
            if (!pressedButtons.empty()) {
                *where += Pressed;
            }
        }
        result = true;
    } else {
        result = false;
    }

    if (util::Updater().set(m_up, newUp).set(m_down, newDown)) {
        requestRedraw();
    }

    return result;
}

void
ui::widgets::Scrollbar::onChange()
{
    // UIScrollbar::scrolled, UIScrollbar::sync
    if (m_widget.getPageSize() >= m_widget.getTotalSize()) {
        m_up += Disabled;
        m_down += Disabled;
    } else {
        m_up -= Disabled;
        m_down -= Disabled;
    }
    requestRedraw();
}

void
ui::widgets::Scrollbar::onTimer()
{
    if (m_up.contains(Pressed)) {
        m_widget.requestFocus();
        m_widget.scroll(ScrollableWidget::LineUp);
    }
    if (m_down.contains(Pressed)) {
        m_widget.requestFocus();
        m_widget.scroll(ScrollableWidget::LineDown);
    }
    m_timer->setInterval(FIRE_INTERVAL_MS);
}

ui::ButtonFlags_t
ui::widgets::Scrollbar::getButtonFlags(LocalButtonFlags_t f)
{
    ButtonFlags_t result;
    if (f.contains(Pressed)) {
        result += PressedButton;
    }
    if (f.contains(Active)) {
        result += ActiveButton;
    }
    if (f.contains(Disabled)) {
        result += DisabledButton;
    }
    return result;
}
