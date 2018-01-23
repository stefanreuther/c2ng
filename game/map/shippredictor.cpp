/**
  *  \file game/map/shippredictor.cpp
  *
  *  FIXME: this is the PCC 2.0 version which has been improved until 2.0.3!
  */

#include <cmath>
#include "game/map/shippredictor.hpp"
#include "game/map/universe.hpp"
#include "game/map/ship.hpp"
#include "game/spec/hullfunction.hpp"
#include "game/map/planet.hpp"
#include "game/spec/hull.hpp"
#include "game/registrationkey.hpp"
#include "util/math.hpp"

using game::spec::HullFunction;
using game::config::HostConfiguration;

namespace {
    int sgn(double d)
    {
        return d < 0 ? -1 : d > 0 ? +1 : 0;
    }

    // /** Perform refinery reaction.
    //     \param s   [in/out] Ship
    //     \param ore [in/out] Ore to consume
    //     \param sup [in/out] Supplies to consume */
    void doRefinery(game::map::ShipData& ship, game::IntegerProperty_t& ore, int& sup, const game::spec::Hull& hull)
    {
        const int haveFuel = ship.neutronium.orElse(0);
        const int haveOre  = ore.orElse(0);
        int n = hull.getMaxFuel() - haveOre;
        if (n > haveOre) {
            n = haveOre;
        }
        if (n > sup) {
            n = sup;
        }
        ship.neutronium = haveFuel + n;
        sup -= n;
        ore = haveOre - n;
    }

    // /** Perform direct alchemy reaction (Alchemy + Refinery function on one ship).
    //     \param ship [in/out] Ship
    //     \param ratio [in] Supply conversion ratio */
    void doDirectRefinery(game::map::ShipData& ship, const int ratio, const game::spec::Hull& hull)
    {
        const int haveFuel = ship.neutronium.orElse(0);
        const int haveSupplies = ship.supplies.orElse(0);
        const int free = hull.getMaxFuel() - haveFuel;
        int prod = std::min(haveSupplies / ratio, free);

        ship.supplies = haveSupplies - prod*ratio;
        ship.neutronium = haveFuel + prod;
    }

    // /** Perform alchemy reaction.
    //     Merlin converts 3 Supplies -> 1 Mineral.
    //     Friendly codes allow target mineral control.

    //     THost and NuHost actually convert 9 Supplies -> 3 Minerals;
    //     this is what the %rounder is used for. */
    void doAlchemy(const String_t& shipFCode, game::map::ShipData& ship, const game::HostVersion& host, const game::RegistrationKey& key)
    {
        // Merlin converts 3 Sup -> 1 Min
        int t = 0, d = 0, m = 0;
        int haveSupplies = ship.supplies.orElse(0);
        int mins = haveSupplies / 3;
        int rounder = host.getKind() == host.PHost ? 1 : 3;           // FIXME: make this a property of game::Host
        if (key.getStatus() == game::RegistrationKey::Registered) {   // FIXME: classic had per-player keys
            if (shipFCode == "alt") {
                t = rounder*(mins/rounder);
            } else if (shipFCode == "ald") {
                d = rounder*(mins/rounder);
            } else if (shipFCode == "alm") {
                m = rounder*(mins/rounder);
            } else if (host.getKind() == host.PHost) {
                if (shipFCode == "nat") {
                    d = m = mins/2;
                } else if (shipFCode == "nad") {
                    t = m = mins/2;
                } else if (shipFCode == "nam") {
                    t = d = mins/2;
                } else {
                    t = d = m = mins/3;
                }
            } else {
                t = d = m = mins/3;
            }
        } else {
            t = d = m = mins/3;
        }

        ship.supplies   = haveSupplies - 3*(t+d+m);
        ship.tritanium  = ship.tritanium.orElse(0)  + t;
        ship.duranium   = ship.duranium.orElse(0)   + d;
        ship.molybdenum = ship.molybdenum.orElse(0) + m;
    }

    int getCloakFuel(int turns,
                     int realOwner,
                     const game::config::HostConfiguration& config,
                     const game::spec::Hull& hull)
    {
        const int cfb = config[config.CloakFuelBurn](realOwner);
        int fuel = hull.getMass() * cfb / 100;
        if (fuel < cfb) {
            fuel = cfb;
        }
        if (turns) {
            fuel *= turns;
        }
        return fuel;
    }

