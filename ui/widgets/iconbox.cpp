/**
  *  \file ui/widgets/iconbox.cpp
  */

#include "ui/widgets/iconbox.hpp"
#include "gfx/clipfilter.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"

namespace {
    const size_t nil = size_t(-1);
}

ui::widgets::IconBox::IconBox(ui::Root& root)
    : SimpleWidget(),
      m_currentItem(0),
      m_hoverItem(nil),
      m_leftX(0),
      m_targetLeftX(0),
      m_scrollSpeed(0),
      m_pendingScroll(false),
      m_mousePressed(false),
      m_mouseBlocked(false),
      m_changeOnClick(false),
      m_root(root),
      m_timer(root.engine().createTimer())
{
    m_timer->sig_fire.add(this, &IconBox::handleTimer);
    m_timer->setInterval(20);
}

void
ui::widgets::IconBox::draw(gfx::Canvas& can)
{
    // ex UIIconBox::drawContent
    size_t count = getNumItems();
    int x = 0;
    for (size_t i = 0; i < count; ++i) {
        int wi = getItemWidth(i);
        int itemx = getExtent().getLeftX() - m_leftX + x;
        int itemy = getExtent().getTopY();
        gfx::Rectangle cliprect(itemx, itemy, wi, getExtent().getHeight());
        cliprect.intersect(getExtent());
        if (cliprect.exists()) {
            gfx::ClipFilter filter(can, cliprect);
            drawItem(filter,
                     gfx::Rectangle(itemx, itemy, wi, getExtent().getHeight()),
                     i,
                     i == m_currentItem ? Selected : i == m_hoverItem ? Hover : Normal);
        }
        x += wi;
    }

    int curx = getExtent().getLeftX() - m_leftX + x;
    if (curx < getExtent().getRightX()) {
        gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
        drawBackground(ctx, gfx::Rectangle(curx, getExtent().getTopY(), getExtent().getRightX() - curx, getExtent().getHeight()));
    }
}

void
ui::widgets::IconBox::handleStateChange(State st, bool enable)
{
    // ex UIIconBox::onStateChange
    if (st == ActiveState && !enable) {
        if (m_hoverItem != nil) {
            m_hoverItem = nil;
            requestRedraw();
        }
        if (m_pendingScroll) {
            if (adjustPosition()) {
                requestRedraw();
            }
            m_pendingScroll = false;
        }
        m_mousePressed = false;
    }
}

void
ui::widgets::IconBox::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    // The typical usecase for this function is that the size is set after initial layout (pack()).
    // If the widget has been configured before layout, it will have an animation scheduled based on a zero-size widget.
    // So we re-do the computation and cancel that animation.

    // Compute new position
    adjustPosition();

    // Cancel animation
    m_leftX = m_targetLeftX;
    m_scrollSpeed = 0;

    requestRedraw();
}

bool
ui::widgets::IconBox::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
ui::widgets::IconBox::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex UIIconBox::handleEvent (part)
    if (getExtent().contains(pt)) {
        // mouse points into this widget
        requestActive();

        // find item mouse points to
        int x = 0;
        size_t new_item = nil;
        size_t count = getNumItems();
        for (size_t i = 0; i < count; ++i) {
            int wi = getItemWidth(i);
            if (pt.getX() >= getExtent().getLeftX() - m_leftX + x && pt.getX() < getExtent().getLeftX() - m_leftX + x + wi) {
                new_item = i;
                break;
            }
            x += wi;
        }

        bool redraw = false;
        if (pressedButtons.nonempty()) {
            if (m_changeOnClick) {
                /* just hover */
                if (new_item != m_hoverItem) {
                    m_hoverItem = new_item;
                    redraw = true;
                }
                m_mousePressed = true;
            } else {
                // mouse button is pressed, so select that item
                // (but don't scroll, that would immediately select another item)
                if (m_hoverItem != nil) {
                    m_hoverItem = nil;
                    redraw = true;
                }
                if (new_item != m_currentItem && new_item != nil && !m_mouseBlocked) {
                    m_currentItem = new_item;
                    m_mousePressed = true;
                    sig_change.raise(m_currentItem);
                    m_mousePressed = false;
                    redraw = true;
                    m_pendingScroll = true;
                }
            }
        } else {
            // mouse not pressed, just hover
            m_mouseBlocked = false;
            if (m_changeOnClick) {
                if (m_mousePressed) {
                    if (new_item != m_currentItem && new_item != nil) {
                        m_currentItem = new_item;
                        m_mousePressed = true;
                        sig_change.raise(m_currentItem);
                        m_mousePressed = false;
                        redraw = true;
                        m_pendingScroll = true;
                    }
                }
                m_mousePressed = false;
            } else {
                if (new_item != m_hoverItem) {
                    m_hoverItem = new_item;
                    redraw = true;
                }
                if (m_pendingScroll) {
                    // we've selected a different item during last mouse-click, so we need to scroll
                    if (adjustPosition()) {
                        redraw = true;
                    }
                    m_pendingScroll = false;
                }
            }
        }
        if (redraw) {
            requestRedraw();
        }
        return true;
    } else {
        m_mouseBlocked = false;
        if (m_hoverItem != nil) {
            m_hoverItem = nil;
            requestRedraw();
        }
        return false;
    }
}

