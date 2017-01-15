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
            int minX  = info.getMinSize().getX();
            int minY  = info.getMinSize().getY();
            bool flexH = info.isGrowHorizontal();
            bool flexV = info.isGrowVertical();

            // override with required values
            int n;
            if (layout.getForcedCellWidth().get(n)) {
                prefX = minX = n;
                flexH = false;
            }
            if (layout.getForcedCellHeight().get(n)) {
                prefY = minY = n;
                flexV = false;
            }

            if (row == 0) {
                // first row, populate hinfo
                hinfo.pref_sizes.push_back(prefX);
                hinfo.min_sizes.push_back(minX);
                hinfo.flex_flags.push_back(flexH);
                hinfo.ignore_flags.push_back(false);
            } else {
                // not first row, update hinfo accordingly
                if (hinfo.pref_sizes[col] < prefX) {
                    hinfo.pref_sizes[col] = prefX;
                }
                if (hinfo.min_sizes[col] < minX) {
                    hinfo.min_sizes[col] = minX;
                }
                hinfo.flex_flags[col] |= flexH;
            }

            if (col == 0) {
                // first column, populate vinfo
                vinfo.pref_sizes.push_back(prefY);
                vinfo.min_sizes.push_back(minY);
                vinfo.flex_flags.push_back(flexV);
                vinfo.ignore_flags.push_back(false);
            } else {
                if (vinfo.pref_sizes.back() < prefY) {
                    vinfo.pref_sizes.back() = prefY;
                }
                if (vinfo.min_sizes.back() < minY) {
                    vinfo.min_sizes.back() = minY;
                }
                vinfo.flex_flags.back() |= flexV;
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
ui::layout::Grid::doLayout(Widget& container, gfx::Rectangle area)
{
    // ex UIGridLayout::doLayout
    AxisLayout h, v;
    computeGridLayoutInfo(*this, container, h, v, m_numColumns);

    if (!area.exists() || h.min_sizes.empty()) {
        return;
    }

    const std::vector<int>& hsizes = h.doLayout(m_space, m_outer, area.getWidth());
    const std::vector<int>& vsizes = v.doLayout(m_space, m_outer, area.getHeight());

    size_t row = 0;
    size_t col = 0;
    int x = h.used_outer;
    int y = v.used_outer;

    for (Widget* p = container.getFirstChild(); p != 0; p = p->getNextSibling()) {
        if (!p->getLayoutInfo().isIgnored()) {
            if (col >= hsizes.size() || row >= vsizes.size()) {
                // Cannot happen
                break;
            }

            p->setExtent(gfx::Rectangle(area.getLeftX() + x, area.getTopY() + y, hsizes[col], vsizes[row]));

            ++col;
            if (col >= m_numColumns) {
                x = h.used_outer;
                y += vsizes[row] + v.used_space;
                col = 0;
                ++row;
            } else {
                x += hsizes[col-1] + h.used_space;
            }
        }
    }
}

ui::layout::Info
ui::layout::Grid::getLayoutInfo(const Widget& container)
{
    // ex UIGridLayout::getLayoutInfo
    AxisLayout h, v;
    computeGridLayoutInfo(*this, container, h, v, m_numColumns);

    if (h.min_sizes.empty()) {
        return gfx::Point(2*m_outer, 2*m_outer);
    } else {
        int minX = 2 * m_outer + (h.min_sizes.size()-1) * m_space;
        int minY = 2 * m_outer + (v.min_sizes.size()-1) * m_space;
        int prefX = minX;
        int prefY = minY;
        bool flexH = false;
        bool flexV = false;

        for (std::vector<int>::size_type i = 0; i < h.min_sizes.size(); ++i) {
            minX += h.min_sizes[i];
            prefX += h.pref_sizes[i];
            flexH |= h.flex_flags[i];
        }
        for (std::vector<int>::size_type i = 0; i < v.min_sizes.size(); ++i) {
            minY += v.min_sizes[i];
            prefY += v.pref_sizes[i];
            flexV |= v.flex_flags[i];
        }

        return Info(gfx::Point(minX, minY), gfx::Point(prefX, prefY), Info::makeGrowthBehaviour(flexH, flexV, false));
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
