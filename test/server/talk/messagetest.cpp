/**
  *  \file test/server/talk/messagetest.cpp
  *  \brief Test for server::talk::Message
  */

#include "server/talk/message.hpp"

#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/root.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/user.hpp"

using afl::data::IntegerList_t;
using afl::net::redis::SortOperation;
using server::talk::Forum;
using server::talk::Topic;
using server::talk::Root;
using server::talk::Configuration;
using server::talk::Message;

namespace {
    // Create some messages
    void createMessages(Root& root, afl::net::redis::IntegerSetKey allMessages)
    {
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
        for (int i = 0; i < N; ++i) {
            int msgId = i+100;
            Message m(root, msgId);
            m.sequenceNumber().set(d[i].sequenceNumber);
            m.editTime().set(d[i].editTime);
            m.postTime().set(d[i].postTime);
            m.topicId().set(d[i].thread);
            m.author().set(d[i].author);
            m.subject().set(d[i].subject);
            allMessages.add(msgId);
        }
    }

    void createMessageChain(Root& root)
    {
        // Database: 20 messages, each referring to their parent, in a thread
        Topic t(root, 42);
        t.firstPostingId().set(1);
        for (int i = 1; i <= 20; ++i) {
            Message m(root, i);
            m.subject().set("a");
            m.sequenceNumber().set(i);
            m.topicId().set(42);
            t.messages().add(i);
            if (i != 1) {
                m.parentMessageId().set(i-1);
            }
        }
    }
}

/** Simple test. */
AFL_TEST("server.talk.Message:basics", a)
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    Root root(db, Configuration());

    // Message
    Message testee(root, 98);
    a.check("01. exists", !testee.header().exists());
    a.checkEqual("02. getId", testee.getId(), 98);

    // Create it by writing header fields
    testee.topicId().set(55);
    testee.parentMessageId().set(97);
    testee.postTime().set(556677);
    testee.editTime().set(556688);
    testee.author().set("1200");
    testee.subject().set("s");

    a.checkEqual("11. topicId",         testee.topicId().get(), 55);
    a.checkEqual("12. parentMessageId", testee.parentMessageId().get(), 97);
    a.checkEqual("13. postTime",        testee.postTime().get(), 556677);
    a.checkEqual("14. editTime",        testee.editTime().get(), 556688);
    a.checkEqual("15. author",          testee.author().get(), "1200");
    a.checkEqual("16. subject",         testee.subject().get(), "s");

    a.check("21. exists", testee.exists());

    // NNTP stuff
    testee.rfcMessageId().set("a@b");
    testee.rfcHeaders().set("h: v");
    testee.sequenceNumber().set(33);
    testee.previousSequenceNumber().set(31);
    testee.previousRfcMessageId().set("a@a");
    testee.sequenceNumberIn(3).set(55);
    testee.sequenceNumberIn(4).set(66);
    testee.previousSequenceNumberIn(3).set(50);
    testee.previousSequenceNumberIn(4).set(60);

    a.checkEqual("31. rfcMessageId",           testee.rfcMessageId().get(), "a@b");
    a.checkEqual("32. rfcHeaders",             testee.rfcHeaders().get(), "h: v");
    a.checkEqual("33. sequenceNumber",         testee.sequenceNumber().get(), 33);
    a.checkEqual("34. previousSequenceNumber", testee.previousSequenceNumber().get(), 31);
    a.checkEqual("35. previousRfcMessageId",   testee.previousRfcMessageId().get(), "a@a");
    a.checkEqual("36. sequenceNumberIn",       testee.sequenceNumberIn(3).get(), 55);
    a.checkEqual("37. sequenceNumberIn",       testee.sequenceNumberIn(4).get(), 66);
    a.checkEqual("38. previousSequenceNumberIn", testee.previousSequenceNumberIn(3).get(), 50);
    a.checkEqual("39. previousSequenceNumberIn", testee.previousSequenceNumberIn(4).get(), 60);

    // Text
    testee.text().set("forum:hi mom");
    a.checkEqual("41. text", testee.text().get(), "forum:hi mom");

    // Description
    server::interface::TalkPost::Info info = testee.describe(root);
    a.checkEqual("51. threadId",     info.threadId, 55);
    a.checkEqual("52. parentPostId", info.parentPostId, 97);
    a.checkEqual("53. postTime",     info.postTime, 556677);
    a.checkEqual("54. editTime",     info.editTime, 556688);
    a.checkEqual("55. author",       info.author, "1200");
    a.checkEqual("56. subject",      info.subject, "s");
    a.checkEqual("57. rfcMessageId", info.rfcMessageId, "a@b");

    // RfC header
    a.checkEqual("61. Date", server::toString(testee.getRfcHeader(root)->get("Date")), "Fri, 22 Jan 1971 14:08:00 +0000");
    a.checkEqual("62. :Bytes", server::toInteger(testee.getRfcHeader(root)->get(":Bytes")), 12);
}

