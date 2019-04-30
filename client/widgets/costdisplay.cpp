/**
  *  \file client/widgets/costdisplay.cpp
  */

#include "client/widgets/costdisplay.hpp"
#include "util/translation.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/format.hpp"

using game::spec::Cost;
using afl::string::Format;

namespace {
    const Cost::Type TYPES[] = {
        Cost::Tritanium,
        Cost::Duranium,
        Cost::Molybdenum,
        Cost::Money,
        Cost::Supplies,
    };
    const char*const NAMES[] = {
        N_("Tritanium"),
        N_("Duranium"),
        N_("Molybdenum"),
        N_("Money"),
        N_("Supplies"),
    };

    const char*const TEXTS[] = {
        N_("You need %d mc"),
        N_("and/or supplies more."),
        N_("%d mc remaining"),
        N_("%d kt remaining"),
        N_("%d mc too little"),
        N_("%d kt too little"),
    };
    enum {
        NeedFunds1,
        NeedFunds2,
        RemainingCash,
        RemainingTons,
        MissingCash,
        MissingTons
    };
}

client::widgets::CostDisplay::CostDisplay(ui::Root& root, Types_t types, util::NumberFormatter fmt)
    : SimpleTable(root, 4, 1),
      m_types(types),
      m_formatter(fmt)
{
    // ex WCostDisplay::WCostDisplay
    init(root);
}

void
client::widgets::CostDisplay::setCost(const game::spec::Cost& cost)
{
    m_cost = cost;
    render();
}

void
client::widgets::CostDisplay::setAvailableAmount(const game::spec::Cost& amount)
{
    m_availableAmount = amount;
    render();
}

void
client::widgets::CostDisplay::setRemainingAmount(const game::spec::Cost& amount)
{
    m_remainingAmount = amount;
    render();
}

void
client::widgets::CostDisplay::setMissingAmount(const game::spec::Cost& amount)
{
    m_missingAmount = amount;
    render();
}

void
client::widgets::CostDisplay::init(ui::Root& root)
{
    // ex WCostDisplay::WCostDisplay
    if (m_types.contains(Cost::Money)) {
        m_types += Cost::Supplies;
    }

    // Determine size
    // ex WCostDisplay::getLines
    size_t numRows = 1
        + m_types.contains(Cost::Tritanium)
        + m_types.contains(Cost::Duranium)
        + m_types.contains(Cost::Molybdenum)
        + m_types.contains(Cost::Supplies)
        + m_types.contains(Cost::Money);
    setNumRows(numRows);

    row(0).setColor(ui::Color_Black);
    cell(0, 0).setText(_("You need:")).setExtraColumns(1).setUnderline(true);
    cell(2, 0).setText(_("You have:")).setExtraColumns(1).setUnderline(true);
    setRowPadding(0, 5);

    afl::base::Ref<gfx::Font> font = root.provider().getFont(gfx::FontRequest());
    int em = font->getEmWidth();
    setColumnWidth(1, 5*em);
    setColumnWidth(2, 5*em);
    setColumnPadding(0, 5);
    setColumnPadding(1, 5);
    setColumnPadding(2, 10);

    size_t r = 1;
    for (size_t i = 0; i < countof(TYPES); ++i) {
        if (m_types.contains(TYPES[i])) {
            row(r).setColor(ui::Color_Black);
            cell(0, r).setText(String_t(_(NAMES[i])) + ":");
            cell(1, r).setTextAlign(2, 0);
            cell(2, r).setTextAlign(2, 0);
            ++r;
        }
    }

    // Width of rightmost column
    int detailWidth = 0;
    for (size_t i = 0; i < countof(TEXTS); ++i) {
        detailWidth = std::max(detailWidth, font->getTextWidth(Format(_(TEXTS[i]), m_formatter.formatNumber(999999))));
    }
    setColumnWidth(3, detailWidth);
}

void
client::widgets::CostDisplay::render()
{
    // ex WCostDisplay::drawContent
    size_t r = 1;
    bool needSupplies = m_cost.get(Cost::Supplies) != 0;
    for (size_t i = 0; i < countof(TYPES); ++i) {
        Type_t ty = TYPES[i];
        if (m_types.contains(ty)) {
            cell(1, r).setText(m_formatter.formatNumber(m_cost.get(ty)));
            cell(2, r).setText(m_formatter.formatNumber(m_availableAmount.get(ty)));
            if (ty == Cost::Money && !needSupplies) {
                // It's the Money line.
                if (int32_t missing = m_missingAmount.get(Cost::Money) + m_missingAmount.get(Cost::Supplies)) {
                    cell(3, r  ).setText(Format(_(TEXTS[NeedFunds1]), m_formatter.formatNumber(missing))).setColor(ui::Color_Red);
                    cell(3, r+1).setText(Format(_(TEXTS[NeedFunds2]), m_formatter.formatNumber(missing))).setColor(ui::Color_Red);;
                } else {
                    cell(3, r  ).setText(Format(_(TEXTS[RemainingCash]),  m_formatter.formatNumber(m_remainingAmount.get(Cost::Money)))).setColor(ui::Color_GreenBlack);
                    cell(3, r+1).setText(Format(_(TEXTS[RemainingTons]), m_formatter.formatNumber(m_remainingAmount.get(Cost::Supplies)))).setColor(ui::Color_GreenBlack);
                }
            } else if (ty == Cost::Supplies && !needSupplies) {
                // It's the Supplies line, but has already been accounted for by the Money line.
            } else {
                // Minerals (or: cost includes supplies)
                if (int32_t missing = m_missingAmount.get(ty)) {
                    size_t index = (ty == Cost::Money ? MissingCash : MissingTons);
                    cell(3, r).setText(Format(_(TEXTS[index]), m_formatter.formatNumber(missing))).setColor(ui::Color_Red);
                } else {
                    size_t index = (ty == Cost::Money ? RemainingCash : RemainingTons);
                    cell(3, r).setText(Format(_(TEXTS[index]), m_formatter.formatNumber(m_remainingAmount.get(ty)))).setColor(ui::Color_GreenBlack);
                }
            }
            ++r;
        }
    }
}

