/**
  *  \file ui/widgets/icongrid.cpp
  *  \brief Class ui::widgets::IconGrid
  */

#include <algorithm>
#include "ui/widgets/icongrid.hpp"
#include "gfx/clipfilter.hpp"
#include "gfx/complex.hpp"
#include "util/math.hpp"
#include "util/updater.hpp"

using util::Updater;

ui::widgets::IconGrid::IconGrid(gfx::Engine& engine, gfx::Point cellSize, int widthInCells, int heightInCells)
    : ScrollableWidget(),
      m_cellSize(cellSize),
      m_widthInCells(widthInCells),
      m_heightInCells(heightInCells),
      m_currentLine(0),
      m_currentColumn(0),
      m_pageTop(0),
      m_padding(0),
      m_cursorOn(true),
      m_icons(),
      m_itemInaccessible(),
      m_timer(engine.createTimer())
{
    // ex UIIconGrid::UIIconGrid
    m_timer->sig_fire.add(this, &IconGrid::onTimer);
}

ui::widgets::IconGrid::~IconGrid()
{ }

void
ui::widgets::IconGrid::setPadding(int pad)
{
    m_padding = pad;
}

void
ui::widgets::IconGrid::setIcon(int x, int y, ui::icons::Icon* pIcon)
{
    const int oldSize = getTotalSize();
    const size_t pos = (y*m_widthInCells) + x;
    while (m_icons.size() <= pos) {
        m_icons.push_back(0);
    }

    if (Updater().set(m_icons[pos], pIcon)) {
        if (oldSize != getTotalSize()) {
            sig_change.raise();
        }
        requestRedraw();
    }
}

void
ui::widgets::IconGrid::addIcon(ui::icons::Icon* pIcon)
{
    const int oldSize = getTotalSize();
    m_icons.push_back(pIcon);
    if (oldSize != getTotalSize()) {
        sig_change.raise();
    }
    requestRedraw();
}

void
ui::widgets::IconGrid::setCurrentItem(size_t index)
{
    // ex UIIconGrid::setCurrentPos
    setCurrentItem(int(index % m_widthInCells), int(index / m_widthInCells));
}

void
ui::widgets::IconGrid::setItemAccessible(size_t index, bool flag)
{
    // We store inverse values so the default is "accessible"
    if (m_itemInaccessible.size() <= index) {
        m_itemInaccessible.resize(index+1);
    }
    m_itemInaccessible[index] = !flag;
}

void
ui::widgets::IconGrid::setItemAccessible(int x, int y, bool flag)
{
    if (x >= 0 && y >= 0) {
        setItemAccessible(static_cast<size_t>(y * m_widthInCells + x), flag);
    }
}

void
ui::widgets::IconGrid::setCurrentItem(int x, int y)
{
    // ex UIIconGrid::setCurrentPos
    // Force Y in range
    // - at least 0
    // - at most max line
    const int effY = std::max(0, std::min(y, getTotalSize()-1));

    // Force X in range
    // - at least 0
    // - at most width, on last line at most on last icon if uneven
    const int effX = std::max(0, std::min(x, std::min(int(m_icons.size()) - effY * m_widthInCells, m_widthInCells)-1));

    // Check for accessibility, change
    if (isItemAccessible(x, y) && (m_currentColumn != effX || m_currentLine != effY)) {
        gfx::Rectangle dirty = getCellPosition(m_currentColumn, m_currentLine);
        m_currentColumn = effX;
        m_currentLine = effY;

        // Check need to scroll
        bool mustScroll;
        if (m_currentLine < m_pageTop) {
            m_pageTop = m_currentLine;
            mustScroll = true;
        } else if (m_currentLine >= m_pageTop + getPageSize()) {
            m_pageTop = std::max(0, m_currentLine - getPageSize() + 1);
            mustScroll = true;
        } else {
            mustScroll = false;
        }

        // Redraw/update
        if (mustScroll) {
            // Signal changes and redraw all
            sig_change.raise();
            requestRedraw();
        } else {
            // Redraw only changed area
            dirty.include(getCellPosition(m_currentColumn, m_currentLine));
            dirty.intersect(getExtent());
            requestRedraw(dirty);
        }

        resetCursorBlink();
        sig_itemSelected.raise();
    }
}

