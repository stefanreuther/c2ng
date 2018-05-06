/**
  *  \file ui/layout/vbox.cpp
  */

#include <algorithm>
#include "ui/layout/vbox.hpp"
#include "ui/layout/axislayout.hpp"

ui::layout::VBox ui::layout::VBox::instance0(0);
ui::layout::VBox ui::layout::VBox::instance5(5);

// /** Vertical Box Layout

//     Widgets will be arranged vertically, all the same width, below
//     each other from top to bottom. This layout will completely cover
//     the container with widgets (subject to space/outer settings, of
//     course). */

// /** Create a Vertical Box layout.
//     \param space space to leave between widgets, in pixels.
//     \param outer space to leave at top/bottom */
ui::layout::VBox::VBox(int space, int outer)
    : Manager(),
      space(space),
      outer(outer)
{ }

void
ui::layout::VBox::doLayout(Widget& container, gfx::Rectangle area)
{
    // ex UIVBoxLayout::doLayout
    AxisLayout lay;
    for (Widget* w = container.getFirstChild(); w; w = w->getNextSibling()) {
        Info i = w->getLayoutInfo();
        lay.pref_sizes.push_back(i.getPreferredSize().getY());
        lay.min_sizes.push_back(i.getMinSize().getY());
        lay.ignore_flags.push_back(i.isIgnored());
        lay.flex_flags.push_back(i.isGrowVertical());
    }
    if (area.getHeight() == 0) {
        return;
    }

    std::vector<int>& sizes = lay.doLayout(space, outer, area.getHeight());
    size_t i = 0;

    int size_x = area.getLeftX();
    int size_y = area.getTopY() + lay.used_outer;
    for (Widget* w = container.getFirstChild(); w; w = w->getNextSibling(), ++i) {
        if (!lay.ignore_flags[i]) {
            w->setExtent(gfx::Rectangle(size_x, size_y, area.getWidth(), sizes[i]));
            size_y += sizes[i];
            size_y += lay.used_space;
        }
    }
}

ui::layout::Info
ui::layout::VBox::getLayoutInfo(const Widget& container)
{
    // ex UIVBoxLayout::getLayoutInfo
    gfx::Point minSize(0, 2*outer);
    gfx::Point prefSize(0, 2*outer);

    bool anyV = false;
    bool allH = true;
    bool allIgnore = true;

    int n = 0;
    for (Widget* w = container.getFirstChild(); w != 0; w = w->getNextSibling()) {
        // Fetch child info
        Info i = w->getLayoutInfo();

        // Update flags
        anyV |= i.isGrowVertical();
        allH &= i.isGrowHorizontal();
        allIgnore &= i.isIgnored();

        // Update sizes
        if (!i.isIgnored()) {
            ++n;
            minSize.addY(i.getMinSize().getY());
            minSize.setX(std::max(minSize.getX(), i.getMinSize().getX()));
            prefSize.addY(i.getPreferredSize().getY());
            prefSize.setX(std::max(prefSize.getX(), i.getPreferredSize().getX()));
        }
    }
    if (n != 0) {
        minSize.addY((n-1) * space);
        prefSize.addY((n-1) * space);
    }

    return Info(minSize, prefSize, Info::makeGrowthBehaviour(allH, anyV, allIgnore));
}
