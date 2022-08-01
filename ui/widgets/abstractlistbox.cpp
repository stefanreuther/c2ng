/**
  *  \file ui/widgets/abstractlistbox.cpp
  */

#include <algorithm>
#include "ui/widgets/abstractlistbox.hpp"
#include "gfx/clipfilter.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"

ui::widgets::AbstractListbox::AbstractListbox()
    : ScrollableWidget(),
      m_flags(),
      m_currentItem(),
      m_topY(),
      m_mouseDown(false)
{ }

// ScrollableWidget virtuals:
int
ui::widgets::AbstractListbox::getPageTop() const
{
    return m_topY;
}

int
ui::widgets::AbstractListbox::getPageSize() const
{
    return std::max(0, getExtent().getHeight() - getHeaderHeight());
}

int
ui::widgets::AbstractListbox::getCursorTop() const
{
    return getRelativeItemPosition(m_currentItem).getTopY();
}

int
ui::widgets::AbstractListbox::getCursorSize() const
{
    return getItemHeight(m_currentItem);
}

int
ui::widgets::AbstractListbox::getTotalSize() const
{
    size_t n = getNumItems();
    if (n > 0) {
        return getRelativeItemPosition(n-1).getBottomY();
    } else {
        return 0;
    }
}

void
ui::widgets::AbstractListbox::setPageTop(int /*top*/)
{
    // FIXME: implement this
}

void
ui::widgets::AbstractListbox::scroll(Operation op)
{
    switch (op) {
     case LineUp:
        if (m_currentItem > 0) {
            setCurrentItem(m_currentItem-1, GoUp);
        }
        break;

     case LineDown:
        setCurrentItem(m_currentItem+1, GoDown);
        break;

     case PageUp: {
        size_t itemNr;
        gfx::Rectangle itemArea;
        if (getItemFromRelativePosition(getRelativeItemPosition(m_currentItem).getTopLeft() - gfx::Point(0, getPageSize()), itemNr, itemArea)) {
            setCurrentItem(itemNr, GoUp);
        } else {
            setCurrentItem(0, GoDown);
        }
        break;
     }

     case PageDown: {
        size_t itemNr;
        gfx::Rectangle itemArea;
        if (getItemFromRelativePosition(getRelativeItemPosition(m_currentItem).getTopLeft() + gfx::Point(0, getPageSize()), itemNr, itemArea)) {
            setCurrentItem(itemNr, GoDown);
        } else if (size_t nr = getNumItems()) {
            setCurrentItem(nr-1, GoUp);
        } else {
            // empty
        }
        break;
     }
    }
}

// Widget virtuals:
void
ui::widgets::AbstractListbox::draw(gfx::Canvas& can)
{
    // ex UIListbox::drawContent (totally different)
    gfx::Rectangle r = getExtent();

    // Draw header and footer
    const int headerHeight = getHeaderHeight();
    if (headerHeight != 0) {
        drawHeader(can, r.splitY(headerHeight));
    }
    const int footerHeight = getFooterHeight();
    if (footerHeight != 0) {
        drawFooter(can, r.splitBottomY(footerHeight));
    }

    // Draw content
    gfx::Rectangle itemArea;
    size_t itemNr;
    if (getItemFromRelativePosition(gfx::Point(0, m_topY), itemNr, itemArea)) {
        // Convert itemArea to absolute coordinates
        itemArea.moveBy(gfx::Point(r.getLeftX(), r.getTopY() - m_topY));

        // Draw top item
        {
            int drawY = itemArea.getBottomY() - r.getTopY();
            gfx::ClipFilter filter(can, r.splitY(drawY));
            drawItem(filter, itemArea, itemNr, getItemState(itemNr));
            ++itemNr;
        }

        // Draw following items
        size_t numItems = getNumItems();
        while (itemNr < numItems && r.getHeight() > 0) {
            int itemHeight = getItemHeight(itemNr);
            itemArea = gfx::Rectangle(itemArea.getLeftX(), itemArea.getBottomY(), itemArea.getWidth(), itemHeight);
            gfx::ClipFilter filter(can, r.splitY(itemHeight));
            drawItem(filter, itemArea, itemNr, getItemState(itemNr));
            ++itemNr;
        }
    }

    // Draw remaining background
    if (r.exists()) {
        gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
        drawBackground(ctx, r);
    }
}

