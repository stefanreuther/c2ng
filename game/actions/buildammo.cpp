/**
  *  \file game/actions/buildammo.cpp
  *  \brief Class game::actions::BuildAmmo
  */

#include <algorithm>
#include "game/actions/buildammo.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"
#include "game/map/planetformula.hpp"
#include "game/map/reverter.hpp"
#include "game/map/universe.hpp"
#include "game/registrationkey.hpp"
#include "game/spec/shiplist.hpp"
#include "game/spec/torpedolauncher.hpp"
#include "util/translation.hpp"

using game::spec::Cost;

/*
 *  Replacements for classic calls:
 *    GStarbaseBuildAmmoAction::canHave -> receiver.canHave()
 */

namespace {
    int getPlanetOwner(const game::map::Planet& planet)
    {
        int owner = 0;
        afl::except::checkAssertion(planet.getOwner(owner), "no owner", "<BuildAmmo>");
        return owner;
    }
}

// Constructor.
game::actions::BuildAmmo::BuildAmmo(game::map::Planet& planet,
                                    CargoContainer& financier,
                                    CargoContainer& receiver,
                                    game::spec::ShipList& shipList,
                                    Root& root)
    : m_planet(planet),
      m_costAction(financier),
      m_receiver(receiver),
      m_shipList(shipList),
      m_root(root),
      m_mustCommitReceiver(&financier != &receiver),
      m_costActionChangeConnection(m_costAction.sig_change.add(this, &BuildAmmo::update)),
      m_receiverChangeConnection(m_receiver.sig_change.add(this, &BuildAmmo::update)),
      m_shipListChangeConnection(m_shipList.sig_change.add(this, &BuildAmmo::update)),
      m_planetChangeConnection(m_planet.sig_change.add(this, &BuildAmmo::update)),
      m_universeChangeConnection(),
      m_pUniverse(0)
{
    mustHavePlayedBase(planet);
    update();
}

// Destructor.
game::actions::BuildAmmo::~BuildAmmo()
{ }

// Set undo information.
void
game::actions::BuildAmmo::setUndoInformation(game::map::Universe& univ)
{
    m_pUniverse = &univ;
    m_universeChangeConnection = univ.sig_universeChange.add(this, &BuildAmmo::update);
    update();
}

// Add ammo.
int
game::actions::BuildAmmo::add(Element::Type type, int count, bool partial)
{
    // ex GStarbaseBuildAmmoAction::setAmount [part]
    if (count == 0 || !m_receiver.canHaveElement(type)) {
        // Trivial case
        return 0;
    }

    int itemTechLevel = 0;
    Cost itemCost;
    if (!getItemCost(type, itemCost, itemTechLevel)) {
        // Invalid type
        return 0;
    }

    int delta;
    if (count > 0) {
        // Buy
        if (!isAccessibleTechLevel(itemTechLevel)) {
            return 0;
        }

        const int room = std::max(0, getMaxAmount(type) - getAmount(type));
        delta = std::min(count, room);
    } else {
        // Sell
        const int removable = std::max(0, getAmount(type) - getMinAmount(type));
        delta = -std::min(-count, removable);
    }

    // Reject partial operation
    if (delta != count && !partial) {
        return 0;
    }

    // Do it
    m_receiver.change(type, delta);
    update();
    return delta;
}

// Add ammo, limiting by cash.
int
game::actions::BuildAmmo::addLimitCash(Element::Type type, int count)
{
    // ex GStarbaseBuildAmmoAction::limitAmount [sort-of]
    // If this is an addition, limit by available resources.
    // Other limitations are applied by add().
    int itemTechLevel = 0;
    Cost itemCost;
    if (count > 0 && getItemCost(type, itemCost, itemTechLevel) && isAccessibleTechLevel(itemTechLevel)) {
        // Determine existing resources
        Cost resources;
        resources.set(Cost::Tritanium,  m_costAction.getRemainingAmount(Element::Tritanium));
        resources.set(Cost::Duranium,   m_costAction.getRemainingAmount(Element::Duranium));
        resources.set(Cost::Molybdenum, m_costAction.getRemainingAmount(Element::Molybdenum));
        resources.set(Cost::Supplies,   m_costAction.getRemainingAmount(Element::Supplies));
        resources.set(Cost::Money,      m_costAction.getRemainingAmount(Element::Money));

        // If we have some tech update to do, account for that
        if (itemTechLevel > m_totalTechLevel) {
            int32_t newMoney = resources.get(Cost::Money) - game::map::getBaseTechCost(getPlanetOwner(m_planet), m_totalTechLevel, itemTechLevel, m_root.hostConfiguration());
            int32_t newSupplies = resources.get(Cost::Supplies);
            if (newMoney < 0) {
                newSupplies += newMoney;
                newMoney = 0;
            }
            if (newSupplies < 0) {
                return 0;
            }
            resources.set(Cost::Supplies, newSupplies);
            resources.set(Cost::Money, newMoney);
        }

        // Limit count
        count = resources.getMaxAmount(count, itemCost);
    }

    return add(type, count, true);
}

