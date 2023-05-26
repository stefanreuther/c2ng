/**
  *  \file ui/layout/hbox.cpp
  *  \brief Class ui::layout::HBox
  */

#include <algorithm>
#include "ui/layout/hbox.hpp"
#include "ui/layout/axislayout.hpp"

const ui::layout::HBox ui::layout::HBox::instance0(0);
const ui::layout::HBox ui::layout::HBox::instance5(5);


ui::layout::HBox::HBox(int space, int outer)
    : space(space), outer(outer)
{
    // ex UIHBoxLayout::UIHBoxLayout
}

void
ui::layout::HBox::doLayout(Widget& container, gfx::Rectangle area) const
{
    // ex UIHBoxLayout::doLayout
    AxisLayout lay;
    for (Widget* w = container.getFirstChild(); w; w = w->getNextSibling()) {
        const Info i = w->getLayoutInfo();
        lay.add(i.getPreferredSize().getX(), i.isGrowHorizontal(), i.isIgnored());
    }
    if (area.getWidth() == 0) {
        return;
    }

    const std::vector<AxisLayout::Position> sizes = lay.computeLayout(space, outer, area.getWidth());
    size_t i = 0;

    const int size_x = area.getLeftX();
    const int size_y = area.getTopY();
    for (Widget* w = container.getFirstChild(); w; w = w->getNextSibling(), ++i) {
        if (!lay.isIgnored(i)) {
            w->setExtent(gfx::Rectangle(size_x + sizes[i].position, size_y, sizes[i].size, area.getHeight()));
        }
    }
}

ui::layout::Info
ui::layout::HBox::getLayoutInfo(const Widget& container) const
{
    // ex UIHBoxLayout::getLayoutInfo
    gfx::Point prefSize(2*outer, 0);

    /* A HBox is...
       ...FlexibleH if it has at least one component which is FlexibleH
          (if we have to enlarge the group, we can enlarge this component
          to compensate)
       ...FlexibleV if all components are FlexibleV (if one of them is not,
          we try not to squeeze it around)
       ...NoLayout if all components are NoLayout (only in this case we can
          ignore this group for layout) */
    bool anyH = false;
    bool allV = true;
    bool allIgnore = true;

    int n = 0;
    for (Widget* w = container.getFirstChild(); w != 0; w = w->getNextSibling()) {
        // Fetch child info
        Info i = w->getLayoutInfo();

        // Update flags
        anyH |= i.isGrowHorizontal();
        allV &= i.isGrowVertical();
        allIgnore &= i.isIgnored();

        // Update sizes
        if (!i.isIgnored()) {
            ++n;
            prefSize.addX(i.getPreferredSize().getX());
            prefSize.setY(std::max(prefSize.getY(), i.getPreferredSize().getY()));
        }
    }
    if (n != 0) {
        prefSize.addX((n-1) * space);
    }

    return Info(prefSize, Info::makeGrowthBehaviour(anyH, allV, allIgnore));
}
