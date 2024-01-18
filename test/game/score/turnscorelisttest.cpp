/**
  *  \file test/game/score/turnscorelisttest.cpp
  *  \brief Test for game::score::TurnScoreList
  */

#include "game/score/turnscorelist.hpp"
#include "afl/test/testrunner.hpp"

namespace gp = game::parser;

/** Test standard schema. */
AFL_TEST("game.score.TurnScoreList:schema", a)
{
    game::score::TurnScoreList testee;
    game::score::TurnScoreList::Slot_t slot;
    game::score::TurnScoreList::Slot_t slot2;
    game::score::ScoreId_t scoreId;

    // Default schema must contain these scores:
    a.check("01. ScoreId_Planets",     testee.getSlot(game::score::ScoreId_Planets).isValid());
    a.check("02. ScoreId_Capital",     testee.getSlot(game::score::ScoreId_Capital).isValid());
    a.check("03. ScoreId_Freighters",  testee.getSlot(game::score::ScoreId_Freighters).isValid());
    a.check("04. ScoreId_Bases",       testee.getSlot(game::score::ScoreId_Bases).isValid());
    a.check("05. ScoreId_BuildPoints", testee.getSlot(game::score::ScoreId_BuildPoints).isValid());

    // Forward mapping:
    a.checkEqual("11. getNumScores", testee.getNumScores(), 5U);
    a.check("12. getScoreByIndex", testee.getScoreByIndex(0).get(scoreId));
    a.check("13. getScoreByIndex", !testee.getScoreByIndex(5).isValid());

    // File must still be "safe"
    a.check("21. hasFutureFeatures", !testee.hasFutureFeatures());

    // Add a slot
    a.check("31. getSlot", !testee.getSlot(1000).isValid());
    slot2 = testee.addSlot(1000);
    a.check("32. getSlot", testee.getSlot(1000).get(slot));
    a.checkEqual("33. slot", slot, slot2);
    a.checkEqual("34. getNumScores", testee.getNumScores(), 6U);
    a.check("35. getScoreByIndex", testee.getScoreByIndex(5).get(scoreId));
    a.checkEqual("36. scoreId", scoreId, 1000);

    // Adding existing slot
    a.check("41. getSlot", testee.getSlot(game::score::ScoreId_BuildPoints).get(slot));
    slot2 = testee.addSlot(game::score::ScoreId_BuildPoints);
    a.checkEqual("42. slot", slot, slot2);
    a.check("43. getSlot", testee.getSlot(game::score::ScoreId_BuildPoints).get(slot));
    a.checkEqual("44. slot", slot, slot2);

    // Verify the "future" flag
    a.check("51. hasFutureFeatures", !testee.hasFutureFeatures());
    testee.setFutureFeatures(true);
    a.check("52. hasFutureFeatures", testee.hasFutureFeatures());
    testee.setFutureFeatures(false);
    a.check("53. hasFutureFeatures", !testee.hasFutureFeatures());
}

/** Test descriptions. */
AFL_TEST("game.score.TurnScoreList:getDescription", a)
{
    game::score::TurnScoreList testee;

    // No descriptions by default
    a.checkNull("01. getDescription", testee.getDescription(game::score::ScoreId_Planets));
    a.checkNull("02. getDescription", testee.getDescription(game::score::ScoreId_Capital));
    a.checkEqual("03. getNumDescriptions", testee.getNumDescriptions(), 0U);
    a.checkNull("04. getDescriptionByIndex", testee.getDescriptionByIndex(0));

    // Add some
    game::score::TurnScoreList::Description d;
    d.name = "FooScore";
    d.scoreId = game::score::ScoreId_Score;
    a.check("11. addDescription", testee.addDescription(d));
    a.check("12. addDescription", !testee.addDescription(d));   // Second add is no change

    // Request it
    const game::score::TurnScoreList::Description* pd = testee.getDescription(game::score::ScoreId_Score);
    a.checkNonNull("21. getDescription", pd);
    a.check("22. description is copied", pd != &d);
    a.checkEqual("23. name", pd->name, "FooScore");

    // Update
    d.name = "BarScore";
    a.check("31. addDescription", testee.addDescription(d));
    pd = testee.getDescription(game::score::ScoreId_Score);
    a.checkNonNull("32. getDescription", pd);
    a.check("33. description is copied", pd != &d);
    a.checkEqual("34. name", pd->name, "BarScore");

    // Test
    a.checkEqual("41. getNumDescriptions", testee.getNumDescriptions(), 1U);
    a.check("42. getDescriptionByIndex", testee.getDescriptionByIndex(0) == pd);
}

