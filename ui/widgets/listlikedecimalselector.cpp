/**
  *  \file ui/widgets/listlikedecimalselector.cpp
  */

#include "ui/widgets/listlikedecimalselector.hpp"
#include "afl/string/format.hpp"
#include "gfx/context.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/draw.hpp"

ui::widgets::ListLikeDecimalSelector::ListLikeDecimalSelector(Root& root, String_t label, afl::base::Observable<int32_t>& value, int32_t min, int32_t max, int32_t step)
    : BaseDecimalSelector(value, min, max, step),
      m_root(root),
      m_label(label)
{
    // ex UIListLikeNumberSelector::UIListLikeNumberSelector
}

ui::widgets::ListLikeDecimalSelector::~ListLikeDecimalSelector()
{ }


void
ui::widgets::ListLikeDecimalSelector::draw(gfx::Canvas& can)
{
    // ex UIListLikeNumberSelector::drawContent
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    gfx::Rectangle area(getExtent());
    prepareColorListItem(ctx, area, getFocusState() != NoFocus ? AbstractListbox::FocusedItem : AbstractListbox::PassiveItem, m_root.colorScheme(), del);

    area.consumeX(5);
    area.consumeRightX(5);
    ctx.setColor(util::SkinColor::Static);

    if (getMode() != Zeroed) {
        String_t value = formatValue(getValue());
        outTextF(ctx, area.splitRightX(ctx.getFont()->getTextWidth(value)), value);
    }
    outTextF(ctx, area, m_label);
}

ui::layout::Info
ui::widgets::ListLikeDecimalSelector::getLayoutInfo() const
{
    // ex UIListLikeNumberSelector::getLayoutInfo
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());
    const int width = font->getTextWidth(m_label)
        + font->getTextWidth(formatValue(getMax()))
        + 2*font->getEmWidth();
    const int height = font->getLineHeight();
    const gfx::Point size(width, height);
    return ui::layout::Info(size, ui::layout::Info::GrowHorizontal);
}
