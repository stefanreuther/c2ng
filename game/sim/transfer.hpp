/**
  *  \file game/sim/transfer.hpp
  *  \brief Class game::sim::Transfer
  */
#ifndef C2NG_GAME_SIM_TRANSFER_HPP
#define C2NG_GAME_SIM_TRANSFER_HPP

#include "game/config/hostconfiguration.hpp"
#include "afl/string/translator.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/sim/ability.hpp"
#include "game/spec/shiplist.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "game/vcr/object.hpp"

namespace game { namespace sim {

    class Ship;
    class Planet;

    /** Transferring objects between game and simulator.
        This is a short-lived utility class whose job is mostly reducing number of parameters in the individual actions.

        BaseTransfer is the base class to Transfer, offering fewer operations with fewer dependencies. */
    class BaseTransfer {
     public:
        /** Constructor.
            Parameters need to exceed the lifetime of the object.
            \param shipList         Ship list (for accessing components, hull functions, missions)
            \param config           Host configuration (for accesing hull functions, missions)
            \param tx               Translator (for accessing names) */
        BaseTransfer(const game::spec::ShipList& shipList,
                     const game::config::HostConfiguration& config,
                     afl::string::Translator& tx);

        /** Copy ship from a battle recording into simulation.
            \param [out] out         Simulation ship. Will be overwritten with battle data.
            \param [in]  in          Ship from battle recording; can be "before" or "after" data
            \param [in]  assumedHull Assumed hull number (pass in.getGuessedHull(), or user selection)
            \param [in]  withESB     Whether ESB was active (see game::vcr::Battle::isESBActive())
            \retval true Copy successful
            \retval false Battle data cannot be parsed */
        bool copyShipFromBattle(Ship& out, const game::vcr::Object& in, int assumedHull, bool withESB) const;

        /** Copy planet from a battle recording into simulation.
            \param [out] out  Simulation planet. Will be overwritten with battle data.
            \param [in]  in   Planet from battle recording
            \retval true Copy successful
            \retval false Battle data cannot be parsed */
        bool copyPlanetFromBattle(Planet& out, const game::vcr::Object& in) const;

     protected:
        const game::spec::ShipList& m_shipList;
        const game::config::HostConfiguration& m_config;
        afl::string::Translator& m_translator;

        void setName(Ship& out, String_t name) const;
    };


    /** Transferring objects between game and simulator.
        This is a short-lived utility class whose job is mostly reducing number of parameters in the individual actions. */
    class Transfer : public BaseTransfer {
     public:
        /** Constructor.
            Parameters need to exceed the lifetime of the object.
            \param shipScores       Ship score definitions (for accessing experience levels)
            \param planetScores     Planet score definitions (for accessing experience levels)
            \param shipList         Ship list (for accessing components, hull functions, missions)
            \param config           Host configuration (for accesing hull functions, missions)
            \param hostVersion      Host version (for accessing missions)
            \param tx               Translator (for accessing names) */
        Transfer(const UnitScoreDefinitionList& shipScores,
                 const UnitScoreDefinitionList& planetScores,
                 const game::spec::ShipList& shipList,
                 const game::config::HostConfiguration& config,
                 afl::string::Translator& tx);

        /** Copy ship from game into simulation.
            \param [out] out  Simulation ship. Will be overwritten with game ship data.
            \param [in]  in   Game ship
            \retval true Copy successful
            \retval false Ship cannot be copied due to lacking data. Hull and owner need to be known. */
        bool copyShipFromGame(Ship& out, const game::map::Ship& in) const;

        /** Copy ship from simulation to game.
            Updates values that can be changed.
            \param [out]    out  Game ship
            \param [in]     in   Simulation ship
            \param [in,out] univ Universe (for transferring ammo)
            \param [in]     mapConfig Starchart geometry configuration
            \retval true Copy successful
            \retval false Game ship does not match simulation ship, not copied */
        bool copyShipToGame(game::map::Ship& out, const Ship& in, game::map::Universe& univ, const game::map::Configuration& mapConfig) const;

        /** Copy planet from game into simulation.
            \param [out] out  Simulation planet. Will be overwritten with game planet data.
            \param [in]  in   Game planet
            \retval true Copy successful
            \retval false Planet cannot be copied due to lacking data. Owner needs to be known. */
        bool copyPlanetFromGame(Planet& out, const game::map::Planet& in) const;

        /** Copy planet from simulation to game.
            Updates values that can be changed.
            \param [out]    out  Game planet
            \param [in]     in   Simulation planet
            \retval true Copy successful
            \retval false Game planet does not match simulation planet, not copied */
        bool copyPlanetToGame(game::map::Planet& out, const Planet& in) const;

     private:
        const UnitScoreDefinitionList& m_shipScores;
        const UnitScoreDefinitionList& m_planetScores;

        void setHullFunction(int32_t& flags, const Ship& out, const game::map::Ship& in, Ability a, int basicHullFunction) const;
    };

} }


#endif