/** Test handling of turns. */
AFL_TEST("game.score.TurnScoreList:turns", a)
{
    game::score::TurnScoreList testee;

    // No turns
    for (int i = 1; i < 10; ++i) {
        a.checkNull("01", testee.getTurn(i));
    }
    a.checkEqual("02. getNumTurns", testee.getNumTurns(), 0U);
    a.checkNull("03. getTurnByIndex", testee.getTurnByIndex(0));
    a.checkEqual("04. getFirstTurnNumber", testee.getFirstTurnNumber(), 0);

    // Add some turns
    testee.addTurn(1, game::Timestamp(2000, 1, 1, 1, 1, 1));
    testee.addTurn(3, game::Timestamp(2000, 3, 1, 1, 1, 1));
    testee.addTurn(5, game::Timestamp(2000, 5, 1, 1, 1, 1));
    testee.addTurn(7, game::Timestamp(2000, 7, 1, 1, 1, 1));

    a.checkNonNull("11. getTurn", testee.getTurn(1));
    a.checkNull   ("12. getTurn", testee.getTurn(2));
    a.checkNonNull("13. getTurn", testee.getTurn(3));
    a.checkNull   ("14. getTurn", testee.getTurn(4));
    a.checkNonNull("15. getTurn", testee.getTurn(5));
    a.checkNull   ("16. getTurn", testee.getTurn(6));
    a.checkNonNull("17. getTurn", testee.getTurn(7));
    a.checkNull   ("18. getTurn", testee.getTurn(8));

    a.checkEqual  ("21. getNumTurns",    testee.getNumTurns(), 4U);
    a.checkNonNull("22. getTurnByIndex", testee.getTurnByIndex(0));
    a.checkNonNull("23. getTurnByIndex", testee.getTurnByIndex(1));
    a.checkNonNull("24. getTurnByIndex", testee.getTurnByIndex(2));
    a.checkNonNull("25. getTurnByIndex", testee.getTurnByIndex(3));
    a.checkNull   ("26. getTurnByIndex", testee.getTurnByIndex(4));
    a.checkEqual  ("27. getTurnNumber",  testee.getTurnByIndex(0)->getTurnNumber(), 1);
    a.checkEqual  ("28. getTurnNumber",  testee.getTurnByIndex(1)->getTurnNumber(), 3);
    a.checkEqual  ("29. getTurnNumber",  testee.getTurnByIndex(2)->getTurnNumber(), 5);
    a.checkEqual  ("30. getTurnNumber",  testee.getTurnByIndex(3)->getTurnNumber(), 7);

    // Add some more turns
    testee.addTurn(2, game::Timestamp(2000, 2, 1, 1, 1, 1));
    testee.addTurn(4, game::Timestamp(2000, 4, 1, 1, 1, 1));
    testee.addTurn(6, game::Timestamp(2000, 6, 1, 1, 1, 1));
    testee.addTurn(8, game::Timestamp(2000, 8, 1, 1, 1, 1));

    a.checkNonNull("31. getTurn", testee.getTurn(1));
    a.checkNonNull("32. getTurn", testee.getTurn(2));
    a.checkNonNull("33. getTurn", testee.getTurn(3));
    a.checkNonNull("34. getTurn", testee.getTurn(4));
    a.checkNonNull("35. getTurn", testee.getTurn(5));
    a.checkNonNull("36. getTurn", testee.getTurn(6));
    a.checkNonNull("37. getTurn", testee.getTurn(7));
    a.checkNonNull("38. getTurn", testee.getTurn(8));

    a.checkEqual  ("41. getNumTurns",        testee.getNumTurns(), 8U);
    a.checkNonNull("42. getTurnByIndex",     testee.getTurnByIndex(0));
    a.checkNonNull("43. getTurnByIndex",     testee.getTurnByIndex(1));
    a.checkNonNull("44. getTurnByIndex",     testee.getTurnByIndex(2));
    a.checkNonNull("45. getTurnByIndex",     testee.getTurnByIndex(3));
    a.checkNonNull("46. getTurnByIndex",     testee.getTurnByIndex(4));
    a.checkNonNull("47. getTurnByIndex",     testee.getTurnByIndex(5));
    a.checkNonNull("48. getTurnByIndex",     testee.getTurnByIndex(6));
    a.checkNonNull("49. getTurnByIndex",     testee.getTurnByIndex(7));
    a.checkNull   ("50. getTurnByIndex",     testee.getTurnByIndex(8));
    a.checkEqual  ("51. getTurnNumber",      testee.getTurnByIndex(0)->getTurnNumber(), 1);
    a.checkEqual  ("52. getTurnNumber",      testee.getTurnByIndex(1)->getTurnNumber(), 2);
    a.checkEqual  ("53. getTurnNumber",      testee.getTurnByIndex(2)->getTurnNumber(), 3);
    a.checkEqual  ("54. getTurnNumber",      testee.getTurnByIndex(3)->getTurnNumber(), 4);
    a.checkEqual  ("55. getTurnNumber",      testee.getTurnByIndex(4)->getTurnNumber(), 5);
    a.checkEqual  ("56. getTurnNumber",      testee.getTurnByIndex(5)->getTurnNumber(), 6);
    a.checkEqual  ("57. getTurnNumber",      testee.getTurnByIndex(6)->getTurnNumber(), 7);
    a.checkEqual  ("58. getTurnNumber",      testee.getTurnByIndex(7)->getTurnNumber(), 8);
    a.checkEqual  ("59. getFirstTurnNumber", testee.getFirstTurnNumber(), 1);

    // Set some scores
    {
        game::score::TurnScore& t = testee.addTurn(6, game::Timestamp(2000, 6, 1, 1, 1, 1));
        t.set(1, 1, 100);
        t.set(1, 2, 200);
    }

    // Verify
    {
        game::score::TurnScore& t = testee.addTurn(6, game::Timestamp(2000, 6, 1, 1, 1, 1));
        a.checkEqual("61. get", t.get(1, 1).orElse(-1), 100);
        a.checkEqual("62. get", t.get(1, 2).orElse(-1), 200);
    }

    // Set again with new timestamp. This clears the original data
    {
        game::score::TurnScore& t = testee.addTurn(6, game::Timestamp(2000, 6, 1, 1, 1, 2));
        t.set(1, 3, 300);
    }

    // Verify
    {
        game::score::TurnScore& t = testee.addTurn(6, game::Timestamp(2000, 6, 1, 1, 1, 2));
        a.check("71. get", !t.get(1, 1).isValid());
        a.check("72. get", !t.get(1, 2).isValid());
        a.checkEqual("73. get", t.get(1, 3).orElse(-1), 300);
    }
}

