/**
  *  \file u/t_server_talk_talknntp.cpp
  *  \brief Test for server::talk::TalkNNTP
  */

#include "server/talk/talknntp.hpp"

#include "t_server_talk.hpp"
#include "afl/data/access.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/subtree.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/message.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/talkforum.hpp"
#include "server/talk/talkgroup.hpp"
#include "server/talk/talkpost.hpp"

using server::talk::TalkNNTP;
using server::talk::TalkForum;
using server::talk::TalkPost;
using afl::data::Access;

/** Test newsgroup access commands: listNewsgroups(), findNewsgroup(), listNewsgroupsByGroup(). */
void
TestServerTalkTalkNNTP::testGroups()
{
    // Environment
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::Session rootSession;
    server::talk::Session userSession;
    server::talk::Session otherSession;
    userSession.setUser("a");
    otherSession.setUser("b");

    // Create a group
    {
        server::talk::TalkGroup g(rootSession, root);
        server::talk::TalkGroup::Description d;
        d.name = "Group";
        g.add("gr", d);
    }

    // Create some forums
    {
        server::talk::TalkForum f(rootSession, root);
        const String_t f1[] = {"name", "Forum 1", "readperm", "u:a", "newsgroup", "ng.one", "parent", "gr"};
        const String_t f2[] = {"name", "Forum 2", "readperm", "all", "newsgroup", "ng.two"};
        const String_t f3[] = {"name", "Forum 3", "readperm", "all",                        "parent", "gr"};
        TS_ASSERT_EQUALS(f.add(f1), 1);
        TS_ASSERT_EQUALS(f.add(f2), 2);
        TS_ASSERT_EQUALS(f.add(f3), 3);
    }


    /*
     *  Test
     */

    // listNewsgroups as user a
    {
        afl::container::PtrVector<TalkNNTP::Info> result;
        TS_ASSERT_THROWS_NOTHING(TalkNNTP(userSession, root).listNewsgroups(result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT(result[1] != 0);

        const TalkNNTP::Info* p1 = (result[0]->forumId == 1 ? result[0] : result[1]);
        const TalkNNTP::Info* p2 = (result[0]->forumId == 2 ? result[0] : result[1]);
        TS_ASSERT_EQUALS(p1->forumId, 1);
        TS_ASSERT_EQUALS(p1->newsgroupName, "ng.one");
        TS_ASSERT_EQUALS(p2->forumId, 2);
        TS_ASSERT_EQUALS(p2->newsgroupName, "ng.two");
    }

    // listNewsgroups as user b, who can only see ng.two
    {
        afl::container::PtrVector<TalkNNTP::Info> result;
        TS_ASSERT_THROWS_NOTHING(TalkNNTP(otherSession, root).listNewsgroups(result));
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT_EQUALS(result[0]->forumId, 2);
        TS_ASSERT_EQUALS(result[0]->newsgroupName, "ng.two");
    }

    // listNewsgroups as root is not allowed
    {
        afl::container::PtrVector<TalkNNTP::Info> result;
        TS_ASSERT_THROWS(TalkNNTP(rootSession, root).listNewsgroups(result), std::exception);
    }

    // findNewsgroup
    TS_ASSERT_EQUALS(TalkNNTP(userSession, root).findNewsgroup("ng.one").forumId, 1);
    TS_ASSERT_EQUALS(TalkNNTP(userSession, root).findNewsgroup("ng.two").forumId, 2);
    TS_ASSERT_THROWS(TalkNNTP(userSession, root).findNewsgroup("ng.three").forumId, std::exception);
    TS_ASSERT_THROWS(TalkNNTP(rootSession, root).findNewsgroup("ng.one").forumId, std::exception);
    TS_ASSERT_THROWS(TalkNNTP(rootSession, root).findNewsgroup("ng.two").forumId, std::exception);
    TS_ASSERT_THROWS(TalkNNTP(rootSession, root).findNewsgroup("ng.three").forumId, std::exception);
    TS_ASSERT_THROWS(TalkNNTP(otherSession, root).findNewsgroup("ng.one").forumId, std::exception);
    TS_ASSERT_EQUALS(TalkNNTP(otherSession, root).findNewsgroup("ng.two").forumId, 2);
    TS_ASSERT_THROWS(TalkNNTP(otherSession, root).findNewsgroup("ng.three").forumId, std::exception);

    // listNewsgroupsByGroup
    // FIXME: this command will produce newsgroup names irrespective of accessibility and presence of a newsgroup.
    {
        afl::data::StringList_t rootResult, otherResult;
        TS_ASSERT_THROWS_NOTHING(TalkNNTP(rootSession, root).listNewsgroupsByGroup("gr", rootResult));
        TS_ASSERT_THROWS_NOTHING(TalkNNTP(otherSession, root).listNewsgroupsByGroup("gr", otherResult));
        TS_ASSERT_EQUALS(rootResult.size(), 2U);
        TS_ASSERT(rootResult[0] == "ng.one" || rootResult[0] == "");
        TS_ASSERT(rootResult[1] == "ng.one" || rootResult[1] == "");
        TS_ASSERT(rootResult[0] != rootResult[1]);
        TS_ASSERT_EQUALS(rootResult, otherResult);
    }
}

/** Test findMessage(). */
void
TestServerTalkTalkNNTP::testFindMessage()
{
    // Environment
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Configuration config;
    config.messageIdSuffix = "@host";
    server::talk::Root root(db, mq, config);
    server::talk::Session session;

    // Create a forum and messages in it
    {
        TS_ASSERT_EQUALS(TalkForum(session, root).add(afl::base::Nothing), 1);

        TalkPost::CreateOptions opts;
        opts.userId = "a";
        TS_ASSERT_EQUALS(TalkPost(session, root).create(1, "subj", "text", opts),   1);
        TS_ASSERT_EQUALS(TalkPost(session, root).create(1, "subj2", "text2", opts), 2);

        // FIXME: normally, we should be able to set the Message-Id in create(). For now, work around
        server::talk::Message msg(root, 2);
        msg.rfcMessageId().set("mid@otherhost");
        msg.addRfcMessageId(root, "mid@otherhost", 2);
    }

    // Test
    TalkNNTP testee(session, root);
    TS_ASSERT_EQUALS(testee.findMessage("1.1@host"), 1);
    TS_ASSERT_EQUALS(testee.findMessage("mid@otherhost"), 2);
    TS_ASSERT_THROWS(testee.findMessage("2.1@host"), std::exception);
    TS_ASSERT_THROWS(testee.findMessage("2.2@host"), std::exception);
    TS_ASSERT_THROWS(testee.findMessage("1.2@host"), std::exception);
    TS_ASSERT_THROWS(testee.findMessage(""), std::exception);
}

/** Test listMessages. */
void
TestServerTalkTalkNNTP::testListMessages()
{
    // Environment
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Configuration config;
    config.messageIdSuffix = "@host";
    server::talk::Root root(db, mq, config);
    server::talk::Session rootSession;
    server::talk::Session userSession;
    userSession.setUser("a");

    // Create a forum and messages in it
    {
        const String_t forumConfig[] = {"name","forum","writeperm","all","readperm","all"};
        TS_ASSERT_EQUALS(TalkForum(rootSession, root).add(forumConfig), 1);
        TS_ASSERT_EQUALS(TalkPost(userSession, root).create(1, "subj",      "text",  TalkPost::CreateOptions()), 1);
        TS_ASSERT_EQUALS(TalkPost(userSession, root).create(1, "subj2",     "text2", TalkPost::CreateOptions()), 2);
        TS_ASSERT_EQUALS(TalkPost(userSession, root).reply (2, "re: subj2", "text3", TalkPost::ReplyOptions()),  3);
        TS_ASSERT_THROWS_NOTHING(TalkPost(userSession, root).edit(2, "subj2", "edit"));
    }

    // Test
    {
        // Result is list of (sequence,post Id), sorted by sequence numbers.
        afl::data::IntegerList_t result;
        TS_ASSERT_THROWS_NOTHING(TalkNNTP(userSession, root).listMessages(1, result));
        TS_ASSERT_EQUALS(result.size(), 6U);
        TS_ASSERT_EQUALS(result[0], 1);
        TS_ASSERT_EQUALS(result[1], 1);
        TS_ASSERT_EQUALS(result[2], 3);
        TS_ASSERT_EQUALS(result[3], 3);
        TS_ASSERT_EQUALS(result[4], 4);
        TS_ASSERT_EQUALS(result[5], 2);

        afl::data::IntegerList_t rootResult;
        TS_ASSERT_THROWS_NOTHING(TalkNNTP(rootSession, root).listMessages(1, rootResult));
        TS_ASSERT_EQUALS(rootResult, result);
    }

    // Error case
    {
        afl::data::IntegerList_t result;
        TS_ASSERT_THROWS(TalkNNTP(userSession, root).listMessages(9, result), std::exception);
    }
}

/** Test message header access. */
void
TestServerTalkTalkNNTP::testMessageHeader()
{
    // Environment
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Configuration config;
    config.messageIdSuffix = "@host";
    server::talk::Root root(db, mq, config);
    server::talk::Session rootSession;
    server::talk::Session userSession;
    userSession.setUser("a");

    // Create a forum and messages in it
    {
        const String_t forumConfig[] = {"name","forum","writeperm","all","readperm","all","newsgroup","ng.name"};
        TS_ASSERT_EQUALS(TalkForum(rootSession, root).add(forumConfig), 1);
        TS_ASSERT_EQUALS(TalkPost(userSession, root).create(1, "subj",      "text",  TalkPost::CreateOptions()), 1);
        TS_ASSERT_EQUALS(TalkPost(userSession, root).create(1, "subj2",     "text2", TalkPost::CreateOptions()), 2);
        TS_ASSERT_EQUALS(TalkPost(userSession, root).reply (2, "re: subj2", "text3", TalkPost::ReplyOptions()),  3);
        TS_ASSERT_THROWS_NOTHING(TalkPost(userSession, root).edit(2, "subj2", "edit"));
    }

    // Get single header
    {
        afl::data::Hash::Ref_t p(TalkNNTP(userSession, root).getMessageHeader(1));
        TS_ASSERT_EQUALS(Access(p->get("Newsgroups")).toString(), "ng.name");
        TS_ASSERT_EQUALS(Access(p->get("Subject")).toString(), "subj");
        TS_ASSERT_EQUALS(Access(p->get("Message-Id")).toString(), "<1.1@host>");
    }
    {
        afl::data::Hash::Ref_t p(TalkNNTP(userSession, root).getMessageHeader(2));
        TS_ASSERT_EQUALS(Access(p->get("Newsgroups")).toString(), "ng.name");
        TS_ASSERT_EQUALS(Access(p->get("Subject")).toString(), "subj2");
        TS_ASSERT_EQUALS(Access(p->get("Message-Id")).toString(), "<2.4@host>");
        TS_ASSERT_EQUALS(Access(p->get("Supersedes")).toString(), "<2.2@host>");
    }

    // Get multiple
    {
        static const int32_t mids[] = {1,9,2};
        afl::data::Segment result;
        TS_ASSERT_THROWS_NOTHING(TalkNNTP(userSession, root).getMessageHeader(mids, result));
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT(result[1] == 0);
        TS_ASSERT(result[2] != 0);
        TS_ASSERT_EQUALS(Access(result[0])("Message-Id").toString(), "<1.1@host>");
        TS_ASSERT_EQUALS(Access(result[2])("Message-Id").toString(), "<2.4@host>");
        TS_ASSERT_EQUALS(Access(result[2])("Supersedes").toString(), "<2.2@host>");
    }

    // Error case: must have user context
    {
        TS_ASSERT_THROWS(TalkNNTP(rootSession, root).getMessageHeader(1), std::exception);
    }
    {
        static const int32_t mids[] = {1,3};
        afl::data::Segment result;
        TS_ASSERT_THROWS(TalkNNTP(rootSession, root).getMessageHeader(mids, result), std::exception);
    }

    // Error case: does not exist
    {
        TS_ASSERT_THROWS(TalkNNTP(userSession, root).getMessageHeader(99), std::exception);
    }
}

