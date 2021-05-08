/**
  *  \file u/t_game_score_scorebuilderbase.cpp
  *  \brief Test for game::score::ScoreBuilderBase
  */

#include "game/score/scorebuilderbase.hpp"

#include "t_game_score.hpp"

namespace {
    class Publisher : public game::score::ScoreBuilderBase {
     public:
        typedef SingleBuilder SingleBuilder_t;
    };
}

/** Test handling of special scores.
    A: use a ScoreBuilderBase::SingleBuilder to add score variants.
    E: verify that correct metadata is added for Score/BuildPoints. */
void
TestGameScoreScoreBuilderBase::testSpecials()
{
    // Environment
    // - Scores
    using game::score::TurnScoreList;
    TurnScoreList scores;
    scores.addDescription(TurnScoreList::Description("Win Score",    game::score::ScoreId_Score, 3, 5000));
    scores.addDescription(TurnScoreList::Description("Build Points", game::score::ScoreId_BuildPoints, 0, 0));
    scores.addSlot(game::score::ScoreId_Score);
    scores.addSlot(game::score::ScoreId_BuildPoints);

    // - Team settings
    game::TeamSettings teams;
    teams.setViewpointPlayer(3);

    // - Host version
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(3,0,0));

    // - Configuration
    game::config::HostConfiguration config;
    config[game::config::HostConfiguration::PALDecayPerTurn].set(17);

    // Scores for testing
    Publisher testee;
    Publisher::SingleBuilder_t b(testee, scores, teams, host, config);
    b.add("Winner", game::score::ScoreId_Score);
    b.add("Builder", game::score::ScoreId_BuildPoints);

    // Verify definitions
    const Publisher::Variant* p1 = testee.findVariant(game::score::CompoundScore(scores, game::score::ScoreId_Score, 1), 0);
    TS_ASSERT(p1 != 0);
    TS_ASSERT_EQUALS(p1->name, "Winner");
    TS_ASSERT_EQUALS(p1->scoreId, game::score::ScoreId_Score);
    TS_ASSERT_EQUALS(p1->decay, 0);
    TS_ASSERT_EQUALS(p1->winLimit, 5000);

    const Publisher::Variant* p2 = testee.findVariant(game::score::CompoundScore(scores, game::score::ScoreId_BuildPoints, 1), 0);
    TS_ASSERT(p2 != 0);
    TS_ASSERT_EQUALS(p2->name, "Builder");
    TS_ASSERT_EQUALS(p2->scoreId, game::score::ScoreId_BuildPoints);
    TS_ASSERT_EQUALS(p2->decay, 17);
    TS_ASSERT_EQUALS(p2->winLimit, 0);
}

