/**
  *  \file game/actions/cargocostaction.cpp
  *  \brief Class game::actions::CargoCostAction
  */

#include "game/actions/cargocostaction.hpp"

// Constructor.
game::actions::CargoCostAction::CargoCostAction(CargoContainer& container)
    : sig_change(),
      m_container(container),
      m_cost(),
      m_changeConnection(container.sig_change.add(this, &CargoCostAction::onChange))
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

// Get remaining amount.
int32_t
game::actions::CargoCostAction::getRemainingAmount(Element::Type type) const
{
    // ex GCargoCostTransaction::getRemainingCargo
    // FIXME: this function uses Element::Type, not Cost::Type.
    return m_container.getEffectiveAmount(type);
}

// Get missing amount.
int32_t
game::actions::CargoCostAction::getMissingAmount(Element::Type type) const
{
    // ex GCargoCostTransaction::getMissingCargo
    // FIXME: this function uses Element::Type, not Cost::Type.
    // FIXME: the original version had some logic about when to report missing supplies.
    int32_t amount = m_container.getEffectiveAmount(type);
    int32_t limit = m_container.getMinAmount(type);
    if (amount < limit) {
        return limit - amount;
    } else {
        return 0;
    }
}

// Check validity.
bool
game::actions::CargoCostAction::isValid() const
{
    // ex GCargoCostTransaction::isValidCost
    return m_container.isValid();
}

// /** Commit transaction. Caller must commit the GCargoContainer to actually
//     use the minerals (and do the thing this cost is for, of course). */
void
game::actions::CargoCostAction::commit()
{
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
        int32_t availableMoney = m_container.getAmount(Element::Money) - m_container.getMinAmount(Element::Money);
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
    update();
    sig_change.raise();
}

// /** Change the cost.
//     \param type type of resource to modify
//     \param amount amount to add (negative to subtract)
//     \see setCost() */
// void
// GCargoCostTransaction::changeCost(GCargoType type, int32_t amount)
// {
//     ASSERT(GCost::isValidType(type));
//     cost.add(type, amount);
//     update();
//     sig_cost_changed.raise();
// }

// /** Change the cost.
//     \param cost cost to add/remove
//     \see setCost() */
// void
// GCargoCostTransaction::changeCost(const GCost& cost)
// {
//     this->cost += cost;
//     update();
//     sig_cost_changed.raise();
// }

// /** Set cost. This does actual re-allocation.
//     \post getCost(type) == amount */
// void
// GCargoCostTransaction::setCost(GCargoType type, int32_t amount)
// {
//     ASSERT(GCost::isValidType(type));
//     cost.set(type, amount);
//     update();
//     sig_cost_changed.raise();
// }



// /** Get reserved amounts of a given type.
//     If the transaction is successful, this value is equal to the
//     actual cost of the transaction, but in case of supplies sold for
//     money, it yields the actual amounts used. */
// int32_t
// GCargoCostTransaction::getReservedCargo(GCargoType type) const
// {
//     ASSERT(GCost::isValidType(type));
//     return reserved.get(type);
// }

// /** Cancel transaction. Caller must cancel the GCargoContainer, too. */
// void
// GCargoCostTransaction::cancel()
// {
//     container.mustCancel();
// }

// /** Get error message. */
// GError
// GCargoCostTransaction::getError()
// {
//     if (!isValidCost())a
//         return GError(GError::eNoResource, _("Not enough resources to perform this action"));
//     return GError();
// }
