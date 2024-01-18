/**
  *  \file test/game/score/scorebuilderbasetest.cpp
  *  \brief Test for game::score::ScoreBuilderBase
  */

#include "game/score/scorebuilderbase.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    class Publisher : public game::score::ScoreBuilderBase {
     public:
        typedef SingleBuilder SingleBuilder_t;
    };
}

/** Test handling of special scores.
    A: use a ScoreBuilderBase::SingleBuilder to add score variants.
    E: verify that correct metadata is added for Score/BuildPoints. */
AFL_TEST("game.score.ScoreBuilderBase", a)
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
    a.checkNonNull("01. findVariant", p1);
    a.checkEqual("02. name",     p1->name, "Winner");
    a.checkEqual("03. scoreId",  p1->scoreId, game::score::ScoreId_Score);
    a.checkEqual("04. decay",    p1->decay, 0);
    a.checkEqual("05. winLimit", p1->winLimit, 5000);

    const Publisher::Variant* p2 = testee.findVariant(game::score::CompoundScore(scores, game::score::ScoreId_BuildPoints, 1), 0);
    a.checkNonNull("11. findVariant", p2);
    a.checkEqual("12. name",     p2->name, "Builder");
    a.checkEqual("13. scoreId",  p2->scoreId, game::score::ScoreId_BuildPoints);
    a.checkEqual("14. decay",    p2->decay, 17);
    a.checkEqual("15. winLimit", p2->winLimit, 0);
}
