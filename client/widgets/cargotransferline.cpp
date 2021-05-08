/**
  *  \file client/widgets/cargotransferline.cpp
  */

#include "client/widgets/cargotransferline.hpp"
#include "gfx/context.hpp"
#include "util/unicodechars.hpp"
#include "util/updater.hpp"

client::widgets::CargoTransferLine::CargoTransferLine(ui::Root& root, afl::string::Translator& tx, String_t name, int id, util::NumberFormatter fmt)
    : m_root(root),
      m_translator(tx),
      m_name(name),
      m_id(id),
      m_numberFormatter(fmt),
      m_available(),
      m_remaining(),
      m_moveLeft(UTF_LEFT_ARROW, util::Key_Left, root),
      m_moveRight(UTF_RIGHT_ARROW, util::Key_Right, root)
{
    // ex WCargoLine::WCargoLine
    addChild(m_moveLeft, 0);
    addChild(m_moveRight, 0);
    m_moveLeft.dispatchKeyTo(*this);
    m_moveRight.dispatchKeyTo(*this);
}

void
client::widgets::CargoTransferLine::setAmounts(bool right, int32_t available, int32_t remaining)
{
    util::Updater u;
    u.set(m_available[right], available);
    u.set(m_remaining[right], remaining);
    if (u) {
        requestRedraw();
    }
}

// Widget:
void
client::widgets::CargoTransferLine::draw(gfx::Canvas& can)
{
    // ex WCargoLine::drawContent
    defaultDrawChildren(can);

    gfx::Rectangle area = getExtent();
    int panelWidth = area.getWidth() / 3;
    int midWidth = area.getWidth() - 2*panelWidth;

    drawAmounts(can, false, area.splitX(panelWidth));

    gfx::Rectangle midArea = area.splitX(midWidth);

    int buttonWidth = m_root.provider().getFont(gfx::FontRequest())->getEmWidth() * 5/3;
    int textWidth = midArea.getWidth() - 2*buttonWidth;
    midArea.consumeX(buttonWidth);
    midArea = midArea.splitX(textWidth);
    midArea.grow(-10, 0);

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    ctx.setColor(util::SkinColor::Static);
    ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
    outTextF(ctx, midArea, m_name);

    drawAmounts(can, true, area);
}

void
client::widgets::CargoTransferLine::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::CargoTransferLine::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::widgets::CargoTransferLine::handleChildAdded(Widget& /*child*/)
{ }

void
client::widgets::CargoTransferLine::handleChildRemove(Widget& /*child*/)
{ }

void
client::widgets::CargoTransferLine::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    // ex WCargoLine::doLayout
    gfx::Rectangle area = getExtent();
    int panelWidth = area.getWidth() / 3;
    int midWidth = area.getWidth() - 2*panelWidth;
    area.grow(0, -1);
    area.consumeX(panelWidth);
    area = area.splitX(midWidth);

    int buttonWidth = m_root.provider().getFont(gfx::FontRequest())->getEmWidth() * 5/3;
    int textWidth = area.getWidth() - 2*buttonWidth;
    m_moveLeft.setExtent(area.splitX(buttonWidth));
    area.consumeX(textWidth);
    m_moveRight.setExtent(area);
}

void
client::widgets::CargoTransferLine::handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::widgets::CargoTransferLine::getLayoutInfo() const
{
    // ex WCargoLine::getLayoutInfo
    // same as CargoTransferHeader, but without the extra pixel which is taken by the focus grid for us
    gfx::Point size = m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(50, 1) + gfx::Point(0, 2);
    return ui::layout::Info(size, size, ui::layout::Info::GrowHorizontal);
}

bool
client::widgets::CargoTransferLine::handleKey(util::Key_t key, int prefix)
{
    // ex WCargoLine::handleEvent, CCargoLine.Handle
    if (hasState(FocusedState)) {
        bool target = (key & util::Key_Mask) == util::Key_Left ? false : true;
        switch (key) {
         case util::Key_Left:
         case util::Key_Right:
            sig_move.raise(m_id, target, prefix ? prefix : 10);
            return true;

         case util::KeyMod_Shift + util::Key_Left:
         case util::KeyMod_Shift + util::Key_Right:
            sig_move.raise(m_id, target, 1);
            return true;

         case util::KeyMod_Ctrl + util::Key_Left:
         case util::KeyMod_Ctrl + util::Key_Right:
            sig_move.raise(m_id, target, 100);
            return true;

         case util::KeyMod_Alt + util::Key_Left:
         case util::KeyMod_Alt + util::Key_Right:
            if (prefix != 0) {
                int32_t delta = prefix - m_available[target];
                if (delta > 0) {
                    sig_move.raise(m_id, target, delta);
                }
                if (delta < 0) {
                    sig_move.raise(m_id, !target, -delta);
                }
            } else {
                sig_move.raise(m_id, target, m_available[!target]);
            }
            return true;
        }
    }
    return defaultHandleKey(key, prefix);
}

bool
client::widgets::CargoTransferLine::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::widgets::CargoTransferLine::drawAmounts(gfx::Canvas& can, bool right, gfx::Rectangle area)
{
    // ex WCargoLine::showText, transfer.pas:ShowStatus
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    ctx.setColor(util::SkinColor::Green);
    ctx.setTextAlign(gfx::RightAlign, gfx::MiddleAlign);
    area.grow(-10, 0);

    outTextF(ctx, area.splitX(area.getWidth()/2), m_numberFormatter.formatNumber(m_available[right]));
    if (m_remaining[right] > 20000) {
        outTextF(ctx, area, m_translator("(unl)"));
    } else {
        outTextF(ctx, area, m_numberFormatter.formatNumber(m_remaining[right]));
    }
}
