/**
  *  \file server/host/rank/victory.cpp
  *  \brief Victory Recognition and Ranking
  *
  *  FIXME: straightforward port, needs love
  */

#include "server/host/rank/victory.hpp"
#include "afl/base/countof.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/pack.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/string/format.hpp"
#include "server/host/rank/levelhandler.hpp"
#include "server/host/rank/refereefilereader.hpp"
#include "server/host/user.hpp"
#include "server/interface/filebaseclient.hpp"

namespace {
    const char LOG_NAME[] = "host.victory";

    using server::host::rank::Rank_t;
    using server::host::Game;
    using server::host::Root;

    struct PlayerInfo {
        int32_t turnsTotal;
        int32_t turnsPlayed;
        int32_t rankLevel;
        int32_t turnsInSlot[Game::NUM_PLAYERS];

        PlayerInfo()
            : turnsTotal(0), turnsPlayed(0), rankLevel(0)
            { afl::base::Memory<int32_t>(turnsInSlot).fill(0); }
    };

    const int16_t POSITION_POINTS[] = { 20, 14, 10, 8, 6, 4, 3, 2 };

    /** Save ranks in game.
        \param ranks [in] Ranks, not necessarily compacted
        \param game  Game to store to */
    void saveRanks(const Rank_t& ranks, Game& game)
    {
        /* Compact ranks using a null tiebreaker */
        game::PlayerSet_t slots = game.getGameSlots();
        Rank_t result;
        Rank_t null;
        server::host::rank::initRanks(null, 0);
        server::host::rank::compactRanks(result, ranks, null, slots);

        /* Save */
        for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
            if (slots.contains(i)) {
                game.getSlot(i).rank().set(result[i-1]);
            } else {
                game.getSlot(i).rank().set(0);
            }
        }
    }

    /** Determine ranks from score, and save them. */
    void saveScoreRanks(server::host::Game& game)
    {
        using afl::bits::Int32LE;

        /* Initialize ranks to INT32_MAX, so unset fields get the worst-possible rank */
        Rank_t ranks;
        server::host::rank::initRanks(ranks, 0x7FFFFFFF);

        /* Fetch all scores, but negate them because a high score is a good rank */
        const int32_t turn = game.turnNumber().get();
        const String_t scoreName   = game.getRefereeScoreName();
        const String_t scoreRecord = game.turn(turn).scores().stringField(scoreName).get();
        for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
            if (scoreRecord.size() >= 4U*i) {
                ranks[i-1] = -Int32LE::unpack(*reinterpret_cast<const Int32LE::Bytes_t*>(scoreRecord.data() + 4*(i-1)));
            }
        }

        /* Save */
        saveRanks(ranks, game);
    }

    /** Compute data for a score condition.
        \param game     Game to work on
        \param fromTurn [in] Start in this turn, inclusive
        \param toTurn   [in] End in this turn, inclusive
        \param players  [in] Players in game
        \param turns    [out] Negated number of turns-over-limit
        \param scores   [out] Final turn's scores */
    void computeScoreCondition(Game& game,
                               const int fromTurn,
                               const int toTurn,
                               const game::PlayerSet_t players,
                               Rank_t& turns,
                               Rank_t& scores)
    {
        /* Get score */
        const int32_t endScore = game.getConfigInt("endScore");
        const String_t endScoreName = game.getRefereeScoreName();

        /* Parse it. Fetch negated turns, so that it can be used as a rank structure */
        server::host::rank::initRanks(turns, 0);
        server::host::rank::initRanks(scores, 0);
        for (int i = fromTurn; i <= toTurn; ++i) {
            /* Fetch scores */
            String_t scoreRecord = game.turn(i).scores().stringField(endScoreName).get();
            afl::bits::unpackArray<afl::bits::Int32LE>(scores, afl::string::toBytes(scoreRecord));

            /* Compare */
            for (int pl = 1; pl <= Game::NUM_PLAYERS; ++pl) {
                if (players.contains(pl)) {
                    if (scores[pl-1] >= endScore) {
                        --turns[pl-1];
                    } else {
                        turns[pl-1] = 0;
                    }
                }
            }
        }
    }

    /** Check "score" condition. Game ends when someone reaches a particular score. */
    bool checkScoreCondition(Root& root, Game& game)
    {
        /* Check turn number. If endTurn is not yet reached, nobody can possibly have enough points. */
        int32_t turn = game.turnNumber().get();
        int32_t endTurn = game.getConfigInt("endTurn");
        if (endTurn < 1) {
            endTurn = 1;
        }
        if (turn < endTurn) {
            return false;
        }

        /* Get score */
        Rank_t turns;
        Rank_t scores;
        const game::PlayerSet_t players = game.getGameSlots();
        computeScoreCondition(game, turn - endTurn + 1, turn, players, turns, scores);

        /* Do we have a winner? */
        bool end = false;
        for (int pl = 1; pl <= Game::NUM_PLAYERS; ++pl) {
            if (-turns[pl-1] >= endTurn) {
                end = true;
            }
        }
        if (!end) {
            return false;
        }
        root.log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("game %d: 'score' condition ends game", game.getId()));

        /* Build ranks */
        Rank_t ranks;
        server::host::rank::compactRanks(ranks, turns, scores, players);

        /* Save it and end game */
        saveRanks(ranks, game);
        return true;
    }

    int logIt(Root& root, int n)
    {
        root.log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("I rolled a %d", n));
        return n;
    }

    /** Check "turn" condition. Game ends after reaching a particular turn. */
    bool checkTurnCondition(Root& root, Game& game)
    {
        /* Check turn number. Game does not end if endTurn not reached */
        int32_t turn = game.turnNumber().get();
        int32_t endTurn = game.getConfigInt("endTurn");
        if (turn < endTurn) {
            return false;
        }

        /* Check endProbability. 0 (unset) or 100 means we don't roll a die. */
        int32_t endProbability = game.getConfigInt("endProbability");
        int32_t effProbability = endProbability * (2 + turn - endTurn) / 2;
        if (effProbability <= 0
            || effProbability >= 100
            || logIt(root, root.rng()(100)) < effProbability)
        {
            /* Game ends */
            root.log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("game %d: 'turn' condition ends game", game.getId()));
            saveScoreRanks(game);
            return true;
        }

        /* Keep going */
        root.log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("game %d: 'turn' condition continues game, probability %d", game.getId(), effProbability));
        return false;
    }
}


