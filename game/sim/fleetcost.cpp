/**
  *  \file game/sim/fleetcost.cpp
  *  \brief Fleet Cost Computation
  */

#include "game/sim/fleetcost.hpp"
#include "afl/string/format.hpp"
#include "game/map/planetformula.hpp"
#include "game/sim/configuration.hpp"
#include "game/sim/object.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"

using afl::string::Format;
using game::spec::Cost;
using game::map::getBaseTechCost;
using game::config::HostConfiguration;
using game::spec::CostSummary;

namespace game { namespace sim { namespace {

    String_t getShipName(const Ship& sh, const game::spec::ShipList& shipList, const PlayerList& playerList, afl::string::Translator& tx)
    {
        const String_t playerAdjective = playerList.getPlayerName(sh.getOwner(), game::Player::AdjectiveName);
        String_t typeInfo;
        if (const game::spec::Hull* h = shipList.hulls().get(sh.getHullType())) {
            typeInfo = Format(tx("%s %s"), playerAdjective, h->getShortName(shipList.componentNamer()));
        } else {
            typeInfo = Format(tx("%s custom ship"), playerAdjective);
        }
        return Format("%s (#%d, %s)", sh.getName(), sh.getId(), typeInfo);
    }

    
    void addTech(Cost& cost, int player, int& haveTech, int needTech, const FleetCostOptions& opts, const HostConfiguration& config)
    {
        switch (opts.shipTechMode) {
         case FleetCostOptions::NoTech:
            break;

         case FleetCostOptions::PlayerTech:
            if (needTech > haveTech) {
                cost.add(Cost::Money, getBaseTechCost(player, haveTech, needTech, config));
                haveTech = needTech;
            }
            break;

         case FleetCostOptions::ShipTech:
            cost.add(Cost::Money, getBaseTechCost(player, 1, needTech, config));
            break;
        }
    }

    void addFighters(Cost& cost, int player, int count, const FleetCostOptions& opts, const HostConfiguration& config)
    {
        // ex ccsim.pas:AddFighters
        switch (opts.fighterMode) {
         case FleetCostOptions::FreeFighters:
            break;
         case FleetCostOptions::ShipFighters:
            cost += config[HostConfiguration::ShipFighterCost](player) * count;
            break;
         case FleetCostOptions::BaseFighters:
            cost += config[HostConfiguration::BaseFighterCost](player) * count;
            break;
        }
    }

} } }


