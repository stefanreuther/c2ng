/**
  *  \file client/widgets/costsummarylist.cpp
  */

#include "client/widgets/costsummarylist.hpp"
#include "util/unicodechars.hpp"
#include "afl/string/format.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"

namespace {
    String_t numToString(int32_t value)
    {
        // FIXME: use preferences for NumberFormatter
        return afl::string::Format("%d", value);
    }

    void showValue(gfx::Context<util::SkinColor::Color>& ctx, int x, int y, int32_t value)
    {
        if (value == 0) {
            ctx.setColor(util::SkinColor::Faded);
            outText(ctx, gfx::Point(x, y), UTF_FIGURE_DASH);
        } else {
            ctx.setColor(util::SkinColor::Static);
            outText(ctx, gfx::Point(x, y), numToString(value));
        }
    }

    void showComparison(gfx::Context<util::SkinColor::Color>& ctx, int x, int y, int h, int32_t have, int32_t total)
    {
        int32_t delta = have - total;
        ctx.setColor(delta >= 0 ? util::SkinColor::Green : util::SkinColor::Red);
        outText(ctx, gfx::Point(x, y), numToString(total));
        outText(ctx, gfx::Point(x, y+h), numToString(delta));
    }
}

client::widgets::CostSummaryList::CostSummaryList(int numLines, bool isList, FooterStyle footerStyle, gfx::ResourceProvider& provider, ui::ColorScheme& scheme, afl::string::Translator& tx)
    : AbstractListbox(),
      m_numLines(numLines),
      m_footerStyle(footerStyle),
      m_provider(provider),
      m_colorScheme(scheme),
      m_translator(tx),
      m_content(),
      m_available()
{
    // ex WBillDisplay::WBillDisplay
    setState(DisabledState, !isList);
}

client::widgets::CostSummaryList::~CostSummaryList()
{ }

void
client::widgets::CostSummaryList::setContent(const game::spec::CostSummary& content)
{
    m_content = content;
    handleModelChange();
}

void
client::widgets::CostSummaryList::setAvailableAmount(game::spec::Cost available)
{
    // ex WBillTotalDisplay::setAvailableResources
    m_available = available;
    requestRedraw();
}

size_t
client::widgets::CostSummaryList::getNumItems()
{
    return m_content.getNumItems();
}

bool
client::widgets::CostSummaryList::isItemAccessible(size_t /*n*/)
{
    return true;
}

int
client::widgets::CostSummaryList::getItemHeight(size_t /*n*/)
{
    return getLineHeight();
}

int
client::widgets::CostSummaryList::getHeaderHeight() const
{
    return getLineHeight();
}

int
client::widgets::CostSummaryList::getFooterHeight() const
{
    // ex WBillTotalDisplay::getLayoutInfo
    int result = 0;
    switch (m_footerStyle) {
     case NoFooter:         result = 0;                       break;
     case TotalsFooter:     result =   getHeaderHeight() + 2; break;
     case ComparisonFooter: result = 2*getHeaderHeight() + 2; break;
    }
    return result;
}

void
client::widgets::CostSummaryList::drawHeader(gfx::Canvas& can, gfx::Rectangle area)
{
    // ex WBillDisplay::drawContent
    afl::base::Ref<gfx::Font> font = m_provider.getFont("");
    const int x = area.getLeftX();
    const int y = area.getTopY();
    const int m = font->getEmWidth();
    const int h = font->getLineHeight();

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*font);
    ctx.setColor(util::SkinColor::Static);

    outText(ctx, gfx::Point(x, y), m_translator("Item"));
    ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
    outText(ctx, gfx::Point(x + 27*m, y), m_translator("mc"));
    outText(ctx, gfx::Point(x + 31*m, y), m_translator("Tri"));
    outText(ctx, gfx::Point(x + 35*m, y), m_translator("Dur"));
    outText(ctx, gfx::Point(x + 39*m, y), m_translator("Mol"));
    drawHLine(ctx, x, y + h-2, x + 23*m);
    for (int i = 0; i < 4; ++i) {
        drawHLine(ctx, x + (24+4*i)*m, y + h-2, x + (27+4*i)*m);
    }
}