/**************************** Public Functions ***************************/



// Check victory condition.
bool
server::host::rank::checkVictory(Root& root, String_t gameDir, Game& game)
{
    // ex planetscentral/host/victory.cc:checkVictory
    /* Read plug-in victory status */
    server::interface::FileBaseClient hostFile(root.hostFile());
    afl::base::Optional<String_t> ref = hostFile.getFileNT(gameDir + "/c2ref.txt");
    if (const String_t* pRef = ref.get()) {
        afl::io::ConstMemoryStream ms(afl::string::toBytes(*pRef));
        RefereeFileReader rdr;
        rdr.parseFile(ms);
        if (rdr.isEnd()) {
            root.log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("game %d: game end declared by add-on", game.getId()));
            saveRanks(rdr.getRanks(), game);
            return true;
        }
    }

    /* Check builtin victory conditions */
    String_t cond = game.getConfig("endCondition");
    if (cond == "turn") {
        /* End after turn, with probability */
        return checkTurnCondition(root, game);
    } else if (cond == "score") {
        /* End when score reached (and kept for some turns) */
        return checkScoreCondition(root, game);
    } else {
        return false;
    }
}

// Check victory for forced game end.
void
server::host::rank::checkForcedGameEnd(Game& game)
{
    int32_t turn = game.turnNumber().get();
    if (turn <= 0) {
        /* There are no scores. Treat everyone equal. */
        Rank_t null;
        server::host::rank::initRanks(null, 0x7FFFFFFF);
        saveRanks(null, game);
    } else if (game.getConfig("endCondition") == "score") {
        /* This must approximate the actual game score. If a "120 planets for 5 turns"
           game is terminated, a player with 120 planets for 4 turns is closer to winning than
           a player with 150 planets for 1 turn. Thus, we count how many turns over limit
           players are. */
        int32_t endTurn = game.getConfigInt("endTurn");
        if (endTurn < 1) {
            endTurn = 1;
        }
        int32_t firstTurnToJudge = turn - endTurn + 1;
        if (firstTurnToJudge <= 0) {
            firstTurnToJudge = 1;
        }

        /* Compute partial score */
        Rank_t turns;
        Rank_t scores;
        const game::PlayerSet_t players = game.getGameSlots();
        computeScoreCondition(game, firstTurnToJudge, turn, players, turns, scores);

        /* Save */
        Rank_t ranks;
        server::host::rank::compactRanks(ranks, turns, scores, players);
        saveRanks(ranks, game);
    } else {
        /* All other scoring modes just use the scores. This is the exact result for "turn"
           condition, and a very rough approximate for add-on conditions (if we play a game
           judged by an add-on, we cannot know how close to winning a player is. */
        saveScoreRanks(game);
    }
}

