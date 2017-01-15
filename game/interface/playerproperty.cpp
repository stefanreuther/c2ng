/**
  *  \file game/interface/playerproperty.cpp
  */

#include "game/interface/playerproperty.hpp"
#include "interpreter/values.hpp"
#include "game/score/compoundscore.hpp"
#include "game/turn.hpp"

using interpreter::makeStringValue;
using interpreter::makeIntegerValue;
using interpreter::makeOptionalIntegerValue;
using game::score::CompoundScore;

/** Get player property.
    \param pid Player to query (out of range values are handled properly)
    \param ipl Player property to query */
afl::data::Value*
game::interface::getPlayerProperty(int pid, PlayerProperty ipl,
                                   const PlayerList& list,
                                   const Game& game,
                                   const game::config::HostConfiguration& config)
{
    // ex int/if/playerif.h:getPlayerProperty

    // Special case for out-of-range values
    const Player* p = list.get(pid);
    if (p == 0 || pid == 0) {
        if (ipl == iplId && pid >= 0) {
            return makeIntegerValue(pid);
        } else {
            return 0;
        }
    }

    // Regular handling
    switch (ipl) {
     case iplAdjName:
        /* @q My.Race.Adj:Str (Global Property)
           @q Owner.Adj:Str (Minefield Property, Planet Property, Ship Property)
           @q Enemy.Adj:Str (Ship Property)
           @q Race.Adj:Str (Player Property)
           Adjective name of this player. */
        if (const Player* p = list.get(pid)) {
            return makeStringValue(p->getName(Player::AdjectiveName));
        } else {
            return 0;
        }

     case iplFullName:
        /* @q My.Race.Full:Str (Global Property)
           @q Race:Str (Player Property)
           Full name of this player. */
        if (const Player* p = list.get(pid)) {
            return makeStringValue(p->getName(Player::LongName));
        } else {
            return 0;
        }

     case iplId:
        /* @q My.Race$:Int (Global Property)
           @q Owner$:Int (Minefield Property, Planet Property, Ship Property)
           @q Race$:Int (Player Property)
           Player number. */
        return makeIntegerValue(pid);

     case iplMission:
        /* @q My.Race.Mission:Int (Global Property)
           @q Race.Mission:Int (Player Property)
           Special mission assigned to this player. */
        return makeIntegerValue(config.getPlayerMissionNumber(pid));

     case iplPBPs:
        /* @q My.PBPs:Int (Global Property)
           @q PBPs:Int (Player Property)
           Number of priority points.
           This reports the build queue priority points for a player,
           no matter what flavour of build points is active (PBPs, PAL).
           @since PCC2 1.99.25, PCC 0.98.5 */
        return makeOptionalIntegerValue(CompoundScore(game.scores(), game::score::ScoreId_BuildPoints, 1).get(game.scores(), game.currentTurn().getTurnNumber(), pid));

     case iplRaceId:
        /* @q My.Race.Id:Int (Global Property)
           @q Race.Id:Int (Player Property)
           Race assigned to this player. */
        return makeIntegerValue(config.getPlayerRaceNumber(pid));

     case iplShortName:
        /* @q My.Race:Str (Global Property)
           @q Owner:Str (Minefield Property, Planet Property, Ship Property)
           @q Race.Short:Str (Player Property)
           @q Enemy:Str (Ship Property)
           Short name of this player. */
        if (const Player* p = list.get(pid)) {
            return makeStringValue(p->getName(Player::ShortName));
        } else {
            return 0;
        }

     case iplTeam:
        /* @q My.Team:Int (Global Property)
           @q Team:Int (Player Property)
           Team this player is in. */
        return makeIntegerValue(game.teamSettings().getPlayerTeam(pid));

     case iplScoreBases:
        /* @q My.Bases:Int (Global Property)
           @q Bases:Int (Player Property)
           Number of bases this player has, according to score. */
        return makeOptionalIntegerValue(CompoundScore(game.scores(), game::score::ScoreId_Bases, 1).get(game.scores(), game.currentTurn().getTurnNumber(), pid));

     case iplScorePlanets:
        /* @q My.Planets:Int (Global Property)
           @q Planets:Int (Player Property)
           Number of planets this player has, according to score. */
        return makeOptionalIntegerValue(CompoundScore(game.scores(), game::score::ScoreId_Planets, 1).get(game.scores(), game.currentTurn().getTurnNumber(), pid));

     case iplScoreShips:
        /* @q My.Ships:Int (Global Property)
           @q Ships:Int (Player Property)
           Number of ships this player has, according to score. */
        return makeOptionalIntegerValue(CompoundScore(game.scores(), CompoundScore::TotalShips).get(game.scores(), game.currentTurn().getTurnNumber(), pid));

     case iplScore:
        /* @q My.Score:Int (Global Property)
           @q Score:Int (Player Property)
           This player's Tim-score. */
        return makeOptionalIntegerValue(CompoundScore(game.scores(), CompoundScore::TimScore).get(game.scores(), game.currentTurn().getTurnNumber(), pid));

     case iplScoreCapital:
        /* @q My.Ships.Capital:Int (Global Property)
           @q Ships.Capital:Int (Player Property)
           Number of capital ships this player has, according to score. */
        return makeOptionalIntegerValue(CompoundScore(game.scores(), game::score::ScoreId_Capital, 1).get(game.scores(), game.currentTurn().getTurnNumber(), pid));

     case iplScoreFreighters:
        /* @q My.Score.Freighters:Int (Global Property)
           @q Score.Freighters:Int (Player Property)
           Number of freighters this player has, according to score. */
        return makeOptionalIntegerValue(CompoundScore(game.scores(), game::score::ScoreId_Freighters, 1).get(game.scores(), game.currentTurn().getTurnNumber(), pid));

     case iplTotalShips:
        /* @q Ships.Total:Int (Global Property)
           Total number of ships (from this player's score information). */
        return makeOptionalIntegerValue(CompoundScore(game.scores(), CompoundScore::TotalShips).get(game.scores(), game.currentTurn().getTurnNumber(), PlayerSet_t::allUpTo(MAX_PLAYERS)));

     case iplTotalFreighters:
        /* @q Ships.Freighters:Int (Global Property)
           Total number of freighters (from this player's score information). */
        return makeOptionalIntegerValue(CompoundScore(game.scores(), game::score::ScoreId_Freighters, 1).get(game.scores(), game.currentTurn().getTurnNumber(), PlayerSet_t::allUpTo(MAX_PLAYERS)));

     case iplTotalCapital:
        /* @q Total.Capital:Int (Global Property)
           Total number of capital ships (from this player's score information). */
        return makeOptionalIntegerValue(CompoundScore(game.scores(), game::score::ScoreId_Capital, 1).get(game.scores(), game.currentTurn().getTurnNumber(), PlayerSet_t::allUpTo(MAX_PLAYERS)));

    }
    return 0;
}
