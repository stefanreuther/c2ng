/**
  *  \file u/t_game_vcr_classic_utils.cpp
  *  \brief Test for game::vcr::classic::Utils
  */

#include "game/vcr/classic/utils.hpp"

#include "t_game_vcr_classic.hpp"
#include "afl/test/translator.hpp"

namespace gvc = game::vcr::classic;
using game::TeamSettings;

/** Test formatBattleResult(). */
void
TestGameVcrClassicUtils::testFormatBattleResult()
{
    afl::test::Translator tx;

    // Unknown
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<unknown. Wait while computing...>");

    // Invalid
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::Invalid),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Battle cannot be played!>");

    // Timeout
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::Timeout),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Battle timed out (too long).>");

    // Stalemate
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::Stalemate),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Stalemate.>");

    // Left destroyed
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<We were destroyed (anno).>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<We were destroyed.>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::ThisPlayer,
                                             "", tx),
                     "<We won.>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<Right won.>");

    // Right destroyed
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightDestroyed),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<We won (anno).>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightDestroyed),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<We won.>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightDestroyed),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::ThisPlayer,
                                             "", tx),
                     "<We were destroyed.>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightDestroyed),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<Left won.>");

    // Left captured
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftCaptured),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<They have captured our ship (anno).>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftCaptured),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<They have captured our ship.>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftCaptured),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::ThisPlayer,
                                             "", tx),
                     "<We captured their ship.>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftCaptured),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<Left was captured.>");

    // Right captured
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightCaptured),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<We captured their ship (anno).>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightCaptured),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<We captured their ship.>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightCaptured),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::ThisPlayer,
                                             "", tx),
                     "<They have captured our ship.>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightCaptured),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<Right was captured.>");

    // Both destroyed
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed) + gvc::RightDestroyed,
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Both were destroyed.>");

    // Mix
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed) + gvc::RightCaptured,
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Both are disabled.>");
    TS_ASSERT_EQUALS(gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftCaptured) + gvc::RightCaptured,
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Both are disabled.>");
}

