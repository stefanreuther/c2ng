/**
  *  \file test/server/talk/talknntptest.cpp
  *  \brief Test for server::talk::TalkNNTP
  */

#include "server/talk/talknntp.hpp"

#include "afl/data/access.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("server.talk.TalkNNTP:groups", a)
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
        a.checkEqual("01. add", f.add(f1), 1);
        a.checkEqual("02. add", f.add(f2), 2);
        a.checkEqual("03. add", f.add(f3), 3);
    }


    /*
     *  Test
     */

    // listNewsgroups as user a
    {
        afl::container::PtrVector<TalkNNTP::Info> result;
        AFL_CHECK_SUCCEEDS(a("11. listNewsgroups"), TalkNNTP(userSession, root).listNewsgroups(result));
        a.checkEqual("12. size", result.size(), 2U);
        a.checkNonNull("13. result", result[0]);
        a.checkNonNull("14. result", result[1]);

        const TalkNNTP::Info* p1 = (result[0]->forumId == 1 ? result[0] : result[1]);
        const TalkNNTP::Info* p2 = (result[0]->forumId == 2 ? result[0] : result[1]);
        a.checkEqual("21. forumId",       p1->forumId, 1);
        a.checkEqual("22. newsgroupName", p1->newsgroupName, "ng.one");
        a.checkEqual("23. forumId",       p2->forumId, 2);
        a.checkEqual("24. newsgroupName", p2->newsgroupName, "ng.two");
    }

    // listNewsgroups as user b, who can only see ng.two
    {
        afl::container::PtrVector<TalkNNTP::Info> result;
        AFL_CHECK_SUCCEEDS(a("31. listNewsgroups"), TalkNNTP(otherSession, root).listNewsgroups(result));
        a.checkEqual("32. size", result.size(), 1U);
        a.checkNonNull("33. result",      result[0]);
        a.checkEqual("34. forumId",       result[0]->forumId, 2);
        a.checkEqual("35. newsgroupName", result[0]->newsgroupName, "ng.two");
    }

    // listNewsgroups as root is not allowed
    {
        afl::container::PtrVector<TalkNNTP::Info> result;
        AFL_CHECK_THROWS(a("41. listNewsgroups as admin"), TalkNNTP(rootSession, root).listNewsgroups(result), std::exception);
    }

    // findNewsgroup
    a.checkEqual      ("51. findNewsgroup",  TalkNNTP(userSession, root).findNewsgroup("ng.one").forumId, 1);
    a.checkEqual      ("52. findNewsgroup",  TalkNNTP(userSession, root).findNewsgroup("ng.two").forumId, 2);
    AFL_CHECK_THROWS(a("53. findNewsgroup"), TalkNNTP(userSession, root).findNewsgroup("ng.three").forumId, std::exception);
    AFL_CHECK_THROWS(a("54. findNewsgroup"), TalkNNTP(rootSession, root).findNewsgroup("ng.one").forumId, std::exception);
    AFL_CHECK_THROWS(a("55. findNewsgroup"), TalkNNTP(rootSession, root).findNewsgroup("ng.two").forumId, std::exception);
    AFL_CHECK_THROWS(a("56. findNewsgroup"), TalkNNTP(rootSession, root).findNewsgroup("ng.three").forumId, std::exception);
    AFL_CHECK_THROWS(a("57. findNewsgroup"), TalkNNTP(otherSession, root).findNewsgroup("ng.one").forumId, std::exception);
    a.checkEqual      ("58. findNewsgroup",  TalkNNTP(otherSession, root).findNewsgroup("ng.two").forumId, 2);
    AFL_CHECK_THROWS(a("59. findNewsgroup"), TalkNNTP(otherSession, root).findNewsgroup("ng.three").forumId, std::exception);

    // listNewsgroupsByGroup
    // FIXME: this command will produce newsgroup names irrespective of accessibility and presence of a newsgroup.
    {
        afl::data::StringList_t rootResult, otherResult;
        AFL_CHECK_SUCCEEDS(a("61. listNewsgroupsByGroup"), TalkNNTP(rootSession, root).listNewsgroupsByGroup("gr", rootResult));
        AFL_CHECK_SUCCEEDS(a("62. listNewsgroupsByGroup"), TalkNNTP(otherSession, root).listNewsgroupsByGroup("gr", otherResult));
        a.checkEqual("63. size", rootResult.size(), 2U);
        a.check("64. result", rootResult[0] == "ng.one" || rootResult[0] == "");
        a.check("65. result", rootResult[1] == "ng.one" || rootResult[1] == "");
        a.check("66. result", rootResult[0] != rootResult[1]);
        a.check("67. result", rootResult == otherResult);
    }
}

/** Test findMessage(). */
AFL_TEST("server.talk.TalkNNTP:findMessage", a)
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
        a.checkEqual("01. add forum", TalkForum(session, root).add(afl::base::Nothing), 1);

        TalkPost::CreateOptions opts;
        opts.userId = "a";
        a.checkEqual("11. create post", TalkPost(session, root).create(1, "subj", "text", opts),   1);
        a.checkEqual("12. create post", TalkPost(session, root).create(1, "subj2", "text2", opts), 2);

        // FIXME: normally, we should be able to set the Message-Id in create(). For now, work around
        server::talk::Message msg(root, 2);
        msg.rfcMessageId().set("mid@otherhost");
        msg.addRfcMessageId(root, "mid@otherhost", 2);
    }

    // Test
    TalkNNTP testee(session, root);
    a.checkEqual("21. findMessage", testee.findMessage("1.1@host"), 1);
    a.checkEqual("22. findMessage", testee.findMessage("mid@otherhost"), 2);
    AFL_CHECK_THROWS(a("23. findMessage"), testee.findMessage("2.1@host"), std::exception);
    AFL_CHECK_THROWS(a("24. findMessage"), testee.findMessage("2.2@host"), std::exception);
    AFL_CHECK_THROWS(a("25. findMessage"), testee.findMessage("1.2@host"), std::exception);
    AFL_CHECK_THROWS(a("26. findMessage"), testee.findMessage(""), std::exception);
}