size_t
ui::widgets::IconGrid::getCurrentItem() const
{
    return size_t(m_currentLine*m_widthInCells + m_currentColumn);
}

int
ui::widgets::IconGrid::getCurrentLine() const
{
    return m_currentLine;
}

int
ui::widgets::IconGrid::getCurrentColumn() const
{
    return m_currentColumn;
}

// ScrollableWidget:
int
ui::widgets::IconGrid::getPageTop() const
{
    return m_pageTop;
}

int
ui::widgets::IconGrid::getPageSize() const
{
    return getExtent().getHeight() / (m_cellSize.getY() + 1 + 2*m_padding);
}

int
ui::widgets::IconGrid::getTotalSize() const
{
    return util::divideAndRoundUp(int(m_icons.size()), m_widthInCells);
}

void
ui::widgets::IconGrid::setPageTop(int top)
{
    int newTop = std::max(0, std::min(getTotalSize()-getPageSize(), top));
    if (Updater().set(m_pageTop, newTop)) {
        sig_change.raise();
        requestRedraw();
    }
}

void
ui::widgets::IconGrid::scroll(Operation op)
{
    switch (op) {
     case LineUp:
        handleVerticalScroll(-1, -1);
        break;
     case LineDown:
        handleVerticalScroll(+1, +1);
        break;
     case PageUp:
        handleVerticalScroll(-getPageSize(), -1);
        break;
     case PageDown:
        handleVerticalScroll(getPageSize(), +1);
        break;
    }
}

// SimpleWidget:
void
ui::widgets::IconGrid::draw(gfx::Canvas& can)
{
    // UIIconGrid::drawContent (totally new)

    // Make sure we don't accidentally draw outside our area
    gfx::Rectangle area = getExtent();
    gfx::ClipFilter filter(can, area);
    gfx::Context<SkinColor::Color> ctx(filter, getColorScheme());

    // Background for everything
    drawBackground(ctx, area);

    // Skip first line
    area.consumeY(1);

    // Draw icons
    size_t index = size_t(m_widthInCells * m_pageTop);
    while (area.exists() && (index < m_icons.size())) {
        gfx::Rectangle lineArea = area.splitY(m_cellSize.getY() + 2*m_padding);
        lineArea.consumeX(1);
        for (int i = 0; i < m_widthInCells; ++i) {
            // Focused?
            bool isFocused = (index == size_t(m_widthInCells*m_currentLine + m_currentColumn));

            // Draw cell
            gfx::Rectangle cellArea = lineArea.splitX(m_cellSize.getX() + 2*m_padding);
            if (index < m_icons.size() && m_icons[index] != 0) {
                ButtonFlags_t flags;
                if (isFocused) {
                    flags += FocusedButton;
                }

                gfx::Rectangle innerArea = cellArea;
                innerArea.grow(-m_padding, -m_padding);

                m_icons[index]->draw(ctx, innerArea, flags);
            }

            // Draw focus frame
            if (isFocused && (m_cursorOn || getFocusState() != PrimaryFocus)) {
                cellArea.grow(1, 1);
                ctx.setLineThickness(1);
                ctx.setColor(SkinColor::Static);
                drawRectangle(ctx, cellArea);
            }

            ++index;
            lineArea.consumeX(1);
        }
        area.consumeY(1);
    }
}

void
ui::widgets::IconGrid::handleStateChange(State st, bool enable)
{
    if (st == FocusedState && enable) {
        resetCursorBlink();
    }
}

void
ui::widgets::IconGrid::handlePositionChange()
{
    m_pageTop = std::max(0, m_currentLine - getPageSize() + 1);
}

ui::layout::Info
ui::widgets::IconGrid::getLayoutInfo() const
{
    // ex UIIconGrid::getLayoutInfo (totally different!)
    const gfx::Point prefSize = (m_cellSize + gfx::Point(1+2*m_padding, 1+2*m_padding)).scaledBy(m_widthInCells, m_heightInCells) + gfx::Point(1, 1);
    return ui::layout::Info(prefSize, ui::layout::Info::GrowVertical);
}

