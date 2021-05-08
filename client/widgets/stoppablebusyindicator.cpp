/**
  *  \file client/widgets/stoppablebusyindicator.cpp
  *  \brief Class client::widgets::StoppableBusyIndicator
  */

#include <algorithm>
#include "client/widgets/stoppablebusyindicator.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"

namespace {
    const int HORIZONTAL_BORDER = 10;
    const int VERTICAL_BORDER = 10;
    const int GAP = 5;
}

client::widgets::StoppableBusyIndicator::StoppableBusyIndicator(ui::Root& root, afl::string::Translator& tx)
    : Widget(),
      m_root(root),
      m_translator(tx),
      m_colors(ui::DARK_COLOR_SET, root.colorScheme()),
      m_button(tx("Stop"), ' ', root),
      m_text(tx("Computing..."), util::SkinColor::Heading, "+", root.provider(), gfx::CenterAlign),
      m_loop(root),
      m_canceled(false),
      m_quit(false)
{
    setState(ModalState, true);

    setColorScheme(m_colors);
    m_button.sig_fire.add(this, &StoppableBusyIndicator::onStop);

    addChild(m_button, 0);
    addChild(m_text, 0);
}

client::widgets::StoppableBusyIndicator::~StoppableBusyIndicator()
{ }

bool
client::widgets::StoppableBusyIndicator::run()
{
    // Revert
    m_canceled = false;
    m_quit = false;
    m_button.setState(DisabledState, false);
    
    // Set my extent to preferred size and center myself
    setExtent(gfx::Rectangle(gfx::Point(), getLayoutInfo().getPreferredSize()));
    m_root.centerWidget(*this);

    // Run
    m_root.add(*this);
    m_loop.run();
    m_root.removeChild(*this);

    if (m_quit) {
        m_root.ungetKeyEvent(util::Key_Quit, 0);
    }

    return !m_canceled;
}

void
client::widgets::StoppableBusyIndicator::stop()
{
    m_loop.stop(0);
}

void
client::widgets::StoppableBusyIndicator::draw(gfx::Canvas& can)
{
    gfx::Rectangle r = getExtent();
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ui::drawFrameUp(ctx, r);
    r.grow(-1, -1);
    m_colors.drawBackground(can, r);

    defaultDrawChildren(can);
}

void
client::widgets::StoppableBusyIndicator::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::StoppableBusyIndicator::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::widgets::StoppableBusyIndicator::handleChildAdded(Widget& /*child*/)
{ }

void
client::widgets::StoppableBusyIndicator::handleChildRemove(Widget& /*child*/)
{ }

void
client::widgets::StoppableBusyIndicator::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    doLayout();
    requestRedraw();
}

void
client::widgets::StoppableBusyIndicator::handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::widgets::StoppableBusyIndicator::getLayoutInfo() const
{
    gfx::Point buttonSize = m_button.getLayoutInfo().getPreferredSize();
    gfx::Point textSize   = m_text.getLayoutInfo().getPreferredSize();

    return gfx::Point(std::max(buttonSize.getX(), textSize.getX()) + 2*HORIZONTAL_BORDER,
                      buttonSize.getY() + textSize.getY() + GAP + 2*VERTICAL_BORDER);
}

bool
client::widgets::StoppableBusyIndicator::handleKey(util::Key_t key, int prefix)
{
    if (key == util::Key_Escape || key == util::Key_Return || key == ' ') {
        onStop();
        return true;
    } else if (key == util::Key_Quit) {
        m_quit = true;
        onStop();
        return true;
    } else {
        return defaultHandleKey(key, prefix);
    }
}

bool
client::widgets::StoppableBusyIndicator::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::widgets::StoppableBusyIndicator::doLayout()
{
    gfx::Rectangle r = getExtent();
    r.grow(-HORIZONTAL_BORDER, -VERTICAL_BORDER);

    // Split bottom area, and center button within it
    gfx::Point buttonSize = m_button.getLayoutInfo().getPreferredSize();
    gfx::Rectangle buttonArea = r.splitBottomY(buttonSize.getY());
    int excess = std::max(0, buttonArea.getWidth() - buttonSize.getX());
    buttonArea.consumeX(excess / 2);
    m_button.setExtent(buttonArea.splitX(buttonSize.getX()));

    // Place text in remainder
    m_text.setExtent(r.splitY(m_text.getLayoutInfo().getPreferredSize().getY()));
}

void
client::widgets::StoppableBusyIndicator::onStop()
{
    if (!m_canceled) {
        m_canceled = true;
        m_button.setState(DisabledState, true);
        sig_stop.raise();
    }
}
