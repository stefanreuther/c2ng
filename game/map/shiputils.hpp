/**
  *  \file game/map/shiputils.hpp
  *  \brief Ship utilities
  */
#ifndef C2NG_GAME_MAP_SHIPUTILS_HPP
#define C2NG_GAME_MAP_SHIPUTILS_HPP

#include "game/cargocontainer.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/element.hpp"
#include "game/spec/friendlycodelist.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/mission.hpp"
#include "game/spec/missionlist.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace map {

    class Configuration;
    class Ship;
    class Planet;
    class Universe;

    /** Get definition of a ship's mission.
        If the ship has a known mission, tries to obtain its definition.
        @param ship     Ship
        @param config   Host configuration
        @param missions Mission definitions
        @return Mission definition, an element of the given MissionList; null if mission not defined or not known */
    const game::spec::Mission* getShipMission(const Ship& ship, const game::config::HostConfiguration& config, const game::spec::MissionList& missions);

    /** Get definition of a ship mission by number.
        Tries to retrieve the definition of a mission that can be set on the ship.
        @param nr       Mission number
        @param ship     Ship
        @param config   Host configuration
        @param missions Mission definitions
        @return Mission definition, an element of the given MissionList; null if mission not defined */
    const game::spec::Mission* getShipMissionByNumber(int nr, const Ship& ship, const game::config::HostConfiguration& config, const game::spec::MissionList& missions);

    /** Set Intercept waypoint.
        If the ship has a valid Intercept mission parameter, and the target ship is known, sets it waypoint accordingly.
        @param [in]     univ      Universe
        @param [in,out] sh       Ship
        @param [in]     mapConfig Map configuration */
    void setInterceptWaypoint(const Universe& univ, Ship& sh, const Configuration& mapConfig);

    /** Cancel all clone orders at a planet.
        For all ships orbiting the planet, if they try to clone, cancels that order.
        @param [in,out] univ      Universe
        @param [in]     pl        Planet
        @param [in]     list      Friendly code list (to generate random friendly codes)
        @param [in]     rng       Random Number Generator (to generate random friendly codes) */
    void cancelAllCloneOrders(Universe& univ, const Planet& pl, const game::spec::FriendlyCodeList& list, util::RandomNumberGenerator& rng);

    /** Get definition of a ship's hull.
        @param ship      Ship
        @param shipList  Ship list
        @return hull definition, an element of the given ShipList's hulls; null if hull not defined or not known */
    const game::spec::Hull* getShipHull(const Ship& ship, const game::spec::ShipList& shipList);

    /** Get maximum amount of cargo for a ship.
        Loading, for example, Tritanium, reduces the amount available for Duranium.
        This method is used to implement CargoContainer::getMaxAmount() for ships.
        @param cont   Cargo container (to retrieve other cargo amounts)
        @param type   Cargo type
        @param ship   Ship (for cargo capacity)
        @param shipList Ship list (for cargo capacity)
        @return Maximum amount allowed */
    int32_t getShipTransferMaxCargo(const CargoContainer& cont, Element::Type type, const Ship& ship, const game::spec::ShipList& shipList);

} }

#endif
