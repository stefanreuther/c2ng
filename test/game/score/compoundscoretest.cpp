/**
  *  \file test/game/score/compoundscoretest.cpp
  *  \brief Test for game::score::CompoundScore
  */

#include "game/score/compoundscore.hpp"

#include "afl/test/testrunner.hpp"
#include "game/score/turnscorelist.hpp"

using game::PlayerSet_t;
using game::score::CompoundScore;
using game::score::TurnScoreList;

/** Simple tests. */
AFL_TEST("game.score.CompoundScore:basics", a)
{
    // Prepare a score file
    TurnScoreList list;
    TurnScoreList::Slot_t freighterSlot = list.addSlot(game::score::ScoreId_Freighters);
    TurnScoreList::Slot_t capitalSlot = list.addSlot(game::score::ScoreId_Capital);
    TurnScoreList::Slot_t planetSlot = list.addSlot(game::score::ScoreId_Planets);
    TurnScoreList::Slot_t baseSlot = list.addSlot(game::score::ScoreId_Bases);
    list.addSlot(game::score::ScoreId_Score);

    game::score::TurnScore& t5 = list.addTurn(5, game::Timestamp());
    t5.set(freighterSlot, 1, 3);
    t5.set(capitalSlot, 1, 5);
    t5.set(planetSlot, 1, 7);
    t5.set(baseSlot, 1, 1);

    t5.set(freighterSlot, 2, 30);
    t5.set(capitalSlot, 2, 50);
    t5.set(planetSlot, 2, 70);
    t5.set(baseSlot, 2, 10);

    t5.set(freighterSlot, 3, 9);

    // Query empty score
    a.checkEqual("01. get", CompoundScore().get(t5, 1).orElse(-1), 0);
    a.checkEqual("02. get", CompoundScore().get(t5, 2).orElse(-1), 0);
    a.checkEqual("03. get", CompoundScore().get(t5, PlayerSet_t()+1+2).orElse(-1), 0);
    a.checkEqual("04. get", CompoundScore().get(list, 5, 1).orElse(-1), 0);
    a.checkEqual("05. get", CompoundScore().get(list, 5, PlayerSet_t()+1+2).orElse(-1), 0);
    a.checkEqual("06. get", CompoundScore().get(list, 9, 1).orElse(-1), 0);

    // Query single-slot score
    a.checkEqual("11. get", CompoundScore(list, game::score::ScoreId_Capital, 1).get(t5, 1).orElse(-1), 5);
    a.checkEqual("12. get", CompoundScore(list, game::score::ScoreId_Capital, 1).get(t5, 2).orElse(-1), 50);
    a.checkEqual("13. get", CompoundScore(list, game::score::ScoreId_Capital, 1).get(t5, 3).isValid(), false);
    a.checkEqual("14. get", CompoundScore(list, game::score::ScoreId_Capital, 1).get(t5, PlayerSet_t()+1+2).orElse(-1), 55);
    a.checkEqual("15. get", CompoundScore(list, game::score::ScoreId_Capital, 1).get(t5, PlayerSet_t()+1+2+3).orElse(-1), 55);
    a.checkEqual("16. get", CompoundScore(list, game::score::ScoreId_Capital, 1).get(t5, PlayerSet_t()+3).isValid(), false);
    a.checkEqual("17. get", CompoundScore(list, game::score::ScoreId_Capital, 1).get(list, 5, 1).orElse(-1), 5);
    a.checkEqual("18. get", CompoundScore(list, game::score::ScoreId_Capital, 1).get(list, 5, PlayerSet_t()+1+2).orElse(-1), 55);

    // Query single-slot score, scaled
    a.checkEqual("21. get", CompoundScore(list, game::score::ScoreId_Capital, 3).get(t5, 1).orElse(-1), 15);
    a.checkEqual("22. get", CompoundScore(list, game::score::ScoreId_Capital, 3).get(t5, 2).orElse(-1), 150);
    a.checkEqual("23. get", CompoundScore(list, game::score::ScoreId_Capital, 3).get(t5, PlayerSet_t()+1+2).orElse(-1), 165);

    // Query default score
    a.checkEqual("31. get", CompoundScore(list, CompoundScore::TotalShips).get(t5, 1).orElse(-1), 8);
    a.checkEqual("32. get", CompoundScore(list, CompoundScore::TotalShips).get(t5, 2).orElse(-1), 80);
    a.checkEqual("33. get", CompoundScore(list, CompoundScore::TotalShips).get(t5, 3).orElse(-1), 9);
    a.checkEqual("34. get", CompoundScore(list, CompoundScore::TotalShips).get(t5, PlayerSet_t()+2+3).orElse(-1), 89);
    a.checkEqual("35. get", CompoundScore(list, CompoundScore::TotalShips).get(list, 5, 3).orElse(-1), 9);
    a.checkEqual("36. get", CompoundScore(list, CompoundScore::TotalShips).get(list, 5, PlayerSet_t()+2+3).orElse(-1), 89);
    a.checkEqual("37. get", CompoundScore(list, CompoundScore::TotalShips).get(list, 9, 3).isValid(), false);
    a.checkEqual("38. get", CompoundScore(list, CompoundScore::TotalShips).get(list, 9, PlayerSet_t()+2+3).isValid(), false);

    a.checkEqual("41. get", CompoundScore(list, CompoundScore::TimScore).get(t5, 1).orElse(-1), 243);

    // Query nonexistant single-slot
    a.checkEqual("51. get", CompoundScore(list, 1000, 1).get(t5, 1).isValid(), false);
    a.checkEqual("52. get", CompoundScore(list, 1000, 1).get(t5, PlayerSet_t()+1+2).isValid(), false);
    a.checkEqual("53. get", CompoundScore(list, 1000, 1).get(list, 5, 1).isValid(), false);
    a.checkEqual("54. get", CompoundScore(list, 1000, 1).get(list, 5, PlayerSet_t()+1+2).isValid(), false);
    a.checkEqual("55. isValid", CompoundScore(list, 1000, 1).isValid(), false);

    // Query overlong score
    {
        CompoundScore longScore;
        longScore.add(list, game::score::ScoreId_Bases, 1);
        longScore.add(list, game::score::ScoreId_Capital, 1);
        longScore.add(list, game::score::ScoreId_Freighters, 1);
        longScore.add(list, game::score::ScoreId_Planets, 1);
        longScore.add(list, game::score::ScoreId_Score, 1);
        a.checkEqual("61. get", longScore.get(t5, 1).isValid(), false);
        a.checkEqual("62. get", longScore.get(t5, PlayerSet_t(1)).isValid(), false);
        a.checkEqual("63. get", longScore.get(list, 5, 1).isValid(), false);
        a.checkEqual("64. get", longScore.get(list, 5, PlayerSet_t(1)).isValid(), false);
        a.checkEqual("65. isValid", longScore.isValid(), false);
    }
}

AFL_TEST("game.score.CompoundScore:compare", a)
{
    // Prepare a score file
    TurnScoreList list;
    list.addSlot(game::score::ScoreId_Freighters);
    list.addSlot(game::score::ScoreId_Capital);
    list.addSlot(game::score::ScoreId_Planets);
    list.addSlot(game::score::ScoreId_Bases);

    CompoundScore s(list, game::score::ScoreId_Freighters, 1);
    a.checkEqual("01. eq", s == s, true);
    a.checkEqual("02. ne", s != s, false);

    a.checkEqual("11. eq", s == CompoundScore(list, game::score::ScoreId_Freighters, 2), false);
    a.checkEqual("12. eq", s == CompoundScore(list, game::score::ScoreId_Capital, 2), false);
    a.checkEqual("13. eq", s == CompoundScore(list, CompoundScore::TotalShips), false);
    a.checkEqual("14. eq", s == CompoundScore(list, 1000, 1), false);
}