// Compute rank points after a game end.
void
server::host::rank::computeGameRankings(Root& root, Game& game)
{
    // ex planetscentral/host/victory.cc:computeGameRankings
    using afl::bits::Int16LE;

    afl::net::redis::HashKey rankPointsHash(game.rankPoints());
    int32_t currentTurn = game.turnNumber().get();
    // int32_t existingTurn = game.getConfigInt("rankTurn");   // we do not use that yet
    std::map<String_t, PlayerInfo> players;

    Rank_t sumOfRankLevels;
    Rank_t lastTurnSeen;
    initRanks(sumOfRankLevels, 0);
    initRanks(lastTurnSeen, 0);

    // Scan through whole game and collect information.
    // Start at turn 2, because everyone "misses" turn 1. This means players who drop in
    // turn one are not registered in this game. Which is not surprising, because players
    // that join and immediately resign are not counted either.
    root.log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("Computing ranks for game %d, %d turns...", game.getId(), currentTurn));
    for (int32_t turnNr = 2; turnNr <= currentTurn; ++turnNr) {
        // Process one turn
        Game::Turn turn(game.turn(turnNr));
        String_t turnStatus = turn.info().turnStatus().get();
        for (int slot = 1; slot <= Game::NUM_PLAYERS; ++slot) {
            // Do we have information about this slot?
            if (turnStatus.size() < 2U*slot) {
                continue;
            }

            // Was this slot in the game at this time?
            int32_t slotTurnStatus = Int16LE::unpack(*reinterpret_cast<const Int16LE::Bytes_t*>(turnStatus.data() + 2*(slot-1)));
            if (slotTurnStatus < 0) {
                continue;
            }
            slotTurnStatus &= Game::TurnStateMask;

            // Was anyone playing this slot?
            String_t player = turn.playerId().stringField(afl::string::Format("%d", slot)).get();
            if (player.empty()) {
                continue;
            }

            // Locate this player
            std::map<String_t, PlayerInfo>::iterator it = players.find(player);
            if (it == players.end()) {
                it = players.insert(std::make_pair(player, PlayerInfo())).first;
                it->second.rankLevel = User(root, player).rankLevel().get() + 1;
            }

            // Count slot
            sumOfRankLevels[slot-1] += it->second.rankLevel;
            lastTurnSeen[slot-1] = turnNr;

            // Count player
            if (slotTurnStatus == Game::TurnYellow || slotTurnStatus == Game::TurnGreen || slotTurnStatus == Game::TurnDead) {
                ++it->second.turnsPlayed;
            }
            ++it->second.turnsTotal;
            ++it->second.turnsInSlot[slot-1];
        }
    }

    // Fetch ranks computed by referee
    Rank_t refRanks;
    server::host::rank::initRanks(refRanks, 0x7FFFFFFF);
    for (int slot = 1; slot <= Game::NUM_PLAYERS; ++slot) {
        int32_t value = game.getSlot(slot).rank().get();
        if (value) {
            refRanks[slot-1] = value;
        }
    }

    // Compute final ranks
    Rank_t finalRanks;
    server::host::rank::compactRanks(finalRanks, refRanks, lastTurnSeen, game::PlayerSet_t::allUpTo(Game::NUM_PLAYERS));

    // Compute game-dependant weighing factor (Game_Difficulty * Turn_Factor)
    double gameFactor = game.getDifficulty(root) / 100.0;
    if (currentTurn < 50) {
        gameFactor *= currentTurn;
        gameFactor /= 50;
    }

    // Compute scores for all players
    LevelHandler lh(root);
    for (std::map<String_t, PlayerInfo>::iterator it = players.begin(); it != players.end(); ++it) {
        int usedSlot = 0;
        int32_t newPoints = 0;
        for (int slot = 1; slot <= Game::NUM_PLAYERS; ++slot) {
            if (it->second.turnsInSlot[slot-1] != 0) {
                // Player was active in this slot, compute score
                const int slotRank = finalRanks[slot-1];
                const int positionPoints = slotRank <= int(countof(POSITION_POINTS)) ? POSITION_POINTS[slotRank-1] : 1;

                double reliabilityRate = double(it->second.turnsPlayed) / it->second.turnsTotal;
                reliabilityRate *= reliabilityRate;

                // Player_Rate_Num
                int numOpponents = 0;
                int32_t sumRanksOfOpponents = 0;
                for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
                    if (finalRanks[i-1] > slotRank) {
                        ++numOpponents;
                        sumRanksOfOpponents += sumOfRankLevels[i-1];
                    }
                }
                const double playerRateNum = (sumRanksOfOpponents / double(currentTurn)) + 110;

                // Player_Rate_Den. Note that dividing by turnsInSlot automatically
                // scales the value down for players that did not play a whole game.
                const double playerRateDen = (it->second.rankLevel
                                              * numOpponents
                                              + 110)
                    * double(currentTurn)
                    / it->second.turnsInSlot[slot-1];

                // Compute totals. Player gets maximum of totals (not sum).
                const int32_t points = int32_t(100 * gameFactor * positionPoints * playerRateNum / playerRateDen * reliabilityRate + 0.5);
                if (points > newPoints) {
                    newPoints = points;
                    usedSlot = slot;
                }
            }
        }

        // Log and store in database
        const String_t& user = it->first;
        afl::net::redis::IntegerField points(rankPointsHash, user);
        int32_t oldPoints = points.get();
        root.log().write(afl::sys::LogListener::Info, LOG_NAME,
                         afl::string::Format("  slot %2d, %5d points (was %5d), user %s") << usedSlot << newPoints << oldPoints << it->first.c_str());
        points.set(newPoints);
        lh.addPlayerRankPoints(user, newPoints - oldPoints);
    }

    // Trigger rank changes
    for (std::map<String_t, PlayerInfo>::const_iterator it = players.begin(); it != players.end(); ++it) {
        lh.handlePlayerRankChanges(it->first);
    }
    game.setConfigInt("rankTurn", currentTurn);

    root.log().write(afl::sys::LogListener::Info, LOG_NAME, "Ranking done");
}