/** Test Message-Id behaviour. */
AFL_TEST("server.talk.Message:message-ids", a)
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    Configuration config;
    config.messageIdSuffix = "@suf";
    Root root(db, config);

    // Database content
    // - message that was created on the web side and never edited
    {
        Message m(root, 50);
        m.subject().set("0");
        m.sequenceNumber().set(3);
        a.checkEqual("01. getRfcMessageId", m.getRfcMessageId(root), "50.3@suf");
        a.checkEqual("02. getPreviousRfcMessageId", m.getPreviousRfcMessageId(root), "");
        a.checkEqual("03. Message-Id", server::toString(m.getRfcHeader(root)->get("Message-Id")), "<50.3@suf>");
        a.checkNull ("04. Supersedes", m.getRfcHeader(root)->get("Supersedes"));
    }

    // - message that was created on the NNTP side and never edited
    {
        Message m(root, 51);
        m.subject().set("1");
        m.sequenceNumber().set(4);
        m.rfcMessageId().set("m1@host");
        Message::addRfcMessageId(root, "m1@host", 51);
        a.checkEqual("11. getRfcMessageId", m.getRfcMessageId(root), "m1@host");
        a.checkEqual("12. getPreviousRfcMessageId", m.getPreviousRfcMessageId(root), "");
        a.checkEqual("13. Message-Id", server::toString(m.getRfcHeader(root)->get("Message-Id")), "<m1@host>");
        a.checkNull ("14. Supersedes", m.getRfcHeader(root)->get("Supersedes"));
    }

    // - message that was created on the web side and edited on the NNTP side
    {
        Message m(root, 52);
        m.subject().set("10");
        m.previousSequenceNumber().set(5);
        m.sequenceNumber().set(6);
        m.rfcMessageId().set("m10@host");
        Message::addRfcMessageId(root, "m10@host", 52);
        a.checkEqual("21. getRfcMessageId", m.getRfcMessageId(root), "m10@host");
        a.checkEqual("22. getPreviousRfcMessageId", m.getPreviousRfcMessageId(root), "52.5@suf");
        a.checkEqual("23. Message-Id", server::toString(m.getRfcHeader(root)->get("Message-Id")), "<m10@host>");
        a.checkEqual("24. Supersedes", server::toString(m.getRfcHeader(root)->get("Supersedes")), "<52.5@suf>");
    }

    // - message that was created and edited on the web side
    {
        Message m(root, 53);
        m.subject().set("00");
        m.previousSequenceNumber().set(7);
        m.sequenceNumber().set(8);
        a.checkEqual("31. getRfcMessageId", m.getRfcMessageId(root), "53.8@suf");
        a.checkEqual("32. getPreviousRfcMessageId", m.getPreviousRfcMessageId(root), "53.7@suf");
        a.checkEqual("33. Message-Id", server::toString(m.getRfcHeader(root)->get("Message-Id")), "<53.8@suf>");
        a.checkEqual("34. Supersedes", server::toString(m.getRfcHeader(root)->get("Supersedes")), "<53.7@suf>");
    }

    // - message that was created on the NNTP side and edited on the web side
    {
        Message m(root, 54);
        m.subject().set("01");
        m.previousSequenceNumber().set(9);
        m.sequenceNumber().set(10);
        m.previousRfcMessageId().set("m01@host");
        a.checkEqual("41. getRfcMessageId", m.getRfcMessageId(root), "54.10@suf");
        a.checkEqual("42. getPreviousRfcMessageId", m.getPreviousRfcMessageId(root), "m01@host");
        a.checkEqual("43. Message-Id", server::toString(m.getRfcHeader(root)->get("Message-Id")), "<54.10@suf>");
        a.checkEqual("44. Supersedes", server::toString(m.getRfcHeader(root)->get("Supersedes")), "<m01@host>");
    }

    // - message that was created and edited on the NNTP side
    {
        Message m(root, 55);
        m.subject().set("11");
        m.previousSequenceNumber().set(11);
        m.sequenceNumber().set(12);
        m.rfcMessageId().set("m11n@host");
        m.previousRfcMessageId().set("m11o@host");
        Message::addRfcMessageId(root, "m11n@host", 55);
        a.checkEqual("51. getRfcMessageId", m.getRfcMessageId(root), "m11n@host");
        a.checkEqual("52. getPreviousRfcMessageId", m.getPreviousRfcMessageId(root), "m11o@host");
        a.checkEqual("53. Message-Id", server::toString(m.getRfcHeader(root)->get("Message-Id")), "<m11n@host>");
        a.checkEqual("54. Supersedes", server::toString(m.getRfcHeader(root)->get("Supersedes")), "<m11o@host>");
    }

    // Resolve message Ids
    a.checkEqual("61", Message::lookupRfcMessageId(root, "50.3@suf"),  50);
    a.checkEqual("62", Message::lookupRfcMessageId(root, "m1@host"),   51);
    a.checkEqual("63", Message::lookupRfcMessageId(root, "m10@host"),  52);
    a.checkEqual("64", Message::lookupRfcMessageId(root, "53.8@suf"),  53);
    a.checkEqual("65", Message::lookupRfcMessageId(root, "54.10@suf"), 54);
    a.checkEqual("66", Message::lookupRfcMessageId(root, "m11n@host"), 55);

    // Failure cases
    a.checkEqual("71", Message::lookupRfcMessageId(root, ""),          0);
    a.checkEqual("72", Message::lookupRfcMessageId(root, "what@ever"), 0);
    a.checkEqual("73", Message::lookupRfcMessageId(root, "50.2@suf"),  0);
    a.checkEqual("74", Message::lookupRfcMessageId(root, "51.4@suf"),  0);
    a.checkEqual("75", Message::lookupRfcMessageId(root, "52.5@suf"),  0);
    a.checkEqual("76", Message::lookupRfcMessageId(root, "53.7@suf"),  0);
    a.checkEqual("77", Message::lookupRfcMessageId(root, "m01@host"),  0);
    a.checkEqual("78", Message::lookupRfcMessageId(root, "55.12@suf"), 0);
}

