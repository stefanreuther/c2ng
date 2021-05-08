/**
  *  \file ui/widgets/menuframe.cpp
  *  \brief Class ui::widgets::MenuFrame
  *
  *  FIXME: this widget still looks pretty dull.
  */

#include "ui/widgets/menuframe.hpp"
#include "afl/base/signalconnection.hpp"
#include "ui/draw.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace {
    // Animation interval. PCC2 used "one per 50 Hz tick".
    const int INTERVAL_MS = 20;
}

// Constructor.
ui::widgets::MenuFrame::MenuFrame(ui::layout::Manager& mgr, Root& root, EventLoop& loop)
    : LayoutableGroup(mgr),
      m_root(root),
      m_loop(loop),
      m_timer(root.engine().createTimer()),
      m_colorScheme(ui::GRAY_COLOR_SET, root.colorScheme())
{
    // ex MenuFrame::MenuFrame
    setState(ModalState, true);
    setColorScheme(m_colorScheme);
    m_timer->sig_fire.add(this, &MenuFrame::onTick);
}

gfx::Rectangle
ui::widgets::MenuFrame::transformSize(gfx::Rectangle size, Transformation kind) const
{
    int delta = (kind == OuterToInner ? -2 : +2);
    size.grow(delta, delta);
    return size;
}

void
ui::widgets::MenuFrame::draw(gfx::Canvas& can)
{
    // ex MenuFrame::drawContent
    // Frame
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    gfx::Rectangle r = getExtent();
    drawFrameUp(ctx, r);
    r.grow(-1, -1);
    drawFrameDown(ctx, r);

    // Content
    defaultDrawChildren(can);
}

void
ui::widgets::MenuFrame::handleStateChange(State /*st*/, bool /*enable*/)
{ }

bool
ui::widgets::MenuFrame::handleKey(util::Key_t key, int prefix)
{
    // ex MenuFrame::handleEvent (part)
    if (key == util::Key_Escape) {
        m_loop.stop(0);
        return true;
    } else if (key == util::Key_Return) {
        m_loop.stop(1);
        return true;
    } else {
        return defaultHandleKey(key, prefix);
    }
}

bool
ui::widgets::MenuFrame::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex MenuFrame::handleEvent (part)
    if (!pressedButtons.empty() && !getExtent().contains(pt)) {
        m_loop.stop(0);
        m_root.postMouseEvent();
        return true;
    } else {
        return defaultHandleMouse(pt, pressedButtons);
    }
}

void
ui::widgets::MenuFrame::animate(gfx::Rectangle startingSize)
{
    setExtent(startingSize);
    onTick();
}

bool
ui::widgets::MenuFrame::doMenu(AbstractListbox& list, gfx::Point anchor)
{
    // ex UIListbox::doStandardMenu (sort-of)
    // FIXME: determine whether we want a scrollbar
    // Configure the list box
    list.setFlag(AbstractListbox::MenuBehaviour, true);
    afl::base::SignalConnection conn(list.sig_itemDoubleClick.add(this, &MenuFrame::onMenuItemClick));

    // Add to MenuFrame and operate
    add(list);
    animate(gfx::Rectangle(anchor, gfx::Point()));
    m_root.add(*this);
    bool result = m_loop.run() != 0;

    // Clean up
    m_root.remove(*this);
    removeChild(list);
    conn.disconnect();
    return result;
}

void
ui::widgets::MenuFrame::onTick()
{
    // Determine current and target size
    gfx::Rectangle currentPosition = getExtent();
    gfx::Point targetSize = getLayoutInfo().getPreferredSize();

    // Update size
    int growth = m_root.provider().getFont(gfx::FontRequest())->getTextHeight("Tp");
    gfx::Rectangle newPosition(currentPosition.getLeftX(),
                               currentPosition.getTopY(),
                               std::min(currentPosition.getWidth()  + 5*growth, targetSize.getX()),
                               std::min(currentPosition.getHeight() +   growth, targetSize.getY()));

    // Force size into screen
    newPosition.moveIntoRectangle(m_root.getExtent());
    if (newPosition != currentPosition) {
        setExtent(newPosition);
        m_timer->setInterval(INTERVAL_MS);
    }
}

void
ui::widgets::MenuFrame::onMenuItemClick()
{
    m_loop.stop(1);
}
