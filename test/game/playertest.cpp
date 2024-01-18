/**
  *  \file test/game/playertest.cpp
  *  \brief Test for game::Player
  */

#include "game/player.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Test setters/getters. */
AFL_TEST("game.Player:basics", a)
{
    afl::string::NullTranslator tx;
    game::Player testee(10);
    a.checkEqual("01. getId", testee.getId(), 10);
    a.check("02. isReal", testee.isReal());

    // Names start out empty
    a.checkEqual("11. getName", testee.getName(game::Player::LongName, tx), "Player 10");

    // Set
    testee.setName(game::Player::LongName, "Long");
    testee.setName(game::Player::EmailAddress, "a@b.c");
    a.checkEqual("21. LongName",         testee.getName(game::Player::LongName, tx), "Long");
    a.checkEqual("22. OriginalLongName", testee.getName(game::Player::OriginalLongName, tx), "Player 10");
    a.checkEqual("23. EmailAddress",     testee.getName(game::Player::EmailAddress, tx), "a@b.c");

    testee.setIsReal(false);
    a.check("31. isReal", !testee.isReal());
}

/** Test init functions. */
AFL_TEST("game.Player:init", a)
{
    // Check alien
    afl::string::NullTranslator tx;
    game::Player pa(10);
    pa.initAlien();
    a.check("01. isReal", !pa.isReal());
    a.checkDifferent("02. getName", pa.getName(game::Player::LongName, tx), "");
    a.checkEqual("03. getName", pa.getName(game::Player::LongName, tx), pa.getName(game::Player::OriginalLongName, tx));

    pa.setName(game::Player::LongName, "blob");
    a.checkEqual("11. getName", pa.getName(game::Player::LongName, tx), "blob");
    a.checkDifferent("12. getName", pa.getName(game::Player::LongName, tx), pa.getName(game::Player::OriginalLongName, tx));

    // Check unowned
    game::Player u(10);
    u.initUnowned();
    a.check("21. isReal", !u.isReal());
    a.checkDifferent("22. getName", u.getName(game::Player::LongName, tx), "");
    a.checkEqual("23. getName", u.getName(game::Player::LongName, tx), u.getName(game::Player::OriginalLongName, tx));

    // Check that alien and unowned are different
    a.checkDifferent("31. getName", u.getName(game::Player::LongName, tx), pa.getName(game::Player::LongName, tx));
}

/** Test change tracking. */
AFL_TEST("game.Player:change", a)
{
    game::Player testee(10);
    a.check("01. isChanged", !testee.isChanged());

    // setName
    testee.setName(game::Player::EmailAddress, "x@y.z");
    a.check("11. isChanged", testee.isChanged());
    testee.markChanged(false);

    // setIsReal
    testee.setIsReal(false);
    a.check("21. isChanged", testee.isChanged());
    testee.markChanged(false);

    // initUnowned
    testee.initUnowned();
    a.check("31. isChanged", testee.isChanged());
    testee.markChanged(false);

    // initAlien
    testee.initAlien();
    a.check("41. isChanged", testee.isChanged());
    testee.markChanged(false);
}

/** Test setOriginalNames. */
AFL_TEST("game.Player:setOriginalNames", a)
{
    afl::string::NullTranslator tx;
    game::Player testee(10);
    testee.setName(game::Player::LongName, "Long");
    testee.setName(game::Player::ShortName, "Short");
    testee.setName(game::Player::AdjectiveName, "Adj");
    a.checkEqual("01. OriginalLongName",      testee.getName(game::Player::OriginalLongName, tx), "Player 10");
    a.checkEqual("02. OriginalShortName",     testee.getName(game::Player::OriginalShortName, tx), "Player 10");
    a.checkEqual("03. OriginalAdjectiveName", testee.getName(game::Player::OriginalAdjectiveName, tx), "Player 10");

    testee.setOriginalNames();

    a.checkEqual("11. LongName",              testee.getName(game::Player::LongName, tx), "Long");
    a.checkEqual("12. ShortName",             testee.getName(game::Player::ShortName, tx), "Short");
    a.checkEqual("13. AdjectiveName",         testee.getName(game::Player::AdjectiveName, tx), "Adj");
    a.checkEqual("14. OriginalLongName",      testee.getName(game::Player::OriginalLongName, tx), "Long");
    a.checkEqual("15. OriginalShortName",     testee.getName(game::Player::OriginalShortName, tx), "Short");
    a.checkEqual("16. OriginalAdjectiveName", testee.getName(game::Player::OriginalAdjectiveName, tx), "Adj");
}
