/**
  *  \file game/interface/playerproperty.hpp
  *  \brief Enum game::interface::PlayerProperty
  */
#ifndef C2NG_GAME_INTERFACE_PLAYERPROPERTY_HPP
#define C2NG_GAME_INTERFACE_PLAYERPROPERTY_HPP

#include "afl/data/value.hpp"
#include "afl/string/translator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/game.hpp"
#include "game/playerlist.hpp"

namespace game { namespace interface {

    /** Player properties. */
    enum PlayerProperty {
        iplAdjName,
        iplFullName,
        iplId,
        iplMission,
        iplPBPs,
        iplRaceId,
        iplTeam,
        iplShortName,
        iplScoreBases,
        iplScorePlanets,
        iplScoreShips,
        iplScore,
        iplScoreCapital,
        iplScoreFreighters,
        iplTotalShips,
        iplTotalFreighters,
        iplTotalCapital
    };

    /** Get player property.
        @param pid Player to query (out of range values are handled properly)
        @param ipl Player property to query
        @param list Player list (to access Player object)
        @param game Game (for scores)
        @param config Host configuration (for race/mission Ids)
        @param tx Translator (for default names)
        @return Value (newly-allocated) */
    afl::data::Value* getPlayerProperty(int pid, PlayerProperty ipl,
                                        const PlayerList& list,
                                        const Game& game,
                                        const game::config::HostConfiguration& config,
                                        afl::string::Translator& tx);

} }

#endif
