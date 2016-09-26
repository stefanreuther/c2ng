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

    afl::data::Value* getPlayerProperty(int pid, PlayerProperty ipl,
                                        const PlayerList& list,
                                        const Game& game,
                                        const game::config::HostConfiguration& config);

} }

#endif
