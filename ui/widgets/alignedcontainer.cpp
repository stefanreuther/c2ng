/**
  *  \file ui/widgets/alignedcontainer.cpp
  *  \brief Class ui::widgets::AlignedContainer
  */

#include "ui/widgets/alignedcontainer.hpp"

namespace {
    struct Pair {
        int pos;
        int size;
    };

    /** Compute layout.
        \param cs       [out] Result
        \param avail    [in] Number of pixels we have
        \param minsize  [in] Minimum size requested by client
        \param prefsize [in] Preferred size requested by client
        \param align    [in] Alignment parameter, 0=left, 1=center, 2=right
        \param margin   [in] Margin parameter, number of pixels on each side to leave free */
    void computeLayout(Pair& cs, const int avail, const int minsize, const int prefsize, const int align, const int margin)
    {
        if (avail >= prefsize + 2*margin) {
            /* We have more room than required to give this item its preferred size. */
            cs.pos = margin + (avail - 2*margin - prefsize)*align/2;
            cs.size = prefsize;
        } else if (avail >= minsize + 2*margin) {
            /* We do not have enough room to give it its preferred size, but we have
               more than its minimum. Thus, expand it to full size. */
            cs.pos = margin;
            cs.size = avail - 2*margin;
        } else if (avail >= minsize) {
            /* We have enough room to give it its minimum size when we reduce the margin */
            cs.pos = (avail - minsize) / 2;
            cs.size = minsize;
        } else {
            /* We're even smaller than its minimum size. Give it everything we have. */
            cs.pos = 0;
            cs.size = avail;
        }
    }
}

ui::widgets::AlignedContainer::AlignedContainer(Widget& content, gfx::HorizontalAlignment alignX, gfx::VerticalAlignment alignY)
    : Widget(),
      m_alignX(alignX),
      m_alignY(alignY),
      m_padX(10),
      m_padY(10)
{
    // ex UIAlignedWidget::UIAlignedWidget
    addChild(content, 0);
}

void
ui::widgets::AlignedContainer::setPadding(int padX, int padY)
{
    m_padX = padX;
    m_padY = padY;
}

void
ui::widgets::AlignedContainer::draw(gfx::Canvas& can)
{
    defaultDrawChildren(can);
}

void
ui::widgets::AlignedContainer::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::widgets::AlignedContainer::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
ui::widgets::AlignedContainer::handleChildAdded(Widget& /*child*/)
{
    // We control widget addition/removal; callback not evaluated
}

void
ui::widgets::AlignedContainer::handleChildRemove(Widget& /*child*/)
{
    // We control widget addition/removal; callback not evaluated
}

void
ui::widgets::AlignedContainer::handlePositionChange()
{
    // ex UIAlignedWidget::onResize
    if (Widget* p = getFirstChild()) {
        const gfx::Rectangle& r = getExtent();
        ui::layout::Info info = p->getLayoutInfo();

        Pair xs, ys;
        computeLayout(xs, r.getWidth(),  info.getMinSize().getX(), info.getPreferredSize().getX(), m_alignX, m_padX);
        computeLayout(ys, r.getHeight(), info.getMinSize().getY(), info.getPreferredSize().getY(), m_alignY, m_padY);
        p->setExtent(gfx::Rectangle(r.getLeftX() + xs.pos,   r.getTopY() + ys.pos, xs.size, ys.size));
    }
}

void
ui::widgets::AlignedContainer::handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
{
    // We control position changes; callback not evaluated.
}

ui::layout::Info
ui::widgets::AlignedContainer::getLayoutInfo() const
{
    // ex UIAlignedWidget::getLayoutInfo
    if (Widget* p = getFirstChild()) {
        ui::layout::Info info = p->getLayoutInfo();
        return ui::layout::Info(info.getMinSize()       + gfx::Point(2*m_padX, 2*m_padY),
                                info.getPreferredSize() + gfx::Point(2*m_padX, 2*m_padY),
                                info.getGrowthBehaviour());
    } else {
        return ui::layout::Info(gfx::Point(),
                                gfx::Point(),
                                ui::layout::Info::GrowBoth);
    }
}

bool
ui::widgets::AlignedContainer::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::widgets::AlignedContainer::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}