/** Test the Description constructor. */
AFL_TEST("game.score.TurnScoreList:Description:constructor", a)
{
    const game::score::TurnScoreList::Description d("name", 30, 5, 300);
    a.checkEqual("01. name",      d.name, "name");
    a.checkEqual("02. scoreId",   d.scoreId, 30);
    a.checkEqual("03. turnLimit", d.turnLimit, 5);
    a.checkEqual("04. winLimit",  d.winLimit, 300);
}

/** Test addMessageInformation(), complete data. */
AFL_TEST("game.score.TurnScoreList:addMessageInformation:full", a)
{
    game::score::TurnScoreList testee;

    // Add message information
    gp::MessageInformation mi(gp::MessageInformation::PlayerScore, 300, 42);
    mi.addValue(gp::mi_ScoreTurnLimit, 5);
    mi.addValue(gp::mi_ScoreWinLimit, 1000);
    mi.addScoreValue(3, 400);
    mi.addScoreValue(9, 100);
    mi.addScoreValue(2, 50);
    mi.addValue(gp::ms_Name, "xScore");
    testee.addMessageInformation(mi, game::Timestamp());

    // Verify resulting description
    const game::score::TurnScoreList::Description* desc = testee.getDescription(300);
    a.checkNonNull("01", desc);
    a.checkEqual("02. name",      desc->name, "xScore");
    a.checkEqual("03. winLimit",  desc->winLimit, 1000);
    a.checkEqual("04. turnLimit", desc->turnLimit, 5);
    a.checkEqual("05. scoreId",   desc->scoreId, 300);

    // Verify resulting slot
    game::score::TurnScoreList::Slot_t id = 0;
    a.check("11", testee.getSlot(300).get(id));

    // Verify resulting score
    const game::score::TurnScore* ts = testee.getTurn(42);
    a.checkNonNull("21. getTurn", ts);
    a.checkEqual("22. getTurnNumber", ts->getTurnNumber(), 42);
    a.checkEqual("23. get", ts->get(id, 3).orElse(0), 400);
    a.checkEqual("24. get", ts->get(id, 9).orElse(0), 100);
    a.checkEqual("25. get", ts->get(id, 2).orElse(0),  50);
    a.checkEqual("26. get", ts->get(id, 1).orElse(-1), -1);
}

/** Test addMessageInformation(), just Id given.
    Must take over partial data. */
