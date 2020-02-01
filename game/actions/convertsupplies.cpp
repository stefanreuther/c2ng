/**
  *  \file game/actions/convertsupplies.cpp
  *  \brief Class game::actions::ConvertSupplies
  */

#include <algorithm>
#include "game/actions/convertsupplies.hpp"
#include "game/actions/preconditions.hpp"
#include "game/map/reverter.hpp"

namespace {
    int32_t getAvailableCargo(const game::map::Planet& pl, const game::Element::Type ele, int32_t reserved)
    {
        return std::max(0, pl.getCargo(ele).orElse(0) - std::max(0, reserved));
    }
}

// Constructor.
game::actions::ConvertSupplies::ConvertSupplies(game::map::Planet& pl)
    : m_planet(pl),
      m_pUniverse(0),
      m_reservedSupplies(0),
      m_reservedMoney(0)
{
    mustBePlayed(pl);
}

// Set undo information.
void
game::actions::ConvertSupplies::setUndoInformation(game::map::Universe& univ)
{
    m_pUniverse = &univ;
}

// Set reserved supplies.
void
game::actions::ConvertSupplies::setReservedSupplies(int32_t amount)
{
    m_reservedSupplies = amount;
}

// Set reserved money.
void
game::actions::ConvertSupplies::setReservedMoney(int32_t amount)
{
    m_reservedMoney = amount;
}

// Sell supplies.
int32_t
game::actions::ConvertSupplies::sellSupplies(int32_t amount, bool partial)
{
    int32_t change;
    if (amount == 0) {
        change = 0;
    } else if (amount > 0) {
        change = std::min(getMaxSuppliesToSell(), amount);
    } else {
        change = -std::min(getMaxSuppliesToBuy(), -amount);
    }

    if (change != amount && !partial) {
        return 0;
    } else {
        m_planet.setCargo(Element::Supplies, m_planet.getCargo(Element::Supplies).orElse(0) - change);
        m_planet.setCargo(Element::Money,    m_planet.getCargo(Element::Money).orElse(0)    + change);
        return change;
    }
}

// Buy supplies.
int32_t
game::actions::ConvertSupplies::buySupplies(int32_t amount, bool partial)
{
    return -sellSupplies(-amount, partial);
}

// Get maximum possible amount to sell.
int32_t
game::actions::ConvertSupplies::getMaxSuppliesToSell() const
{
    return getAvailableCargo(m_planet, game::Element::Supplies, m_reservedSupplies);
}

// Get maximum possible amount to buy.
int32_t
game::actions::ConvertSupplies::getMaxSuppliesToBuy() const
{
    int32_t buyLimit = 0;
    if (m_pUniverse != 0) {
        if (game::map::Reverter* rev = m_pUniverse->getReverter()) {
            buyLimit = std::min(getAvailableCargo(m_planet, game::Element::Money, m_reservedMoney),
                                rev->getSuppliesAllowedToBuy(m_planet.getId()));
        }
    }
    return buyLimit;
}
