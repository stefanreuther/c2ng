/**
  *  \file game/sim/transfer.cpp
  *  \brief Class game::sim::Transfer
  */

#include "game/sim/transfer.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/actions/cargotransfersetup.hpp"
#include "game/map/fleetmember.hpp"
#include "game/map/planetformula.hpp"
#include "game/map/planetstorage.hpp"
#include "game/map/shipstorage.hpp"
#include "game/sim/configuration.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/ship.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/hullfunction.hpp"
#include "game/spec/mission.hpp"

using game::Element;
using game::spec::HullFunction;
using game::spec::Mission;

game::sim::Transfer::Transfer(const UnitScoreDefinitionList& scoreDefinitions,
                              const game::spec::ShipList& shipList,
                              const game::config::HostConfiguration& config,
                              HostVersion hostVersion,
                              afl::string::Translator& tx)
    : m_scoreDefinitions(scoreDefinitions),
      m_shipList(shipList),
      m_config(config),
      m_hostVersion(hostVersion),
      m_translator(tx)
{ }

bool
game::sim::Transfer::copyShipFromGame(Ship& out, const game::map::Ship& in) const
{
    // ex GSimulatorRealGameInterface::copyShip (totally reworked)
    // Must know at least owner and hull
    int owner, hullNr;
    if (!in.getOwner(owner) || !in.getHull().get(hullNr)) {
        return false;
    }
    const game::spec::Hull* pHull = m_shipList.hulls().get(hullNr);
    if (pHull == 0) {
        return false;
    }

    // Id
    out.setId(in.getId());

    // Name
    {
        String_t name = in.getName();
        if (!name.empty()) {
            out.setName(name);
        }
        if (out.getName().empty()) {
            out.setDefaultName(m_translator);
        }
    }

    // FCode
    out.setFriendlyCode(in.getFriendlyCode().orElse("???"));

    // Damage
    out.setDamage(std::max(0, in.getDamage().orElse(0) - in.getCargo(Element::Supplies).orElse(0) / 5));

    // Shield
    out.setShield(std::max(0, 100 - out.getDamage()));

    // Owner
    out.setOwner(in.getRealOwner().orElse(owner));

    // Experience
    out.setExperienceLevel(in.getScore(ScoreId_ExpLevel, m_scoreDefinitions).orElse(0));

    // Crew
    out.setCrew(in.getCrew().orElse(pHull->getMaxCrew()));

    // Hull Type
    out.setHullTypeOnly(hullNr);

    // Mass
    out.setMass(pHull->getMass());

    // Beams
    {
        int numBeams = in.getNumBeams().orElse(pHull->getMaxBeams());
        int beamType = in.getBeamType().orElse(m_shipList.beams().size());
        if (numBeams > 0 && beamType > 0) {
            out.setNumBeams(numBeams);
            out.setBeamType(beamType);
        } else {
            out.setNumBeams(0);
            out.setBeamType(0);
        }
    }

    // Launchers
    {
        int numLaunchers = in.getNumLaunchers().orElse(pHull->getMaxLaunchers());
        int torpedoType = in.getTorpedoType().orElse(m_shipList.launchers().size());
        if (numLaunchers > 0 && torpedoType > 0) {
            out.setNumLaunchers(numLaunchers);
            out.setTorpedoType(torpedoType);
        } else {
            out.setNumLaunchers(0);
            out.setTorpedoType(0);
        }
    }

    // Bays
    out.setNumBays(in.getNumBays().orElse(pHull->getNumBays()));

    // Ammo
    if (out.getNumLaunchers() != 0 || out.getNumBays() != 0) {
        out.setAmmo(in.getAmmo().orElse(pHull->getMaxCargo()));
    } else {
        out.setAmmo(0);
    }

    // Engine
    out.setEngineType(in.getEngineType().orElse(m_shipList.engines().size()));

    // Aggressiveness
    const int fuel = in.getCargo(Element::Neutronium).orElse(-1);
    const int mission = in.getMission().orElse(Mission::msn_Kill);
    const int pe = in.getPrimaryEnemy().orElse(0);
    if (fuel == 0) {
        out.setAggressiveness(Ship::agg_NoFuel);
    } else if (mission == Mission::msn_Kill) {
        out.setAggressiveness(Ship::agg_Kill);
    } else {
        out.setAggressiveness(pe);
    }

    // Intercept
    const bool cloakable = in.hasSpecialFunction(HullFunction::Cloak, m_scoreDefinitions, m_shipList, m_config);
    if (mission == Mission::msn_Intercept && cloakable) {
        out.setInterceptId(in.getMissionParameter(InterceptParameter).orElse(0));
    } else {
        out.setInterceptId(0);
    }

    // Flags
    int32_t flags = 0;
    if (fuel > 0 && cloakable && m_shipList.missions().isMissionCloaking(mission, out.getOwner(), m_config, m_hostVersion)) {
        flags |= Ship::fl_Cloaked;
    }
    setHullFunction(flags, out, in, FullWeaponryAbility,   HullFunction::FullWeaponry);
    setHullFunction(flags, out, in, PlanetImmunityAbility, HullFunction::PlanetImmunity);
    setHullFunction(flags, out, in, CommanderAbility,      HullFunction::Commander);
    out.setFlags(flags);
    return true;
}