void
ui::widgets::AbstractListbox::handleStateChange(State st, bool enable)
{
    // ex UIListbox::onStateChange
    if (st == FocusedState) {
        updateCurrentItem();
    }
    if (st == ActiveState && !enable) {
        m_mouseDown = false;
    }
    if (st == DisabledState) {
        requestRedraw();
    }
}

void
ui::widgets::AbstractListbox::defaultHandlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    makeVisible(getRelativeItemPosition(m_currentItem));

    // A scrollbar may need to be redrawn
    sig_change.raise();
}

bool
ui::widgets::AbstractListbox::defaultHandleKey(util::Key_t key, int /*prefix*/)
{
    // ex UIListbox::handleEvent, sort-of
    if (hasState(FocusedState) && !hasState(DisabledState)) {
        switch (key) {
         case util::Key_Up:
         case util::Key_WheelUp:
            requestActive();
            if (!hasFlag(Blocked)) {
                scroll(LineUp);
            }
            return true;
         case util::Key_Down:
         case util::Key_WheelDown:
            requestActive();
            if (!hasFlag(Blocked)) {
                scroll(LineDown);
            }
            return true;
         case util::Key_Home:
            requestActive();
            if (!hasFlag(Blocked)) {
                setCurrentItem(0, GoDown);
            }
            return true;
         case util::Key_End:
            requestActive();
            if (!hasFlag(Blocked)) {
                if (size_t nr = getNumItems()) {
                    setCurrentItem(nr-1, GoUp);
                }
            }
            return true;
         case util::Key_PgUp:
         case util::Key_PgUp + util::KeyMod_Shift:
            if (!hasFlag(NoPageKeys) || (key & util::KeyMod_Shift) != 0) {
                requestActive();
                if (!hasFlag(Blocked)) {
                    scroll(PageUp);
                }
                return true;
            }
            break;
         case util::Key_PgDn:
         case util::Key_PgDn + util::KeyMod_Shift:
            if (!hasFlag(NoPageKeys) || (key & util::KeyMod_Shift) != 0) {
                requestActive();
                if (!hasFlag(Blocked)) {
                    scroll(PageDown);
                }
                return true;
            }
            break;
         case '#':
         case '\\':
         case util::Key_Menu:      // why not
            if (hasFlag(KeyboardMenu)) {
                requestActive();
                if (!hasFlag(Blocked)) {
                    sig_menuRequest.raise(getRelativeItemPosition(m_currentItem).getTopLeft()
                                          + getRelativeToAbsoluteOffset()
                                          + gfx::Point(getExtent().getWidth() / 10, getItemHeight(m_currentItem)));
                }
                return true;
            }
            break;
         // c-pgup, c-pgdn, c-home, c-end
        }
        return false;
    } else {
        return false;
    }
}

bool
ui::widgets::AbstractListbox::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex UIListbox::handleEvent, sort-of
    // Do not handle anything if disabled
    if (hasState(DisabledState)) {
        return false;
    }

    // Check for mouse in widget
    // FIXME: PCC2 checks with fuzz factor slightly outside in second pass
    gfx::Rectangle r(getExtent());
    r.consumeY(getHeaderHeight());
    r.consumeBottomY(getFooterHeight());
    if (r.contains(pt)) {
        if (hasFlag(Blocked)) {
            // Widget is blocked. Request activation but do not do anything.
            requestActive();
            m_mouseDown = false;
            return true;
        } else if ((!pressedButtons.empty() || hasFlag(MenuBehaviour))) {
            // Mouse is being pressed or we have menu behaviour (=cursor follows mouse)
            requestActive();
            requestFocus();

            // Find item the mouse is in
            gfx::Rectangle itemArea;
            size_t itemNr;
            if (getItemFromRelativePosition(pt - r.getTopLeft() + gfx::Point(0, m_topY), itemNr, itemArea) && isItemAccessible(itemNr)) {
                // Cursor is over an accessible element; move it
                // FIXME: adjust fuzz
                // FIXME: right-click outside an item should still pop up the menu
                m_currentItem = itemNr;
                makeVisible(itemArea);
                requestRedraw();
                sig_change.raise();

                // Further processing
                if (hasFlag(MenuBehaviour)) {
                    // Cursor follows mouse, single click posts events
                    if (pressedButtons.empty()) {
                        if (m_mouseDown) {
                            sig_itemDoubleClick.raise(itemNr);
                        }
                        m_mouseDown = false;
                    } else {
                        m_mouseDown = true;
                    }
                } else if (pressedButtons.contains(RightButton)) {
                    // Right click
                    // FIXME: wait for release?
                    sig_menuRequest.raise(pt);
                } else if (pressedButtons.contains(DoubleClick)) {
                    // Double-click
                    sig_itemDoubleClick.raise(itemNr);
                    m_mouseDown = false;
                } else {
                    m_mouseDown = true;
                }
            } else {
                // Cursor over inaccessible element
                m_mouseDown = false;
            }
            return true;
        } else {
            // Mouse released in a non-menu: generate onItemClick, but continue normal processing.
            if (m_mouseDown && r.contains(pt)) {
                gfx::Rectangle itemArea;
                size_t itemNr;
                if (getItemFromRelativePosition(pt - r.getTopLeft() + gfx::Point(0, m_topY), itemNr, itemArea) && isItemAccessible(itemNr)) {
                    m_currentItem = itemNr;
                    makeVisible(itemArea);
                    requestActive();
                    requestFocus();
                    requestRedraw();
                    sig_change.raise();
                    sig_itemClick.raise(itemNr);
                    sig_itemClickAt.raise(itemNr, pt - r.getTopLeft());
                }
            }
            m_mouseDown = false;
            return true;
        }
    } else {
        m_mouseDown = false;
        return false;
    }
}