bool
ui::widgets::IconGrid::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex UIIconGrid::handleEvent (part)
    if (hasState(FocusedState)) {
        size_t n;
        switch (key) {
         case util::Key_Right:
            n = getCurrentItem() + 1;
            while (n < m_icons.size() && !isItemAccessible(n)) {
                ++n;
            }
            if (isItemAccessible(n)) {
                requestActive();
                setCurrentItem(n);
                return true;
            } else {
                return false;
            }
         case util::Key_Left:
            requestActive();
            n = getCurrentItem();
            while (n > 0) {
                --n;
                if (isItemAccessible(n)) {
                    break;
                }
            }
            if (isItemAccessible(n)) {
                requestActive();
                setCurrentItem(n);
                return true;
            } else {
                return false;
            }
         case util::Key_Up:
            return handleVerticalScroll(-1, -1);
         case util::Key_Down:
            return handleVerticalScroll(+1, +1);
         case util::Key_PgUp:
            return handleVerticalScroll(-getPageSize(), -1);
         case util::Key_PgDn:
            return handleVerticalScroll(getPageSize(), +1);
         case util::Key_Home:
            requestActive();
            setCurrentItem(0);
            return true;
         case util::Key_End:
            if (!m_icons.empty()) {
                requestActive();
                setCurrentItem(m_icons.size() - 1);
            }
            return true;
        }
    }
    return false;
}

bool
ui::widgets::IconGrid::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex UIIconGrid::handleEvent (part)
    if (!pressedButtons.empty() && getExtent().contains(pt)) {
        // Focus ourselves
        requestFocus();

        // Move selected item
        int nx = (pt.getX() - getExtent().getLeftX()) / (m_cellSize.getX() + 1);
        int ny = (pt.getY() - getExtent().getTopY())  / (m_cellSize.getY() + 1);

        size_t pos = size_t(nx + ny*m_widthInCells);
        if (nx < m_widthInCells && pos < m_icons.size()) {
            requestActive();
            setCurrentItem(nx, ny);
        }

        if (pressedButtons.contains(DoubleClick)) {
            sig_doubleClick.raise();
        }

        return true;
    } else {
        return false;
    }
}

gfx::Rectangle
ui::widgets::IconGrid::getCellPosition(int x, int y) const
{
    return gfx::Rectangle(getExtent().getLeftX() + (m_cellSize.getX() + 1 + 2*m_padding) * x,
                          getExtent().getTopY()  + (m_cellSize.getY() + 1 + 2*m_padding) * (y - m_pageTop),
                          m_cellSize.getX() + 2 + 2*m_padding,
                          m_cellSize.getY() + 2 + 2*m_padding);
}

bool
ui::widgets::IconGrid::handleVerticalScroll(int delta, int adjust)
{
    const bool isMultiline = m_icons.size() > size_t(m_widthInCells);
    if (isMultiline) {
        int newLine = m_currentLine + delta;
        int numLines = getTotalSize();
        while (newLine >= 0 && newLine <= numLines && !isItemAccessible(m_currentColumn, newLine)) {
            newLine += adjust;
        }
        if (isItemAccessible(m_currentColumn, newLine)) {
            requestActive();
            setCurrentItem(m_currentColumn, newLine);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool
ui::widgets::IconGrid::isItemAccessible(size_t pos) const
{
    return (pos < m_icons.size())
        && !(pos < m_itemInaccessible.size()
             && m_itemInaccessible[pos]);
}

bool
ui::widgets::IconGrid::isItemAccessible(int x, int y) const
{
    return x >= 0
        && y >= 0
        && isItemAccessible(static_cast<size_t>(y * m_widthInCells + x));
}

void
ui::widgets::IconGrid::onTimer()
{
    if (hasState(FocusedState)) {
        m_cursorOn = !m_cursorOn;
        requestRedraw(getCellPosition(m_currentColumn, m_currentLine));
        m_timer->setInterval(CURSOR_BLINK_INTERVAL);
    }
}

void
ui::widgets::IconGrid::resetCursorBlink()
{
    m_timer->setInterval(CURSOR_BLINK_INTERVAL);
    if (Updater().set(m_cursorOn, true)) {
        requestRedraw(getCellPosition(m_currentColumn, m_currentLine));
    }
}
