/**
  *  \file u/t_game_playerset.cpp
  *  \brief Test for game::PlayerSet
  */

#include "game/playerset.hpp"

#include "t_game.hpp"
#include "game/playerlist.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test formatPlayerSet(). */
void
TestGamePlayerSet::testFormat()
{
    using game::PlayerList;
    using game::PlayerSet_t;
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
    using game::PlayerList;
    using game::PlayerSet_t;
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

