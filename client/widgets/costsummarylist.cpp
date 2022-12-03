/**
  *  \file client/widgets/costsummarylist.cpp
  *  \brief Class client::widgets::CostSummaryList
  */

#include "client/widgets/costsummarylist.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/export.hpp"
#include "game/proxy/costsummaryadaptor.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "util/unicodechars.hpp"

using afl::string::Format;
using game::spec::Cost;
using util::SkinColor;

namespace {
    void showValue(gfx::Context<SkinColor::Color>& ctx, int x, int y, const util::NumberFormatter& fmt, int32_t value)
    {
        if (value == 0) {
            ctx.setColor(SkinColor::Faded);
            outText(ctx, gfx::Point(x, y), UTF_FIGURE_DASH);
        } else {
            ctx.setColor(SkinColor::Static);
            outText(ctx, gfx::Point(x, y), fmt.formatNumber(value));
        }
    }

    void showComparison(gfx::Context<SkinColor::Color>& ctx, int x, int y, int h, const util::NumberFormatter& fmt, int32_t have, int32_t total)
    {
        int32_t delta = have - total;
        ctx.setColor(delta >= 0 ? SkinColor::Green : SkinColor::Red);
        outText(ctx, gfx::Point(x, y), fmt.formatNumber(total));
        outText(ctx, gfx::Point(x, y+h), fmt.formatNumber(delta));
    }
}

client::widgets::CostSummaryList::CostSummaryList(int numLines, bool isList, FooterStyle footerStyle, ui::Root& root, util::NumberFormatter fmt, afl::string::Translator& tx)
    : AbstractListbox(),
      m_numLines(numLines),
      m_footerStyle(footerStyle),
      m_root(root),
      m_translator(tx),
      m_content(),
      m_available(),
      m_numberFormatter(fmt)
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

void
client::widgets::CostSummaryList::doExport(util::RequestSender<game::Session> gameSender)
{
    // ex WBillDisplay::doExport, doBillExport
    if (m_content.getNumItems() != 0) {
        client::dialogs::doExport(m_root, gameSender.makeTemporary(game::proxy::makeCostSummaryAdaptor(m_content)), gameSender, m_translator);
    }
}

afl::base::Closure<void(int)>*
client::widgets::CostSummaryList::makeExporter(util::RequestSender<game::Session> gameSender)
{
    class Exporter : public afl::base::Closure<void(int)> {
     public:
        Exporter(const util::RequestSender<game::Session> gameSender, CostSummaryList& list)
            : m_gameSender(gameSender), m_list(list)
            { }
        virtual void call(int)
            { m_list.doExport(m_gameSender); }
     private:
        util::RequestSender<game::Session> m_gameSender;
        CostSummaryList& m_list;
    };
    return new Exporter(gameSender, *this);
}

size_t
client::widgets::CostSummaryList::getNumItems() const
{
    return m_content.getNumItems();
}

