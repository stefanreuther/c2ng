/**
  *  \file u/t_server_talk_message.cpp
  *  \brief Test for server::talk::Message
  */

#include "server/talk/message.hpp"

#include "t_server_talk.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/talk/root.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/user.hpp"

/** Simple test. */
void
TestServerTalkMessage::testIt()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Message
    server::talk::Message testee(root, 98);
    TS_ASSERT(!testee.header().exists());
    TS_ASSERT_EQUALS(testee.getId(), 98);

    // Create it by writing header fields
    testee.topicId().set(55);
    testee.parentMessageId().set(97);
    testee.postTime().set(556677);
    testee.editTime().set(556688);
    testee.author().set("1200");
    testee.subject().set("s");

    TS_ASSERT_EQUALS(testee.topicId().get(), 55);
    TS_ASSERT_EQUALS(testee.parentMessageId().get(), 97);
    TS_ASSERT_EQUALS(testee.postTime().get(), 556677);
    TS_ASSERT_EQUALS(testee.editTime().get(), 556688);
    TS_ASSERT_EQUALS(testee.author().get(), "1200");
    TS_ASSERT_EQUALS(testee.subject().get(), "s");

    TS_ASSERT(testee.exists());

    // NNTP stuff
    testee.rfcMessageId().set("a@b");
    testee.rfcHeaders().set("h: v");
    testee.sequenceNumber().set(33);
    testee.previousSequenceNumber().set(31);
    testee.previousRfcMessageId().set("a@a");

    TS_ASSERT_EQUALS(testee.rfcMessageId().get(), "a@b");
    TS_ASSERT_EQUALS(testee.rfcHeaders().get(), "h: v");
    TS_ASSERT_EQUALS(testee.sequenceNumber().get(), 33);
    TS_ASSERT_EQUALS(testee.previousSequenceNumber().get(), 31);
    TS_ASSERT_EQUALS(testee.previousRfcMessageId().get(), "a@a");

    // Text
    testee.text().set("forum:hi mom");
    TS_ASSERT_EQUALS(testee.text().get(), "forum:hi mom");

    // Description
    server::interface::TalkPost::Info info = testee.describe(root);
    TS_ASSERT_EQUALS(info.threadId, 55);
    TS_ASSERT_EQUALS(info.parentPostId, 97);
    TS_ASSERT_EQUALS(info.postTime, 556677);
    TS_ASSERT_EQUALS(info.editTime, 556688);
    TS_ASSERT_EQUALS(info.author, "1200");
    TS_ASSERT_EQUALS(info.subject, "s");
    TS_ASSERT_EQUALS(info.rfcMessageId, "a@b");

    // RfC header
    TS_ASSERT_EQUALS(server::toString(testee.getRfcHeader(root)->get("Date")), "Fri, 22 Jan 1971 14:08:00 +0000");
    TS_ASSERT_EQUALS(server::toInteger(testee.getRfcHeader(root)->get(":Bytes")), 12);
}