    void normalizePosition(game::map::ShipData& ship, const game::map::Configuration& config)
    {
        game::map::Point new_pos = config.getCanonicalLocation(game::map::Point(ship.x.orElse(0), ship.y.orElse(0)));
        ship.x = new_pos.getX();
        ship.y = new_pos.getY();
    }

    int
    getEngineLoad(const game::map::Universe& univ,
                  const game::map::ShipData& ship,
                  int towee_id,
                  const game::map::ShipData* towee_override,
                  bool tow_corr,
                  const game::spec::ShipList& shipList)
    {
        int mass = getShipMass(ship, shipList).orElse(0);
        if (ship.mission.orElse(0) == game::spec::Mission::msn_Tow) {
            int towee_mass = 0;
            int mission_towee_id = ship.missionTowParameter.orElse(0);
            if (towee_override != 0 && towee_id == mission_towee_id) {
                towee_mass = getShipMass(*towee_override, shipList).orElse(0);
            } else if (game::map::Ship* p = univ.ships().get(mission_towee_id)) {
                towee_mass = p->getMass(shipList).orElse(0);
            } else {
                // towee not accessible
            }
            if (tow_corr) {
                towee_mass = 10*(towee_mass/10);
            }
            mass += towee_mass;
        }
        return mass;
    }

    int computeFuelUsage(const game::map::Universe& univ,
                         const game::map::ShipData& ship,
                         int towee_id,
                         const game::map::ShipData* towee_override,
                         bool grav_acc, double dist,
                         const game::spec::ShipList& shipList,
                         const game::config::HostConfiguration& config,
                         const game::HostVersion& host)
    {
        int warp = ship.warpFactor.orElse(0);
        if (warp <= 0) {
            return 0;
        }

        int way = warp*warp;
        if (grav_acc) {
            way *= 2;
        }

        const game::spec::Engine* pEngine = shipList.engines().get(ship.engineType.orElse(0));
        if (!pEngine) {
            return 0;
        }
        int32_t ff;
        if (!pEngine->getFuelFactor(warp, ff)) {
            return 0;
        }

        bool isTHost = host.getKind() != game::HostVersion::PHost;
        int load = getEngineLoad(univ, ship, towee_id, towee_override, isTHost, shipList);

        if (isTHost) {
            // THost formula
            return int((ff * (load / 10) * int32_t(dist)) / (10000L * way));
        } else if (!config[config.UseAccurateFuelModel]()) {
            // PHost, standard formula
            return int(util::divideAndRoundToEven(load, 10, 0) * ff * long(dist) / (10000L * way));
        } else {
            // PHost, "accurate" formula
            return util::roundToInt(load * (1 - std::exp(-(ff * dist) / (way*100000.0))));
        }
    }
}

const int game::map::ShipPredictor::MOVEMENT_TIME_LIMIT;

// /** Constructor. Create a predictor for a single ship.
//     \param univ universe containing the ship
//     \param id   ship Id */
game::map::ShipPredictor::ShipPredictor(const Universe& univ, Id_t id,
                                        const UnitScoreDefinitionList& scoreDefinitions,
                                        const game::spec::ShipList& shipList,
                                        const game::config::HostConfiguration& config,
                                        const HostVersion& hostVersion,
                                        const RegistrationKey& key)
    : m_scoreDefinitions(scoreDefinitions),
      m_shipList(shipList),
      m_hostConfiguration(config),
      m_hostVersion(hostVersion),
      m_key(key),
      id(id), ship(), valid(false), towee_override(0),
      univ(univ), movement_fuel_used(0), cloak_fuel_used(0), turns_computed(0)
{
    // ex GShipTurnPredictor::GShipTurnPredictor
    init();
}

// /** Constructor. Create a predictor for a ship towing another ship.
//     \param univ   universe containing the ship
//     \param id     ship Id
//     \param towee_override predictor for towed ship */
game::map::ShipPredictor::ShipPredictor(const Universe& univ, Id_t id, ShipPredictor& towee,
                                        const UnitScoreDefinitionList& scoreDefinitions,
                                        const game::spec::ShipList& shipList,
                                        const game::config::HostConfiguration& config,
                                        const HostVersion& hostVersion,
                                        const RegistrationKey& key)
    : m_scoreDefinitions(scoreDefinitions),
      m_shipList(shipList),
      m_hostConfiguration(config),
      m_hostVersion(hostVersion),
      m_key(key),
      id(id), ship(), valid(false), towee_override(&towee),
      univ(univ), movement_fuel_used(0), cloak_fuel_used(0), turns_computed(0)
{
    // ex GShipTurnPredictor::GShipTurnPredictor
    init();
}

