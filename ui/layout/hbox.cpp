/**
  *  \file ui/layout/hbox.cpp
  */

#include <algorithm>
#include "ui/layout/hbox.hpp"
#include "ui/layout/axislayout.hpp"

ui::layout::HBox ui::layout::HBox::instance0(0);
ui::layout::HBox ui::layout::HBox::instance5(5);

// /** \class UIHBoxLayout
//     \brief Horizontal Box Layout

//     Widgets will be arranged horizontally, all the same height, from
//     left to right. This layout will completely cover the container
//     with widgets (subject to space/outer settings, of course). */

// /** Create a Horizontal Box layout.
//     \param space space to leave between widgets, in pixels.
//     \param outer space to leave at left/right side. */
ui::layout::HBox::HBox(int space, int outer)
    : space(space), outer(outer)
{
    // ex UIHBoxLayout::UIHBoxLayout
}

void
ui::layout::HBox::doLayout(Widget& container, gfx::Rectangle area)
{
    // ex UIHBoxLayout::doLayout
    AxisLayout lay;
    for (Widget* w = container.getFirstChild(); w; w = w->getNextSibling()) {
        Info i = w->getLayoutInfo();
        lay.pref_sizes.push_back(i.getPreferredSize().getX());
        lay.min_sizes.push_back(i.getMinSize().getX());
        lay.ignore_flags.push_back(i.isIgnored());
        lay.flex_flags.push_back(i.isGrowHorizontal());
    }
    if (area.getWidth() == 0) {
        return;
    }

    std::vector<int>& sizes = lay.doLayout(space, outer, area.getWidth());
    size_t i = 0;

    int size_x = area.getLeftX() + lay.used_outer;
    int size_y = area.getTopY();
    for (Widget* w = container.getFirstChild(); w; w = w->getNextSibling(), ++i) {
        if (!lay.ignore_flags[i]) {
            w->setExtent(gfx::Rectangle(size_x, size_y, sizes[i], area.getHeight()));
            size_x += sizes[i];
            size_x += lay.used_space;
        }
    }
}

ui::layout::Info
ui::layout::HBox::getLayoutInfo(const Widget& container)
{
    // ex UIHBoxLayout::getLayoutInfo
    gfx::Point minSize(2*outer, 0);
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
            minSize.addX(i.getMinSize().getX());
            minSize.setY(std::max(minSize.getY(), i.getMinSize().getY()));
            prefSize.addX(i.getPreferredSize().getX());
            prefSize.setY(std::max(prefSize.getY(), i.getPreferredSize().getY()));
        }
    }
    if (n != 0) {
        minSize.addX((n-1) * space);
        prefSize.addX((n-1) * space);
    }

    return Info(minSize, prefSize, Info::makeGrowthBehaviour(anyH, allV, allIgnore));
}