bool
game::sim::Transfer::copyShipToGame(game::map::Ship& out, const Ship& in, game::map::Universe& univ) const
{
    // ex GSimulatorRealGameInterface::updateToGame
    int owner, hullNr;
    if (!out.getOwner(owner) || !out.getHull().get(hullNr)) {
        return false;
    }
    const int realOwner = out.getRealOwner().orElse(owner);
    if (!out.isPlayable(game::map::Object::Playable) || realOwner != in.getOwner() || hullNr != in.getHullType()) {
        return false;
    }

    // Mission
    // Do not touch fleet leaders/members here for now, but use FleetMember for implicit intercept waypoint propagation
    if (out.getFleetNumber() == 0) {
        game::map::FleetMember mem(univ, out);
        if (in.getAggressiveness() == Ship::agg_Kill) {
            // Aggressiveness Kill -> set Kill mission
            mem.setMission(Mission::msn_Kill, 0, 0, m_config, m_shipList);
        } else {
            const int oldMission = out.getMission().orElse(0);
            const bool isCloaking = m_shipList.missions().isMissionCloaking(oldMission, realOwner, m_config, m_hostVersion);
            if (out.hasSpecialFunction(HullFunction::Cloak, m_scoreDefinitions, m_shipList, m_config) && (in.getFlags() & Ship::fl_Cloaked) != 0) {
                // Ship can cloak -> set a cloak mission unless it already has one
                if (!isCloaking) {
                    mem.setMission(Mission::msn_Cloak, 0, 0, m_config, m_shipList);
                }
            } else {
                // Ship shall not cloak -> reset Cloak mission if any
                // Ship shall not kill -> reset Kill mission if any
                if (isCloaking || oldMission == Mission::msn_Kill) {
                    mem.setMission(0, 0, 0, m_config, m_shipList);
                }

                const int intId = in.getInterceptId();
                if (intId != 0) {
                    // Take over Intercept mission if allowed
                    const game::map::Ship* intShip = univ.ships().get(intId);
                    if (intShip != 0 && intShip->isReliablyVisible(0)) {
                        mem.setMission(Mission::msn_Intercept, intId, 0, m_config, m_shipList);
                    }
                }
            }
        }
    }

    // PE
    if (Ship::isPrimaryEnemy(in.getAggressiveness())) {
        out.setPrimaryEnemy(in.getAggressiveness());
    } else {
        out.setPrimaryEnemy(0);
    }

    // FCode, Name
    out.setFriendlyCode(in.getFriendlyCode());
    out.setName(in.getName());

    // Ammo
    // FIXME: in case we handle mkt/lfm someday, we would have to revert it here
    int simAmmo = in.getAmmo();
    int shipAmmo = out.getAmmo().orElse(simAmmo);
    game::map::Point pt;
    game::map::Planet* planet = 0;
    if (simAmmo != shipAmmo
        && out.getPosition(pt)
        && (planet = univ.planets().get(univ.findPlanetAt(pt))) != 0
        && planet->isPlayable(game::map::Object::Playable))
    {
        // Preconditions for client-side transfer are fulfilled.
        // Use CargoTransfer to check correctness of the transfer.
        // Otherwise, we build the transfer manually because CargoTransferSetup has larger dependencies than we offer.
        try {
            game::actions::CargoTransfer tr;
            tr.addNew(new game::map::PlanetStorage(*planet, m_config));
            tr.addNew(new game::map::ShipStorage(out, m_shipList));

            if (out.getNumBays().orElse(0) != 0) {
                tr.move(Element::Fighters, simAmmo - shipAmmo, 0, 1, true, false);
            } else if (out.getNumLaunchers().orElse(0) != 0) {
                tr.move(Element::fromTorpedoType(out.getTorpedoType().orElse(1)), simAmmo - shipAmmo, 0, 1, true, false);
            } else {
                // should not happen
            }
            tr.commit();
        }
        catch (...) {
            // Ignore possible errors, e.g. planet doesn't have a starbase or we failed at the precondition check
        }
    }

    return true;
}