void
client::widgets::CostSummaryList::drawFooter(gfx::Canvas& can, gfx::Rectangle area)
{
    // ex WBillTotalDisplay::drawContent
    afl::base::Ref<gfx::Font> font = m_provider.getFont("");
    const int x = area.getLeftX();
    int y = area.getTopY();
    const int m = font->getEmWidth();
    const int h = font->getLineHeight();

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*font);
    ctx.setColor(util::SkinColor::Static);
    // FIXME -> drawBackground(ctx, getExtent());

    // Divider
    y++;
    drawHLine(ctx, x, y, x + 23*m);
    for (int i = 0; i < 4; ++i) {
        drawHLine(ctx, x + (24+4*i)*m, y, x + (27+4*i)*m);
    }
    y++;

    // Compute totals
    game::spec::Cost total = m_content.getTotalCost();
    outText(ctx, gfx::Point(x, y), m_translator("Total:"));

    // Display it
    if (m_footerStyle == TotalsFooter) {
        // Simple version
        ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
        showValue(ctx, x + 27*m, y, total.get(game::spec::Cost::Money) + total.get(game::spec::Cost::Supplies));
        showValue(ctx, x + 31*m, y, total.get(game::spec::Cost::Tritanium));
        showValue(ctx, x + 35*m, y, total.get(game::spec::Cost::Duranium));
        showValue(ctx, x + 39*m, y, total.get(game::spec::Cost::Molybdenum));
    } else {
        // With comparison
        outText(ctx, gfx::Point(x, y+h), m_translator("Remaining:"));
        ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);

        // MC/Supplies
        int32_t haveMC  = m_available.get(game::spec::Cost::Money);
        int32_t haveSup = m_available.get(game::spec::Cost::Supplies);
        int32_t needMC  = total.get(game::spec::Cost::Money);
        int32_t needSup = total.get(game::spec::Cost::Supplies);
        if (haveMC+haveSup < needMC+needSup) {
            // There's a shortage of mc+sup, so we need more supplies
            ctx.setColor(util::SkinColor::Red);
            outText(ctx, gfx::Point(x + 27*m, y+h), afl::string::Format(m_translator("(sup) %d"), numToString(haveSup+haveMC - needSup - needMC)));
        } else if (haveSup < needSup) {
            // Enough money, but not enough supplies
            ctx.setColor(util::SkinColor::Red);
            outText(ctx, gfx::Point(x + 27*m, y+h), afl::string::Format(m_translator("(sup) %d"), numToString(haveSup - needSup)));
        } else {
            // Enough money and supplies, but we may need to sell supplies
            ctx.setColor(util::SkinColor::Green);
            if (haveMC < needMC) {
                outText(ctx, gfx::Point(x + 27*m, y+h), afl::string::Format(m_translator("(sup) %d"), numToString(haveSup+haveMC - needSup - needMC)));
            } else {
                outText(ctx, gfx::Point(x + 27*m, y+h), afl::string::Format(m_translator("(mc) %d"), numToString(haveMC - needMC)));
            }
        }
        outText(ctx, gfx::Point(x + 27*m, y), numToString(total.get(game::spec::Cost::Money) + total.get(game::spec::Cost::Supplies)));

        // Minerals
        showComparison(ctx, x + 31*m, y, h, m_available.get(game::spec::Cost::Tritanium),  total.get(game::spec::Cost::Tritanium));
        showComparison(ctx, x + 35*m, y, h, m_available.get(game::spec::Cost::Duranium),   total.get(game::spec::Cost::Duranium));
        showComparison(ctx, x + 39*m, y, h, m_available.get(game::spec::Cost::Molybdenum), total.get(game::spec::Cost::Molybdenum));
    }
}

void
client::widgets::CostSummaryList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WBillDisplay::drawPart
    afl::base::Ref<gfx::Font> font = m_provider.getFont("");
    const int m = font->getEmWidth();

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*font);

    afl::base::Deleter del;
    ui::prepareColorListItem(ctx, area, hasState(DisabledState) ? PassiveItem : state, m_colorScheme, del);
    if (const game::spec::CostSummary::Item* p = m_content.get(item)) {
        const int x = area.getLeftX();
        const int y = area.getTopY();

        ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
        outTextF(ctx, gfx::Point(x + 3*m, y), m*20, p->name);
        ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
        outText(ctx, gfx::Point(x + 3*m,  y), afl::string::Format("%d %s ", p->multiplier, UTF_TIMES));
        showValue(ctx, x + 27*m, y, p->cost.get(game::spec::Cost::Money) + p->cost.get(game::spec::Cost::Supplies));
        showValue(ctx, x + 31*m, y, p->cost.get(game::spec::Cost::Tritanium));
        showValue(ctx, x + 35*m, y, p->cost.get(game::spec::Cost::Duranium));
        showValue(ctx, x + 39*m, y, p->cost.get(game::spec::Cost::Molybdenum));
    }
}

void
client::widgets::CostSummaryList::handlePositionChange(gfx::Rectangle& oldPosition)
{
    defaultHandlePositionChange(oldPosition);
}

ui::layout::Info
client::widgets::CostSummaryList::getLayoutInfo() const
{
    const int emWidth  = m_provider.getFont("")->getEmWidth();
    const int numLines = (m_numLines != 0 ? m_numLines : int(m_content.getNumItems()));
    const int height   = numLines * getLineHeight() + getHeaderHeight() + getFooterHeight();

    int width  = 39 * emWidth;
    if (!hasState(DisabledState)) {
        width += 5;
    }

    return gfx::Point(width, height);
}

bool
client::widgets::CostSummaryList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

int
client::widgets::CostSummaryList::getLineHeight() const
{
    return m_provider.getFont("")->getLineHeight();
}
