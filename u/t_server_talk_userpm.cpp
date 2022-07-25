/**
  *  \file u/t_server_talk_userpm.cpp
  *  \brief Test for server::talk::UserPM
  */

#include "server/talk/userpm.hpp"

#include "t_server_talk.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/sortoperation.hpp"
#include "server/talk/root.hpp"

/** Basic test for UserPM. */
void
TestServerTalkUserPM::testIt()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Verify properties of a new message
    server::talk::UserPM testee(root, 1);
    TS_ASSERT_EQUALS(testee.getId(), 1);

    testee.author().set("a");
    TS_ASSERT_EQUALS(testee.author().get(), "a");

    testee.receivers().set("r");
    TS_ASSERT_EQUALS(testee.receivers().get(), "r");

    testee.time().set(9988);
    TS_ASSERT_EQUALS(testee.time().get(), 9988);

    testee.subject().set("s");
    TS_ASSERT_EQUALS(testee.subject().get(), "s");

    testee.text().set("text:t");
    TS_ASSERT_EQUALS(testee.text().get(), "text:t");

    // These fields need a known "uninitialized" value
    TS_ASSERT_EQUALS(testee.parentMessageId().get(), 0);
    testee.parentMessageId().set(9);
    TS_ASSERT_EQUALS(testee.parentMessageId().get(), 9);

    TS_ASSERT_EQUALS(testee.flags("1009").get(), 0);
    testee.flags("1009").set(3);
    TS_ASSERT_EQUALS(testee.flags("1009").get(), 3);

    TS_ASSERT_EQUALS(testee.referenceCounter().get(), 0);
    testee.addReference();
    TS_ASSERT_EQUALS(testee.referenceCounter().get(), 1);

    // Describe
    server::interface::TalkPM::Info info = testee.describe("1009", 1);
    TS_ASSERT_EQUALS(info.author, "a");
    TS_ASSERT_EQUALS(info.receivers, "r");
    TS_ASSERT_EQUALS(info.time, 9988);
    TS_ASSERT_EQUALS(info.subject, "s");
    TS_ASSERT_EQUALS(info.flags, 3);
    TS_ASSERT_EQUALS(info.parent.orElse(-1), 9);
    TS_ASSERT_EQUALS(info.parentFolder.isValid(), false);
    TS_ASSERT_EQUALS(info.suggestedFolder.isValid(), false);

    // Describe for another user
    info = testee.describe("1010", 0);
    TS_ASSERT_EQUALS(info.author, "a");
    TS_ASSERT_EQUALS(info.receivers, "r");
    TS_ASSERT_EQUALS(info.time, 9988);
    TS_ASSERT_EQUALS(info.subject, "s");
    TS_ASSERT_EQUALS(info.flags, 0);        // <-- difference!
    TS_ASSERT_EQUALS(info.parent.orElse(-1), 9);

    // Remove reference. This makes everything go away
    testee.removeReference();
    TS_ASSERT_EQUALS(testee.referenceCounter().get(), 0);
    TS_ASSERT(!testee.header().exists());
    TS_ASSERT(!testee.text().exists());
}

/** Test allocatePM. */
void
TestServerTalkUserPM::testAllocate()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Test
    int32_t a = server::talk::UserPM::allocatePM(root);
    int32_t b = server::talk::UserPM::allocatePM(root);
    TS_ASSERT(a != 0);
    TS_ASSERT(b != 0);
    TS_ASSERT(a != b);
}

/** Test sorting. */
void
TestServerTalkUserPM::testSort()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Preload database
    static const int N = 3;
    struct Data {
        const char* author;
        const char* subject;
        int time;
    };
    static const Data d[N] = {
        { "a",    "subj",     12000 },      // #200, first author
        { "c",    "a text",   50000 },      // #201, first subject
        { "b",    "reply",     9000 },      // #202, first time
    };
    afl::net::redis::IntegerSetKey key(db, "list_key");
    for (int i = 0; i < N; ++i) {
        const int id = i + 200;
        server::talk::UserPM pm(root, id);
        pm.author().set(d[i].author);
        pm.subject().set(d[i].subject);
        pm.time().set(d[i].time);
        key.add(id);
    }

    // Test it
    using afl::net::redis::SortOperation;
    using afl::data::IntegerList_t;
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::UserPM::PMSorter(root).applySortKey(op, "AUTHOR");
        op.getResult(result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 200);
    }
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::UserPM::PMSorter(root).applySortKey(op, "SUBJECT");
        op.getResult(result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 201);
    }
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::UserPM::PMSorter(root).applySortKey(op, "TIME");
        op.getResult(result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 202);
    }
    {
        SortOperation op(key.sort());
        TS_ASSERT_THROWS(server::talk::UserPM::PMSorter(root).applySortKey(op, ""), std::exception);
        TS_ASSERT_THROWS(server::talk::UserPM::PMSorter(root).applySortKey(op, "time"), std::exception);
        TS_ASSERT_THROWS(server::talk::UserPM::PMSorter(root).applySortKey(op, "HUH"), std::exception);
    }
}