/** Test Message-Id behaviour. */
void
TestServerTalkMessage::testMessageIds()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Configuration config;
    config.messageIdSuffix = "@suf";
    server::talk::Root root(db, mq, config);

    // Database content
    // - message that was created on the web side and never edited
    {
        server::talk::Message m(root, 50);
        m.subject().set("0");
        m.sequenceNumber().set(3);
        TS_ASSERT_EQUALS(m.getRfcMessageId(root), "50.3@suf");
        TS_ASSERT_EQUALS(m.getPreviousRfcMessageId(root), "");
        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("Message-Id")), "<50.3@suf>");
        TS_ASSERT(m.getRfcHeader(root)->get("Supersedes") == 0);
    }

    // - message that was created on the NNTP side and never edited
    {
        server::talk::Message m(root, 51);
        m.subject().set("1");
        m.sequenceNumber().set(4);
        m.rfcMessageId().set("m1@host");
        server::talk::Message::addRfcMessageId(root, "m1@host", 51);
        TS_ASSERT_EQUALS(m.getRfcMessageId(root), "m1@host");
        TS_ASSERT_EQUALS(m.getPreviousRfcMessageId(root), "");
        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("Message-Id")), "<m1@host>");
        TS_ASSERT(m.getRfcHeader(root)->get("Supersedes") == 0);
    }

    // - message that was created on the web side and edited on the NNTP side
    {
        server::talk::Message m(root, 52);
        m.subject().set("10");
        m.previousSequenceNumber().set(5);
        m.sequenceNumber().set(6);
        m.rfcMessageId().set("m10@host");
        server::talk::Message::addRfcMessageId(root, "m10@host", 52);
        TS_ASSERT_EQUALS(m.getRfcMessageId(root), "m10@host");
        TS_ASSERT_EQUALS(m.getPreviousRfcMessageId(root), "52.5@suf");
        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("Message-Id")), "<m10@host>");
        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("Supersedes")), "<52.5@suf>");
    }

    // - message that was created and edited on the web side
    {
        server::talk::Message m(root, 53);
        m.subject().set("00");
        m.previousSequenceNumber().set(7);
        m.sequenceNumber().set(8);
        TS_ASSERT_EQUALS(m.getRfcMessageId(root), "53.8@suf");
        TS_ASSERT_EQUALS(m.getPreviousRfcMessageId(root), "53.7@suf");
        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("Message-Id")), "<53.8@suf>");
        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("Supersedes")), "<53.7@suf>");
    }

    // - message that was created on the NNTP side and edited on the web side
    {
        server::talk::Message m(root, 54);
        m.subject().set("01");
        m.previousSequenceNumber().set(9);
        m.sequenceNumber().set(10);
        m.previousRfcMessageId().set("m01@host");
        TS_ASSERT_EQUALS(m.getRfcMessageId(root), "54.10@suf");
        TS_ASSERT_EQUALS(m.getPreviousRfcMessageId(root), "m01@host");
        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("Message-Id")), "<54.10@suf>");
        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("Supersedes")), "<m01@host>");
    }

    // - message that was created and edited on the NNTP side
    {
        server::talk::Message m(root, 55);
        m.subject().set("11");
        m.previousSequenceNumber().set(11);
        m.sequenceNumber().set(12);
        m.rfcMessageId().set("m11n@host");
        m.previousRfcMessageId().set("m11o@host");
        server::talk::Message::addRfcMessageId(root, "m11n@host", 55);
        TS_ASSERT_EQUALS(m.getRfcMessageId(root), "m11n@host");
        TS_ASSERT_EQUALS(m.getPreviousRfcMessageId(root), "m11o@host");
        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("Message-Id")), "<m11n@host>");
        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("Supersedes")), "<m11o@host>");
    }

    // Resolve message Ids
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "50.3@suf"),  50);
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "m1@host"),   51);
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "m10@host"),  52);
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "53.8@suf"),  53);
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "54.10@suf"), 54);
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "m11n@host"), 55);

    // Failure cases
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, ""),          0);
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "what@ever"), 0);
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "50.2@suf"),  0);
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "51.4@suf"),  0);
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "52.5@suf"),  0);
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "53.7@suf"),  0);
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "m01@host"),  0);
    TS_ASSERT_EQUALS(server::talk::Message::lookupRfcMessageId(root, "55.12@suf"), 0);
}