// /** Get total fuel used for movement. */
int32_t
game::map::ShipPredictor::getMovementFuelUsed() const
{
    // ex GShipTurnPredictor::getMovementFuelUsed
    return movement_fuel_used;
}

// /** Get total amount of fuel used for cloaking. */
int32_t
game::map::ShipPredictor::getCloakFuelUsed() const
{
    // ex GShipTurnPredictor::getCloakFuelUsed
    return cloak_fuel_used;
}

// /** Get number of turns computed. */
int
game::map::ShipPredictor::getNumTurns() const
{
    // ex GShipTurnPredictor::getNumTurns
    return turns_computed;
}

// /** Check whether computation was stopped because the turn limit was exceeded. */
bool
game::map::ShipPredictor::isAtTurnLimit() const
{
    // ex GShipTurnPredictor::isAtTurnLimit
    return turns_computed >= MOVEMENT_TIME_LIMIT;
}

// /** Compute one turn. This will compute all changes for the ship. It
//     will also update the towed ship predictor, if any. */
void
game::map::ShipPredictor::computeTurn()
{
    // ex GShipTurnPredictor::computeTurn
    // is this actually a predictable ship?
    if (!valid) {
        return;
    }

    // where are we?
    int pid = univ.getPlanetAt(Point(ship.x.orElse(0), ship.y.orElse(0)));
    const Ship* real_ship = univ.ships().get(id);
    const game::spec::Hull* pHull = m_shipList.hulls().get(ship.hullType.orElse(0));
    if (real_ship == 0 || pHull == 0) {
        ++turns_computed; // FIXME: needed to make the computeMovement() loop exit eventually. Give this function a success return instead?
        return;
    }

// //     {+++ Mine Laying +++}
// //     IF (s.Neutronium<>0) AND (ComputeMineDrop(@s, md)) THEN BEGIN
// //       Dec(s.Torps, md.torps);
// //     END;

//     // Special Missions I (Super Refit, Self Repair, Hiss, Rob) would go here
// //   {+++ lfm, mkt, ... +++}
// //   IF ((GetStr(s.fc, 3)='mkt')
// //       OR ((pconf<>NIL) AND (s.Mission = pconf^.main.ExtMissionsStartAt + pmsn_BuildTorpsFromCargo)))
// //    AND (s.TLauncher>0) AND (s.TLaunCnt>0) AND (s.Neutronium>0) THEN
// //   BEGIN
// //     { mkt }
// //     i := s.Tritanium;
// //     IF i > s.Duranium THEN i := s.Duranium;
// //     IF i > s.Molybdenum THEN i := s.Molybdenum;
// //     IF Torps[s.TLauncher].TorpCost>0 THEN BEGIN
// //       j := s.Money DIV Torps[s.TLauncher].TorpCost;
// //       IF i > j THEN i := j;
// //     END;
// //     Inc(s.Torps, i);
// //     Dec(s.Tritanium, i);
// //     Dec(s.Duranium, i);
// //     Dec(s.Molybdenum, i);
// //     Dec(s.Money, Torps[s.TLauncher].TorpCost * i);
// //   END;

    // Alchemy
    const String_t shipFCode = ship.friendlyCode.orElse("");
    if (shipFCode != "NAL") {
        if (real_ship->hasSpecialFunction(HullFunction::MerlinAlchemy, m_scoreDefinitions, m_shipList, m_hostConfiguration)) {
            if (m_hostConfiguration[HostConfiguration::AllowAdvancedRefinery]() != 0
                && real_ship->hasSpecialFunction(HullFunction::AriesRefinery, m_scoreDefinitions, m_shipList, m_hostConfiguration))
            {
                // FIXME: handle alm, nam, etc.
                doDirectRefinery(ship, 3, *pHull);
            } else if (real_ship->hasSpecialFunction(HullFunction::NeutronicRefinery, m_scoreDefinitions, m_shipList, m_hostConfiguration)) {
                // FIXME: handle alm, nam, etc.
                doDirectRefinery(ship, 4, *pHull);
            } else {
                doAlchemy(shipFCode, ship, m_hostVersion, m_key);
            }
        } else if (real_ship->hasSpecialFunction(HullFunction::NeutronicRefinery, m_scoreDefinitions, m_shipList, m_hostConfiguration)) {
            // Neutronic refinery converts 1 Sup + 1 Min -> 1 Neu
            // FIXME: handle alm, nam, etc.
            int supplies = ship.supplies.orElse(0);
            doRefinery(ship, ship.tritanium,  supplies, *pHull);
            doRefinery(ship, ship.duranium,   supplies, *pHull);
            doRefinery(ship, ship.molybdenum, supplies, *pHull);
            ship.supplies = supplies;
        } else if (real_ship->hasSpecialFunction(HullFunction::AriesRefinery, m_scoreDefinitions, m_shipList, m_hostConfiguration)) {
            // Aries converts 1 Min -> 1 Neu
            // FIXME: handle alm, nam, etc.
            int supplies = 0x7FFF;  // we assume that no ship has more cargo
            doRefinery(ship, ship.tritanium,  supplies, *pHull);
            doRefinery(ship, ship.duranium,   supplies, *pHull);
            doRefinery(ship, ship.molybdenum, supplies, *pHull);
        } else {
            // Not an alchemy ship
        }
    }

    // Starbase Missions I (Fix, [Recycle, Load Torps])
    if (const Planet* p = univ.planets().get(pid)) {
        // @change PCC2 required Playable but I don't see why ReadOnly shouldn't do.
        if (p->isPlayable(Object::ReadOnly)
            && p->hasFullBaseData()
            && p->getBaseShipyardAction().orElse(0) == FixShipyardAction
            && p->getBaseShipyardId().orElse(0) == id)
        {
            // we're at a base which is fixing us
            ship.damage = 0;
            ship.crew = pHull->getMaxCrew();
        }
    }

    // Supply Repair
    int shipDamage = ship.damage.orElse(0);
    int shipSupplies = ship.supplies.orElse(0);
    if (shipDamage > 0 && shipSupplies > 0) {
        int max = shipSupplies / 5;
        if (max > shipDamage) {
            max = shipDamage;
        }
        ship.damage = shipDamage - max;
        ship.supplies = shipSupplies - 5*max;
    }

    // Cloak Fuel Burn
    bool canCloak = real_ship->hasSpecialFunction(HullFunction::Cloak, m_scoreDefinitions, m_shipList, m_hostConfiguration);
    bool canAdvancedCloak = real_ship->hasSpecialFunction(HullFunction::AdvancedCloak, m_scoreDefinitions, m_shipList, m_hostConfiguration);
    if ((canCloak || canAdvancedCloak) && m_shipList.missions().isMissionCloaking(ship.mission.orElse(0), ship.owner.orElse(0), m_hostConfiguration, m_hostVersion)) {
        int neededFuel = canAdvancedCloak ? 0 : getCloakFuel(0, real_ship->getRealOwner().orElse(0), m_hostConfiguration, *pHull);
        int haveFuel = ship.neutronium.orElse(0);
        if (haveFuel <= neededFuel
            || (ship.damage.orElse(0) >= m_hostConfiguration[HostConfiguration::DamageLevelForCloakFail]()
                && !real_ship->hasSpecialFunction(HullFunction::HardenedCloak, m_scoreDefinitions, m_shipList, m_hostConfiguration)))
        {
            // We cancel only cloak missions here. Other missions are NOT canceled, see below.
            ship.mission = 0;
        } else {
            ship.neutronium = haveFuel - neededFuel;
            cloak_fuel_used += neededFuel;
        }
    }

    /* This used to check for fuelless ships at this place, and set
       them to mission zero. However, since this is mainly used for
       fuel predictions, this is not a good idea. We want to know how
       much we need, even if you have too little. This mainly affects
       the Tow mission. */

    /* damage speed limit */
    shipDamage = ship.damage.orElse(0);
    if (shipDamage > 0 && !real_ship->hasSpecialFunction(HullFunction::HardenedEngines, m_scoreDefinitions, m_shipList, m_hostConfiguration)) {
        int limit = (m_hostConfiguration.getPlayerRaceNumber(ship.owner.orElse(0)) == 2
                     ? (m_hostVersion.getKind() == HostVersion::PHost
                        ? 15 - shipDamage/10
                        : 14 - shipDamage/10)
                     : 10 - shipDamage/10);
        if (limit < 0) {
            limit = 0;
        }
        if (ship.warpFactor.orElse(0) > limit) {
            ship.warpFactor = limit;
        }
    }

    /* Actual movement here */
    int32_t dist2 = (int32_t(ship.waypointDX.orElse(0) * ship.waypointDX.orElse(0))
                     + int32_t(ship.waypointDY.orElse(0) * ship.waypointDY.orElse(0)));
    if (real_ship->hasSpecialFunction(HullFunction::Hyperdrive, m_scoreDefinitions, m_shipList, m_hostConfiguration)
        && shipFCode == "HYP"
        && ship.warpFactor.orElse(0) > 0
        && dist2 >= 20*20)   // FIXME: minimum distance not in PHost?
    {
        // It's hyperjumping
        ship.neutronium = ship.neutronium.orElse(0) - 50;      // FIXME: do not produce negative values!!!1
        movement_fuel_used += 50;

        // If it's jumping, it can't tow. Advance time in towee's world anyway.
        if (ship.mission.orElse(0) == game::spec::Mission::msn_Tow) {
            ship.mission = 0;
        }
        if (towee_override) {
            towee_override->computeTurn();
        }

        // Now move that bugger.
        if (m_hostVersion.isExactHyperjumpDistance2(dist2)) {
            ship.x = ship.x.orElse(0) + ship.waypointDX.orElse(0);
            ship.y = ship.y.orElse(0) + ship.waypointDY.orElse(0);
        } else {
            // non-exact jump
            double dist = 350 / std::sqrt(double(dist2));
            int mx = int(dist * std::abs(ship.waypointDX.orElse(0)) + 0.4999999);
            int my = int(dist * std::abs(ship.waypointDY.orElse(0)) + 0.4999999);
            if (ship.waypointDX.orElse(0) < 0) {
                mx = -mx;
            }
            if (ship.waypointDY.orElse(0) < 0) {
                my = -my;
            }
            ship.x = ship.x.orElse(0) + mx;
            ship.y = ship.y.orElse(0) + my;
        }
        ship.waypointDX = ship.waypointDY = 0;
        ship.warpFactor = 0;
        normalizePosition(ship, univ.config());
        // // FIXME: gravity wells?
    } else if (dist2 > 0 && ship.warpFactor.orElse(0) > 0) {
        // Normal movement
        // First, compute new position in mx,my
        double dist = std::sqrt(double(dist2));
        int way = ship.warpFactor.orElse(0) * ship.warpFactor.orElse(0);
        if (real_ship->hasSpecialFunction(HullFunction::Gravitonic, m_scoreDefinitions, m_shipList, m_hostConfiguration)) {
            way *= 2;
        }

        int mx = ship.waypointDX.orElse(0), my = ship.waypointDY.orElse(0);
        if (dist > way) {
            int dx, dy;
            if (m_hostVersion.getKind() != HostVersion::PHost) {
                // THost movement formulas, from Donovan's
                if (std::abs(mx) > std::abs(my)) {
                    dx = int(double(way) * std::abs(mx) / dist + 0.5);
                    dy = int(double(dx) * std::abs(my) / std::abs(mx) + 0.5);
                } else {
                    dy = int(double(way) * std::abs(my) / dist + 0.5);
                    dx = int(double(dy) * std::abs(mx) / std::abs(my) + 0.5);
                }
                if (mx < 0) {
                    dx = -dx;
                }
                if (my < 0) {
                    dy = -dy;
                }
            } else {
                // PHost. From docs and source.
                double head = util::getHeadingRad(mx, my);
                double fx = std::sin(head) * way;
                double fy = std::cos(head) * way;
                dx = int(fx);
                dy = int(fy);
                if (dx != fx) {
                    dx += sgn(fx);
                }
                if (dy != fy) {
                    dy += sgn(fy);
                }
                if (mx == 0) {
                    dx = 0;
                }
                if (my == 0) {
                    dy = 0;
                }
            }

            // we now have the dx,dy we want to move
            mx = dx;
            my = dy;
            dist = way;         // FIXME: use distFromDX(dx,dy) instead?
        }

        // Advance time in towee. Must be here because we need to know its "post-movement" mass.
        if (towee_override) {
            if (ship.mission.orElse(0) == game::spec::Mission::msn_Tow) {
                // we assume the tow succeeds. FIXME: be more clever?
                if (towee_override->ship.mission.orElse(0) == game::spec::Mission::msn_Tow) {
                    towee_override->ship.mission = 0;
                }
                towee_override->ship.warpFactor = 0;
            }
            towee_override->computeTurn();
        }

        // Compute fuel usage.
        int fuel = computeFuelUsage(univ, ship,
                                    towee_override ? towee_override->id : 0,
                                    towee_override ? &towee_override->ship : 0,
                                    real_ship->hasSpecialFunction(HullFunction::Gravitonic, m_scoreDefinitions, m_shipList, m_hostConfiguration),
                                    dist,
                                    m_shipList, m_hostConfiguration, m_hostVersion);
        ship.neutronium = ship.neutronium.orElse(0) - fuel;
        movement_fuel_used += fuel;

        // We still have the position offset in mx,my. Move it.
        ship.x = ship.x.orElse(0) + mx;
        ship.y = ship.y.orElse(0) + my;
        ship.waypointDX = ship.waypointDX.orElse(0) - mx;
        ship.waypointDY = ship.waypointDY.orElse(0) - my;
        normalizePosition(ship, univ.config());

        // Warp wells
        int wp_x = ship.x.orElse(0) + ship.waypointDX.orElse(0);
        int wp_y = ship.y.orElse(0) + ship.waypointDY.orElse(0);

        if (ship.warpFactor.orElse(0) > 0 && univ.getPlanetAt(Point(ship.x.orElse(0), ship.y.orElse(0))) == 0) {
            Id_t gpid = univ.getGravityPlanetAt(Point(ship.x.orElse(0), ship.y.orElse(0)), m_hostConfiguration, m_hostVersion);
            if (const Planet* p = univ.planets().get(gpid)) {
                // Okay, there is a planet. Move the ship.
                Point ppos;
                if (p->getPosition(ppos)) {
                    ship.x = ppos.getX();
                    ship.y = ppos.getY();

                    // Now adjust the waypoint. If the waypoint was inside
                    // the warp well of the same planet, assume the end of
                    // this movement order. Otherwise, when users set a
                    // waypoint at the edge of the warp well, the ship
                    // would try to get there for ever. This is consistent
                    // with the CCScript `MoveTo' command and with what
                    // most people expect.
                    if (univ.getPlanetAt(Point(wp_x, wp_y), true, m_hostConfiguration, m_hostVersion) == gpid) {
                        ship.waypointDX = 0;
                        ship.waypointDY = 0;
                    } else {
                        Point new_wp = univ.config().getSimpleNearestAlias(Point(wp_x, wp_y), Point(ship.x.orElse(0), ship.y.orElse(0)));
                        ship.waypointDX = new_wp.getX() - ship.x.orElse(0);
                        ship.waypointDY = new_wp.getY() - ship.y.orElse(0);
                    }
                }
            }
        }

        // Update towee position
        if (towee_override) {
            if (ship.mission.orElse(0) == game::spec::Mission::msn_Tow) {
                towee_override->ship.x = ship.x;
                towee_override->ship.y = ship.y;
                towee_override->ship.waypointDX = 0;
                towee_override->ship.waypointDY = 0;
            }
        }
    } else {
        // No sensible movement order for this ship. Advance towee's time anyway.
        if (towee_override) {
            towee_override->computeTurn();
        }
    }

    // turn is over.
    ++turns_computed;
}

