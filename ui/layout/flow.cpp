/**
  *  \file ui/layout/flow.cpp
  */

#include <algorithm>
#include "ui/layout/flow.hpp"

void
ui::layout::Flow::doLayout(Widget& container, gfx::Rectangle area) const
{
    // ex UIFlowLayout::doLayout
    const Info containerInfo = getLayoutInfo(container);

    // Compute height of one line.
    const int lineHeight = (m_numLines <= 0
                            ? area.getHeight()
                            : (containerInfo.getPreferredSize().getY() - (m_verticalGap * (m_numLines-1))) / m_numLines);
    int xoffs = 0;
    int yoffs = 0;

    for (Widget* p = (m_rightJustified ? container.getLastChild() : container.getFirstChild());
         p != 0;
         p = (m_rightJustified ? p->getPreviousSibling() : p->getNextSibling()))
    {
        const Info info = p->getLayoutInfo();
        if (!info.isIgnored()) {
            // Allocate widget on this line; new line if it does not fit.
            const gfx::Point size = info.getPreferredSize();
            if (xoffs + size.getX() > area.getWidth()) {
                yoffs += lineHeight + m_verticalGap;
                xoffs = 0;
            }

            // "Invert" coordinates when working right-justified.
            if (m_rightJustified) {
                p->setExtent(gfx::Rectangle(area.getRightX() - xoffs - size.getX(),
                                            area.getBottomY() - yoffs - size.getY(),
                                            size.getX(),
                                            size.getY()));
            } else {
                p->setExtent(gfx::Rectangle(area.getLeftX() + xoffs,
                                            area.getTopY() + yoffs,
                                            size.getX(),
                                            size.getY()));
            }

            xoffs += size.getX() + m_horizontalGap;
        }
    }
}

ui::layout::Info
ui::layout::Flow::getLayoutInfo(const Widget& container) const
{
    // ex UIFlowLayout::getLayoutInfo

    // Compute maximum and total minimum/preferred sizes.
    int prefX = 0;
    int prefY = 0;
    int numWidgets = 0;
    int totalPrefWidth = 0;
    for (Widget* w = container.getFirstChild(); w != 0; w = w->getNextSibling()) {
        const Info info = w->getLayoutInfo();
        if (!info.isIgnored()) {
            ++numWidgets;
            prefX = std::max(prefX, info.getPreferredSize().getX());
            prefY = std::max(prefY, info.getPreferredSize().getY());
            totalPrefWidth += info.getPreferredSize().getX();
        }
    }

    // Height comes directly from configuration
    prefY = prefY * m_numLines + m_verticalGap * (m_numLines-1);

    // Width
    if (numWidgets > m_numLines) {
        // We have more widgets than lines (normal case), so we must pack multiple widgets onto one line.
        // Total width will include some gaps.
        totalPrefWidth += (numWidgets - m_numLines) * m_horizontalGap;

        if (m_numLines > 1) {
            // More than one line: distribute space evenly, and then pack one additional widget per line.
            prefX = totalPrefWidth / m_numLines + prefX;
        } else {
            // One line: just report the totals.
            prefX = totalPrefWidth;
        }
    }

    return Info(gfx::Point(prefX, prefY), Info::GrowHorizontal);
}