/** Test behaviour of email addresses in messages. */
void
TestServerTalkMessage::testEmail()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Configuration config;
    config.messageIdSuffix = "@suf";
    server::talk::Root root(db, mq, config);

    // Confirmed, enabled email, screen name only
    {
        // User
        server::talk::User u(root, "1001");
        u.profile().stringField("email").set("a@b");
        u.profile().intField("infoemailflag").set(1);
        u.profile().stringField("screenname").set("ozzi");
        root.userRoot().subtree("1001").stringKey("name").set("oz");
        root.emailRoot().subtree("a@b").hashKey("status").stringField("status/1001").set("c");

        // Message
        server::talk::Message m(root, 1);
        m.author().set("1001");

        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("From")), "ozzi <a@b>");
    }

    // Unconfirmed, enabled email, screen name only
    {
        // User
        server::talk::User u(root, "1002");
        u.profile().stringField("email").set("a@b");
        u.profile().intField("infoemailflag").set(1);
        u.profile().stringField("screenname").set("azzi");
        root.userRoot().subtree("1002").stringKey("name").set("az");
        // no root.emailRoot().subtree("a@b").hashKey("status").stringField("status/1002").set("c");

        // Message
        server::talk::Message m(root, 2);
        m.author().set("1002");

        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("From")), "azzi <az@invalid.invalid>");
    }

    // Confirmed, disabled email, screen name only
    {
        // User
        server::talk::User u(root, "1003");
        u.profile().stringField("email").set("a@b");
        u.profile().intField("infoemailflag").set(0);
        u.profile().stringField("screenname").set("uzzi");
        root.userRoot().subtree("1003").stringKey("name").set("uz");
        root.emailRoot().subtree("a@b").hashKey("status").stringField("status/1003").set("c");

        // Message
        server::talk::Message m(root, 3);
        m.author().set("1003");

        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("From")), "uzzi <uz@invalid.invalid>");
    }

    // Confirmed, enabled email, disabled real name
    {
        // User
        server::talk::User u(root, "1004");
        u.profile().stringField("email").set("a@b");
        u.profile().intField("infoemailflag").set(1);
        u.profile().stringField("screenname").set("yzzi");
        u.profile().stringField("realname").set("Y. Zzi");
        root.userRoot().subtree("1004").stringKey("name").set("yz");
        root.emailRoot().subtree("a@b").hashKey("status").stringField("status/1004").set("c");

        // Message
        server::talk::Message m(root, 4);
        m.author().set("1004");

        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("From")), "yzzi <a@b>");
    }

    // Confirmed, enabled email, enabled real name
    {
        // User
        server::talk::User u(root, "1005");
        u.profile().stringField("email").set("a@b");
        u.profile().intField("infoemailflag").set(1);
        u.profile().intField("inforealnameflag").set(1);
        u.profile().stringField("screenname").set("ezzi");
        u.profile().stringField("realname").set("E. Zzi");
        root.userRoot().subtree("1005").stringKey("name").set("ez");
        root.emailRoot().subtree("a@b").hashKey("status").stringField("status/1005").set("c");

        // Message
        server::talk::Message m(root, 5);
        m.author().set("1005");

        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("From")), "E. Zzi <a@b>");
    }

    // Confirmed, enabled email, enabled real name, with Unicode
    {
        // User
        server::talk::User u(root, "1006");
        u.profile().stringField("email").set("a@b");
        u.profile().intField("infoemailflag").set(1);
        u.profile().stringField("screenname").set("oezzi");
        u.profile().stringField("realname").set("\xc3\x96. Zzi");
        root.userRoot().subtree("1006").stringKey("name").set("oez");
        root.emailRoot().subtree("a@b").hashKey("status").stringField("status/1006").set("c");

        // Message
        server::talk::Message m(root, 6);
        m.author().set("1006");

        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("From")), "oezzi <a@b>");
    }

    // Confirmed, disabled email, enabled real name
    {
        // User
        server::talk::User u(root, "1007");
        u.profile().stringField("email").set("a@b");
        u.profile().intField("inforealnameflag").set(1);
        u.profile().stringField("screenname").set("izzi");
        u.profile().stringField("realname").set("I. Zzi");
        root.userRoot().subtree("1007").stringKey("name").set("iz");
        root.emailRoot().subtree("a@b").hashKey("status").stringField("status/1007").set("c");

        // Message
        server::talk::Message m(root, 6);
        m.author().set("1007");

        TS_ASSERT_EQUALS(server::toString(m.getRfcHeader(root)->get("From")), "I. Zzi <iz@invalid.invalid>");
    }
}

