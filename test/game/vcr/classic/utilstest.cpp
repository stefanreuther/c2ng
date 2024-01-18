/**
  *  \file test/game/vcr/classic/utilstest.cpp
  *  \brief Test for game::vcr::classic::Utils
  */

#include "game/vcr/classic/utils.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/test/translator.hpp"

namespace gvc = game::vcr::classic;
using game::TeamSettings;

/** Test formatBattleResult(). */
AFL_TEST("game.vcr.classic.Utils:formatBattleResult", a)
{
    afl::test::Translator tx;

    // Unknown
    a.checkEqual("01", gvc::formatBattleResult(gvc::BattleResult_t(),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<unknown. Wait while computing...>");

    // Invalid
    a.checkEqual("11", gvc::formatBattleResult(gvc::BattleResult_t(gvc::Invalid),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Battle cannot be played!>");

    // Timeout
    a.checkEqual("21", gvc::formatBattleResult(gvc::BattleResult_t(gvc::Timeout),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Battle timed out (too long).>");

    // Stalemate
    a.checkEqual("31", gvc::formatBattleResult(gvc::BattleResult_t(gvc::Stalemate),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Stalemate.>");

    // Left destroyed
    a.checkEqual("41", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<We were destroyed (anno).>");
    a.checkEqual("42", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<We were destroyed.>");
    a.checkEqual("43", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::ThisPlayer,
                                             "", tx),
                     "<We won.>");
    a.checkEqual("44", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<Right won.>");
    a.checkEqual("45", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Right won (anno).>");

    // Right destroyed
    a.checkEqual("51", gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightDestroyed),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<We won (anno).>");
    a.checkEqual("52", gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightDestroyed),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<We won.>");
    a.checkEqual("53", gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightDestroyed),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::ThisPlayer,
                                             "", tx),
                     "<We were destroyed.>");
    a.checkEqual("54", gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightDestroyed),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<Left won.>");
    a.checkEqual("55", gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightDestroyed),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Left won (anno).>");

    // Left captured
    a.checkEqual("61", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftCaptured),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<They have captured our ship (anno).>");
    a.checkEqual("62", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftCaptured),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<They have captured our ship.>");
    a.checkEqual("63", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftCaptured),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::ThisPlayer,
                                             "", tx),
                     "<We captured their ship.>");
    a.checkEqual("64", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftCaptured),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<Left was captured.>");
    a.checkEqual("65", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftCaptured),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Left was captured (anno).>");

    // Right captured
    a.checkEqual("71", gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightCaptured),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<We captured their ship (anno).>");
    a.checkEqual("72", gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightCaptured),
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<We captured their ship.>");
    a.checkEqual("73", gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightCaptured),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::ThisPlayer,
                                             "", tx),
                     "<They have captured our ship.>");
    a.checkEqual("74", gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightCaptured),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "", tx),
                     "<Right was captured.>");
    a.checkEqual("75", gvc::formatBattleResult(gvc::BattleResult_t(gvc::RightCaptured),
                                             "Left", TeamSettings::EnemyPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Right was captured (anno).>");

    // Both destroyed
    a.checkEqual("81", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed) + gvc::RightDestroyed,
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Both were destroyed.>");

    // Mix
    a.checkEqual("91", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftDestroyed) + gvc::RightCaptured,
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Both are disabled.>");
    a.checkEqual("92", gvc::formatBattleResult(gvc::BattleResult_t(gvc::LeftCaptured) + gvc::RightCaptured,
                                             "Left", TeamSettings::ThisPlayer,
                                             "Right", TeamSettings::EnemyPlayer,
                                             "anno", tx),
                     "<Both are disabled.>");
}
