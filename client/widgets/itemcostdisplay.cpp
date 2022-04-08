/**
  *  \file client/widgets/itemcostdisplay.cpp
  *  \brief Class client::widgets::ItemCostDisplay
  */

#include "client/widgets/itemcostdisplay.hpp"

using game::spec::Cost;

namespace {
    /* Color scheme.
       The PCC2 version uses skin colors.
       As of 20210228, the SimpleTable this is based on does not support skin colors. */
    const uint8_t Color_Header = ui::Color_White;
    const uint8_t Color_Static = ui::Color_Gray;
    const uint8_t Color_Red    = ui::Color_Red;
    const uint8_t Color_Green  = ui::Color_Green;
}

/*
 *  5 columns, 7 lines
 *
 *                This Part     Total    Available
 *    Tritanium       2,000     3,000        4,000  kt
 *    Duranium
 *    Molybdenum
 *    Megacredits
 *    Supplies
 *    Tech Level
 */

client::widgets::ItemCostDisplay::ItemCostDisplay(ui::Root& root, afl::string::Translator& tx)
    : SimpleTable(root, 5, 7),
      m_formatter(false, false),
      m_available(), m_partCost(), m_totalCost(),
      m_mode(),
      m_haveTech(), m_needTech()
{
    buildTable(root, tx);
}

void
client::widgets::ItemCostDisplay::setNumberFormatter(util::NumberFormatter fmt)
{
    m_formatter = fmt;
    renderPartCost();
    renderTotalCost();
    renderAvailableAmount();
    renderTechLevels();
}

void
client::widgets::ItemCostDisplay::setHighlightingMode(Mode mode)
{
    m_mode = mode;
    renderPartCost();
    renderTotalCost();
    renderAvailableAmount();
}

void
client::widgets::ItemCostDisplay::setAvailableAmount(game::spec::Cost cost)
{
    m_available = cost;
    renderPartCost();
    renderTotalCost();
    renderAvailableAmount();
}

void
client::widgets::ItemCostDisplay::setPartCost(game::spec::Cost cost)
{
    // ex WItemCostDisplay::setPartCost
    m_partCost = cost;
    renderPartCost();
}

void
client::widgets::ItemCostDisplay::setPartTechLevel(int have, int need)
{
    // ex WItemCostDisplay::setPartTech
    m_haveTech = have;
    m_needTech = need;
    renderTechLevels();
}

void
client::widgets::ItemCostDisplay::setTotalCost(game::spec::Cost cost)
{
    // ex WItemCostDisplay::setTotalCost
    m_totalCost = cost;
    renderTotalCost();
}

void
client::widgets::ItemCostDisplay::buildTable(ui::Root& root, afl::string::Translator& tx)
{
    // ex WItemCostDisplay::drawContent (part)
    column(0).setColor(Color_Static);
    setColumnPadding(0, 5);
    cell(0, 1).setText(tx("Tritanium"));
    cell(0, 2).setText(tx("Duranium"));
    cell(0, 3).setText(tx("Molybdenum"));
    cell(0, 4).setText(tx("Money"));
    cell(0, 5).setText(tx("Supplies"));
    cell(0, 6).setText(tx("Tech Level"));

    cell(1, 0).setText(tx("This Part")).setColor(Color_Header);
    cell(2, 0).setText(tx("Total")).setColor(Color_Header);
    cell(3, 0).setText(tx("Available")).setColor(Color_Header);

    int width = 5 * root.provider().getFont(gfx::FontRequest())->getEmWidth();
    for (int i = 1; i <= 3; ++i) {
        setColumnWidth(i, width);
        setColumnPadding(3, 5);
        column(i).setTextAlign(gfx::RightAlign, gfx::TopAlign);
    }

    column(4).setColor(Color_Green);
    cell(4, 1).setText(tx("kt"));
    cell(4, 2).setText(tx("kt"));
    cell(4, 3).setText(tx("kt"));
    cell(4, 4).setText(tx("mc"));
    cell(4, 5).setText(tx("kt"));
    setColumnPadding(4, 10);
}

int32_t
client::widgets::ItemCostDisplay::getAvailableAmount(bool flag, game::spec::Cost::Type type) const
{
    int32_t n = m_available.get(type);
    if (flag) {
        n -= m_totalCost.get(type);
    }
    return n;
}

void
client::widgets::ItemCostDisplay::renderPartCost()
{
    renderCost(m_partCost, 1, (m_mode == TotalMode));
}

void
client::widgets::ItemCostDisplay::renderTotalCost()
{
    renderCost(m_totalCost, 2, false);
}

void
client::widgets::ItemCostDisplay::renderAvailableAmount()
{
    renderCost(m_available, 3, false);
}

void
client::widgets::ItemCostDisplay::renderCost(const game::spec::Cost& cost, int column, bool flag)
{
    // ex WItemCostDisplay::drawContent (part)
    renderCost(column, 1, cost.get(Cost::Tritanium),  getAvailableAmount(flag, Cost::Tritanium)  - cost.get(Cost::Tritanium));
    renderCost(column, 2, cost.get(Cost::Duranium),   getAvailableAmount(flag, Cost::Duranium)   - cost.get(Cost::Duranium));
    renderCost(column, 3, cost.get(Cost::Molybdenum), getAvailableAmount(flag, Cost::Molybdenum) - cost.get(Cost::Molybdenum));
    renderCost(column, 4, cost.get(Cost::Money),      getAvailableAmount(flag, Cost::Money) + getAvailableAmount(flag, Cost::Supplies) - cost.get(Cost::Money) - cost.get(Cost::Supplies));

    if (&cost == &m_available || cost.get(Cost::Supplies) != 0) {
        renderCost(column, 5, cost.get(Cost::Supplies), getAvailableAmount(flag, Cost::Supplies) - cost.get(Cost::Supplies));
    } else {
        cell(column, 5).setText(String_t());
    }
}

void
client::widgets::ItemCostDisplay::renderCost(int column, int row, int32_t need, int32_t remain)
{
    // ex WItemCostDisplay::writeCost
    cell(column, row).setText(m_formatter.formatNumber(need))
        .setColor(remain < 0 ? Color_Red : Color_Green);
}

void
client::widgets::ItemCostDisplay::renderTechLevels()
{
    renderCost(1, 6, m_needTech, m_haveTech - m_needTech);
    renderCost(3, 6, m_haveTech, 0);
}