/** Test behaviour of parent messages/references in messages. */
void
TestServerTalkMessage::testParent()
{
    using server::talk::Message;

    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Configuration config;
    config.messageIdSuffix = "@suf";
    server::talk::Root root(db, mq, config);

    // Database: 20 messages, each referring to their parent, in a thread
    server::talk::Topic t(root, 42);
    t.firstPostingId().set(1);
    for (int i = 1; i <= 20; ++i) {
        Message m(root, i);
        m.subject().set("a");
        m.sequenceNumber().set(i);
        m.topicId().set(42);
        t.messages().add(i);
        if (i != 0) {
            m.parentMessageId().set(i-1);
        }
    }

    // Test Message-Id header for reference
    TS_ASSERT_EQUALS(server::toString(Message(root, 1).getRfcHeader(root)->get("Message-Id")), "<1.1@suf>");
    TS_ASSERT_EQUALS(server::toString(Message(root, 20).getRfcHeader(root)->get("Message-Id")), "<20.20@suf>");

    // Test References
    // - up to 5 parents
    TS_ASSERT(Message(root, 1).getRfcHeader(root)->get("References") == 0);
    TS_ASSERT_EQUALS(server::toString(Message(root, 2).getRfcHeader(root)->get("References")), "<1.1@suf>");
    TS_ASSERT_EQUALS(server::toString(Message(root, 3).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <2.2@suf>");
    TS_ASSERT_EQUALS(server::toString(Message(root, 4).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <2.2@suf>\r\n <3.3@suf>");
    TS_ASSERT_EQUALS(server::toString(Message(root, 5).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <2.2@suf>\r\n <3.3@suf>\r\n <4.4@suf>");
    TS_ASSERT_EQUALS(server::toString(Message(root, 6).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <2.2@suf>\r\n <3.3@suf>\r\n <4.4@suf>\r\n <5.5@suf>");
    // - now we start with 5 parents + root
    TS_ASSERT_EQUALS(server::toString(Message(root, 7).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <2.2@suf>\r\n <3.3@suf>\r\n <4.4@suf>\r\n <5.5@suf>\r\n <6.6@suf>");
    TS_ASSERT_EQUALS(server::toString(Message(root, 8).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <3.3@suf>\r\n <4.4@suf>\r\n <5.5@suf>\r\n <6.6@suf>\r\n <7.7@suf>");
    TS_ASSERT_EQUALS(server::toString(Message(root, 20).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <15.15@suf>\r\n <16.16@suf>\r\n <17.17@suf>\r\n <18.18@suf>\r\n <19.19@suf>");
}

/** Test sorting functions. */
void
TestServerTalkMessage::testSort()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Database
    struct Data {
        int sequenceNumber;
        int editTime;
        int postTime;
        int thread;
        const char* author;
        const char* subject;
    };
    static const int N = 6;
    static const Data d[N] = {
        {  3,    8000,   10000,  10, "fred",      "subj" },        // #100, lowest sequence number
        {  4,       0,   10001,  11, "barney",    "whatever" },    // #101, lowest editTime
        { 10,      10,    9000,  12, "wilma",     "more" },        // #102, lowest postTime
        {  6,      20,    9500,   8, "pebbles",   "other" },       // #103, lowest thread
        {  7,    5000,   12000,  20, "bamm bamm", "bam" },         // #104, lowest author
        {  8,    4000,   15000,  13, "betty",     "aaa" },         // #105, lowest subject
    };

    afl::net::redis::IntegerSetKey allMessages(db, "test_key");
    for (int i = 0; i < N; ++i) {
        int msgId = i+100;
        server::talk::Message m(root, msgId);
        m.sequenceNumber().set(d[i].sequenceNumber);
        m.editTime().set(d[i].editTime);
        m.postTime().set(d[i].postTime);
        m.topicId().set(d[i].thread);
        m.author().set(d[i].author);
        m.subject().set(d[i].subject);
        allMessages.add(msgId);
    }

    // Test
    using afl::data::IntegerList_t;
    using afl::net::redis::SortOperation;
    using server::talk::Message;
    {
        // applySortBySequence
        IntegerList_t result;
        SortOperation op(allMessages.sort());
        Message::applySortBySequence(root, op);
        op.getResult(result);

        TS_ASSERT_EQUALS(result.size(), 6U);
        TS_ASSERT_EQUALS(result[0], 100);
        TS_ASSERT_EQUALS(result[1], 101);
        TS_ASSERT_EQUALS(result[2], 103);
        TS_ASSERT_EQUALS(result[3], 104);
        TS_ASSERT_EQUALS(result[4], 105);
        TS_ASSERT_EQUALS(result[5], 102);
    }
    {
        // applySortBySequenceMap
        IntegerList_t result;
        SortOperation op(allMessages.sort());
        Message::applySortBySequenceMap(root, op);
        op.getResult(result);

        TS_ASSERT_EQUALS(result.size(), 12U);
        TS_ASSERT_EQUALS(result[0],  3);
        TS_ASSERT_EQUALS(result[1],  100);
        TS_ASSERT_EQUALS(result[2],  4);
        TS_ASSERT_EQUALS(result[3],  101);
        TS_ASSERT_EQUALS(result[4],  6);
        TS_ASSERT_EQUALS(result[5],  103);
        TS_ASSERT_EQUALS(result[6],  7);
        TS_ASSERT_EQUALS(result[7],  104);
        TS_ASSERT_EQUALS(result[8],  8);
        TS_ASSERT_EQUALS(result[9],  105);
        TS_ASSERT_EQUALS(result[10], 10);
        TS_ASSERT_EQUALS(result[11], 102);
    }
    {
        // MessageSorter, author
        IntegerList_t result;
        SortOperation op(allMessages.sort());
        Message::MessageSorter(root).applySortKey(op, "AUTHOR");
        op.getResult(result);

        TS_ASSERT_EQUALS(result.size(), 6U);
        TS_ASSERT_EQUALS(result[0], 104);
    }
    {
        // MessageSorter, edittime
        IntegerList_t result;
        SortOperation op(allMessages.sort());
        Message::MessageSorter(root).applySortKey(op, "EDITTIME");
        op.getResult(result);

        TS_ASSERT_EQUALS(result.size(), 6U);
        TS_ASSERT_EQUALS(result[0], 101);
    }
    {
        // MessageSorter, edittime
        IntegerList_t result;
        SortOperation op(allMessages.sort());
        Message::MessageSorter(root).applySortKey(op, "SUBJECT");
        op.getResult(result);

        TS_ASSERT_EQUALS(result.size(), 6U);
        TS_ASSERT_EQUALS(result[0], 105);
    }
    {
        // MessageSorter, thread
        IntegerList_t result;
        SortOperation op(allMessages.sort());
        Message::MessageSorter(root).applySortKey(op, "THREAD");
        op.getResult(result);

        TS_ASSERT_EQUALS(result.size(), 6U);
        TS_ASSERT_EQUALS(result[0], 103);
    }
    {
        // MessageSorter, time
        IntegerList_t result;
        SortOperation op(allMessages.sort());
        Message::MessageSorter(root).applySortKey(op, "TIME");
        op.getResult(result);

        TS_ASSERT_EQUALS(result.size(), 6U);
        TS_ASSERT_EQUALS(result[0], 102);
    }
    {
        // MessageSorter, errors
        SortOperation op(allMessages.sort());
        TS_ASSERT_THROWS(Message::MessageSorter(root).applySortKey(op, "time"),  std::exception);
        TS_ASSERT_THROWS(Message::MessageSorter(root).applySortKey(op, "OTHER"), std::exception);
        TS_ASSERT_THROWS(Message::MessageSorter(root).applySortKey(op, ""),      std::exception);
    }
}

