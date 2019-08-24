/**
  *  \file game/map/shiputils.cpp
  */

#include "game/map/shiputils.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"

namespace {
    const int32_t MAX_CARGO = game::MAX_NUMBER;
    const int32_t MAX_OVERLOAD = 20000;
}

const game::spec::Mission*
game::map::getShipMission(const Ship& ship, const game::config::HostConfiguration& config, const game::spec::MissionList& missions)
{
    int nr, owner;
    if (ship.getMission().get(nr) && ship.getRealOwner().get(owner)) {
        return missions.getMissionByNumber(nr, PlayerSet_t(config.getPlayerMissionNumber(owner)));
    } else {
        return 0;
    }
}

const game::spec::Mission*
game::map::getShipMissionByNumber(int nr, const Ship& ship, const game::config::HostConfiguration& config, const game::spec::MissionList& missions)
{
    int owner;
    if (ship.getRealOwner().get(owner)) {
        return missions.getMissionByNumber(nr, PlayerSet_t(config.getPlayerMissionNumber(owner)));
    } else {
        return 0;
    }
}

// /** Manipulating: set waypoint for intercepting ship. */
void
game::map::setInterceptWaypoint(Universe& univ, Ship& sh)
{
    // ex game/fleet.h:setInterceptWaypoint
    int i;
    if (sh.getMissionParameter(InterceptParameter).get(i)) {
        if (Ship* target = univ.ships().get(i)) {
            // FIXME: handle THost where intercept does not cross the seam?
            Point shipPos, targetPos;
            if (target->getPosition(targetPos) && sh.getPosition(shipPos)) {
                sh.setWaypoint(univ.config().getSimpleNearestAlias(targetPos, shipPos));
            }
        }
    }
}

const game::spec::Hull*
game::map::getShipHull(const Ship& ship, const game::spec::ShipList& shipList)
{
    return shipList.hulls().get(ship.getHull().orElse(-1));
}

int32_t
game::map::getShipTransferMaxCargo(const CargoContainer& cont, Element::Type type, const Ship& ship, const game::spec::ShipList& shipList)
{
    switch (type) {
     case Element::Neutronium:
        if (cont.isOverload()) {
            return MAX_CARGO;
        } else if (const game::spec::Hull* pHull = getShipHull(ship, shipList)) {
            return pHull->getMaxFuel();
        } else {
            return 0;
        }

     case Element::Money:
        return MAX_CARGO;

     default:
        const game::spec::Hull* pHull = getShipHull(ship, shipList);
        int32_t available = (cont.isOverload() ? MAX_OVERLOAD
                             : pHull != 0 ? pHull->getMaxCargo()
                             : 0);
        int32_t rest = available
            - cont.getEffectiveAmount(Element::Tritanium) - cont.getEffectiveAmount(Element::Duranium)
            - cont.getEffectiveAmount(Element::Molybdenum) - cont.getEffectiveAmount(Element::Supplies)
            - cont.getEffectiveAmount(Element::Colonists);

        const int numLaunchers = ship.getNumLaunchers().orElse(0);
        const int torpType = ship.getTorpedoType().orElse(0);
        if (torpType > 0 && numLaunchers > 0) {
            rest -= cont.getEffectiveAmount(Element::fromTorpedoType(torpType));
        }

        const int numBays = ship.getNumBays().orElse(0);
        if (numBays > 0) {
            rest -= cont.getEffectiveAmount(Element::Fighters);
        }

        rest += cont.getEffectiveAmount(type);

        // This result may be negative. Caller needs to deal with it.
        return std::min(MAX_CARGO, rest);
    }
}
