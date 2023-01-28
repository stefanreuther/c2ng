/**
  *  \file client/widgets/shipspeedwidget.cpp
  */

#include "client/widgets/shipspeedwidget.hpp"
#include "afl/string/format.hpp"
#include "gfx/context.hpp"
#include "util/skincolor.hpp"

client::widgets::ShipSpeedWidget::ShipSpeedWidget(afl::base::Observable<int32_t>& value, int32_t limit, int32_t hyp, ui::Root& root)
    : NumberSelector(value, 0, limit, 1),
      m_hyp(hyp),
      m_root(root)
{ }

void
client::widgets::ShipSpeedWidget::draw(gfx::Canvas& can)
{
    // ex WShipSpeedSelector::drawContent
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addSize(1)));
    ctx.setColor(util::SkinColor::Input);
    ctx.setSolidBackground();

    String_t text;
    if (getValue() == m_hyp) {
        text = "Hyp";
    } else {
        text = afl::string::Format("%d", getValue());
    }

    outTextF(ctx, getExtent(), text);
}

void
client::widgets::ShipSpeedWidget::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::ShipSpeedWidget::handlePositionChange()
{ }

ui::layout::Info
client::widgets::ShipSpeedWidget::getLayoutInfo() const
{
    // ex WShipSpeedSelector::getLayoutInfo
    gfx::Point size = m_root.provider().getFont(gfx::FontRequest().addSize(1))->getCellSize().scaledBy(3, 1);
    return ui::layout::Info(size, size, ui::layout::Info::GrowHorizontal);
}

bool
client::widgets::ShipSpeedWidget::handleKey(util::Key_t key, int prefix)
{
    // ex WShipSpeedSelector::handleEvent, ship.pas:NSetWarpSpeed [part]
    if (hasState(FocusedState)) {
        if (key >= '0' && key <= '9') {
            requestActive();
            setValue(key - '0');
            return true;
        }
        if (key == 'y' && getMax() >= m_hyp) {
            requestActive();
            setValue(m_hyp);
            return true;
        }
        // FIXME: new in PCC2; PCC1 even does setOptimumWarp() considering all movement effects
        // if (e.key.code == ' ') {
        //     const GEngine& e = getEngine(ship.getEngineType());
        //     setValue(e.getMaxEfficientWarp());
        //     return true;
        // }
    }
    return defaultHandleKey(key, prefix);
}

bool
client::widgets::ShipSpeedWidget::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // FIXME: this appears often
    if (!pressedButtons.empty() && getExtent().contains(pt)) {
        requestFocus();
        return true;
    } else {
        return false;
    }
}
