/**
  *  \file game/interface/playerproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_PLAYERPROPERTY_HPP
#define C2NG_GAME_INTERFACE_PLAYERPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/playerlist.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/game.hpp"

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
        @return Value (newly-allocated) */
    afl::data::Value* getPlayerProperty(int pid, PlayerProperty ipl,
                                        const PlayerList& list,
                                        const Game& game,
                                        const game::config::HostConfiguration& config);

} }

#endif