void
game::sim::computeFleetCosts(game::spec::CostSummary& out,
                             const Setup& in,
                             const Configuration& simConfig,
                             const FleetCostOptions& opts,
                             const game::spec::ShipList& shipList,
                             const game::config::HostConfiguration& config,
                             const PlayerList& playerList,
                             PlayerSet_t players,
                             afl::string::Translator& tx)
{
    // ex WFleetCostDialog::generateSummary (part), ccsim.pas:GenerateFleetCostBill

    // Ships
    int hullTech = 1;
    int beamTech = 1;
    int torpTech = 1;
    int engTech = 1;
    for (Setup::Slot_t i = 0, n = in.getNumShips(); i < n; ++i) {
        if (const Ship* sh = in.getShip(i)) {
            if (players.contains(sh->getOwner())) {
                Cost cost;

                // Hull, Engine
                int numEngines = 0;
                if (const game::spec::Hull* p = shipList.hulls().get(sh->getHullType())) {
                    cost += p->cost();
                    numEngines = p->getNumEngines();
                    addTech(cost, sh->getOwner(), hullTech, p->getTechLevel(), opts, config);
                }
                if (const game::spec::Engine* p = shipList.engines().get(sh->getEngineType())) {
                    if (opts.useEngines) {
                        cost += p->cost() * numEngines;
                        addTech(cost, sh->getOwner(), engTech, p->getTechLevel(), opts, config);
                    }
                }

                // Beams
                if (sh->getNumBeams() != 0) {
                    if (const game::spec::Beam* p = shipList.beams().get(sh->getBeamType())) {
                        cost += p->cost() * sh->getNumBeams();
                        addTech(cost, sh->getOwner(), beamTech, p->getTechLevel(), opts, config);
                    }
                }

                // Torps/Fighters
                if (sh->getNumLaunchers() != 0) {
                    if (const game::spec::TorpedoLauncher* p = shipList.launchers().get(sh->getTorpedoType())) {
                        cost += p->cost() * sh->getNumLaunchers();
                        if (opts.useTorpedoes) {
                            cost += p->torpedoCost() * sh->getAmmo();
                        }
                        addTech(cost, sh->getOwner(), torpTech, p->getTechLevel(), opts, config);
                    }
                } else if (sh->getNumBays() != 0) {
                    addFighters(cost, sh->getOwner(), sh->getAmmo(), opts, config);
                }

                out.add(CostSummary::Item(0, 1, getShipName(*sh, shipList, playerList, tx), cost));
            }
        }
    }

    // Planet
    const Planet* pl = in.getPlanet();
    if (pl != 0 && players.contains(pl->getOwner())) {
        if (opts.usePlanetDefense && pl->getDefense() != 0) {
            Cost cost;
            cost.set(Cost::Money, 10*pl->getDefense());
            cost.set(Cost::Supplies, pl->getDefense());
            out.add(CostSummary::Item(0, 1, tx("Planet"), cost));
        }
        if (pl->hasBase()) {
            Cost cost;
            if (opts.useBaseCost) {
                cost += config[HostConfiguration::StarbaseCost](pl->getOwner());
                addFighters(cost, pl->getOwner(), pl->getNumBaseFighters(), opts, config);
                cost.add(Cost::Money, 10*pl->getBaseDefense());
                cost.add(Cost::Duranium, pl->getBaseDefense());
            }
            if (opts.useBaseTech) {
                cost.add(Cost::Money, getBaseTechCost(pl->getOwner(), 1, pl->getBaseBeamTech(), config));
            }
            if (config[HostConfiguration::PlanetsHaveTubes]() && simConfig.hasAlternativeCombat()) {
                if (opts.useBaseTech) {
                    cost.add(Cost::Money, getBaseTechCost(pl->getOwner(), 1, pl->getBaseTorpedoTech(), config));
                }
                if (opts.useTorpedoes && opts.useBaseCost) {
                    for (int i = 1; i <= Planet::NUM_TORPEDO_TYPES; ++i) {
                        if (const game::spec::TorpedoLauncher* p = shipList.launchers().get(i)) {
                            cost += p->torpedoCost() * pl->getNumBaseTorpedoes(i);
                        }
                    }
                }
            }
            if (!cost.isZero()) {
                out.add(CostSummary::Item(0, 1, tx("Starbase"), cost));
            }
        }
    }
}

game::PlayerSet_t
game::sim::getInvolvedPlayers(const Setup& in)
{
    PlayerSet_t result;
    for (size_t i = 0, n = in.getNumObjects(); i < n; ++i) {
        if (const Object* obj = in.getObject(i)) {
            result += obj->getOwner();
        }
    }
    return result;
}

game::PlayerSet_t
game::sim::getInvolvedTeams(const Setup& in, const TeamSettings& teams)
{
    PlayerSet_t result;
    PlayerSet_t players = getInvolvedPlayers(in);
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (players.contains(i)) {
            if (int team = teams.getPlayerTeam(i)) {
                result += team;
            }
        }
    }
    return result;
}

String_t
game::sim::toString(FleetCostOptions::FighterMode mode, afl::string::Translator& tx)
{
    switch (mode) {
     case FleetCostOptions::FreeFighters: return tx("not included");
     case FleetCostOptions::ShipFighters: return tx("built by \"lfm\"");
     case FleetCostOptions::BaseFighters: return tx("built on starbase");
    }
    return String_t();
}

String_t
game::sim::toString(FleetCostOptions::TechMode mode, afl::string::Translator& tx)
{
    switch (mode) {
     case FleetCostOptions::NoTech:     return tx("not included");
     case FleetCostOptions::PlayerTech: return tx("once per player");
     case FleetCostOptions::ShipTech:   return tx("once per ship");
    }
    return String_t();
}

game::sim::FleetCostOptions::FighterMode
game::sim::getNext(FleetCostOptions::FighterMode mode)
{
    switch (mode) {
     case FleetCostOptions::FreeFighters:
     case FleetCostOptions::ShipFighters:
        return static_cast<FleetCostOptions::FighterMode>(mode + 1);
     case FleetCostOptions::BaseFighters:
        return FleetCostOptions::FreeFighters;
    }
    return FleetCostOptions::FreeFighters;
}

game::sim::FleetCostOptions::TechMode
game::sim::getNext(FleetCostOptions::TechMode mode)
{
    switch (mode) {
     case FleetCostOptions::NoTech:
     case FleetCostOptions::PlayerTech:
        return static_cast<FleetCostOptions::TechMode>(mode + 1);
     case FleetCostOptions::ShipTech:
        return FleetCostOptions::NoTech;
    }
    return FleetCostOptions::NoTech;
}
