/**
  *  \file test/game/playersettest.cpp
  *  \brief Test for game::PlayerSet
  */

#include "game/playerset.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/limits.hpp"
#include "game/playerlist.hpp"

using game::PlayerList;
using game::PlayerSet_t;

/** Test formatPlayerSet(). */

// Base set has multiple players
AFL_TEST("game.PlayerSet:formatPlayerSet:normal", a)
{
    afl::string::NullTranslator tx;
    PlayerList pl;
    pl.create(1);
    pl.create(2);
    pl.create(3);
    pl.create(4);

    a.checkEqual("01", game::formatPlayerSet(PlayerSet_t(),                     pl, tx), "nobody");
    a.checkEqual("02", game::formatPlayerSet(PlayerSet_t()                 + 5, pl, tx), "nobody");
    a.checkEqual("03", game::formatPlayerSet(PlayerSet_t() + 1,                 pl, tx), "player 1");
    a.checkEqual("04", game::formatPlayerSet(PlayerSet_t() + 1 + 2 + 3 + 4,     pl, tx), "");
    a.checkEqual("05", game::formatPlayerSet(PlayerSet_t() + 1 + 2 + 3 + 4 + 5, pl, tx), "");
    a.checkEqual("06", game::formatPlayerSet(PlayerSet_t() + 1             + 5, pl, tx), "player 1");
    a.checkEqual("07", game::formatPlayerSet(PlayerSet_t() + 1 + 2     + 4,     pl, tx), "all but player 3");
    a.checkEqual("08", game::formatPlayerSet(PlayerSet_t() + 1 + 2     + 4 + 5, pl, tx), "all but player 3");
    a.checkEqual("09", game::formatPlayerSet(PlayerSet_t() + 1         + 4,     pl, tx), "players 1, 4");
    a.checkEqual("10", game::formatPlayerSet(PlayerSet_t() + 1         + 4 + 5, pl, tx), "players 1, 4");
}

// Empty base set.
AFL_TEST("game.PlayerSet:formatPlayerSet:empty-base-set", a)
{
    afl::string::NullTranslator tx;
    PlayerList pl;
    a.checkEqual("01", game::formatPlayerSet(PlayerSet_t(),                     pl, tx), "");
    a.checkEqual("02", game::formatPlayerSet(PlayerSet_t() + 1,                 pl, tx), "");
    a.checkEqual("03", game::formatPlayerSet(PlayerSet_t() + 1 + 2 + 3 + 4,     pl, tx), "");
}

AFL_TEST("game.PlayerSet:formatPlayerSet:unit-base-set", a)
{
    afl::string::NullTranslator tx;
    PlayerList pl;
    pl.create(3);
    a.checkEqual("01", game::formatPlayerSet(PlayerSet_t(),                     pl, tx), "nobody");
    a.checkEqual("02", game::formatPlayerSet(PlayerSet_t() + 1,                 pl, tx), "nobody");
    a.checkEqual("03", game::formatPlayerSet(PlayerSet_t() + 1 + 2 + 3 + 4,     pl, tx), "");
}

/** Test formatPlayerHostSet(). */

// Base set has multiple players
AFL_TEST("game.PlayerSet:formatPlayerHostSet", a)
{
    afl::string::NullTranslator tx;
    PlayerList pl;
    pl.create(1);
    pl.create(2);
    pl.create(3);
    pl.create(4);

    a.checkEqual("01", game::formatPlayerHostSet(PlayerSet_t(),                         pl, tx), "nobody");
    a.checkEqual("02", game::formatPlayerHostSet(PlayerSet_t() + 0,                     pl, tx), "host");
    a.checkEqual("03", game::formatPlayerHostSet(PlayerSet_t()                     + 5, pl, tx), "nobody");
    a.checkEqual("04", game::formatPlayerHostSet(PlayerSet_t()     + 1,                 pl, tx), "player 1");
    a.checkEqual("05", game::formatPlayerHostSet(PlayerSet_t() + 0 + 1,                 pl, tx), "host, player 1");
    a.checkEqual("06", game::formatPlayerHostSet(PlayerSet_t() + 0 + 1 + 2 + 3 + 4,     pl, tx), "host, all players");
    a.checkEqual("07", game::formatPlayerHostSet(PlayerSet_t()     + 1 + 2 + 3 + 4,     pl, tx), "all players");
    a.checkEqual("08", game::formatPlayerHostSet(PlayerSet_t()     + 1 + 2 + 3 + 4 + 5, pl, tx), "all players");
    a.checkEqual("09", game::formatPlayerHostSet(PlayerSet_t()     + 1             + 5, pl, tx), "player 1");
    a.checkEqual("10", game::formatPlayerHostSet(PlayerSet_t()     + 1 + 2     + 4,     pl, tx), "all but player 3");
    a.checkEqual("11", game::formatPlayerHostSet(PlayerSet_t() + 0 + 1 + 2     + 4,     pl, tx), "host, all but player 3");
    a.checkEqual("12", game::formatPlayerHostSet(PlayerSet_t()     + 1 + 2     + 4 + 5, pl, tx), "all but player 3");
    a.checkEqual("13", game::formatPlayerHostSet(PlayerSet_t()     + 1         + 4,     pl, tx), "players 1, 4");
    a.checkEqual("14", game::formatPlayerHostSet(PlayerSet_t() + 0 + 1         + 4,     pl, tx), "host, players 1, 4");
    a.checkEqual("15", game::formatPlayerHostSet(PlayerSet_t()     + 1         + 4 + 5, pl, tx), "players 1, 4");
}

/** Test formatPlayerSetAsList(). */
AFL_TEST("game.PlayerSet:formatPlayerSetAsList", a)
{
    a.checkEqual("01", game::MAX_PLAYERS, 31);

    a.checkEqual("11", game::formatPlayerSetAsList(PlayerSet_t()),                      "");
    a.checkEqual("12", game::formatPlayerSetAsList(PlayerSet_t() + 0),                  "0");
    a.checkEqual("13", game::formatPlayerSetAsList(PlayerSet_t() + 31),                 "31");
    a.checkEqual("14", game::formatPlayerSetAsList(PlayerSet_t() + 1 + 2 + 3 + 5 + 11), "1 2 3 5 11");
}


/** Test parsePlayerListAsSet(). */
AFL_TEST("game.PlayerSet:parsePlayerListAsSet", a)
{
    a.checkEqual("01", game::MAX_PLAYERS, 31);

    // Good cases
    a.checkEqual("11", game::parsePlayerListAsSet(""),           PlayerSet_t());
    a.checkEqual("12", game::parsePlayerListAsSet("0"),          PlayerSet_t() + 0);
    a.checkEqual("13", game::parsePlayerListAsSet("31"),         PlayerSet_t() + 31);
    a.checkEqual("14", game::parsePlayerListAsSet("1 2 3 5 11"), PlayerSet_t() + 1 + 2 + 3 + 5 + 11);

    // "Bad" cases
    a.checkEqual("21", game::parsePlayerListAsSet("nope"),       PlayerSet_t());
    a.checkEqual("22", game::parsePlayerListAsSet("c00l"),       PlayerSet_t() + 0);
    a.checkEqual("23", game::parsePlayerListAsSet("150"),        PlayerSet_t());
    a.checkEqual("24", game::parsePlayerListAsSet("-3"),         PlayerSet_t());      // parsed as number -3, not as "-" + number 3
}