// /** Compute this ship's movement, until movement is over or time runs out. */
void
game::map::ShipPredictor::computeMovement()
{
    // ex GShipTurnPredictor::computeMovement
    if (valid) {
        const int final_turn = turns_computed + MOVEMENT_TIME_LIMIT;
        while ((ship.waypointDX.orElse(0) || ship.waypointDY.orElse(0)) && turns_computed < final_turn) {
            computeTurn();
            if (ship.neutronium.orElse(0) < 0) {
                ship.neutronium = 0;
            }
        }
    }
}

// /** Override this ship's position. */
void
game::map::ShipPredictor::setPosition(Point pt)
{
    // ex GShipTurnPredictor::setPosition
    pt = univ.config().getSimpleCanonicalLocation(pt);
    ship.x = pt.getX();
    ship.y = pt.getY();
}

// /** Override this ship's waypoint. */
void
game::map::ShipPredictor::setWaypoint(Point pt)
{
    // ex GShipTurnPredictor::setWaypoint
    pt = univ.config().getSimpleNearestAlias(pt, Point(ship.x.orElse(0), ship.y.orElse(0)));
    ship.waypointDX = pt.getX() - ship.x.orElse(0);
    ship.waypointDY = pt.getY() - ship.y.orElse(0);
}

// /** Override this ship's speed. */
void
game::map::ShipPredictor::setWarpFactor(int warp)
{
    // ex GShipTurnPredictor::setSpeed
    ship.warpFactor = warp;
}

