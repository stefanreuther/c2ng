/**
  *  \file server/host/rank/levelhandler.cpp
  *  \brief Class server::host::rank::LevelHandler
  */

#include "server/host/rank/levelhandler.hpp"
#include "afl/base/countof.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/format.hpp"
#include "server/host/user.hpp"

namespace {
    /** Logging channel for this module. */
    const char LOG_NAME[] = "host.rank";

    /** System parameter: scale factor for reliabilities.
        A 90% reliable player will have the value 90 * RELIABILITY_SCALE stored in the database.
        This parameter should be constant for a database instance because it affects storage. */
    const int32_t RELIABILITY_SCALE = 1000;

    /** Parameter: reliability speed factor.
        This parameter configures how much a new reliability value affects the existing reliability.
        This is a percentage, lower means less effect.
        At 100%, the new reliability will equal the most recent single reliability score.
        This parameter affects formulas only and can thus be changed. */
    const int32_t RELIABILITY_SPEED = 3;

    /** Parameter: penalty for dropping.
        This parameter configures the penalty for dropping.
        This is a percentage, lower means less effect.
        At 100%, a player dropping from a game they're leading will have 0% reliability remaining.
        This parameter affects formulas only and can thus be changed. */
    const int32_t DROP_PENALTY = 66;


    /*
        Ranks for reference

        These are the PlanetsServer ranks.
        Names are provided by the front-end [possible German equivalents in brackets].
        Skill and number-of-turns can only grow, but reliability can drop.
        We therefore use a hysteresis of rank-to-get and rank-to-keep.

          1.          Admiral                    50000    90  >95    500       15            [Admiral]
          2.          Vice Admiral               25000    90  >95    400       14            [Vizeadmiral]
          3.          Rear Admiral               16000    90  >95    300       13            [Konteradmiral]
          4.          Captain                    12000    80  >85    300       12            [Kapitän [zur See]]
          5.          Commander                  9000     80  >85    250       11            [Fregattenkapitän]
          6.          Lieutenant Commander       6500     80  >85    200       10            [~Korvettenkapitän]
          7.          Lieutenant                 4000     70  >75    200       9             [Kapitänleutnant]
          8.          Lieutenant Junior Grade    2500     70  >75    150       8             [Oberleutnant zur See]
          9.          Ensign                     1750     70  >75    100       7             [Leutnant zur See, ~Fähnrich]
          10.         Senior Chief Petty Officer 1200     60  >65    100       6             [~Stabsbootsmann]
          11.         Chief Petty Officer        800      60  >65    80        5             [~Oberbootsmann]
          12.         Petty Officer              500      60  >65    60        4             [Bootsmann]
          13.         Spaceman                   250      50  >55    40        3             [Raumflieger]
          14.         Spaceman Apprentice        100      50  >55    20        2             [Raumflieger-Anwärter]
          15.         Spaceman Recruit           0        0          0         1             [Raumflieger-Rekrut]

     */

    struct RankDefinition {
        int32_t minRankPoints;                  // Minimum rankpoints to get this rank
        int8_t  minReliabilityToGet;            // Minimum turnreliability to get this rank
        int8_t  minReliabilityToKeep;           // Minimum turnreliability to keep this rank
        int16_t minTurnsPlayed;                 // Minimum turns played to get this rank
    };

    const RankDefinition RANK_DEFINITIONS[] = {
        {   100, 55, 50,  20 },    // Spaceman Apprentice
        {   250, 55, 50,  40 },    // Spaceman
        {   500, 65, 60,  60 },    // Petty Officer
        {   800, 65, 60,  80 },    // Chief Petty Officer
        {  1200, 65, 60, 100 },    // Senior Chief Petty Officer
        {  1750, 75, 70, 100 },    // Ensign
        {  2500, 75, 70, 150 },    // Lieutenant Junior Grade
        {  4000, 75, 70, 200 },    // Lieutenant
        {  6500, 85, 80, 200 },    // Lieutenant Commander
        {  9000, 85, 80, 250 },    // Commander
        { 12000, 85, 80, 300 },    // Captain
        { 16000, 95, 90, 300 },    // Rear Admiral
        { 25000, 95, 90, 400 },    // Vice Admiral
        { 50000, 95, 90, 500 },    // Admiral
    };

    const int32_t MAX_RANK = countof(RANK_DEFINITIONS);
}

// Constructor.
server::host::rank::LevelHandler::LevelHandler(Root& root)
    : m_root(root)
{ }

// Handle player turn (non-)submission.
void
server::host::rank::LevelHandler::handlePlayerTurn(String_t userId, bool submit, uint32_t level)
{
    // ex planetscentral/host/ranking.h:handlePlayerTurn
    afl::net::redis::HashKey profile(User(m_root, userId).profile());

    // Count this turn
    if (submit) {
        ++profile.intField("turnsplayed");
    } else {
        ++profile.intField("turnsmissed");
    }

    // Adjust reliability
    int32_t newPoints = RELIABILITY_SCALE * RELIABILITY_SPEED;
    if (!submit && level < 30) {
        newPoints -= newPoints >> level;
    }
    profile.intField("turnreliability").set(profile.intField("turnreliability").get() * (100 - RELIABILITY_SPEED) / 100 + newPoints);

    // Log
    m_root.log().write(afl::sys::LogListener::Info, LOG_NAME,
                       afl::string::Format("player '%s': %d points (%s, level %d)") << userId << newPoints << (submit?"submit":"miss") << level);
}