bool
client::widgets::CostSummaryList::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
client::widgets::CostSummaryList::getItemHeight(size_t /*n*/) const
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
    // ex WBillDisplay::drawContent, CBillWidget.Draw (part)
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont("");
    const int x = area.getLeftX();
    const int y = area.getTopY();
    const int m = font->getEmWidth();
    const int h = font->getLineHeight();

    gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*font);
    ctx.setColor(SkinColor::Static);

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
    // ex WBillTotalDisplay::drawContent, CBillWidget.Draw (part)
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont("");
    const int x = area.getLeftX();
    int y = area.getTopY();
    const int m = font->getEmWidth();
    const int h = font->getLineHeight();

    gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*font);
    ctx.setColor(SkinColor::Static);
    // FIXME -> drawBackground(ctx, getExtent());

    // Divider
    y++;
    drawHLine(ctx, x, y, x + 23*m);
    for (int i = 0; i < 4; ++i) {
        drawHLine(ctx, x + (24+4*i)*m, y, x + (27+4*i)*m);
    }
    y++;

    // Compute totals
    Cost total = m_content.getTotalCost();
    outText(ctx, gfx::Point(x, y), m_translator("Total:"));

    // Display it
    if (m_footerStyle == TotalsFooter) {
        // Simple version
        ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
        showValue(ctx, x + 27*m, y, m_numberFormatter, total.get(Cost::Money) + total.get(Cost::Supplies));
        showValue(ctx, x + 31*m, y, m_numberFormatter, total.get(Cost::Tritanium));
        showValue(ctx, x + 35*m, y, m_numberFormatter, total.get(Cost::Duranium));
        showValue(ctx, x + 39*m, y, m_numberFormatter, total.get(Cost::Molybdenum));
    } else {
        // With comparison
        outText(ctx, gfx::Point(x, y+h), m_translator("Remaining:"));
        ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);

        // MC/Supplies
        int32_t haveMC  = m_available.get(Cost::Money);
        int32_t haveSup = m_available.get(Cost::Supplies);
        int32_t needMC  = total.get(Cost::Money);
        int32_t needSup = total.get(Cost::Supplies);
        if (haveMC+haveSup < needMC+needSup) {
            // There's a shortage of mc+sup, so we need more supplies
            ctx.setColor(SkinColor::Red);
            outText(ctx, gfx::Point(x + 27*m, y+h), Format(m_translator("(sup) %d"), m_numberFormatter.formatNumber(haveSup+haveMC - needSup - needMC)));
        } else if (haveSup < needSup) {
            // Enough money, but not enough supplies
            ctx.setColor(SkinColor::Red);
            outText(ctx, gfx::Point(x + 27*m, y+h), Format(m_translator("(sup) %d"), m_numberFormatter.formatNumber(haveSup - needSup)));
        } else {
            // Enough money and supplies, but we may need to sell supplies
            ctx.setColor(SkinColor::Green);
            if (haveMC < needMC) {
                outText(ctx, gfx::Point(x + 27*m, y+h), Format(m_translator("(sup) %d"), m_numberFormatter.formatNumber(haveSup+haveMC - needSup - needMC)));
            } else {
                outText(ctx, gfx::Point(x + 27*m, y+h), Format(m_translator("(mc) %d"), m_numberFormatter.formatNumber(haveMC - needMC)));
            }
        }
        outText(ctx, gfx::Point(x + 27*m, y), m_numberFormatter.formatNumber(total.get(Cost::Money) + total.get(Cost::Supplies)));

        // Minerals
        showComparison(ctx, x + 31*m, y, h, m_numberFormatter, m_available.get(Cost::Tritanium),  total.get(Cost::Tritanium));
        showComparison(ctx, x + 35*m, y, h, m_numberFormatter, m_available.get(Cost::Duranium),   total.get(Cost::Duranium));
        showComparison(ctx, x + 39*m, y, h, m_numberFormatter, m_available.get(Cost::Molybdenum), total.get(Cost::Molybdenum));
    }
}

void
client::widgets::CostSummaryList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WBillDisplay::drawPart, CBillWidget.Draw (part)
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont("");
    const int m = font->getEmWidth();

    gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*font);

    afl::base::Deleter del;
    ui::prepareColorListItem(ctx, area, hasState(DisabledState) ? PassiveItem : state, m_root.colorScheme(), del);
    if (const game::spec::CostSummary::Item* p = m_content.get(item)) {
        const int x = area.getLeftX();
        const int y = area.getTopY();

        ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
        outTextF(ctx, gfx::Point(x + 3*m, y), m*20, p->name);
        ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
        outText(ctx, gfx::Point(x + 3*m,  y), Format("%d %s ", p->multiplier, UTF_TIMES));
        showValue(ctx, x + 27*m, y, m_numberFormatter, p->cost.get(Cost::Money) + p->cost.get(Cost::Supplies));
        showValue(ctx, x + 31*m, y, m_numberFormatter, p->cost.get(Cost::Tritanium));
        showValue(ctx, x + 35*m, y, m_numberFormatter, p->cost.get(Cost::Duranium));
        showValue(ctx, x + 39*m, y, m_numberFormatter, p->cost.get(Cost::Molybdenum));
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
    const int emWidth  = m_root.provider().getFont("")->getEmWidth();
    const int numLines = (m_numLines != 0 ? m_numLines : int(m_content.getNumItems()));
    const int height   = numLines * getLineHeight() + getHeaderHeight() + getFooterHeight();

    int width  = 39 * emWidth;
    if (!hasState(DisabledState)) {
        width += 5;
    }

    return ui::layout::Info(gfx::Point(width, height), gfx::Point(width, height), ui::layout::Info::GrowBoth);
}

bool
client::widgets::CostSummaryList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

int
client::widgets::CostSummaryList::getLineHeight() const
{
    return m_root.provider().getFont("")->getLineHeight();
}