void
ui::widgets::IconBox::setCurrentItem(size_t nr)
{
    // ex UIIconBox::setCurrent
    bool redraw_needed = false;
    bool signal_needed = false;

    size_t total = getNumItems();
    if (total > 0) {
        // range check
        if (nr >= total) {
            nr = total-1;
        }

        // select the item
        if (m_currentItem != nr) {
            m_currentItem = nr;
            signal_needed = true;
            redraw_needed = true;
            adjustPosition();
        }
    } else {
        // degenerate case
        if (m_currentItem > 0 || m_leftX > 0) {
            redraw_needed = true;
        }
        m_currentItem = 0;
        m_leftX = 0;
        m_targetLeftX = 0;
        m_scrollSpeed = 0;
    }

    // redraw?
    if (redraw_needed) {
        requestRedraw();
    }
    if (signal_needed) {
        sig_change.raise(m_currentItem);
    }
}

size_t
ui::widgets::IconBox::getCurrentItem() const
{
    // ex UIIconBox::getCurrent
    return m_currentItem;
}

void
ui::widgets::IconBox::setChangeOnClick(bool enable)
{
    // ex UIIconBox::setChangeOnClick
    m_changeOnClick = enable;
}

void
ui::widgets::IconBox::handleStructureChange(size_t n)
{
    // ex UIIconBox::onStructureChange

    // Set current position
    setCurrentItem(n);

    // Additional adjustments
    size_t total = getNumItems();
    if (total > 0) {
        /* compute width before item */
        int before_width = 0;
        for (size_t i = 0; i < m_currentItem; ++i) {
            before_width += getItemWidth(i);
        }
        int item_width = getItemWidth(m_currentItem);
        int additional_width = (m_currentItem >= total-1 ? 0 : 10);

        if (before_width + item_width + additional_width <= getExtent().getWidth()) {
            /* everything fits into the widget */
            m_targetLeftX = 0;
        } else {
            /* does not fit */
            m_targetLeftX = before_width + item_width + additional_width - getExtent().getWidth();
        }
    }

    /* If mouse is pressed, block it to avoid that it immediately "clicks" again */
    if (m_mousePressed) {
        m_mouseBlocked = true;
    }

    requestRedraw();
}

/** Adjust position so that current item is completely visible.
    \retval true if redraw needed
    \retval false if no change */
bool
ui::widgets::IconBox::adjustPosition()
{
    // ex UIIconBox::adjustPosition
    // Make sure item is visible
    int this_item_width = getItemWidth(m_currentItem);
    int this_item_x = 0;
    for (size_t i = 0; i < m_currentItem; ++i) {
        this_item_x += getItemWidth(i);
    }

    int new_left_x = m_leftX;
    int this_item_right = this_item_x + this_item_width - getExtent().getWidth();

    if (this_item_x < m_leftX) {
        /* item is to the left of visible area */
        new_left_x = this_item_x;
    } else if (this_item_x + this_item_width > m_leftX + getExtent().getWidth()) {
        /* item is to the right of visible area */
        new_left_x = this_item_right;
    } else {
        // Item is within visible area
    }

    /* If new_left_x is at a boundary, and there is an item to the left/right of us, move that into visible range */
    if (new_left_x <= this_item_right + 20 && m_currentItem+1 != getNumItems()) {
        new_left_x = this_item_right + 20;
    }
    if (new_left_x >= this_item_x - 20 && m_currentItem != 0) {
        new_left_x -= 20;
        if (new_left_x < 0) {
            new_left_x = 0;
        }
    }

    /* commit change */
    m_pendingScroll = false;
    if (new_left_x != m_targetLeftX) {
        /* set new X */
        m_targetLeftX = new_left_x;

        /* cancel animation if we're not visible? */
        if (getExtent().getWidth() == 0) {
            m_leftX = m_targetLeftX;
            m_scrollSpeed = 0;
        }
        return true;
    } else {
        return false;
    }
}

void
ui::widgets::IconBox::handleTimer()
{
    /* Process pending movement */
    if (m_leftX != m_targetLeftX) {
        /* Adjust speed */
        int distance = std::abs(m_leftX - m_targetLeftX);
        if (distance > m_scrollSpeed*m_scrollSpeed)
            ++m_scrollSpeed;
        else if (m_scrollSpeed > 1)
            --m_scrollSpeed;

        /* Move. Post a mouse event when reaching the target,
           to update the Hover status */
        if (m_leftX < m_targetLeftX) {
            m_leftX += m_scrollSpeed;
            if (m_leftX >= m_targetLeftX) {
                m_leftX = m_targetLeftX;
                m_scrollSpeed = 0;
                m_root.postMouseEvent();
            }
        } else if (m_leftX > m_targetLeftX) {
            m_leftX -= m_scrollSpeed;
            if (m_leftX <= m_targetLeftX) {
                m_leftX = m_targetLeftX;
                m_scrollSpeed = 0;
                m_root.postMouseEvent();
            }
        }
        requestRedraw();
    }

    m_timer->setInterval(20);
}
