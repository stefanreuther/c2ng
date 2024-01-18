/**
  *  \file test/server/talk/newsrctest.cpp
  *  \brief Test for server::talk::Newsrc
  */

#include "server/talk/newsrc.hpp"

#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/test/testrunner.hpp"

/** Basic newsrc test. */
AFL_TEST("server.talk.Newsrc:basics", a)
{
    // Set up
    afl::net::redis::InternalDatabase db;
    afl::net::redis::Subtree tree(db, "x:");
    server::talk::Newsrc testee(tree);

    // Initial state is everything unread
    const int32_t LINE = 8192;
    const int32_t MAX = 10*LINE;
    for (int32_t i = 0; i < MAX; ++i) {
        a.check("01", !testee.get(i));
    }

    // Set every other post to read
    for (int32_t i = 0; i < MAX; i += 2) {
        testee.set(i);
    }
    testee.save();

    // At this point, there must be a few items in the hash
    a.check("11", tree.hashKey("data").size() > 0);
    for (int32_t i = 0; i < MAX; i += 2) {
        a.check("12", testee.get(i));
        a.check("13", !testee.get(i+1));
    }

    // Set every post to read
    for (int32_t i = 0; i < MAX; ++i) {
        testee.set(i);
    }
    testee.save();

    // At this point, the hash must be empty
    a.checkEqual("21", tree.hashKey("data").size(), 0);
    a.checkEqual("22", tree.intKey("index").get(), 10);
    for (int32_t i = 0; i < MAX; ++i) {
        a.check("23", testee.get(i));
    }
    for (int32_t i = MAX; i < MAX+1000; ++i) {
        a.check("24", !testee.get(i));
    }

    // Clear something in the final page
    testee.clear(MAX-1);
    testee.save();

    // Hash must now contain one page
    a.checkEqual("31", tree.hashKey("data").size(), 1);
    a.checkEqual("32", tree.intKey("index").get(), 9);
    for (int32_t i = 0; i < MAX-1; ++i) {
        a.check("33", testee.get(i));
    }
    for (int32_t i = MAX-1; i < MAX+1000; ++i) {
        a.check("34", !testee.get(i));
    }
}

/** Similar test as above, but with backward operations.
    In particular, this triggers the "when completing the final page, we find more complete pages" case. */
AFL_TEST("server.talk.Newsrc:backward", a)
{
    // Set up
    afl::net::redis::InternalDatabase db;
    afl::net::redis::Subtree tree(db, "x:");
    server::talk::Newsrc testee(tree);

    // Initial state is everything unread
    const int32_t LINE = 8192;
    const int32_t MAX = 10*LINE;
    for (int32_t i = MAX; i > 0; --i) {
        a.check("01", !testee.get(i-1));
    }

    // Set every other post to read
    for (int32_t i = MAX; i > 1; i -= 2) {
        testee.set(i-1);
    }
    testee.save();

    // At this point, there must be a few items in the hash
    a.check("11", tree.hashKey("data").size() > 0);
    for (int32_t i = MAX; i > 1; i -= 2) {
        a.check("12", testee.get(i-1));
        a.check("13", !testee.get(i-2));
    }

    // Set every post to read
    for (int32_t i = MAX; i > 0; --i) {
        testee.set(i-1);
    }
    testee.save();

    // At this point, the hash must be empty
    a.checkEqual("21", tree.hashKey("data").size(), 0);
    a.checkEqual("22", tree.intKey("index").get(), 10);
    for (int32_t i = MAX; i > 0; --i) {
        a.check("23", testee.get(i-1));
    }
    for (int32_t i = MAX; i < MAX+1000; ++i) {
        a.check("24", !testee.get(i));
    }
}
