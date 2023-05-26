/**
  *  \file ui/layout/axislayout.cpp
  *  \brief Class ui::layout::AxisLayout
  */

#include <cassert>
#include <algorithm>
#include "ui/layout/axislayout.hpp"
#include "util/math.hpp"

namespace {
    typedef std::vector<int>::size_type index_t;


    /*
     *  Sort predicate for slots
     *  (templated, to avoid having to make AxisLayout::Info public)
     */

    template<typename T>
    class Sorter {
     public:
        Sorter(const std::vector<T>& info)
            : m_info(info)
            { }
        bool operator()(index_t a, index_t b) const
            {
                // Flexible before fixed
                bool flexA = m_info[a].isFlexible;
                bool flexB = m_info[b].isFlexible;
                if (flexA != flexB) {
                    return flexB < flexA;
                }

                // Small before big
                int sizeA = m_info[a].prefSize;
                int sizeB = m_info[b].prefSize;
                return sizeA < sizeB;
            }

     private:
        const std::vector<T>& m_info;
    };


    int take(int& total, int amount)
    {
        int n = std::min(total, amount);
        total -= n;
        return n;
    }
}


/*
 *  AxisLayout
 */

ui::layout::AxisLayout::AxisLayout()
    : m_info()
{ }

void
ui::layout::AxisLayout::add(int prefSize, bool isFlexible, bool isIgnored)
{
    if (isIgnored) {
        m_info.push_back(Info(0, false, true));
    } else {
        m_info.push_back(Info(prefSize, isFlexible, isIgnored));
    }
}

void
ui::layout::AxisLayout::update(size_t index, int prefSize, bool isFlexible)
{
    if (index < m_info.size()) {
        m_info[index].prefSize = std::max(m_info[index].prefSize, prefSize);
        if (!isFlexible) {
            m_info[index].isFlexible = false;
        }
    }
}

int
ui::layout::AxisLayout::getTotalSize() const
{
    int total = 0;
    for (size_t i = 0, n = m_info.size(); i < n; ++i) {
        total += m_info[i].prefSize;
    }
    return total;
}

bool
ui::layout::AxisLayout::isFlexible() const
{
    for (size_t i = 0, n = m_info.size(); i < n; ++i) {
        if (m_info[i].isFlexible) {
            return true;
        }
    }
    return false;
}

bool
ui::layout::AxisLayout::isIgnored(size_t index) const
{
    return (index < m_info.size() && m_info[index].isIgnored);
}


std::vector<ui::layout::AxisLayout::Position>
ui::layout::AxisLayout::computeLayout(int space, int outer, int availableSize) const
{
    // ex ui/layout.cc:LayoutInfo::doLayout

    // Determine widget counts, sizes, and indexes
    std::vector<Position> result;
    std::vector<index_t> indexes;
    int totalRequested = 0;
    int numWidgets = 0;
    int numFlexible = 0;
    for (index_t i = 0; i < m_info.size(); ++i) {
        if (!m_info[i].isIgnored) {
            totalRequested += m_info[i].prefSize;
            ++numWidgets;
            if (m_info[i].isFlexible) {
                ++numFlexible;
            }
            indexes.push_back(i);
        }
        result.push_back(Position(0, 0));
    }

    // Determine sizes
    // - try to assign space to widgets
    int totalWidgetSizes = take(availableSize, totalRequested);
    // - assign space to outer margins (rationale: try to keep margins equal in case we are stacking multiple layouts)
    int totalOuter = take(availableSize, 2*outer);
    // - assign inter-widget spaces
    int totalSpace = take(availableSize, numWidgets > 1 ? (numWidgets-1)*space : 0);
    // - give remainder to widgets
    totalWidgetSizes += availableSize;

    // Adjust sizes, beginning with smallest flexible widget
    std::sort(indexes.begin(), indexes.end(), Sorter<Info>(m_info));
    int toGrow = totalWidgetSizes - totalRequested;
    for (size_t i = 0; i < indexes.size(); ++i) {
        index_t idx = indexes[i];
        assert(!m_info[idx].isIgnored);
        if (toGrow == 0) {
            // Just keep
            result[idx].size = m_info[idx].prefSize;
        } else if (toGrow < 0) {
            // Need to shrink
            int thisReduction;
            if (numFlexible > 0) {
                assert(m_info[idx].isFlexible);
                thisReduction = util::divideAndRoundUp(-toGrow, numFlexible--);
            } else {
                assert(!m_info[idx].isFlexible);
                thisReduction = util::divideAndRoundUp(-toGrow, int(indexes.size() - i));
            }
            thisReduction = std::min(m_info[idx].prefSize, thisReduction);
            result[idx].size = m_info[idx].prefSize - thisReduction;
            toGrow += thisReduction;
        } else {
            // Need to grow
            int thisGrowth;
            if (numFlexible > 0) {
                assert(m_info[idx].isFlexible);
                thisGrowth = util::divideAndRoundUp(toGrow, numFlexible--);
            } else {
                assert(!m_info[idx].isFlexible);
                thisGrowth = util::divideAndRoundUp(toGrow, int(indexes.size() - i));
            }
            result[idx].size = m_info[idx].prefSize + thisGrowth;
            toGrow -= thisGrowth;
        }
    }

    // Prepare positions and assign margins
    int pos = totalOuter / 2;
    for (size_t i = 0; i < result.size(); ++i) {
        if (!m_info[i].isIgnored) {
            result[i].position = pos;
            pos += result[i].size;
            if (--numWidgets > 0) {
                pos += take(totalSpace, util::divideAndRoundUp(totalSpace, numWidgets));
            }
        }
    }

    return result;
}
