/**
  *  \file u/t_game_player.cpp
  *  \brief Test for game::Player
  */

#include "game/player.hpp"

#include "t_game.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test setters/getters. */
void
TestGamePlayer::testSet()
{
    afl::string::NullTranslator tx;
    game::Player testee(10);
    TS_ASSERT_EQUALS(testee.getId(), 10);
    TS_ASSERT(testee.isReal());

    // Names start out empty
    TS_ASSERT_EQUALS(testee.getName(game::Player::LongName, tx), "Player 10");

    // Set
    testee.setName(game::Player::LongName, "Long");
    testee.setName(game::Player::EmailAddress, "a@b.c");
    TS_ASSERT_EQUALS(testee.getName(game::Player::LongName, tx), "Long");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalLongName, tx), "Player 10");
    TS_ASSERT_EQUALS(testee.getName(game::Player::EmailAddress, tx), "a@b.c");

    testee.setIsReal(false);
    TS_ASSERT(!testee.isReal());
}

/** Test init functions. */
void
TestGamePlayer::testInit()
{
    // Check alien
    afl::string::NullTranslator tx;
    game::Player a(10);
    a.initAlien();
    TS_ASSERT(!a.isReal());
    TS_ASSERT_DIFFERS(a.getName(game::Player::LongName, tx), "");
    TS_ASSERT_EQUALS(a.getName(game::Player::LongName, tx), a.getName(game::Player::OriginalLongName, tx));

    a.setName(game::Player::LongName, "blob");
    TS_ASSERT_EQUALS(a.getName(game::Player::LongName, tx), "blob");
    TS_ASSERT_DIFFERS(a.getName(game::Player::LongName, tx), a.getName(game::Player::OriginalLongName, tx));

    // Check unowned
    game::Player u(10);
    u.initUnowned();
    TS_ASSERT(!u.isReal());
    TS_ASSERT_DIFFERS(u.getName(game::Player::LongName, tx), "");
    TS_ASSERT_EQUALS(u.getName(game::Player::LongName, tx), u.getName(game::Player::OriginalLongName, tx));

    // Check that alien and unowned are different
    TS_ASSERT_DIFFERS(u.getName(game::Player::LongName, tx), a.getName(game::Player::LongName, tx));
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
    afl::string::NullTranslator tx;
    game::Player testee(10);
    testee.setName(game::Player::LongName, "Long");
    testee.setName(game::Player::ShortName, "Short");
    testee.setName(game::Player::AdjectiveName, "Adj");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalLongName, tx), "Player 10");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalShortName, tx), "Player 10");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalAdjectiveName, tx), "Player 10");

    testee.setOriginalNames();

    TS_ASSERT_EQUALS(testee.getName(game::Player::LongName, tx), "Long");
    TS_ASSERT_EQUALS(testee.getName(game::Player::ShortName, tx), "Short");
    TS_ASSERT_EQUALS(testee.getName(game::Player::AdjectiveName, tx), "Adj");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalLongName, tx), "Long");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalShortName, tx), "Short");
    TS_ASSERT_EQUALS(testee.getName(game::Player::OriginalAdjectiveName, tx), "Adj");
}
