/**
  *  \file game/actions/basefixrecycle.cpp
  *  \brief Class game::actions::BaseFixRecycle
  */

#include "game/actions/basefixrecycle.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"

game::actions::BaseFixRecycle::BaseFixRecycle(game::map::Planet& planet)
    : m_planet(planet)
{
    // ex GStarbaseFixRecycleAction::GStarbaseFixRecycleAction
    mustHavePlayedBase(planet);
}

game::actions::BaseFixRecycle::ShipyardActionSet_t
game::actions::BaseFixRecycle::getValidActions(const game::map::Ship& sh) const
{
    // ex GStarbaseFixRecycleAction::getValidBaseActionsForShip

    // Verify positions
    game::map::Point planetPos, shipPos;
    if (!m_planet.getPosition().get(planetPos) || !sh.getPosition().get(shipPos)) {
        return ShipyardActionSet_t();
    }
    if (planetPos != shipPos) {
        return ShipyardActionSet_t();
    }

    // Verify owner
    int planetOwner, shipOwner;
    if (!m_planet.getOwner().get(planetOwner) || !sh.getOwner().get(shipOwner)) {
        return ShipyardActionSet_t();
    }

    ShipyardActionSet_t result;
    if (planetOwner == shipOwner) {
        // Same owner: can do everything
        result += FixShipyardAction;
        result += RecycleShipyardAction;
    } else {
        // Different owner.
        // FIXME: this should have actual host rules.
        // PHost: allows Fix to allies
        // THost: allows everything to everyone up to a 3.22.23, only to owner from then
        result += FixShipyardAction;
    }
    return result;
}

game::actions::BaseFixRecycle::ShipyardActionSet_t
game::actions::BaseFixRecycle::getValidActions(const game::map::Universe& univ) const
{
    // ex GStarbaseFixRecycleAction::getValidBaseActions
    ShipyardActionSet_t result;
    for (Id_t i = 1, n = univ.ships().size(); i <= n; ++i) {
        if (const game::map::Ship* pShip = univ.ships().get(i)) {
            result += getValidActions(*pShip);
        }
    }
    return result;
}

std::vector<game::Id_t>
game::actions::BaseFixRecycle::getValidShipIds(const game::map::Universe& univ, ShipyardAction action) const
{
    // ex GStarbaseFixRecycleAction::enumerateShipsFor
    std::vector<Id_t> result;
    if (action != NoShipyardAction) {
        for (Id_t i = 1, n = univ.ships().size(); i <= n; ++i) {
            if (const game::map::Ship* pShip = univ.ships().get(i)) {
                if (getValidActions(*pShip).contains(action)) {
                    result.push_back(i);
                }
            }
        }
    }
    return result;
}

bool
game::actions::BaseFixRecycle::set(ShipyardAction action, game::map::Universe& univ, game::map::Ship* ship)
{
    // ex GStarbaseFixRecycleAction::setAction
    // Mark old ship (its status changes implicitly from "being fixed" to "not being worked on")
    if (game::map::Ship* oldShip = univ.ships().get(m_planet.getBaseShipyardId().orElse(0))) {
        oldShip->markDirty();
    }

    if (action == NoShipyardAction) {
        // Clearing the action is always allowed
        m_planet.setBaseShipyardOrder(NoShipyardAction, 0);
        return true;
    } else {
        // Verify action
        if (ship != 0 && getValidActions(*ship).contains(action)) {
            m_planet.setBaseShipyardOrder(action, ship->getId());
            ship->markDirty();
            return true;
        } else {
            return false;
        }
    }
}