/** Test listMessages. */
AFL_TEST("server.talk.TalkNNTP:listMessages", a)
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
        a.checkEqual("01. add forum", TalkForum(rootSession, root).add(forumConfig), 1);
        a.checkEqual("02. create post", TalkPost(userSession, root).create(1, "subj",      "text",  TalkPost::CreateOptions()), 1);
        a.checkEqual("03. create post", TalkPost(userSession, root).create(1, "subj2",     "text2", TalkPost::CreateOptions()), 2);
        a.checkEqual("04. reply post", TalkPost(userSession, root).reply (2, "re: subj2", "text3", TalkPost::ReplyOptions()),  3);
        AFL_CHECK_SUCCEEDS(a("05. edit post"), TalkPost(userSession, root).edit(2, "subj2", "edit"));
    }

    // Test
    {
        // Result is list of (sequence,post Id), sorted by sequence numbers.
        afl::data::IntegerList_t result;
        AFL_CHECK_SUCCEEDS(a("11. listMessages"), TalkNNTP(userSession, root).listMessages(1, result));
        a.checkEqual("12. size", result.size(), 6U);
        a.checkEqual("13. result", result[0], 1);
        a.checkEqual("14. result", result[1], 1);
        a.checkEqual("15. result", result[2], 3);
        a.checkEqual("16. result", result[3], 3);
        a.checkEqual("17. result", result[4], 4);
        a.checkEqual("18. result", result[5], 2);

        afl::data::IntegerList_t rootResult;
        AFL_CHECK_SUCCEEDS(a("21. listMessages"), TalkNNTP(rootSession, root).listMessages(1, rootResult));
        a.check("22. result", rootResult == result);
    }

    // Error case
    {
        afl::data::IntegerList_t result;
        AFL_CHECK_THROWS(a("31. listMessages"), TalkNNTP(userSession, root).listMessages(9, result), std::exception);
    }
}

/** Test message header access. */
AFL_TEST("server.talk.TalkNNTP:getMessageHeader", a)
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
        a.checkEqual("01. add forum", TalkForum(rootSession, root).add(forumConfig), 1);
        a.checkEqual("02. create post", TalkPost(userSession, root).create(1, "subj",      "text",  TalkPost::CreateOptions()), 1);
        a.checkEqual("03. create post", TalkPost(userSession, root).create(1, "subj2",     "text2", TalkPost::CreateOptions()), 2);
        a.checkEqual("04. reply post", TalkPost(userSession, root).reply (2, "re: subj2", "text3", TalkPost::ReplyOptions()),  3);
        AFL_CHECK_SUCCEEDS(a("05. edit post"), TalkPost(userSession, root).edit(2, "subj2", "edit"));
    }

    // Get single header
    {
        afl::data::Hash::Ref_t p(TalkNNTP(userSession, root).getMessageHeader(1));
        a.checkEqual("11. Newsgroups", Access(p->get("Newsgroups")).toString(), "ng.name");
        a.checkEqual("12. Subject",    Access(p->get("Subject")).toString(), "subj");
        a.checkEqual("13. Message-Id", Access(p->get("Message-Id")).toString(), "<1.1@host>");
    }
    {
        afl::data::Hash::Ref_t p(TalkNNTP(userSession, root).getMessageHeader(2));
        a.checkEqual("14. Newsgroups", Access(p->get("Newsgroups")).toString(), "ng.name");
        a.checkEqual("15. Subject",    Access(p->get("Subject")).toString(), "subj2");
        a.checkEqual("16. Message-Id", Access(p->get("Message-Id")).toString(), "<2.4@host>");
        a.checkEqual("17. Supersedes", Access(p->get("Supersedes")).toString(), "<2.2@host>");
    }

    // Get multiple
    {
        static const int32_t mids[] = {1,9,2};
        afl::data::Segment result;
        AFL_CHECK_SUCCEEDS(a("21. getMessageHeader"), TalkNNTP(userSession, root).getMessageHeader(mids, result));
        a.checkEqual  ("22. size",       result.size(), 3U);
        a.checkNonNull("23. result",     result[0]);
        a.checkNull   ("24. result",     result[1]);
        a.checkNonNull("25. result",     result[2]);
        a.checkEqual  ("26. Message-Id", Access(result[0])("Message-Id").toString(), "<1.1@host>");
        a.checkEqual  ("27. Message-Id", Access(result[2])("Message-Id").toString(), "<2.4@host>");
        a.checkEqual  ("28. Supersedes", Access(result[2])("Supersedes").toString(), "<2.2@host>");
    }

    // Error case: must have user context
    {
        AFL_CHECK_THROWS(a("31. getMessageHeader"), TalkNNTP(rootSession, root).getMessageHeader(1), std::exception);
    }
    {
        static const int32_t mids[] = {1,3};
        afl::data::Segment result;
        AFL_CHECK_THROWS(a("32. getMessageHeader"), TalkNNTP(rootSession, root).getMessageHeader(mids, result), std::exception);
    }

    // Error case: does not exist
    {
        AFL_CHECK_THROWS(a("41. getMessageHeader"), TalkNNTP(userSession, root).getMessageHeader(99), std::exception);
    }
}
