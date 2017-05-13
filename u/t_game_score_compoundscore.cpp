/**
  *  \file u/t_game_score_compoundscore.cpp
  *  \brief Test for game::score::CompoundScore
  */

#include "game/score/compoundscore.hpp"

#include "t_game_score.hpp"
#include "game/score/turnscorelist.hpp"

/** Simple tests. */
void
TestGameScoreCompoundScore::testIt()
{
    using game::score::CompoundScore;
    using game::PlayerSet_t;

    // Prepare a score file
    game::score::TurnScoreList list;
    game::score::TurnScoreList::Slot_t freighterSlot = list.addSlot(game::score::ScoreId_Freighters);
    game::score::TurnScoreList::Slot_t capitalSlot = list.addSlot(game::score::ScoreId_Capital);
    game::score::TurnScoreList::Slot_t planetSlot = list.addSlot(game::score::ScoreId_Planets);
    game::score::TurnScoreList::Slot_t baseSlot = list.addSlot(game::score::ScoreId_Bases);
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
    TS_ASSERT_EQUALS(CompoundScore().get(t5, 1).orElse(-1), 0);
    TS_ASSERT_EQUALS(CompoundScore().get(t5, 2).orElse(-1), 0);
    TS_ASSERT_EQUALS(CompoundScore().get(t5, PlayerSet_t()+1+2).orElse(-1), 0);
    TS_ASSERT_EQUALS(CompoundScore().get(list, 5, 1).orElse(-1), 0);
    TS_ASSERT_EQUALS(CompoundScore().get(list, 5, PlayerSet_t()+1+2).orElse(-1), 0);
    TS_ASSERT_EQUALS(CompoundScore().get(list, 9, 1).orElse(-1), 0);

    // Query single-slot score
    TS_ASSERT_EQUALS(CompoundScore(list, game::score::ScoreId_Capital, 1).get(t5, 1).orElse(-1), 5);
    TS_ASSERT_EQUALS(CompoundScore(list, game::score::ScoreId_Capital, 1).get(t5, 2).orElse(-1), 50);
    TS_ASSERT_EQUALS(CompoundScore(list, game::score::ScoreId_Capital, 1).get(t5, 3).isValid(), false);
    TS_ASSERT_EQUALS(CompoundScore(list, game::score::ScoreId_Capital, 1).get(t5, PlayerSet_t()+1+2).orElse(-1), 55);
    TS_ASSERT_EQUALS(CompoundScore(list, game::score::ScoreId_Capital, 1).get(t5, PlayerSet_t()+1+2+3).orElse(-1), 55);
    TS_ASSERT_EQUALS(CompoundScore(list, game::score::ScoreId_Capital, 1).get(t5, PlayerSet_t()+3).isValid(), false);
    TS_ASSERT_EQUALS(CompoundScore(list, game::score::ScoreId_Capital, 1).get(list, 5, 1).orElse(-1), 5);
    TS_ASSERT_EQUALS(CompoundScore(list, game::score::ScoreId_Capital, 1).get(list, 5, PlayerSet_t()+1+2).orElse(-1), 55);

    // Query single-slot score, scaled
    TS_ASSERT_EQUALS(CompoundScore(list, game::score::ScoreId_Capital, 3).get(t5, 1).orElse(-1), 15);
    TS_ASSERT_EQUALS(CompoundScore(list, game::score::ScoreId_Capital, 3).get(t5, 2).orElse(-1), 150);
    TS_ASSERT_EQUALS(CompoundScore(list, game::score::ScoreId_Capital, 3).get(t5, PlayerSet_t()+1+2).orElse(-1), 165);

    // Query default score
    TS_ASSERT_EQUALS(CompoundScore(list, CompoundScore::TotalShips).get(t5, 1).orElse(-1), 8);
    TS_ASSERT_EQUALS(CompoundScore(list, CompoundScore::TotalShips).get(t5, 2).orElse(-1), 80);
    TS_ASSERT_EQUALS(CompoundScore(list, CompoundScore::TotalShips).get(t5, 3).orElse(-1), 9);
    TS_ASSERT_EQUALS(CompoundScore(list, CompoundScore::TotalShips).get(t5, PlayerSet_t()+2+3).orElse(-1), 89);
    TS_ASSERT_EQUALS(CompoundScore(list, CompoundScore::TotalShips).get(list, 5, 3).orElse(-1), 9);
    TS_ASSERT_EQUALS(CompoundScore(list, CompoundScore::TotalShips).get(list, 5, PlayerSet_t()+2+3).orElse(-1), 89);
    TS_ASSERT_EQUALS(CompoundScore(list, CompoundScore::TotalShips).get(list, 9, 3).isValid(), false);
    TS_ASSERT_EQUALS(CompoundScore(list, CompoundScore::TotalShips).get(list, 9, PlayerSet_t()+2+3).isValid(), false);

    TS_ASSERT_EQUALS(CompoundScore(list, CompoundScore::TimScore).get(t5, 1).orElse(-1), 243);

    // Query nonexistant single-slot
    TS_ASSERT_EQUALS(CompoundScore(list, 1000, 1).get(t5, 1).isValid(), false);
    TS_ASSERT_EQUALS(CompoundScore(list, 1000, 1).get(t5, PlayerSet_t()+1+2).isValid(), false);
    TS_ASSERT_EQUALS(CompoundScore(list, 1000, 1).get(list, 5, 1).isValid(), false);
    TS_ASSERT_EQUALS(CompoundScore(list, 1000, 1).get(list, 5, PlayerSet_t()+1+2).isValid(), false);

    // Query overlong score
    {
        CompoundScore longScore;
        longScore.add(list, game::score::ScoreId_Bases, 1);
        longScore.add(list, game::score::ScoreId_Capital, 1);
        longScore.add(list, game::score::ScoreId_Freighters, 1);
        longScore.add(list, game::score::ScoreId_Planets, 1);
        longScore.add(list, game::score::ScoreId_Score, 1);
        TS_ASSERT_EQUALS(longScore.get(t5, 1).isValid(), false);
        TS_ASSERT_EQUALS(longScore.get(t5, PlayerSet_t(1)).isValid(), false);
        TS_ASSERT_EQUALS(longScore.get(list, 5, 1).isValid(), false);
        TS_ASSERT_EQUALS(longScore.get(list, 5, PlayerSet_t(1)).isValid(), false);
    }
}