void
ui::widgets::AbstractListbox::setFlag(Flag flag, bool enable)
{
    if (enable) {
        m_flags += flag;
    } else {
        m_flags -= flag;
    }
}

bool
ui::widgets::AbstractListbox::hasFlag(Flag flag) const
{
    return m_flags.contains(flag);
}

void
ui::widgets::AbstractListbox::updateItem(size_t item)
{
    gfx::Rectangle pos(getRelativeItemPosition(item));
    pos.moveBy(getRelativeToAbsoluteOffset());

    gfx::Rectangle view(getExtent());
    view.consumeY(getHeaderHeight());
    view.consumeBottomY(getFooterHeight());
    pos.intersect(view);
    if (pos.exists()) {
        requestRedraw(pos);
    }
}

void
ui::widgets::AbstractListbox::updateCurrentItem()
{
    // ex UIListbox::updateCurrent (totally different)
    updateItem(m_currentItem);
}

gfx::Rectangle
ui::widgets::AbstractListbox::getRelativeItemPosition(size_t item) const
{
    gfx::Rectangle result(0, 0, getExtent().getWidth(), getItemHeight(item));
    if (hasFlag(EqualSizes)) {
        result.moveBy(gfx::Point(0, result.getHeight() * static_cast<int>(item)));
    } else {
        for (size_t i = 0; i < item; ++i) {
            result.moveBy(gfx::Point(0, getItemHeight(i)));
        }
    }
    return result;
}

gfx::Rectangle
ui::widgets::AbstractListbox::getAbsoluteItemPosition(size_t item) const
{
    gfx::Rectangle r = getRelativeItemPosition(item);
    r.moveBy(gfx::Point(0, -m_topY));
    r.moveBy(getExtent().getTopLeft()); // FIXME: ok?
    return r;
}

bool
ui::widgets::AbstractListbox::getItemFromRelativePosition(gfx::Point pt, size_t& item, gfx::Rectangle& area)
{
    if (pt.getX() < 0 || pt.getX() >= getExtent().getWidth() || pt.getY() < 0) {
        // Quick failure
        return false;
    } else {
        // Check content
        if (hasFlag(EqualSizes)) {
            // All the same size: just divide.
            int itemHeight = getItemHeight(0);
            if (itemHeight <= 0) {
                // Error
                return false;
            }

            size_t pos = pt.getY() / itemHeight;
            if (pos >= getNumItems()) {
                // Out of bounds
                return false;
            }

            // OK
            item = pos;
            area = gfx::Rectangle(0, static_cast<int>(item)*itemHeight, getExtent().getWidth(), itemHeight);
            return true;
        } else {
            // Check it
            int y = 0;
            for (size_t i = 0, n = getNumItems(); i < n; ++i) {
                int itemHeight = getItemHeight(i);
                if (pt.getY() >= y && pt.getY() < y + itemHeight) {
                    // Success
                    item = i;
                    area = gfx::Rectangle(0, y, getExtent().getWidth(), itemHeight);
                    return true;
                }
                y += itemHeight;
            }
            return false;
        }
    }
}

