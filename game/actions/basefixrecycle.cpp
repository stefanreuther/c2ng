/**
  *  \file game/actions/basefixrecycle.cpp
  */

#include "game/actions/basefixrecycle.hpp"
#include "game/exception.hpp"
#include "game/actions/preconditions.hpp"

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
    if (!m_planet.getPosition(planetPos) || !sh.getPosition(shipPos)) {
        return ShipyardActionSet_t();
    }
    if (planetPos != shipPos) {
        return ShipyardActionSet_t();
    }

    // Verify owner
    int planetOwner, shipOwner;
    if (!m_planet.getOwner(planetOwner) || !sh.getOwner(shipOwner)) {
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
        // THost: allows everything to everyone up to a certain version, only to owner from then
        result += FixShipyardAction;
    }
    return result;
}

bool
game::actions::BaseFixRecycle::set(ShipyardAction action, game::map::Ship* ship)
{
    // ex GStarbaseFixRecycleAction::setAction
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
