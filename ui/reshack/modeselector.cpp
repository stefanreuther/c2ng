/**
  *  \file ui/reshack/modeselector.cpp
  *  \brief Base class ui::reshack::ModeSelector
  */

#include "ui/reshack/modeselector.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"

const size_t ui::reshack::ModeSelector::nil;

ui::reshack::ModeSelector::ModeSelector(Root& root)
    : m_root(root)
{ }

ui::reshack::ModeSelector::~ModeSelector()
{ }

void
ui::reshack::ModeSelector::draw(gfx::Canvas& can)
{
    // RHModeSelector::drawContent(GfxCanvas& can)
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());
    ctx.useFont(*font);

    gfx::Rectangle area(getExtent());
    int lineHeight = font->getLineHeight();
    for (size_t i = 0, e = getNumSlots(); i < e; ++i) {
        gfx::Rectangle thisLine = area.splitY(lineHeight);
        if (isActive(i)) {
            drawSolidBar(ctx, thisLine, util::SkinColor::Static);
            ctx.setColor(util::SkinColor::InvStatic);
        } else {
            drawBackground(ctx, thisLine);
            if (isUsable(i)) {
                ctx.setColor(util::SkinColor::Static);
            } else {
                ctx.setColor(util::SkinColor::Faded);
            }
        }
        outTextF(ctx, thisLine, getName(i));
    }
}

void
ui::reshack::ModeSelector::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::reshack::ModeSelector::handlePositionChange()
{
    requestRedraw();
}

ui::layout::Info
ui::reshack::ModeSelector::getLayoutInfo() const
{
    // RHModeSelector::getLayoutInfo(LayoutInfo& info)
    return ui::layout::Info(m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(7, int(getNumSlots())));
}

bool
ui::reshack::ModeSelector::handleKey(util::Key_t key, int prefix)
{
    // RHModeSelector::handleEvent(const UIEvent& event, bool second_pass) (part)
    size_t n = getSlotFromKey(key);
    if (n != nil && !isActive(n) && isUsable(n)) {
        requestActive();
        activate(n);
        requestRedraw();
        return true;
    } else {
        return defaultHandleKey(key, prefix);
    }
}

bool
ui::reshack::ModeSelector::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // RHModeSelector::handleEvent(const UIEvent& event, bool second_pass) (part)
    if (!pressedButtons.empty() && getExtent().contains(pt)) {
        requestActive();

        int lineHeight = std::max(m_root.provider().getFont(gfx::FontRequest())->getLineHeight(), 1);
        size_t n = static_cast<size_t>((pt.getY() - getExtent().getTopY()) / lineHeight);
        if (n < getNumSlots()) {
            if (!isActive(n) && isUsable(n)) {
                activate(n);
                requestRedraw();
            }
        }
        return true;
    } else {
        return false;
    }
}
