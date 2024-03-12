/**
  *  \file ui/widgets/simpleiconbox.cpp
  */

#include "ui/widgets/simpleiconbox.hpp"
#include "afl/charset/utf8.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"

namespace {
    const int GAP_X = 4;
    const int GAP_Y = 2;
}

ui::widgets::SimpleIconBox::SimpleIconBox(gfx::Point size, ui::Root& root)
    : IconBox(root),
      m_items(),
      m_size(size),
      m_root(root),
      m_itemKeys()
{ }

ui::widgets::SimpleIconBox::~SimpleIconBox()
{ }

void
ui::widgets::SimpleIconBox::setItemKeys(int itemKeys)
{
    m_itemKeys = itemKeys;
}

ui::layout::Info
ui::widgets::SimpleIconBox::getLayoutInfo() const
{
    gfx::Point pt(m_size.getX(), m_size.getY() + GAP_Y*2);
    return ui::layout::Info(pt, ui::layout::Info::GrowHorizontal);
}

// IconBox:
int
ui::widgets::SimpleIconBox::getItemWidth(size_t nr) const
{
    // ex UIDirectoryCrumbTrail::getItemWidth (sort-of)
    if (nr < m_items.size()) {
        afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_items[nr].font);
        return font->getTextWidth(m_items[nr].text) + GAP_X*2;
    }
    return 0;
}

bool
ui::widgets::SimpleIconBox::isItemKey(size_t nr, util::Key_t key) const
{
    util::Key_t k;
    if (nr < m_items.size()) {
        k = afl::charset::getLowerCase(afl::charset::Utf8().charAt(m_items[nr].text, 0));
    } else {
        k = 0;
    }

    return (k != 0)
        && (((m_itemKeys & UsePlainKeys) != 0 && k == key)
            || ((m_itemKeys & UseAltKeys) != 0 && (k + util::KeyMod_Alt) == key));
}

size_t
ui::widgets::SimpleIconBox::getNumItems() const
{
    return m_items.size();
}

void
ui::widgets::SimpleIconBox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex UIDirectoryCrumbTrail::drawItem
    if (item < m_items.size()) {
        gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
        afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_items[item].font);
        ctx.useFont(*font);

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
ui::widgets::SimpleIconBox::drawBlank(gfx::Canvas& can, gfx::Rectangle area)
{
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    drawBackground(ctx, area);
}

void
ui::widgets::SimpleIconBox::swapContent(Items_t& items, size_t current)
{
    m_items.swap(items);
    handleStructureChange(current);
}
