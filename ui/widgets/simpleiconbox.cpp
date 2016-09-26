/**
  *  \file ui/widgets/simpleiconbox.cpp
  */

#include "ui/widgets/simpleiconbox.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "gfx/complex.hpp"

namespace {
    const int GAP_X = 4;
    const int GAP_Y = 2;
}

ui::widgets::SimpleIconBox::SimpleIconBox(gfx::Point size, ui::Root& root)
    : IconBox(root),
      m_items(),
      m_size(size),
      m_root(root)
{ }

ui::widgets::SimpleIconBox::~SimpleIconBox()
{ }

ui::layout::Info
ui::widgets::SimpleIconBox::getLayoutInfo() const
{
    gfx::Point pt(m_size.getX(), m_size.getY() + GAP_Y*2);
    return ui::layout::Info(pt, pt, ui::layout::Info::GrowHorizontal);
}

void
ui::widgets::SimpleIconBox::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    // FIXME: do we need to do something?
    requestRedraw();
}

// IconBox:
int
ui::widgets::SimpleIconBox::getItemWidth(size_t nr)
{
    // ex UIDirectoryCrumbTrail::getItemWidth (sort-of)
    if (nr < m_items.size()) {
        afl::base::Ptr<gfx::Font> font = m_root.provider().getFont(m_items[nr].font);
        if (font.get() != 0) {
            return font->getTextWidth(m_items[nr].text) + GAP_X*2;
        }
    }
    return 0;
}

size_t
ui::widgets::SimpleIconBox::getNumItems()
{
    return m_items.size();
}

void
ui::widgets::SimpleIconBox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex UIDirectoryCrumbTrail::drawItem
    if (item < m_items.size()) {
        gfx::Context ctx(can);
        ctx.useColorScheme(getColorScheme());
        afl::base::Ptr<gfx::Font> font = m_root.provider().getFont(m_items[item].font);
        if (font.get() != 0) {
            ctx.useFont(*font);
        }

        /* Draw the box */
        if (state == Selected) {
            drawSolidBar(ctx, area, SkinColor::Static);
            ctx.setColor(SkinColor::InvStatic);
        } else if (state == Hover) {
            drawSolidBar(ctx, area, SkinColor::Faded);
            ctx.setColor(SkinColor::Static);
        } else {
            drawBackground(ctx, area);
            ctx.setColor(SkinColor::Static);
        }
    
        outText(ctx, gfx::Point(area.getLeftX() + GAP_X, area.getTopY() + GAP_Y), m_items[item].text);
    }
}

void
ui::widgets::SimpleIconBox::swapContent(Items_t& items, size_t current)
{
    m_items.swap(items);
    handleStructureChange(current);
}
