/**
  *  \file u/t_game_authcache.cpp
  *  \brief Test for game::AuthCache
  */

#include "game/authcache.hpp"

#include "t_game.hpp"

using game::AuthCache;

/** Simple test. */
void
TestGameAuthCache::testIt()
{
    // Setup
    AuthCache testee;
    AuthCache::Item* p = new AuthCache::Item();
    p->playerNr = 7;
    p->password = "xyzzy";
    testee.addNew(p);

    // Query
    AuthCache::Item q;
    q.playerNr = 7;
    AuthCache::Items_t result = testee.find(q);

    TS_ASSERT_EQUALS(result.size(), 1U);
    TS_ASSERT_EQUALS(*result[0]->password.get(), "xyzzy");
}

/** Test with multiple results. */
void
TestGameAuthCache::testMulti()
{
    // Setup
    game::AuthCache testee;

    AuthCache::Item* p1 = new AuthCache::Item();
    p1->playerNr = 7;
    p1->password = "xyzzy";
    testee.addNew(p1);

    AuthCache::Item* p2 = new AuthCache::Item();
    p2->playerNr = 3;
    p2->password = "sesame";
    testee.addNew(p2);

    AuthCache::Item* p3 = new AuthCache::Item();
    p3->password = "1234";
    testee.addNew(p3);

    // Query
    AuthCache::Item q;
    q.playerNr = 3;
    AuthCache::Items_t result = testee.find(q);

    TS_ASSERT_EQUALS(result.size(), 2U);
    TS_ASSERT_EQUALS(*result[0]->password.get(), "sesame");
    TS_ASSERT_EQUALS(*result[1]->password.get(), "1234");
}

/** Test match failure. */
void
TestGameAuthCache::testFail()
{
    // Setup
    game::AuthCache testee;
    AuthCache::Item* p = new AuthCache::Item();
    p->playerNr = 7;
    p->password = "xyzzy";
    testee.addNew(p);

    // Query
    AuthCache::Item q;
    q.playerNr = 3;
    AuthCache::Items_t result = testee.find(q);

    TS_ASSERT_EQUALS(result.size(), 0U);
}