// Get ammo that must remain.
int
game::actions::BuildAmmo::getMinAmount(Element::Type type) const
{
    int amount = m_receiver.getAmount(type);
    if (m_pUniverse != 0) {
        if (game::map::Reverter* pRev = m_pUniverse->getReverter()) {
            // Find number of sellable items
            int sellable = 0;
            int torpType;
            if (type == Element::Fighters) {
                sellable = pRev->getNumFightersAllowedToSell(m_planet.getId());
            } else if (Element::isTorpedoType(type, torpType)) {
                sellable = pRev->getNumTorpedoesAllowedToSell(m_planet.getId(), torpType);
            } else {
                sellable = 0;
            }

            // Determine new lower limit
            amount = std::max(0, amount - sellable);
        }
    }
    return amount;
}

// Get maximum ammo.
int
game::actions::BuildAmmo::getMaxAmount(Element::Type type) const
{
    return m_receiver.getMaxAmount(type);
}

// Get current amount (number on receiver plus build order).
int
game::actions::BuildAmmo::getAmount(Element::Type type) const
{
    // ex GStarbaseBuildAmmoAction::getAmount (sort-of)
    return m_receiver.getEffectiveAmount(type);
}

// Get current status.
game::actions::BuildAmmo::Status
game::actions::BuildAmmo::getStatus()
{
    update();
    if (!m_receiver.isValid()) {
        return MissingRoom;
    }
    if (!m_costAction.isValid()) {
        return MissingResources;
    }
    if (!isAccessibleTechLevel(m_totalTechLevel)) {
        return DisallowedTech;
    }
    return Success;
}

// Commit.
void
game::actions::BuildAmmo::commit()
{
    switch (getStatus()) {
     case MissingResources:
        throw Exception(Exception::eNoResource, _("Not enough resources to perform this action"));

     case DisallowedTech:
        throw Exception(Exception::ePerm, _("Tech level not accessible"));

     case MissingRoom:
        throw Exception(Exception::ePerm, _("Target unit overloaded"));

     case Success:
        break;
    }

    // Disconnect events to avoid retriggering ourselves
    m_costActionChangeConnection.disconnect();
    m_receiverChangeConnection.disconnect();
    m_shipListChangeConnection.disconnect();
    m_planetChangeConnection.disconnect();
    m_universeChangeConnection.disconnect();

    // All tests pass, commit!
    m_costAction.commit();
    if (m_mustCommitReceiver) {
        m_receiver.commit();
    }
    m_planet.setBaseTechLevel(TorpedoTech, m_totalTechLevel);
}

// Check validity.
bool
game::actions::BuildAmmo::isValid()
{
    return getStatus() == Success;
}

// Access underlying CargoCostAction.
const game::actions::CargoCostAction&
game::actions::BuildAmmo::costAction() const
{
    return m_costAction;
}

void
game::actions::BuildAmmo::update()
{
    // ex GStarbaseBuildAmmoAction::setAmount [part]
    Cost totalCost;
    int totalTechLevel = m_planet.getBaseTechLevel(TorpedoTech).orElse(1);

    // All deltas are in m_receiver.
    for (Element::Type i = Element::begin(), n = m_receiver.getTypeLimit(); i < n; ++i) {
        int delta = m_receiver.getChange(i);
        int itemTechLevel = 0;
        Cost itemCost;
        if (delta != 0 && getItemCost(i, itemCost, itemTechLevel)) {
            totalCost += itemCost * delta;
            totalTechLevel = std::max(totalTechLevel, itemTechLevel);
        }
    }

    // Do we need a tech upgrade?
    int existingTechLevel = m_planet.getBaseTechLevel(TorpedoTech).orElse(1);
    if (totalTechLevel > existingTechLevel) {
        totalCost.add(Cost::Money, game::map::getBaseTechCost(getPlanetOwner(m_planet), existingTechLevel, totalTechLevel, m_root.hostConfiguration()));
    }

    // Finish
    m_costAction.setCost(totalCost);
    m_totalTechLevel = totalTechLevel;
}

bool
game::actions::BuildAmmo::getItemCost(Element::Type type, game::spec::Cost& cost, int& techLevel) const
{
    int torpType;
    if (type == Element::Fighters) {
        int owner = getPlanetOwner(m_planet);
        cost = m_root.hostConfiguration()[game::config::HostConfiguration::BaseFighterCost](owner);
        techLevel = 1;
        return true;
    } else if (Element::isTorpedoType(type, torpType)) {
        if (game::spec::TorpedoLauncher* pTL = m_shipList.launchers().get(torpType)) {
            cost = pTL->torpedoCost();
            techLevel = pTL->getTechLevel();
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool
game::actions::BuildAmmo::isAccessibleTechLevel(int techLevel) const
{
    int existingTech = m_planet.getBaseTechLevel(TorpedoTech).orElse(1);
    if (techLevel <= existingTech) {
        return true;
    }
    if (techLevel > m_root.registrationKey().getMaxTechLevel(TorpedoTech)) {
        return false;
    }
    return true;
}
