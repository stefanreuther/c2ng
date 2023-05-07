/**
  *  \file game/actions/cloneship.cpp
  *  \brief Class game::actions::CloneShip
  */

#include "game/actions/cloneship.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "game/actions/preconditions.hpp"
#include "game/map/fleetmember.hpp"
#include "game/map/shiputils.hpp"
#include "game/registrationkey.hpp"
#include "game/spec/cost.hpp"
#include "game/actions/convertsupplies.hpp"

using game::spec::Cost;

namespace {
    const char*const CLN_FCODE = "cln";

    /* Find ship cloning at a planet; do not count a given ship */
    game::Id_t findPreviousCloningShip(const game::map::Universe& univ, game::Id_t planetId, game::Id_t notThisShip)
    {
        game::Id_t found = univ.findShipCloningAt(planetId, 0);
        if (found == notThisShip) {
            found = univ.findShipCloningAt(planetId, notThisShip);
        }
        return found;
    }
}

game::actions::CloneShip::CloneShip(game::map::Planet& planet,
                                    game::map::Ship& ship,
                                    game::map::Universe& univ,
                                    const UnitScoreDefinitionList& shipScores,
                                    game::spec::ShipList& shipList,
                                    Root& root)
    : m_ship(ship),
      m_planet(planet),
      m_universe(univ),
      m_root(root),
      m_shipList(shipList),
      m_shipScores(shipScores),
      m_costPlanet(planet, root.hostConfiguration()),
      m_costAction(m_costPlanet),
      m_techPlanet(planet, root.hostConfiguration()),
      m_techUpgrade(planet, m_techPlanet, shipList, root),
      m_shipCost(),
      m_techFailure(false)
{
    update();
}

void
game::actions::CloneShip::commit(const game::map::Configuration& mapConfig, util::RandomNumberGenerator& rng)
{
    // Commit the tech upgrade
    // This will fail with an exception if there is not enough money.
    m_techUpgrade.commit();

    // Sell supplies to (try to) have enough cash available.
    int32_t availableMoney = m_planet.getCargo(Element::Money).orElse(0);
    int32_t neededMoney = m_shipCost.get(Cost::Money);
    if (availableMoney < neededMoney) {
        ConvertSupplies conv(m_planet);
        conv.setReservedSupplies(m_shipCost.get(Cost::Supplies));
        conv.sellSupplies(neededMoney - availableMoney, true);
    }

    // Leave fleet
    game::map::FleetMember fm(m_universe, m_ship, mapConfig);
    fm.setFleetNumber(0, m_root.hostConfiguration(), m_shipList);

    // Give the order
    cancelAllCloneOrders(m_universe, m_planet, m_shipList.friendlyCodes(), rng);
    m_ship.setFriendlyCode(String_t(CLN_FCODE));

    game::map::Point pos;
    if (m_ship.getPosition().get(pos)) {
        // Clear waypoint
        fm.setWaypoint(pos, m_root.hostConfiguration(), m_shipList);
        fm.setWarpFactor(0, m_root.hostConfiguration(), m_shipList);

        // PCC2 would explicitly cancel intercept here.
        // That has already been done in setWaypoint() if the position differs from the waypoint.
        // Only intercepts to ships at this location remain; this ship won't move anyway due to warp 0.
    }
}

game::ShipBuildOrder
game::actions::CloneShip::getBuildOrder() const
{
    ShipBuildOrder result;
    result.setHullIndex(m_ship.getHull().orElse(0));
    result.setEngineType(m_ship.getEngineType().orElse(0));
    result.setBeamType(m_ship.getBeamType().orElse(0));
    result.setNumBeams(m_ship.getNumBeams().orElse(0));
    result.setTorpedoType(m_ship.getTorpedoType().orElse(0));
    result.setNumLaunchers(m_ship.getNumLaunchers().orElse(0));
    return result;
}

game::actions::CloneShip::OrderStatus
game::actions::CloneShip::getOrderStatus() const
{
    // ex doCloneShip (part), bdata.pas:CloneAShip (part)
    const int planetOwner = m_planet.getOwner().orElse(0);
    const int hullNr = m_ship.getHull().orElse(0);
    const int hullSlot = m_shipList.hullAssignments().getIndexFromHull(m_root.hostConfiguration(), planetOwner, hullNr);

    // What do we do?
    if (hullSlot != 0) {
        return CanBuild;
    } else {
        // Check registration status
        // @change If we have a definition of 'cln', check that instead of the registration status only
        const int rso = m_ship.getRealOwner().orElse(0);
        if (!m_shipList.friendlyCodes().isAcceptedFriendlyCode(CLN_FCODE, game::spec::FriendlyCode::Filter::fromShip(m_ship, m_shipScores, m_shipList, m_root.hostConfiguration()),
                                                               m_root.registrationKey(), game::spec::FriendlyCodeList::DefaultRegistered))
        {
            return PlayerCannotClone;
        }

        // Check host-specific rule
        if (!m_root.hostVersion().isPHost()) {
            // Tim-Host: some players may not be able to clone
            int playerRace = m_root.hostConfiguration().getPlayerRaceNumber(rso);
            if (playerRace == 5 || playerRace == 7) {
                return PlayerCannotClone;
            }
        } else {
            // PHost: everyone can clone (at possibly prohibitive prices), but we need to deal with remote-control
            if (m_shipList.hullAssignments().getIndexFromHull(m_root.hostConfiguration(), rso, hullNr) != 0) {
                return RemoteOwnerCanBuild;
            }
        }

        // Check hull functions
        if (m_ship.hasSpecialFunction(game::spec::BasicHullFunction::Unclonable, m_shipScores, m_shipList, m_root.hostConfiguration())) {
            return ShipIsUnclonable;
        }

        // Tech failure status, defined in update()
        if (m_techFailure) {
            return TechLimitExceeded;
        }

        // When we're here, we'll try to clone.
        // Check getPaymentStatus() for further errors.
        return CanClone;
    }
}

