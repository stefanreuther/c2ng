/**
  *  \file test/game/authcachetest.cpp
  *  \brief Test for game::AuthCache
  */

#include "game/authcache.hpp"
#include "afl/test/testrunner.hpp"

using game::AuthCache;

/** Simple test. */
AFL_TEST("game.AuthCache:basics", a)
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

    a.checkEqual("01. size", result.size(), 1U);
    a.checkEqual("02. password", *result[0]->password.get(), "xyzzy");
}

/** Test with multiple results. */
AFL_TEST("game.AuthCache:multiple-results", a)
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

    a.checkEqual("01. size", result.size(), 2U);
    a.checkEqual("02. password", *result[0]->password.get(), "sesame");
    a.checkEqual("03. password", *result[1]->password.get(), "1234");
}

/** Test match failure. */
AFL_TEST("game.AuthCache:fail", a)
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

    a.checkEqual("01. size", result.size(), 0U);
}

/** Test clear(). */
AFL_TEST("game.AuthCache:clear", a)
{
    // Setup
    AuthCache testee;
    AuthCache::Item* p = new AuthCache::Item();
    p->playerNr = 7;
    p->password = "xyzzy";
    testee.addNew(p);

    // Query
    {
        AuthCache::Item q;
        q.playerNr = 7;
        AuthCache::Items_t result = testee.find(q);
        a.checkEqual("01. size", result.size(), 1U);
    }

    // Clear
    testee.clear();
    {
        AuthCache::Item q;
        q.playerNr = 7;
        AuthCache::Items_t result = testee.find(q);
        a.checkEqual("11. size", result.size(), 0U);
    }
}
