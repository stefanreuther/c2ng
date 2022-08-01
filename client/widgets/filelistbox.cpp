/**
  *  \file client/widgets/filelistbox.cpp
  *
  *  FIXME: this widget is very similar to FolderListbox.
  *  - FileListbox: supports multiple columns which we really want for file requesters
  *  - FolderListbox: supports just one column but implements the regular AbstractListbox interface
  */

#include <algorithm>
#include "client/widgets/filelistbox.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "ui/widgets/abstractlistbox.hpp"

using ui::widgets::AbstractListbox;

client::widgets::FileListbox::FileListbox(int columns, int lines, ui::Root& root)
    : ScrollableWidget(),
      m_root(root),
      m_columns(columns),
      m_lines(lines),
      m_currentColumns(0),
      m_currentColumnWidth(1),
      m_currentLines(0),
      m_firstItem(0),
      m_currentItem(0),
      m_items(),
      m_icons(),
      conn_imageChange(root.provider().sig_imageChange.add(this, &FileListbox::onImageChange))
{ }

client::widgets::FileListbox::~FileListbox()
{ }

// FileListbox:
void
client::widgets::FileListbox::swapItems(Items_t& items)
{
    m_items.swap(items);
    m_currentItem = 0;
    m_firstItem = 0;
    sig_change.raise();
    requestRedraw();
}

const client::widgets::FileListbox::Item*
client::widgets::FileListbox::getItem(size_t n)
{
    if (n < m_items.size()) {
        return &m_items[n];
    } else {
        return 0;
    }
}

void
client::widgets::FileListbox::setCurrentIndex(size_t n)
{
    if (/*n >= 0 &&*/ n < m_items.size() && n != m_currentItem) {
        m_currentItem = n;
        updatePageTop();
        sig_change.raise();
        requestRedraw();
    }
}

// ScrollableWidget:
int
client::widgets::FileListbox::getPageTop() const
{
    return int(m_firstItem);
}

int
client::widgets::FileListbox::getPageSize() const
{
    return m_currentColumns * m_currentLines;
}

int
client::widgets::FileListbox::getCursorTop() const
{
    return int(m_currentItem);
}

int
client::widgets::FileListbox::getCursorSize() const
{
    return 1;
}

int
client::widgets::FileListbox::getTotalSize() const
{
    return int(m_items.size());
}

void
client::widgets::FileListbox::setPageTop(int top)
{
    if (top >= 0 && top < getTotalSize()) {
        size_t t = size_t(top);
        if (t != m_firstItem) {
            m_firstItem = t;
            updateCurrentItem();
            sig_change.raise();
            requestRedraw();
        }
    }
}

void
client::widgets::FileListbox::scroll(Operation op)
{
    switch (op) {
     case LineUp:   scrollUp(1);                                           break;
     case LineDown: scrollDown(1);                                         break;
     case PageUp:   scrollUp(size_t(m_currentColumns * m_currentLines));   break;
     case PageDown: scrollDown(size_t(m_currentColumns * m_currentLines)); break;
    }
}

// Widget:
void
client::widgets::FileListbox::draw(gfx::Canvas& can)
{
    // ex UIFileChooser::drawContent
    // Trigger image loading
    onImageChange();

    // Prepare
    afl::base::Ref<gfx::Font> normalFont = m_root.provider().getFont(gfx::FontRequest());
    afl::base::Ref<gfx::Font> boldFont   = m_root.provider().getFont(gfx::FontRequest().addWeight(1));
    int lineHeight = normalFont->getCellSize().getY();
    gfx::Rectangle area = getExtent();
    size_t itemIndex = getPageTop();

    // Draw
    for (int column = 0; column < m_currentColumns; ++column) {
        gfx::Rectangle columnArea = area.splitX(m_currentColumnWidth);
        for (int line = 0; line < m_currentLines; ++line) {
            gfx::Rectangle itemArea = columnArea.splitY(lineHeight);

            gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
            afl::base::Deleter del;
            if (const Item* pItem = getItem(itemIndex)) {
                // Indent
                if (pItem->indent != 0) {
                    drawBackground(ctx, itemArea.splitX(normalFont->getEmWidth() * pItem->indent));
                }

                // Determine item state
                // FIXME: logic copied from AbstractListbox
                AbstractListbox::ItemState itemState =
                    (hasState(DisabledState)
                     ? AbstractListbox::DisabledItem
                     // : !isItemAccessible(itemIndex)
                     // ? AbstractListbox::DisabledItem
                     : itemIndex == m_currentItem
                     ? (getFocusState() == NoFocus
                        ? AbstractListbox::ActiveItem
                        : AbstractListbox::FocusedItem)
                     : AbstractListbox::PassiveItem);
                prepareColorListItem(ctx, itemArea, itemState, m_root.colorScheme(), del);

                // Icon
                const int iconSize = 16;
                gfx::Rectangle iconArea = itemArea.splitX(iconSize);
                itemArea.consumeX(4);
                if (pItem->icon != iNone && m_icons.get() != 0) {
                    // The "files" image has rows of 2 icons of 16x16 each
                    int iconIndex = pItem->icon - 1;
                    gfx::Point anchor(iconSize * (iconIndex % 2), iconSize * (iconIndex / 2));
                    can.blit(iconArea.getTopLeft() - anchor, *m_icons, gfx::Rectangle(anchor, gfx::Point(iconSize, iconSize)));
                }

                // Text
                if (pItem->icon == iNone || pItem->icon == iFile) {
                    ctx.useFont(*normalFont);
                } else {
                    ctx.useFont(*boldFont);
                }
                outTextF(ctx, itemArea, pItem->name);
            } else {
                drawBackground(ctx, itemArea);
            }
            ++itemIndex;
        }
    }
}

