/**
  *  \file u/t_server_talk_newsrc.cpp
  *  \brief Test for server::talk::Newsrc
  */

#include "server/talk/newsrc.hpp"

#include "t_server_talk.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/subtree.hpp"

/** Basic newsrc test. */
void
TestServerTalkNewsrc::testIt()
{
    // Set up
    afl::net::redis::InternalDatabase db;
    afl::net::redis::Subtree tree(db, "x:");
    server::talk::Newsrc testee(tree);

    // Initial state is everything unread
    const int32_t LINE = 8192;
    const int32_t MAX = 10*LINE;
    for (int32_t i = 0; i < MAX; ++i) {
        TS_ASSERT(!testee.get(i));
    }

    // Set every other post to read
    for (int32_t i = 0; i < MAX; i += 2) {
        testee.set(i);
    }
    testee.save();

    // At this point, there must be a few items in the hash
    TS_ASSERT(tree.hashKey("data").size() > 0);
    for (int32_t i = 0; i < MAX; i += 2) {
        TS_ASSERT(testee.get(i));
        TS_ASSERT(!testee.get(i+1));
    }

    // Set every post to read
    for (int32_t i = 0; i < MAX; ++i) {
        testee.set(i);
    }
    testee.save();

    // At this point, the hash must be empty
    TS_ASSERT_EQUALS(tree.hashKey("data").size(), 0);
    TS_ASSERT_EQUALS(tree.intKey("index").get(), 10);
    for (int32_t i = 0; i < MAX; ++i) {
        TS_ASSERT(testee.get(i));
    }
    for (int32_t i = MAX; i < MAX+1000; ++i) {
        TS_ASSERT(!testee.get(i));
    }

    // Clear something in the final page
    testee.clear(MAX-1);
    testee.save();

    // Hash must now contain one page
    TS_ASSERT_EQUALS(tree.hashKey("data").size(), 1);
    TS_ASSERT_EQUALS(tree.intKey("index").get(), 9);
    for (int32_t i = 0; i < MAX-1; ++i) {
        TS_ASSERT(testee.get(i));
    }
    for (int32_t i = MAX-1; i < MAX+1000; ++i) {
        TS_ASSERT(!testee.get(i));
    }
}

/** Similar test as above, but with backward operations.
    In particular, this triggers the "when completing the final page, we find more complete pages" case. */
void
TestServerTalkNewsrc::testBackward()
{
    // Set up
    afl::net::redis::InternalDatabase db;
    afl::net::redis::Subtree tree(db, "x:");
    server::talk::Newsrc testee(tree);

    // Initial state is everything unread
    const int32_t LINE = 8192;
    const int32_t MAX = 10*LINE;
    for (int32_t i = MAX; i > 0; --i) {
        TS_ASSERT(!testee.get(i-1));
    }

    // Set every other post to read
    for (int32_t i = MAX; i > 1; i -= 2) {
        testee.set(i-1);
    }
    testee.save();

    // At this point, there must be a few items in the hash
    TS_ASSERT(tree.hashKey("data").size() > 0);
    for (int32_t i = MAX; i > 1; i -= 2) {
        TS_ASSERT(testee.get(i-1));
        TS_ASSERT(!testee.get(i-2));
    }

    // Set every post to read
    for (int32_t i = MAX; i > 0; --i) {
        testee.set(i-1);
    }
    testee.save();

    // At this point, the hash must be empty
    TS_ASSERT_EQUALS(tree.hashKey("data").size(), 0);
    TS_ASSERT_EQUALS(tree.intKey("index").get(), 10);
    for (int32_t i = MAX; i > 0; --i) {
        TS_ASSERT(testee.get(i-1));
    }
    for (int32_t i = MAX; i < MAX+1000; ++i) {
        TS_ASSERT(!testee.get(i));
    }
}