/** Test behaviour of email addresses in messages. */
AFL_TEST("server.talk.Message:email", a)
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    Configuration config;
    config.messageIdSuffix = "@suf";
    Root root(db, config);

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
        Message m(root, 1);
        m.author().set("1001");

        a.checkEqual("01. From", server::toString(m.getRfcHeader(root)->get("From")), "ozzi <a@b>");
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
        Message m(root, 2);
        m.author().set("1002");

        a.checkEqual("11. From", server::toString(m.getRfcHeader(root)->get("From")), "azzi <az@invalid.invalid>");
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
        Message m(root, 3);
        m.author().set("1003");

        a.checkEqual("21. From", server::toString(m.getRfcHeader(root)->get("From")), "uzzi <uz@invalid.invalid>");
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
        Message m(root, 4);
        m.author().set("1004");

        a.checkEqual("31. From", server::toString(m.getRfcHeader(root)->get("From")), "yzzi <a@b>");
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
        Message m(root, 5);
        m.author().set("1005");

        a.checkEqual("41. From", server::toString(m.getRfcHeader(root)->get("From")), "E. Zzi <a@b>");
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
        Message m(root, 6);
        m.author().set("1006");

        a.checkEqual("51. From", server::toString(m.getRfcHeader(root)->get("From")), "oezzi <a@b>");
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
        Message m(root, 6);
        m.author().set("1007");

        a.checkEqual("61. From", server::toString(m.getRfcHeader(root)->get("From")), "I. Zzi <iz@invalid.invalid>");
    }
}

