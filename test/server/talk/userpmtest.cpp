/**
  *  \file test/server/talk/userpmtest.cpp
  *  \brief Test for server::talk::UserPM
  */

#include "server/talk/userpm.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/sortoperation.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/root.hpp"

/** Basic test for UserPM. */
AFL_TEST("server.talk.UserPM:basics", a)
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Verify properties of a new message
    server::talk::UserPM testee(root, 1);
    a.checkEqual("01. getId", testee.getId(), 1);

    testee.author().set("a");
    a.checkEqual("11. author", testee.author().get(), "a");

    testee.receivers().set("r");
    a.checkEqual("21. receivers", testee.receivers().get(), "r");

    testee.time().set(9988);
    a.checkEqual("31. time", testee.time().get(), 9988);

    testee.subject().set("s");
    a.checkEqual("41. subject", testee.subject().get(), "s");

    testee.text().set("text:t");
    a.checkEqual("51. text", testee.text().get(), "text:t");

    // These fields need a known "uninitialized" value
    a.checkEqual("61. parentMessageId", testee.parentMessageId().get(), 0);
    testee.parentMessageId().set(9);
    a.checkEqual("62. parentMessageId", testee.parentMessageId().get(), 9);

    a.checkEqual("71. flags", testee.flags("1009").get(), 0);
    testee.flags("1009").set(3);
    a.checkEqual("72. flags", testee.flags("1009").get(), 3);

    a.checkEqual("81. referenceCounter", testee.referenceCounter().get(), 0);
    testee.addReference();
    a.checkEqual("82. referenceCounter", testee.referenceCounter().get(), 1);

    // Describe
    server::interface::TalkPM::Info info = testee.describe("1009", 1);
    a.checkEqual("91. author",          info.author, "a");
    a.checkEqual("92. receivers",       info.receivers, "r");
    a.checkEqual("93. time",            info.time, 9988);
    a.checkEqual("94. subject",         info.subject, "s");
    a.checkEqual("95. flags",           info.flags, 3);
    a.checkEqual("96. parent",          info.parent.orElse(-1), 9);
    a.checkEqual("97. parentFolder",    info.parentFolder.isValid(), false);
    a.checkEqual("98. suggestedFolder", info.suggestedFolder.isValid(), false);

    // Describe for another user
    info = testee.describe("1010", 0);
    a.checkEqual("101. author",    info.author, "a");
    a.checkEqual("102. receivers", info.receivers, "r");
    a.checkEqual("103. time",      info.time, 9988);
    a.checkEqual("104. subject",   info.subject, "s");
    a.checkEqual("105. flags",     info.flags, 0);        // <-- difference!
    a.checkEqual("106. parent",    info.parent.orElse(-1), 9);

    // Remove reference. This makes everything go away
    testee.removeReference();
    a.checkEqual("111. referenceCounter", testee.referenceCounter().get(), 0);
    a.check("112. header", !testee.header().exists());
    a.check("113. text", !testee.text().exists());
}

/** Test allocatePM. */
AFL_TEST("server.talk.UserPM:allocatePM", a)
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Test
    int32_t aa = server::talk::UserPM::allocatePM(root);
    int32_t bb = server::talk::UserPM::allocatePM(root);
    a.check("01", aa != 0);
    a.check("02", bb != 0);
    a.check("03", aa != bb);
}

/** Test sorting. */
AFL_TEST("server.talk.UserPM:sort", a)
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
        a.checkEqual("01. size", result.size(), 3U);
        a.checkEqual("02. result", result[0], 200);
    }
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::UserPM::PMSorter(root).applySortKey(op, "SUBJECT");
        op.getResult(result);
        a.checkEqual("03. size", result.size(), 3U);
        a.checkEqual("04. result", result[0], 201);
    }
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::UserPM::PMSorter(root).applySortKey(op, "TIME");
        op.getResult(result);
        a.checkEqual("05. size", result.size(), 3U);
        a.checkEqual("06. result", result[0], 202);
    }
    {
        SortOperation op(key.sort());
        AFL_CHECK_THROWS(a("07. bad key"), server::talk::UserPM::PMSorter(root).applySortKey(op, ""), std::exception);
        AFL_CHECK_THROWS(a("08. bad key"), server::talk::UserPM::PMSorter(root).applySortKey(op, "time"), std::exception);
        AFL_CHECK_THROWS(a("09. bad key"), server::talk::UserPM::PMSorter(root).applySortKey(op, "HUH"), std::exception);
    }
}
