/**
  *  \file game/sim/fleetcost.hpp
  *  \brief Fleet Cost Computation
  */
#ifndef C2NG_GAME_SIM_FLEETCOST_HPP
#define C2NG_GAME_SIM_FLEETCOST_HPP

#include "afl/string/translator.hpp"
#include "game/playerlist.hpp"
#include "game/playerset.hpp"
#include "game/spec/costsummary.hpp"
#include "game/spec/shiplist.hpp"
#include "game/teamsettings.hpp"

namespace game { namespace sim {

    class Setup;
    class Configuration;

    /** Options for fleet cost computation. */
    struct FleetCostOptions {
        enum FighterMode {
            FreeFighters,          ///< Do not count fighters. ex ff_Free.
            ShipFighters,          ///< Count fighters as ShipFighterCost. ex ff_LFM.
            BaseFighters           ///< Count fighters as BaseFighterCost. ex ff_Build.
        };

        enum TechMode {
            NoTech,                ///< Do not count ship tech. ex st_No.
            PlayerTech,            ///< Count ship tech once per player. ex st_Player.
            ShipTech               ///< Count ship tech once per ship. ex st_Ship.
        };

        FighterMode fighterMode;   ///< Handling of fighters. ex fighters, cost_Fighters.
        TechMode shipTechMode;     ///< Handling of ship tech levels. ex shiptech (not in PCC1).
        bool useTorpedoes;         ///< Include torpedo costs. ex torps, cost_Torps.
        bool useEngines;           ///< Include engine cost. ex engine, cost_Engine.
        bool usePlanetDefense;     ///< Include planet defense cost. ex planet, cost_Planet.
        bool useBaseCost;          ///< Include starbase cost. ex base, cost_Base.
        bool useBaseTech;          ///< Include starbase tech level cost. ex tech, cost_Tech.
        // bool byTeam;            ///< Display by team, not by player. ex team (not in PCC1). For now, this option is outside this module.

        FleetCostOptions()
            : fighterMode(ShipFighters),
              shipTechMode(NoTech),
              useTorpedoes(true),
              useEngines(false),
              usePlanetDefense(true),
              useBaseCost(true),
              useBaseTech(false)
            { }
    };

    /** Compute fleet cost for a setup.
        Adds up all the costs for a given set of players, according to the given FleetCostOptions.
        To add team costs, convert the team into a player set before.
        \param [out] out        Result
        \param [in]  in         Setup
        \param [in]  simConfig  Simulation configuration (affects use of planetary torpedoes)
        \param [in]  opts       Fleet cost options
        \param [in]  shipList   Ship list (for component costs)
        \param [in]  config     Configuration (for tech level costs, use of planetary torpedoes)
        \param [in]  players    Players to consider
        \param [in]  tx         Translator */
    void computeFleetCosts(game::spec::CostSummary& out,
                           const Setup& in,
                           const Configuration& simConfig,
                           const FleetCostOptions& opts,
                           const game::spec::ShipList& shipList,
                           const game::config::HostConfiguration& config,
                           const PlayerList& playerList,
                           PlayerSet_t players,
                           afl::string::Translator& tx);

    /** Get set of players involved in setup.
        \param in Setup
        \return player set */
    PlayerSet_t getInvolvedPlayers(const Setup& in);

    /** Get set of teams involved in setup.
        \param in Setup
        \param teams Team setup
        \return team set */
    PlayerSet_t getInvolvedTeams(const Setup& in, const TeamSettings& teams);


    /** Format a FighterMode.
        \param mode Value
        \param tx   Translator
        \return human-readable string */
    String_t toString(FleetCostOptions::FighterMode mode, afl::string::Translator& tx);

    /** Format a TechMode.
        \param mode Value
        \param tx   Translator
        \return human-readable string */
    String_t toString(FleetCostOptions::TechMode mode, afl::string::Translator& tx);


    /** Get next FighterMode.
        If the last value has been reached, restarts at the first.
        \param mode Mode
        \return next mode */
    FleetCostOptions::FighterMode getNext(FleetCostOptions::FighterMode mode);

    /** Get next TechMode.
        If the last value has been reached, restarts at the first.
        \param mode Mode
        \return next mode */
    FleetCostOptions::TechMode getNext(FleetCostOptions::TechMode mode);

} }

#endif
