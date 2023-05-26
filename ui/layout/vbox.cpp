/**
  *  \file ui/layout/vbox.cpp
  *  \brief Class ui::layout::VBox
  */

#include <algorithm>
#include "ui/layout/vbox.hpp"
#include "ui/layout/axislayout.hpp"

const ui::layout::VBox ui::layout::VBox::instance0(0);
const ui::layout::VBox ui::layout::VBox::instance5(5);


ui::layout::VBox::VBox(int space, int outer)
    : Manager(),
      space(space),
      outer(outer)
{ }

void
ui::layout::VBox::doLayout(Widget& container, gfx::Rectangle area) const
{
    // ex UIVBoxLayout::doLayout
    AxisLayout lay;
    for (Widget* w = container.getFirstChild(); w; w = w->getNextSibling()) {
        const Info i = w->getLayoutInfo();
        lay.add(i.getPreferredSize().getY(), i.isGrowVertical(), i.isIgnored());
    }
    if (area.getHeight() == 0) {
        return;
    }

    const std::vector<AxisLayout::Position> sizes = lay.computeLayout(space, outer, area.getHeight());
    size_t i = 0;

    const int size_x = area.getLeftX();
    const int size_y = area.getTopY();
    for (Widget* w = container.getFirstChild(); w; w = w->getNextSibling(), ++i) {
        if (!lay.isIgnored(i)) {
            w->setExtent(gfx::Rectangle(size_x, size_y + sizes[i].position, area.getWidth(), sizes[i].size));
        }
    }
}

ui::layout::Info
ui::layout::VBox::getLayoutInfo(const Widget& container) const
{
    // ex UIVBoxLayout::getLayoutInfo
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
            prefSize.addY(i.getPreferredSize().getY());
            prefSize.setX(std::max(prefSize.getX(), i.getPreferredSize().getX()));
        }
    }
    if (n != 0) {
        prefSize.addY((n-1) * space);
    }

    return Info(prefSize, Info::makeGrowthBehaviour(allH, anyV, allIgnore));
}