// /** Override this ship's engine. */
void
game::map::ShipPredictor::setEngineType(int engine)
{
    // ex GShipTurnPredictor::setEngine
    ship.engineType = engine;
}

// /** Override this ship's mission. */
void
game::map::ShipPredictor::setMission(int m, int i, int t)
{
    // ex GShipTurnPredictor::setMission
    ship.mission                   = m;
    ship.missionInterceptParameter = i;
    ship.missionTowParameter       = t;
}

// /** Override this ship's friendly code. */
void
game::map::ShipPredictor::setFriendlyCode(String_t s)
{
    // ex GShipTurnPredictor::setFCode
    ship.friendlyCode = s;
}

// /** Override this ship's amount of fuel. */
void
game::map::ShipPredictor::setFuel(int fuel)
{
    // ex GShipTurnPredictor::setFuel
    ship.neutronium = fuel;
}

// /** Check whether this ship has reached its waypoint. */
bool
game::map::ShipPredictor::isAtWaypoint() const
{
    // ex GShipTurnPredictor::isAtWaypoint
    return ship.waypointDX.orElse(0) == 0 && ship.waypointDY.orElse(0) == 0;
}

// /** Get this ship's current position. */
game::map::Point
game::map::ShipPredictor::getPosition() const
{
    // ex GShipTurnPredictor::getPosition
    return Point(ship.x.orElse(0), ship.y.orElse(0));
}

