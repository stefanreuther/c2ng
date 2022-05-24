/**
  *  \file game/actions/cargocostaction.cpp
  *  \brief Class game::actions::CargoCostAction
  */

#include "game/actions/cargocostaction.hpp"

namespace {
    int32_t getCostAmount(const game::spec::Cost& c, game::Element::Type type)
    {
        switch (type) {
         case game::Element::Tritanium:    return c.get(game::spec::Cost::Tritanium);
         case game::Element::Duranium:     return c.get(game::spec::Cost::Duranium);
         case game::Element::Molybdenum:   return c.get(game::spec::Cost::Molybdenum);
         case game::Element::Supplies:     return c.get(game::spec::Cost::Supplies);
         case game::Element::Money:        return c.get(game::spec::Cost::Money);
         default:                          return 0;
        }
    }
}

// Constructor.
game::actions::CargoCostAction::CargoCostAction(CargoContainer& container)
    : sig_change(),
      m_container(container),
      m_cost(),
      m_changeConnection(container.sig_change.add(this, &CargoCostAction::onChange)),
      m_updating(false)
{
    // ex GCargoCostTransaction::GCargoCostTransaction
}

// Destructor.
game::actions::CargoCostAction::~CargoCostAction()
{ }

// Set cost.
void
game::actions::CargoCostAction::setCost(game::spec::Cost cost)
{
    // ex GCargoCostTransaction::setCost
    // Stash away the new value. Note that the parameter is passed by value to avoid aliasing.
    if (cost != m_cost) {
        m_cost = cost;
        update();
        sig_change.raise();
    }
}

// Get cost.
const game::spec::Cost&
game::actions::CargoCostAction::getCost() const
{
    // ex GCargoCostTransaction::getCost
    return m_cost;
}

// Set reserved amount.
void
game::actions::CargoCostAction::setReservedAmount(game::spec::Cost cost)
{
    if (m_reservedAmount != cost) {
        m_reservedAmount = cost;
        update();
        sig_change.raise();
    }
}

// Get remaining amount.
int32_t
game::actions::CargoCostAction::getRemainingAmount(Element::Type type) const
{
    // ex GCargoCostTransaction::getRemainingCargo
    return m_container.getEffectiveAmount(type) - getCostAmount(m_reservedAmount, type);
}

// Get remaining amount as Cost structure.
game::spec::Cost
game::actions::CargoCostAction::getRemainingAmountAsCost() const
{
    game::spec::Cost result;
    result.set(m_cost.Tritanium,  getRemainingAmount(Element::Tritanium));
    result.set(m_cost.Duranium,   getRemainingAmount(Element::Duranium));
    result.set(m_cost.Molybdenum, getRemainingAmount(Element::Molybdenum));
    result.set(m_cost.Money,      getRemainingAmount(Element::Money));
    result.set(m_cost.Supplies,   getRemainingAmount(Element::Supplies));
    return result;
}

// Get missing amount.
int32_t
game::actions::CargoCostAction::getMissingAmount(Element::Type type) const
{
    // ex GCargoCostTransaction::getMissingCargo
    // FIXME: the original version had some logic about when to report missing supplies.
    int32_t amount = m_container.getEffectiveAmount(type);
    int32_t limit = m_container.getMinAmount(type) + getCostAmount(m_reservedAmount, type);
    if (amount < limit) {
        return limit - amount;
    } else {
        return 0;
    }
}

// Get missing amount as Cost structure.
game::spec::Cost
game::actions::CargoCostAction::getMissingAmountAsCost() const
{
    game::spec::Cost result;
    result.set(m_cost.Tritanium,  getMissingAmount(Element::Tritanium));
    result.set(m_cost.Duranium,   getMissingAmount(Element::Duranium));
    result.set(m_cost.Molybdenum, getMissingAmount(Element::Molybdenum));
    result.set(m_cost.Money,      getMissingAmount(Element::Money));
    result.set(m_cost.Supplies,   getMissingAmount(Element::Supplies));
    return result;
}

// Get available amount as Cost structure.
game::spec::Cost
game::actions::CargoCostAction::getAvailableAmountAsCost() const
{
    game::spec::Cost result;
    result.set(m_cost.Tritanium,  m_container.getAmount(Element::Tritanium)  - m_reservedAmount.get(game::spec::Cost::Tritanium));
    result.set(m_cost.Duranium,   m_container.getAmount(Element::Duranium)   - m_reservedAmount.get(game::spec::Cost::Duranium));
    result.set(m_cost.Molybdenum, m_container.getAmount(Element::Molybdenum) - m_reservedAmount.get(game::spec::Cost::Molybdenum));
    result.set(m_cost.Money,      m_container.getAmount(Element::Money)      - m_reservedAmount.get(game::spec::Cost::Money));
    result.set(m_cost.Supplies,   m_container.getAmount(Element::Supplies)   - m_reservedAmount.get(game::spec::Cost::Supplies));
    return result;
}

// Check validity.
bool
game::actions::CargoCostAction::isValid() const
{
    // ex GCargoCostTransaction::isValidCost
    return m_container.isValid();
}

// Commit transaction.
void
game::actions::CargoCostAction::commit()
{
    // In c2ng, this commits the container; in PCC2 this would have to be done by caller.
    m_container.commit();
}

void
game::actions::CargoCostAction::update()
{
    // ex GCargoCostTransaction::update
    // Minerals
    int32_t targetT = -m_cost.get(m_cost.Tritanium);
    m_container.change(Element::Tritanium, targetT - m_container.getChange(Element::Tritanium));

    int32_t targetD = -m_cost.get(m_cost.Duranium);
    m_container.change(Element::Duranium, targetD - m_container.getChange(Element::Duranium));

    int32_t targetM = -m_cost.get(m_cost.Molybdenum);
    m_container.change(Element::Molybdenum, targetM - m_container.getChange(Element::Molybdenum));

    // Money/supplies
    int32_t neededMoney = m_cost.get(m_cost.Money);
    int32_t neededSupplies = m_cost.get(m_cost.Supplies);
    if (m_container.getFlags().contains(CargoContainer::SupplySale)) {
        int32_t availableMoney = m_container.getAmount(Element::Money) - m_container.getMinAmount(Element::Money) - m_reservedAmount.get(m_cost.Money);
        if (availableMoney < neededMoney) {
            neededSupplies += (neededMoney - availableMoney);
            neededMoney = availableMoney;
        }
    }
    int32_t targetMoney = -neededMoney;
    m_container.change(Element::Money, targetMoney - m_container.getChange(Element::Money));

    int32_t targetSupplies = -neededSupplies;
    m_container.change(Element::Supplies, targetSupplies - m_container.getChange(Element::Supplies));
}

void
game::actions::CargoCostAction::onChange()
{
    // We must avoid re-triggering ourselves
    if (!m_updating) {
        try {
            m_updating = true;
            update();
            sig_change.raise();
            m_updating = false;
        }
        catch (...) {
            m_updating = false;
            throw;
        }
    }
}