void
client::widgets::FileListbox::handleStateChange(State st, bool /*enable*/)
{
    // ex UIFileChooser::onStateChange
    if (st == FocusedState) {
        requestRedraw();
    }
}

void
client::widgets::FileListbox::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    // ex UIFileChooser::onResize
    updateSize();
}

ui::layout::Info
client::widgets::FileListbox::getLayoutInfo() const
{
    // ex UIFileChooser::getLayoutInfo
    gfx::Point size = getPreferredCellSize().scaledBy(m_columns, m_lines);
    return ui::layout::Info(size, size, ui::layout::Info::GrowBoth);
}

bool
client::widgets::FileListbox::handleKey(util::Key_t key, int /*prefix*/)
{
    // FIXME: handle prefix?
    if (hasState(FocusedState) && !hasState(DisabledState)) {
        switch (key) {
         case util::Key_Up:
         case util::Key_WheelUp:
            requestActive();
            scroll(LineUp);
            return true;
         case util::Key_Down:
         case util::Key_WheelDown:
            requestActive();
            scroll(LineDown);
            return true;
         case util::Key_Home:
            requestActive();
            setCurrentIndex(0);
            return true;
         case util::Key_End:
            requestActive();
            if (!m_items.empty()) {
                setCurrentIndex(m_items.size() - 1);
            }
            return true;
         case util::Key_PgUp:
            requestActive();
            scroll(PageUp);
            return true;
         case util::Key_PgDn:
            requestActive();
            scroll(PageDown);
            return true;
         case util::Key_Right:
            requestActive();
            scrollDown(size_t(m_currentLines));
            return true;
         case util::Key_Left:
            requestActive();
            scrollUp(size_t(m_currentLines));
            return true;
        }
    }
    return false;
}

bool
client::widgets::FileListbox::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // FIXME: AbstractListbox manages m_mouseDown.
    const int lineHeight = m_root.provider().getFont(gfx::FontRequest())->getCellSize().getY();
    if (!hasState(DisabledState) && getExtent().contains(pt)) {
        requestActive();
        if (!pressedButtons.empty()) {
            requestFocus();

            int x = (pt.getX() - getExtent().getLeftX()) / m_currentColumnWidth;
            int y = (pt.getY() - getExtent().getTopY())  / lineHeight;
            if (x >= 0 && x < m_currentColumns && y >= 0 && y < m_currentLines) {
                size_t itemNr = m_firstItem + x*m_currentLines + y;
                setCurrentIndex(itemNr);
                if (m_currentItem == itemNr && pressedButtons.contains(DoubleClick)) {
                    sig_itemDoubleClick.raise(itemNr);
                }
            }
        }
        return true;
    } else {
        // FIXME: port this?
        // /* We're focused, and nobody is interested in this mouse
        //    click: check whether it's just above our first
        //    displayed item, or just below our last displayed item,
        //    to allow mouse scrolling. */
        // if (event.mouse.x >= extent.x && event.mouse.x <= extent.x + current_column_width
        //     && event.mouse.y < extent.y && event.mouse.y >= extent.y - font_heights[FONT_NORMAL])
        // {
        //     setSelectedWidget();
        //     scrollTo(getTop() - 1, -1);
        //     return true;
        // }
        // if (event.mouse.x >= extent.getRightX() - current_column_width
        //     && event.mouse.x < extent.getRightX()
        //     && event.mouse.y >= extent.getBottomY()
        //     && event.mouse.y < extent.getBottomY() + font_heights[FONT_NORMAL])
        // {
        //     setSelectedWidget();
        //     scrollTo(getTop() + getPageSize(), +1);
        //     return true;
        // }
        return false;
    }
}

gfx::Point
client::widgets::FileListbox::getPreferredCellSize() const
{
    return m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(15, 1);
}

void
client::widgets::FileListbox::onImageChange()
{
    if (m_icons.get() == 0) {
        m_icons = m_root.provider().getImage("files");
        if (m_icons.get() != 0) {
            requestRedraw();
        }
    }
}

void
client::widgets::FileListbox::updateSize()
{
    // ex UIFileChooser::updateSize
    gfx::Point prefSize = getPreferredCellSize();

    // Determine width
    int width = getExtent().getWidth();
    int numColumns = std::max(width / prefSize.getX(), 1);
    int columnWidth = (width + (numColumns-1)) / numColumns;
    m_currentColumns = numColumns;
    m_currentColumnWidth = columnWidth;

    // Determine height
    int height = getExtent().getHeight();
    m_currentLines = std::max(height / prefSize.getY(), 1);

    // Adjust position of scrollbar
    updatePageTop();
    sig_change.raise();
    requestRedraw();
}

void
client::widgets::FileListbox::updatePageTop()
{
    if (m_currentItem < m_firstItem) {
        m_firstItem = m_currentItem;
    }

    size_t pageSize = size_t(getPageSize());
    if (m_currentItem - m_firstItem >= pageSize) {
        m_firstItem = m_currentItem - (pageSize-1);
    }
}

void
client::widgets::FileListbox::updateCurrentItem()
{
    if (m_currentItem < m_firstItem) {
        m_currentItem = m_firstItem;
    }

    size_t pageSize = size_t(getPageSize());
    if (m_currentItem - m_firstItem >= pageSize) {
        m_currentItem = m_firstItem + pageSize-1;
    }
}

void
client::widgets::FileListbox::scrollUp(size_t amount)
{
    setCurrentIndex(m_currentItem - std::min(m_currentItem, amount));
}

void
client::widgets::FileListbox::scrollDown(size_t amount)
{
    if (!m_items.empty()) {
        setCurrentIndex(m_currentItem + std::min(m_items.size() - m_currentItem - 1, amount));
    }
}