ui::widgets::AbstractListbox::ItemState
ui::widgets::AbstractListbox::getItemState(size_t nr)
{
    if (hasState(DisabledState)) {
        return DisabledItem;
    } else if (!isItemAccessible(nr)) {
        return DisabledItem;
    } else if (nr == m_currentItem) {
        if (getFocusState() == NoFocus) {
            return ActiveItem;
        } else {
            return FocusedItem;
        }
    } else {
        return PassiveItem;
    }
}

size_t
ui::widgets::AbstractListbox::getCurrentItem() const
{
    return m_currentItem;
}

void
ui::widgets::AbstractListbox::setCurrentItem(size_t nr, Direction dir)
{
    size_t count = getNumItems();
    if (count != 0) {
        // Fix trivial out-of-bounds
        if (nr >= count) {
            nr = count-1;
        }

        // Locate accessible item
        while (!isItemAccessible(nr)) {
            if (dir == GoUp) {
                if (nr == 0) {
                    // Cannot go further up. Fail.
                    return;
                }
                --nr;
            } else {
                ++nr;
                if (nr >= count) {
                    // Cannot go further down. Fail.
                    return;
                }
            }
        }

        // OK
        if (nr != m_currentItem) {
            m_currentItem = nr;

            gfx::Rectangle itemPos = getRelativeItemPosition(nr);

            // If this is the first selectable item, scroll up all the way to the top.
            // This is required to make unselectable headings visible.
            if (isFirstAccessibleItem(nr) && itemPos.getBottomY() <= getExtent().getHeight() - getHeaderHeight() - getFooterHeight()) {
                itemPos.include(gfx::Point());
            }

            makeVisible(itemPos);
            requestRedraw();          // FIXME: can we optimize to redraw only changed items?
            sig_change.raise();
        }
    }
}

void
ui::widgets::AbstractListbox::handleModelChange()
{
    size_t count = getNumItems();
    size_t nr = m_currentItem;
    if (count != 0) {
        if (nr >= count) {
            nr = count;
        }
        while (nr < count && !isItemAccessible(nr)) {
            ++nr;
        }
        if (nr >= count) {
            do {
                --nr;
            } while (nr > 0 && !isItemAccessible(nr));
        }
        m_currentItem = nr;
        makeVisible(getRelativeItemPosition(nr));
    } else {
        m_currentItem = 0;
        m_topY = 0;
    }
    requestRedraw();
    sig_change.raise();
}

bool
ui::widgets::AbstractListbox::isFirstAccessibleItem(size_t nr)
{
    while (nr > 0 && !isItemAccessible(nr-1)) {
        --nr;
    }
    return (nr == 0);
}

void
ui::widgets::AbstractListbox::makeVisible(const gfx::Rectangle& relativeArea)
{
    // ex UIScrollable::adjustPosition (totally new)
    const int topY = relativeArea.getTopY();
    const int h    = relativeArea.getHeight();
    const int totalHeight = getTotalSize();
    const int oldTopY = m_topY;

    const int availableHeight = getExtent().getHeight() - getHeaderHeight() - getFooterHeight();
    if (availableHeight <= 0) {
        // Don't change anything, it's not visible
    } else if (availableHeight >= totalHeight) {
        // We have enough space for all content. No need for scrolling.
        m_topY = 0;
    } else if (h > availableHeight) {
        // This item is larger than the widget. Moving it to top is the best we can do.
        m_topY = topY;
    } else {
        if (topY < m_topY) {
            // Widget is above top. Move up.
            m_topY = topY;
        } else if (topY + h > m_topY + availableHeight) {
            // Widget is below bottom. Move down.
            m_topY = topY + h - availableHeight;
        } else {
            // No change needed
        }

        // Do not scroll down to leave unoccupied space
        const int maxTopY = totalHeight - availableHeight;
        if (m_topY > maxTopY) {
            m_topY = maxTopY;
        }
    }

    if (oldTopY != m_topY) {
        requestRedraw();
    }
}

gfx::Point
ui::widgets::AbstractListbox::getRelativeToAbsoluteOffset()
{
    return getExtent().getTopLeft()
        - gfx::Point(0, m_topY)
        + gfx::Point(0, getHeaderHeight());
}