/** Test behaviour of parent messages/references in messages. */
AFL_TEST("server.talk.Message:parent", a)
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    Configuration config;
    config.messageIdSuffix = "@suf";
    Root root(db, config);
    createMessageChain(root);

    // Test Message-Id header for reference
    a.checkEqual("01", server::toString(Message(root, 1).getRfcHeader(root)->get("Message-Id")), "<1.1@suf>");
    a.checkEqual("02", server::toString(Message(root, 20).getRfcHeader(root)->get("Message-Id")), "<20.20@suf>");

    // Test References
    // - up to 5 parents
    a.checkNull("11", Message(root, 1).getRfcHeader(root)->get("References"));
    a.checkEqual("12", server::toString(Message(root, 2).getRfcHeader(root)->get("References")), "<1.1@suf>");
    a.checkEqual("13", server::toString(Message(root, 3).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <2.2@suf>");
    a.checkEqual("14", server::toString(Message(root, 4).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <2.2@suf>\r\n <3.3@suf>");
    a.checkEqual("15", server::toString(Message(root, 5).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <2.2@suf>\r\n <3.3@suf>\r\n <4.4@suf>");
    a.checkEqual("16", server::toString(Message(root, 6).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <2.2@suf>\r\n <3.3@suf>\r\n <4.4@suf>\r\n <5.5@suf>");
    // - now we start with 5 parents + root
    a.checkEqual("17", server::toString(Message(root, 7).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <2.2@suf>\r\n <3.3@suf>\r\n <4.4@suf>\r\n <5.5@suf>\r\n <6.6@suf>");
    a.checkEqual("18", server::toString(Message(root, 8).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <3.3@suf>\r\n <4.4@suf>\r\n <5.5@suf>\r\n <6.6@suf>\r\n <7.7@suf>");
    a.checkEqual("19", server::toString(Message(root, 20).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <15.15@suf>\r\n <16.16@suf>\r\n <17.17@suf>\r\n <18.18@suf>\r\n <19.19@suf>");
}

/** Test behaviour of parent messages/references in messages when thread starter has been deleted. */
AFL_TEST("server.talk.Message:parent:delete-first", a)
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    Configuration config;
    config.messageIdSuffix = "@suf";
    Root root(db, config);

    // Create 20 messages, delete thread starter
    createMessageChain(root);
    Message(root, 1).remove(root);

    // Test References
    // - up to 5 parents
    // 1 has been deleted; thus, 2 has no references
    a.checkNull("12", Message(root, 2).getRfcHeader(root)->get("References"));
    a.checkEqual("13", server::toString(Message(root, 3).getRfcHeader(root)->get("References")), "<2.2@suf>");
    a.checkEqual("14", server::toString(Message(root, 4).getRfcHeader(root)->get("References")), "<2.2@suf>\r\n <3.3@suf>");
    a.checkEqual("15", server::toString(Message(root, 5).getRfcHeader(root)->get("References")), "<2.2@suf>\r\n <3.3@suf>\r\n <4.4@suf>");
    a.checkEqual("16", server::toString(Message(root, 6).getRfcHeader(root)->get("References")), "<2.2@suf>\r\n <3.3@suf>\r\n <4.4@suf>\r\n <5.5@suf>");
    // - now we start with 5 parents; no root because that's gone
    a.checkEqual("17", server::toString(Message(root, 7).getRfcHeader(root)->get("References")), "<2.2@suf>\r\n <3.3@suf>\r\n <4.4@suf>\r\n <5.5@suf>\r\n <6.6@suf>");
    a.checkEqual("18", server::toString(Message(root, 8).getRfcHeader(root)->get("References")), "<3.3@suf>\r\n <4.4@suf>\r\n <5.5@suf>\r\n <6.6@suf>\r\n <7.7@suf>");
    a.checkEqual("19", server::toString(Message(root, 20).getRfcHeader(root)->get("References")), "<15.15@suf>\r\n <16.16@suf>\r\n <17.17@suf>\r\n <18.18@suf>\r\n <19.19@suf>");
}

/** Test behaviour of parent messages/references in messages when post in the middle of the thread has been deleted. */
AFL_TEST("server.talk.Message:parent:delete-mid", a)
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    Configuration config;
    config.messageIdSuffix = "@suf";
    Root root(db, config);

    // Create 20 messages, delete one in the middle
    createMessageChain(root);
    Message(root, 5).remove(root);

    // Test References
    // - up to 5 parents
    a.checkNull("11", Message(root, 1).getRfcHeader(root)->get("References"));
    a.checkEqual("12", server::toString(Message(root, 2).getRfcHeader(root)->get("References")), "<1.1@suf>");
    a.checkEqual("13", server::toString(Message(root, 3).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <2.2@suf>");
    a.checkEqual("14", server::toString(Message(root, 4).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <2.2@suf>\r\n <3.3@suf>");
    // 5 has been deleted, 6 will not have immediate parent but shows starter
    a.checkEqual("16", server::toString(Message(root, 6).getRfcHeader(root)->get("References")), "<1.1@suf>");
    a.checkEqual("17", server::toString(Message(root, 7).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <6.6@suf>");
    a.checkEqual("18", server::toString(Message(root, 8).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <6.6@suf>\r\n <7.7@suf>");
    a.checkEqual("19", server::toString(Message(root, 20).getRfcHeader(root)->get("References")), "<1.1@suf>\r\n <15.15@suf>\r\n <16.16@suf>\r\n <17.17@suf>\r\n <18.18@suf>\r\n <19.19@suf>");
}

/** Test crossposted messages. */
AFL_TEST("server.talk.Message:crosspost", a)
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    Configuration config;
    config.messageIdSuffix = "@suf";
    config.pathHost = "pp";
    Root root(db, config);
    createMessageChain(root);

    // Forums
    Forum(root, 3).setNewsgroup("ng.one", root);
    Forum(root, 5).setNewsgroup("ng.two", root);
    Topic(root, 42).forumId().set(5);

    // First message is crossposted
    Topic(root, 42).alsoPostedTo().add(3);
    Message(root, 1).sequenceNumberIn(3).set(77);

    // Check Newsgroups
    a.checkEqual("01", server::toString(Message(root, 1).getRfcHeader(root)->get("Newsgroups")), "ng.two,ng.one");
    a.checkEqual("02", server::toString(Message(root, 2).getRfcHeader(root)->get("Newsgroups")), "ng.two");

    // Check Followup-To
    a.checkEqual("11", server::toString(Message(root, 1).getRfcHeader(root)->get("Followup-To")), "ng.two");
    a.checkNull ("12", Message(root, 2).getRfcHeader(root)->get("Followup-To"));

    // Check Xref
    a.checkEqual("21", server::toString(Message(root, 1).getRfcHeader(root)->get("Xref")), "pp ng.two:1 ng.one:77");
    a.checkEqual("22", server::toString(Message(root, 2).getRfcHeader(root)->get("Xref")), "pp ng.two:2");
}

/** Test sorting functions. */
AFL_TEST("server.talk.Message:sort", a)
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    Root root(db, Configuration());

    // Database
    afl::net::redis::IntegerSetKey allMessages(db, "test_key");
    createMessages(root, allMessages);

    // Test
    {
        // MessageSorter, author
        IntegerList_t result;
        SortOperation op(allMessages.sort());
        Message::MessageSorter(root).applySortKey(op, "AUTHOR");
        op.getResult(result);

        a.checkEqual("31. size", result.size(), 6U);
        a.checkEqual("32. result", result[0], 104);
    }
    {
        // MessageSorter, edittime
        IntegerList_t result;
        SortOperation op(allMessages.sort());
        Message::MessageSorter(root).applySortKey(op, "EDITTIME");
        op.getResult(result);

        a.checkEqual("41. size", result.size(), 6U);
        a.checkEqual("42. result", result[0], 101);
    }
    {
        // MessageSorter, edittime
        IntegerList_t result;
        SortOperation op(allMessages.sort());
        Message::MessageSorter(root).applySortKey(op, "SUBJECT");
        op.getResult(result);

        a.checkEqual("51. size", result.size(), 6U);
        a.checkEqual("52. result", result[0], 105);
    }
    {
        // MessageSorter, thread
        IntegerList_t result;
        SortOperation op(allMessages.sort());
        Message::MessageSorter(root).applySortKey(op, "THREAD");
        op.getResult(result);

        a.checkEqual("61. size", result.size(), 6U);
        a.checkEqual("62. result", result[0], 103);
    }
    {
        // MessageSorter, time
        IntegerList_t result;
        SortOperation op(allMessages.sort());
        Message::MessageSorter(root).applySortKey(op, "TIME");
        op.getResult(result);

        a.checkEqual("71. size", result.size(), 6U);
        a.checkEqual("72. result", result[0], 102);
    }
    {
        // MessageSorter, errors
        SortOperation op(allMessages.sort());
        AFL_CHECK_THROWS(a("73. bad key"), Message::MessageSorter(root).applySortKey(op, "time"),  std::exception);
        AFL_CHECK_THROWS(a("74. bad key"), Message::MessageSorter(root).applySortKey(op, "OTHER"), std::exception);
        AFL_CHECK_THROWS(a("75. bad key"), Message::MessageSorter(root).applySortKey(op, ""),      std::exception);
    }
}

/** Test getMessageSequenceNumbers. */
AFL_TEST("server.talk.Message:getMessageSequenceNumbers", a)
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    Root root(db, Configuration());
    afl::net::redis::IntegerSetKey allMessages(db, "test_key");
    createMessages(root, allMessages);

    // getMessageSequenceNumbers
    IntegerList_t result;
    Message::getMessageSequenceNumbers(root, allMessages, 33, result);
    a.checkEqual("11. size", result.size(), 12U);
    a.checkEqual("12. result", result[0],  3);
    a.checkEqual("13. result", result[1],  100);
    a.checkEqual("14. result", result[2],  4);
    a.checkEqual("15. result", result[3],  101);
    a.checkEqual("16. result", result[4],  6);
    a.checkEqual("17. result", result[5],  103);
    a.checkEqual("18. result", result[6],  7);
    a.checkEqual("19. result", result[7],  104);
    a.checkEqual("20. result", result[8],  8);
    a.checkEqual("21. result", result[9],  105);
    a.checkEqual("22. result", result[10], 10);
    a.checkEqual("23. result", result[11], 102);
}

/** Test getMessageSequenceNumbers with crossposted message. */
AFL_TEST("server.talk.Message:getMessageSequenceNumbers:cross", a)
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    Root root(db, Configuration());
    afl::net::redis::IntegerSetKey allMessages(db, "test_key");
    createMessages(root, allMessages);

    Message(root, 103).sequenceNumberIn(33).set(40);

    // getMessageSequenceNumbers
    IntegerList_t result;
    Message::getMessageSequenceNumbers(root, allMessages, 33, result);
    a.checkEqual("11. size", result.size(), 12U);
    a.checkEqual("12. result", result[0],  3);
    a.checkEqual("13. result", result[1],  100);
    a.checkEqual("14. result", result[2],  4);
    a.checkEqual("15. result", result[3],  101);
    a.checkEqual("16. result", result[4],  7);
    a.checkEqual("17. result", result[5],  104);
    a.checkEqual("18. result", result[6],  8);
    a.checkEqual("19. result", result[7],  105);
    a.checkEqual("20. result", result[8],  10);
    a.checkEqual("21. result", result[9],  102);
    a.checkEqual("22. result", result[10], 40);
    a.checkEqual("23. result", result[11], 103);
}