AFL_TEST("game.score.TurnScoreList:addMessageInformation:just-id", a)
{
    game::score::TurnScoreList testee;

    // Define pre-existing score
    game::score::TurnScoreList::Description origDesc;
    origDesc.name = "orig name";
    origDesc.scoreId = 30;
    origDesc.turnLimit = 3;
    origDesc.winLimit = 900;
    testee.addDescription(origDesc);

    // Add message information
    gp::MessageInformation mi(gp::MessageInformation::PlayerScore, 30, 42);
    mi.addValue(gp::mi_ScoreTurnLimit, 5);
    mi.addScoreValue(3, 400);
    testee.addMessageInformation(mi, game::Timestamp());

    // Verify resulting description
    const game::score::TurnScoreList::Description* desc = testee.getDescription(30);
    a.checkNonNull("01. getDescription", desc);
    a.checkEqual("02. name",      desc->name, "orig name");    // kept
    a.checkEqual("03. winLimit",  desc->winLimit, 900);        // kept
    a.checkEqual("04. turnLimit", desc->turnLimit, 5);         // overridden
    a.checkEqual("05. scoreId",   desc->scoreId, 30);          // kept

    // Verify resulting slot
    game::score::TurnScoreList::Slot_t id = 0;
    a.check("11. getSlot", testee.getSlot(30).get(id));

    // Verify resulting score
    const game::score::TurnScore* ts = testee.getTurn(42);
    a.checkNonNull("21. getTurn", ts);
    a.checkEqual("22. getTurnNumber", ts->getTurnNumber(), 42);
    a.checkEqual("23. get", ts->get(id, 3).orElse(0), 400);
}

/** Test addMessageInformation(), just name given.
    Must take over partial data. */
AFL_TEST("game.score.TurnScoreList:addMessageInformation:just-name", a)
{
    game::score::TurnScoreList testee;

    // Define pre-existing score
    game::score::TurnScoreList::Description origDesc;
    origDesc.name = "name";
    origDesc.scoreId = 777;
    origDesc.turnLimit = 3;
    origDesc.winLimit = 900;
    testee.addDescription(origDesc);

    // Add message information
    gp::MessageInformation mi(gp::MessageInformation::PlayerScore, 0, 42);
    mi.addValue(gp::mi_ScoreWinLimit, 200);
    mi.addValue(gp::ms_Name, "name");
    mi.addScoreValue(3, 400);
    testee.addMessageInformation(mi, game::Timestamp());

    // Verify resulting description
    const game::score::TurnScoreList::Description* desc = testee.getDescription(777);
    a.checkNonNull("01. getDescription", desc);
    a.checkEqual("02. name",      desc->name, "name");         // kept
    a.checkEqual("03. winLimit",  desc->winLimit, 200);        // overridden
    a.checkEqual("04. turnLimit", desc->turnLimit, 3);         // kept
    a.checkEqual("05. scoreId",   desc->scoreId, 777);         // kept

    // Verify resulting slot
    game::score::TurnScoreList::Slot_t id = 0;
    a.check("11. getSlot", testee.getSlot(777).get(id));

    // Verify resulting score
    const game::score::TurnScore* ts = testee.getTurn(42);
    a.checkNonNull("21. getTurn", ts);
    a.checkEqual("22. getTurnNumber", ts->getTurnNumber(), 42);
    a.checkEqual("23. get", ts->get(id, 3).orElse(0), 400);
}

/** Test addMessageInformation(), just name given, no pre-existing value.
    Must take over partial data. */
AFL_TEST("game.score.TurnScoreList:addMessageInformation:new-name", a)
{
    game::score::TurnScoreList testee;

    // Add message information
    gp::MessageInformation mi(gp::MessageInformation::PlayerScore, 0, 42);
    mi.addValue(gp::mi_ScoreWinLimit, 200);
    mi.addValue(gp::ms_Name, "new name");
    mi.addScoreValue(3, 400);
    testee.addMessageInformation(mi, game::Timestamp());

    // Verify resulting description
    a.check("01. getNumDescriptions", testee.getNumDescriptions() > 0);
    const game::score::TurnScoreList::Description* desc = testee.getDescriptionByIndex(testee.getNumDescriptions()-1);
    a.checkNonNull("02. getDescriptionByIndex", desc);
    a.checkEqual("03. name",        desc->name, "new name");
    a.checkEqual("04. winLimit",    desc->winLimit, 200);
    a.checkEqual("05. turnLimit",   desc->turnLimit, -1);      // not given, set to default
    a.checkDifferent("06. scoreId", desc->scoreId, 0);
}