bool
game::sim::Transfer::copyPlanetFromGame(Planet& out, const game::map::Planet& in) const
{
    // ex GSimulatorRealGameInterface::copyPlanet
    // We cannot do anything sensible if we don't know the owner
    int owner = 0;
    if (!in.getOwner(owner) || owner == 0) {
        return false;
    }

    // Id, Name
    out.setId(in.getId());
    out.setName(in.getName(m_translator));

    // Friendly Code
    out.setFriendlyCode(in.getFriendlyCode().orElse("???"));

    // Damage/Shield
    // FIXME: can we do better?
    out.setDamage(0);
    out.setShield(100);

    // Owner
    out.setOwner(owner);

    // Experience
    out.setExperienceLevel(0);      // FIXME: need to access planet (same problem in PCC2)

    // Flags: there are no flags relevant for planets so far
    out.setFlags(0);

    // Defense
    {
        int defense;
        int32_t maxDefense;
        if (in.getNumBuildings(DefenseBuilding).get(defense)) {
            out.setDefense(defense);
        } else if (getMaxBuildings(in, DefenseBuilding, m_config).get(maxDefense)) {
            out.setDefense(maxDefense);
        } else {
            out.setDefense(10);
        }
    }

    // Starbase
    int beamTech;
    if (in.hasBase() && in.getBaseTechLevel(BeamTech).get(beamTech) && beamTech != 0) {
        // Base present
        out.setBaseBeamTech(beamTech);
        out.setBaseDefense(in.getNumBuildings(BaseDefenseBuilding).orElse(0));
        out.setBaseTorpedoTech(in.getBaseTechLevel(TorpedoTech).orElse(1));
        out.setNumBaseFighters(in.getCargo(Element::Fighters).orElse(0));
        for (int i = 1; i <= Planet::NUM_TORPEDO_TYPES; ++i) {
            out.setNumBaseTorpedoes(i, in.getCargo(Element::fromTorpedoType(i)).orElse(0));
        }
        out.setBaseDamage(in.getBaseDamage().orElse(0));
    } else {
        // No base
        out.setBaseBeamTech(0);
        out.setBaseDefense(0);
        out.setBaseTorpedoTech(0);
        out.setNumBaseFighters(0);
        for (int i = 1; i <= Planet::NUM_TORPEDO_TYPES; ++i) {
            out.setNumBaseTorpedoes(i, 0);
        }
        out.setBaseDamage(0);
    }

    return true;
}

bool
game::sim::Transfer::copyPlanetToGame(game::map::Planet& out, const Planet& in) const
{
    // Check applicability
    int owner;
    if (!out.getOwner(owner)) {
        return false;
    }
    if (!out.isPlayable(game::map::Object::Playable) || owner != in.getOwner()) {
        return false;
    }

    // Copy
    // The only thing we can safely copy is the friendly code for now
    out.setFriendlyCode(in.getFriendlyCode());
    return true;
}

void
game::sim::Transfer::setHullFunction(int32_t& flags, const Ship& out, const game::map::Ship& in, Ability a, int basicHullFunction) const
{
    // ex game/sim-game.cc:setHullFunction
    // FIXME: we pass a blank Configuration() to hasImpliedFunction. For now, this configuration does not affect anything.
    // If it starts affecting things, we should pass one that matches version/config, because we probably want to sim
    // "this game's host" when we add "this game's ship".
    const bool shipCanDo = in.hasSpecialFunction(basicHullFunction, m_scoreDefinitions, m_shipList, m_config);
    const bool simCanDo = out.hasImpliedAbility(a, Configuration(), m_shipList, m_config);
    if (shipCanDo != simCanDo) {
        const Object::AbilityInfo info = Object::getAbilityInfo(a);
        flags |= info.setBit;
        if (shipCanDo) {
            flags |= info.activeBit;
        }
    }
}
