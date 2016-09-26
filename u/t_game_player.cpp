/**
  *  \file u/t_game_player.cpp
  *  \brief Test for game::Player
  */

#include "game/player.hpp"

#include "t_game.hpp"

/** Test setters/getters. */
void
TestGamePlayer::testSet()
{
    game::Player testee(10);
    TS_ASSERT_EQUALS(testee.getId(), 10);
    TS_ASSERT(testee.isReal());

    // Names start out empty
    TS_ASSERT_EQUALS(testee.getName(game::Player::LongName), "");

    // Set
    testee.setName(game::Player::LongName, "Long");
    testee.setName(game::Player::EmailAddress, "a@b.c");
    TS_ASSERT_EQUALS(testee.getName(game::Player::LongName), "Long");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalLongName), "");
    TS_ASSERT_EQUALS(testee.getName(game::Player::EmailAddress), "a@b.c");

    testee.setIsReal(false);
    TS_ASSERT(!testee.isReal());
}

/** Test init functions. */
void
TestGamePlayer::testInit()
{
    // Check alien
    game::Player a(10);
    a.initAlien();
    TS_ASSERT(!a.isReal());
    TS_ASSERT_DIFFERS(a.getName(game::Player::LongName), "");
    TS_ASSERT_EQUALS(a.getName(game::Player::LongName), a.getName(game::Player::OriginalLongName));

    // Check unowned
    game::Player u(10);
    u.initUnowned();
    TS_ASSERT(!u.isReal());
    TS_ASSERT_DIFFERS(u.getName(game::Player::LongName), "");
    TS_ASSERT_EQUALS(u.getName(game::Player::LongName), u.getName(game::Player::OriginalLongName));

    // Check that alien and unowned are different
    TS_ASSERT_DIFFERS(u.getName(game::Player::LongName), a.getName(game::Player::LongName));
}

/** Test change tracking. */
void
TestGamePlayer::testChange()
{
    game::Player testee(10);
    TS_ASSERT(!testee.isChanged());

    // setName
    testee.setName(game::Player::EmailAddress, "x@y.z");
    TS_ASSERT(testee.isChanged());
    testee.markChanged(false);

    // setIsReal
    testee.setIsReal(false);
    TS_ASSERT(testee.isChanged());
    testee.markChanged(false);

    // initUnowned
    testee.initUnowned();
    TS_ASSERT(testee.isChanged());
    testee.markChanged(false);

    // initAlien
    testee.initAlien();
    TS_ASSERT(testee.isChanged());
    testee.markChanged(false);
}

/** Test setOriginalNames. */
void
TestGamePlayer::testOriginal()
{
    game::Player testee(10);
    testee.setName(game::Player::LongName, "Long");
    testee.setName(game::Player::ShortName, "Short");
    testee.setName(game::Player::AdjectiveName, "Adj");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalLongName), "");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalShortName), "");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalAdjectiveName), "");

    testee.setOriginalNames();

    TS_ASSERT_EQUALS(testee.getName(game::Player::LongName), "Long");
    TS_ASSERT_EQUALS(testee.getName(game::Player::ShortName), "Short");
    TS_ASSERT_EQUALS(testee.getName(game::Player::AdjectiveName), "Adj");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalLongName), "Long");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalShortName), "Short");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalAdjectiveName), "Adj");
}
