/**
  *  \file u/t_game_playerset.cpp
  *  \brief Test for game::PlayerSet
  */

#include "game/playerset.hpp"

#include "t_game.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/limits.hpp"
#include "game/playerlist.hpp"

using game::PlayerList;
using game::PlayerSet_t;

/** Test formatPlayerSet(). */
void
TestGamePlayerSet::testFormat()
{
    afl::string::NullTranslator tx;

    // Base set has multiple players
    {
        PlayerList a;
        a.create(1);
        a.create(2);
        a.create(3);
        a.create(4);

        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t(),                     a, tx), "nobody");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t()                 + 5, a, tx), "nobody");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t() + 1,                 a, tx), "player 1");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t() + 1 + 2 + 3 + 4,     a, tx), "");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t() + 1 + 2 + 3 + 4 + 5, a, tx), "");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t() + 1             + 5, a, tx), "player 1");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t() + 1 + 2     + 4,     a, tx), "all but player 3");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t() + 1 + 2     + 4 + 5, a, tx), "all but player 3");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t() + 1         + 4,     a, tx), "players 1, 4");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t() + 1         + 4 + 5, a, tx), "players 1, 4");
    }

    // Empty base set.
    {
        PlayerList a;
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t(),                     a, tx), "");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t() + 1,                 a, tx), "");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t() + 1 + 2 + 3 + 4,     a, tx), "");
    }
    {
        PlayerList a;
        a.create(3);
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t(),                     a, tx), "nobody");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t() + 1,                 a, tx), "nobody");
        TS_ASSERT_EQUALS(game::formatPlayerSet(PlayerSet_t() + 1 + 2 + 3 + 4,     a, tx), "");
    }
}

/** Test formatPlayerHostSet(). */
void
TestGamePlayerSet::testFormatPlayerHostSet()
{
    afl::string::NullTranslator tx;

    // Base set has multiple players
    {
        PlayerList a;
        a.create(1);
        a.create(2);
        a.create(3);
        a.create(4);

        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t(),                         a, tx), "nobody");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t() + 0,                     a, tx), "host");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t()                     + 5, a, tx), "nobody");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t()     + 1,                 a, tx), "player 1");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t() + 0 + 1,                 a, tx), "host, player 1");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t() + 0 + 1 + 2 + 3 + 4,     a, tx), "host, all players");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t()     + 1 + 2 + 3 + 4,     a, tx), "all players");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t()     + 1 + 2 + 3 + 4 + 5, a, tx), "all players");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t()     + 1             + 5, a, tx), "player 1");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t()     + 1 + 2     + 4,     a, tx), "all but player 3");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t() + 0 + 1 + 2     + 4,     a, tx), "host, all but player 3");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t()     + 1 + 2     + 4 + 5, a, tx), "all but player 3");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t()     + 1         + 4,     a, tx), "players 1, 4");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t() + 0 + 1         + 4,     a, tx), "host, players 1, 4");
        TS_ASSERT_EQUALS(game::formatPlayerHostSet(PlayerSet_t()     + 1         + 4 + 5, a, tx), "players 1, 4");
    }
}

/** Test formatPlayerSetAsList(). */
void
TestGamePlayerSet::testFormatList()
{
    TS_ASSERT_EQUALS(game::MAX_PLAYERS, 31);

    TS_ASSERT_EQUALS(game::formatPlayerSetAsList(PlayerSet_t()),                      "");
    TS_ASSERT_EQUALS(game::formatPlayerSetAsList(PlayerSet_t() + 0),                  "0");
    TS_ASSERT_EQUALS(game::formatPlayerSetAsList(PlayerSet_t() + 31),                 "31");
    TS_ASSERT_EQUALS(game::formatPlayerSetAsList(PlayerSet_t() + 1 + 2 + 3 + 5 + 11), "1 2 3 5 11");
}


/** Test parsePlayerListAsSet(). */
void
TestGamePlayerSet::testParseList()
{
    TS_ASSERT_EQUALS(game::MAX_PLAYERS, 31);

    // Good cases
    TS_ASSERT_EQUALS(game::parsePlayerListAsSet(""),           PlayerSet_t());
    TS_ASSERT_EQUALS(game::parsePlayerListAsSet("0"),          PlayerSet_t() + 0);
    TS_ASSERT_EQUALS(game::parsePlayerListAsSet("31"),         PlayerSet_t() + 31);
    TS_ASSERT_EQUALS(game::parsePlayerListAsSet("1 2 3 5 11"), PlayerSet_t() + 1 + 2 + 3 + 5 + 11);

    // "Bad" cases
    TS_ASSERT_EQUALS(game::parsePlayerListAsSet("nope"),       PlayerSet_t());
    TS_ASSERT_EQUALS(game::parsePlayerListAsSet("c00l"),       PlayerSet_t() + 0);
    TS_ASSERT_EQUALS(game::parsePlayerListAsSet("150"),        PlayerSet_t());
    TS_ASSERT_EQUALS(game::parsePlayerListAsSet("-3"),         PlayerSet_t());      // parsed as number -3, not as "-" + number 3
}