// Handle player dropout.
void
server::host::rank::LevelHandler::handlePlayerDrop(String_t userId, Game& game, int slot)
{
    // ex planetscentral/host/ranking.h:handlePlayerDrop
    using afl::bits::Int32LE;

    // The game must have started; otherwise, we cannot give a penalty.
    int32_t turn = game.turnNumber().get();
    if (turn <= 0) {
        return;
    }

    // Get current turn's scores
    String_t packedScores = game.turn(turn).scores().stringField(game.getRefereeScoreName()).get();

    // Do we actually have scores for this player?
    if (packedScores.size() < slot*4U) {
        m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("player '%s': no penalty, player has no score", userId));
        return;
    }

    int32_t playerScore = Int32LE::unpack(*reinterpret_cast<const Int32LE::Bytes_t*>(packedScores.data() + 4*(slot-1)));

    // Find maximum score
    int32_t maxScore = -1;
    for (size_t i = 1; i*4 <= packedScores.size(); ++i) {
        int32_t thisScore = Int32LE::unpack(*reinterpret_cast<const Int32LE::Bytes_t*>(packedScores.data() + 4*(i-1)));
        if (thisScore > maxScore) {
            maxScore = thisScore;
        }
    }

    // Can we compute a penalty?
    if (playerScore < 0 || maxScore <= 0) {
        m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("player '%s': no penalty, score is 0", userId));
        return;
    }

    // Give penalty
    afl::net::redis::HashKey profile(User(m_root, userId).profile());
    int32_t oldReliability = profile.intField("turnreliability").get();
    int32_t newReliability = int32_t(oldReliability * double(maxScore*100 - playerScore*DROP_PENALTY) / (maxScore*100));
    profile.intField("turnreliability").set(newReliability);
    m_root.log().write(afl::sys::LogListener::Info, LOG_NAME,
                       afl::string::Format("player '%s': reliability %d->%d due to dropout, score %d/%d") << userId << oldReliability << newReliability << playerScore << maxScore);
}

// Add skill points.
void
server::host::rank::LevelHandler::addPlayerRankPoints(String_t userId, int32_t pts)
{
    // ex planetscentral/host/ranking.h:addPlayerRankPoints
    afl::net::redis::HashKey profile(User(m_root, userId).profile());
    profile.intField("rankpoints") += pts;
}

// Check possible required rank changes.
void
server::host::rank::LevelHandler::handlePlayerRankChanges(String_t userId)
{
    // ex planetscentral/host/ranking.h:handlePlayerRankChanges
    afl::net::redis::HashKey profile(User(m_root, userId).profile());

    int32_t currentRank        = profile.intField("rank").get();
    int32_t currentRankPoints  = profile.intField("rankpoints").get();
    int32_t currentReliability = profile.intField("turnreliability").get();
    int32_t currentTurns       = profile.intField("turnsplayed").get();

    int32_t roundedReliability = (currentReliability + RELIABILITY_SCALE/2) / RELIABILITY_SCALE;

    const char* mail = 0;
    while (currentRank < MAX_RANK
           && currentRankPoints >= RANK_DEFINITIONS[currentRank].minRankPoints
           && roundedReliability >= RANK_DEFINITIONS[currentRank].minReliabilityToGet
           && currentTurns >= RANK_DEFINITIONS[currentRank].minTurnsPlayed)
    {
        // promotion
        ++currentRank;
        profile.intField("rank").set(currentRank);
        mail = "rank-promotion";
    }

    while (currentRank > 0
           && (currentRankPoints < RANK_DEFINITIONS[currentRank-1].minRankPoints
               || roundedReliability < RANK_DEFINITIONS[currentRank-1].minReliabilityToKeep
               || currentTurns < RANK_DEFINITIONS[currentRank-1].minTurnsPlayed))
    {
        // demotion
        --currentRank;
        profile.intField("rank").set(currentRank);
        mail = "rank-demotion";
    }

    // So, did something happen?
    if (mail != 0) {
        m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("%s for user %s, new rank: %d", mail, userId, currentRank));

        server::interface::MailQueue& mq = m_root.mailQueue();
        mq.startMessage(mail, String_t("rank-" + userId));
        mq.addParameter("rank",            afl::string::Format("%d", currentRank));
        mq.addParameter("rankpoints",      afl::string::Format("%d", currentRankPoints));
        mq.addParameter("turnreliability", afl::string::Format("%d", roundedReliability));
        mq.addParameter("turnsplayed",     afl::string::Format("%d", currentTurns));

        const String_t to[] = { "user:" + userId };
        mq.send(to);
    }
}
