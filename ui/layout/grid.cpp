/**
  *  \file ui/layout/grid.cpp
  *  \brief Class ui::layout::Grid
  */

#include "ui/layout/grid.hpp"
#include "ui/layout/axislayout.hpp"

namespace {
    void computeGridLayoutInfo(const ui::layout::Grid& layout,
                               const ui::Widget& container,
                               ui::layout::AxisLayout& hinfo,
                               ui::layout::AxisLayout& vinfo,
                               const size_t numColumns)
    {
        size_t row = 0;
        size_t col = 0;

        for (ui::Widget* w = container.getFirstChild(); w != 0; w = w->getNextSibling()) {
            ui::layout::Info info = w->getLayoutInfo();
            if (info.isIgnored()) {
                continue;
            }

            // Copy values
            int prefX = info.getPreferredSize().getX();
            int prefY = info.getPreferredSize().getY();
            uint8_t flexH = info.isGrowHorizontal();
            uint8_t flexV = info.isGrowVertical();

            // override with required values
            int n;
            if (layout.getForcedCellWidth().get(n)) {
                prefX = n;
                flexH = false;
            }
            if (layout.getForcedCellHeight().get(n)) {
                prefY = n;
                flexV = false;
            }

            if (row == 0) {
                // first row, populate hinfo
                hinfo.add(prefX, flexH, false);
            } else {
                // not first row, update hinfo accordingly
                hinfo.update(col, prefX, flexH);
            }

            if (col == 0) {
                // first column, populate vinfo
                vinfo.add(prefY, flexV, false);
            } else {
                vinfo.update(row, prefY, flexV);
            }

            ++col;
            if (col >= numColumns) {
                col = 0;
                ++row;
            }
        }
    }
}


// Construct new grid.
ui::layout::Grid::Grid(size_t numColumns, int space, int outer)
    : Manager(),
      m_numColumns(numColumns),
      m_space(space),
      m_outer(outer),
      m_forcedCellWidth(),
      m_forcedCellHeight()
{
    // ex UIGridLayout::UIGridLayout
}

void
ui::layout::Grid::doLayout(Widget& container, gfx::Rectangle area) const
{
    // ex UIGridLayout::doLayout
    AxisLayout h, v;
    computeGridLayoutInfo(*this, container, h, v, m_numColumns);

    if (!area.exists() || h.empty()) {
        return;
    }

    const std::vector<AxisLayout::Position> hsizes = h.computeLayout(m_space, m_outer, area.getWidth());
    const std::vector<AxisLayout::Position> vsizes = v.computeLayout(m_space, m_outer, area.getHeight());

    size_t row = 0;
    size_t col = 0;
    for (Widget* p = container.getFirstChild(); p != 0; p = p->getNextSibling()) {
        if (!p->getLayoutInfo().isIgnored()) {
            if (col >= hsizes.size() || row >= vsizes.size()) {
                // Cannot happen
                break;
            }

            p->setExtent(gfx::Rectangle(area.getLeftX() + hsizes[col].position, area.getTopY() + vsizes[row].position, hsizes[col].size, vsizes[row].size));

            ++col;
            if (col >= m_numColumns) {
                col = 0;
                ++row;
            }
        }
    }
}

ui::layout::Info
ui::layout::Grid::getLayoutInfo(const Widget& container) const
{
    // ex UIGridLayout::getLayoutInfo
    AxisLayout h, v;
    computeGridLayoutInfo(*this, container, h, v, m_numColumns);

    if (h.empty()) {
        return gfx::Point(2*m_outer, 2*m_outer);
    } else {
        int prefX = 2 * m_outer + int(h.size()-1) * m_space + h.getTotalSize();
        int prefY = 2 * m_outer + int(v.size()-1) * m_space + v.getTotalSize();
        bool flexH = h.isFlexible();
        bool flexV = v.isFlexible();
        return Info(gfx::Point(prefX, prefY), Info::makeGrowthBehaviour(flexH, flexV, false));
    }
}

// Set forced cell size.
void
ui::layout::Grid::setForcedCellSize(afl::base::Optional<int> forcedCellWidth, afl::base::Optional<int> forcedCellHeight)
{
    // ex UIGridLayout::setForcedCellSize
    m_forcedCellWidth  = forcedCellWidth;
    m_forcedCellHeight = forcedCellHeight;
}