// /** Get this ship's current amount of fuel. */
int
game::map::ShipPredictor::getFuel() const
{
    // ex GShipTurnPredictor::getFuel
    return ship.neutronium.orElse(0);
}

// /** Get cargo amount. */
int
game::map::ShipPredictor::getCargo(Element::Type el) const
{
    // ex GShipTurnPredictor::getCargo
    switch (el) {
     case Element::Neutronium:
        return ship.neutronium.orElse(0);
     case Element::Tritanium:
        return ship.tritanium.orElse(0);
     case Element::Duranium:
        return ship.duranium.orElse(0);
     case Element::Molybdenum:
        return ship.molybdenum.orElse(0);
     case Element::Fighters:
        return ship.numBays.orElse(0) > 0 ? ship.ammo.orElse(0) : 0;
     case Element::Colonists:
        return ship.colonists.orElse(0);
     case Element::Supplies:
        return ship.supplies.orElse(0);
     case Element::Money:
        return ship.money.orElse(0);
     default:
        int tt;
        if (Element::isTorpedoType(el, tt) && tt == ship.launcherType.orElse(0) && ship.numLaunchers.orElse(0) > 0) {
            // We know it has this type of torpedoes
            return ship.ammo.orElse(0);
        } else {
            return 0;
        }
    }
}

// /** Get this ship's current speed. */
int
game::map::ShipPredictor::getWarpFactor() const
{
    // ex GShipTurnPredictor::getSpeed
    return ship.warpFactor.orElse(0);
}

// /** Get this ship's real owner. */
int
game::map::ShipPredictor::getRealOwner() const
{
    // ex GShipTurnPredictor::getRealOwner
    return ship.owner.orElse(0);
}

// /** Get this ship's friendly code. */
String_t
game::map::ShipPredictor::getFriendlyCode() const
{
    // ex GShipTurnPredictor::getFCode
    return ship.friendlyCode.orElse(0);
}

// /** Get the universe used for predicting. */
const game::map::Universe&
game::map::ShipPredictor::getUniverse() const
{
    // ex GShipTurnPredictor::getUniverse
    return univ;
}


void
game::map::ShipPredictor::init()
{
    // ex GShipTurnPredictor::init
    const Ship* p = univ.ships().get(id);
    if (p != 0 && p->hasFullShipData()) {
        p->getCurrentShipData(this->ship);
        ship.owner = p->getRealOwner();
        valid = true;
    } else {
        valid = false;
    }
}