game::actions::CloneShip::PaymentStatus
game::actions::CloneShip::getPaymentStatus() const
{
    if (!const_cast<TechUpgrade&>(m_techUpgrade).isValid()) {
        // Not enough cash to upgrate tech
        return CannotPayTech;
    } else if (!m_costAction.isValid()) {
        // Can upgrade tech, but not build ship
        return CannotPayComponents;
    } else {
        // Everything fine
        return CanPay;
    }
}

const game::actions::CargoCostAction&
game::actions::CloneShip::getCloneAction() const
{
    return m_costAction;
}

const game::actions::CargoCostAction&
game::actions::CloneShip::getTechUpgradeAction() const
{
    return m_techUpgrade.costAction();
}

game::actions::CloneShip::ConflictStatus
game::actions::CloneShip::findConflict(Conflict* result, afl::string::Translator& tx, const InterpreterInterface& iface) const
{
    // ex doCloneShip (part), bdata.pas:CloneAShip (part)
    if (Id_t previousCloningShip = findPreviousCloningShip(m_universe, m_planet.getId(), m_ship.getId())) {
        // It's cloning
        if (result != 0) {
            result->id = previousCloningShip;
            if (const game::map::Ship* p = m_universe.ships().get(previousCloningShip)) {
                result->name = p->getName(LongName, tx, iface);
            }
        }
        return IsCloning;
    } else if (int hullNr = m_planet.getBaseBuildHull(m_root.hostConfiguration(), m_shipList.hullAssignments()).orElse(0)) {
        // It's building normally
        if (result != 0) {
            result->id = hullNr;
            if (const game::spec::Hull* p = m_shipList.hulls().get(hullNr)) {
                result->name = p->getName(m_shipList.componentNamer());
            }
        }
        return IsBuilding;
    } else {
        // No conflict
        return NoConflict;
    }
}

bool
game::actions::CloneShip::isCloneOnce() const
{
    return m_ship.hasSpecialFunction(game::spec::BasicHullFunction::CloneOnce, m_shipScores, m_shipList, m_root.hostConfiguration());
}

const game::map::Ship&
game::actions::CloneShip::ship() const
{
    return m_ship;
}

const game::map::Planet&
game::actions::CloneShip::planet() const
{
    return m_planet;
}

void
game::actions::CloneShip::update()
{
    // ex WCloneCargoCostTransaction::update()
    // Compute ship cost
    int needTech[NUM_TECH_AREAS];

    game::actions::mustBePlayed(m_ship);
    game::actions::mustHavePlayedBase(m_planet);

    // Hull
    const game::spec::Hull* hull = m_shipList.hulls().get(m_ship.getHull().orElse(0));
    afl::except::checkAssertion(hull != 0, "<CloneShip: hull>");

    Cost shipCost = hull->cost();
    needTech[HullTech] = hull->getTechLevel();

    // Engine
    const game::spec::Engine* engine = m_shipList.engines().get(m_ship.getEngineType().orElse(0));
    afl::except::checkAssertion(engine != 0, "<CloneShip: engine>");

    shipCost += engine->cost() * hull->getNumEngines();
    needTech[EngineTech] = engine->getTechLevel();

    // Beams
    const int numBeams = m_ship.getNumBeams().orElse(0);
    if (numBeams > 0) {
        const game::spec::Beam* beam = m_shipList.beams().get(m_ship.getBeamType().orElse(0));
        afl::except::checkAssertion(beam != 0, "<CloneShip: beam>");

        shipCost += beam->cost() * numBeams;
        needTech[BeamTech] = beam->getTechLevel();
    } else {
        needTech[BeamTech] = 1;
    }

    // Torpedo launchers
    const int numLaunchers = m_ship.getNumLaunchers().orElse(0);
    if (numLaunchers > 0) {
        const game::spec::TorpedoLauncher* tl = m_shipList.launchers().get(m_ship.getTorpedoType().orElse(0));
        afl::except::checkAssertion(tl != 0, "<CloneShip: launcher>");

        shipCost += tl->cost() * numLaunchers;
        needTech[TorpedoTech] = tl->getTechLevel();
    } else {
        needTech[TorpedoTech] = 1;
    }

    // Determine CloneCostRate.
    int ccr = m_root.hostConfiguration()[game::config::HostConfiguration::ShipCloneCostRate](m_ship.getRealOwner().orElse(0));

    // Apply CloneCostRate. We must be careful to avoid overflow.
    // The naive computation, mc*ccr/100, would overflow when ccr=32767 and mc>65536.
    // The most expensive ship known to me is a fully-loaded T10 Basilisk at $35k,
    // so it's not hard to invent a $70k ship.
    // This computation multiplies with no more than 327, allowing for ship costs up to $6.5M.
    shipCost.set(Cost::Money,
                 shipCost.get(Cost::Money) * (ccr / 100)
                 + (shipCost.get(Cost::Money) * (ccr % 100)) / 100);
    m_shipCost = shipCost;

    // Check tech
    m_techFailure = false;
    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
        if (!m_techUpgrade.upgradeTechLevel(TechLevel(i), needTech[i])) {
            m_techFailure = true;
        }
    }

    // Configure cargo cost
    Cost total = shipCost;
    total += m_techUpgrade.costAction().getCost();
    m_costAction.setCost(total);
}
