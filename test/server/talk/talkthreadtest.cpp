/**
  *  \file test/server/talk/talkthreadtest.cpp
  *  \brief Test for server::talk::TalkThread
  */

#include "server/talk/talkthread.hpp"

#include "afl/data/access.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/talkforum.hpp"
#include "server/talk/talkpost.hpp"
#include <memory>

/** Simple tests. */
AFL_TEST("server.talk.TalkThread", a)
{
    using server::talk::TalkPost;
    using server::talk::TalkForum;
    using server::talk::TalkThread;
    using afl::data::Access;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Create some forums
    {
        const String_t f1[] = {"name", "forum1", "readperm", "all", "deleteperm", "u:b", "writeperm", "all"};
        const String_t f2[] = {"name", "forum2", "readperm", "all"};
        server::talk::Session s;
        a.checkEqual("01. add", TalkForum(s, root).add(f1), 1);
        a.checkEqual("02. add", TalkForum(s, root).add(f2), 2);
    }

    // Create messages by posting stuff
    {
        server::talk::Session s;
        s.setUser("a");

        // One thread
        a.checkEqual("11. create",  TalkPost(s, root).create(1, "subj", "text:content", TalkPost::CreateOptions()), 1);
        a.checkEqual("12. reply",   TalkPost(s, root).reply(1, "re: subj", "text:more", TalkPost::ReplyOptions()), 2);
        a.checkEqual("13. reply",   TalkPost(s, root).reply(1, "re: subj", "text:more", TalkPost::ReplyOptions()), 3);
        a.checkEqual("14. getInfo", TalkPost(s, root).getInfo(2).threadId, 1);

        // Another
        a.checkEqual("21. create",  TalkPost(s, root).create(1, "subj2", "text:content", TalkPost::CreateOptions()), 4);
        a.checkEqual("22. reply",   TalkPost(s, root).reply(4, "re: subj2", "text:more", TalkPost::ReplyOptions()), 5);
        a.checkEqual("23. reply",   TalkPost(s, root).reply(5, "re: re: subj2", "text:more", TalkPost::ReplyOptions()), 6);
        a.checkEqual("24. getInfo", TalkPost(s, root).getInfo(4).threadId, 2);
    }

    /*
     *  Test as user
     */

    server::talk::Session rootSession;
    server::talk::Session userSession; userSession.setUser("a");
    server::talk::Session otherSession; otherSession.setUser("b");

    // getInfo
    {
        // - ok case
        TalkThread::Info i = TalkThread(userSession, root).getInfo(1);
        a.checkEqual("31. subject",     i.subject, "subj");
        a.checkEqual("32. forumId",     i.forumId, 1);
        a.checkEqual("33. firstPostId", i.firstPostId, 1);
        a.checkEqual("34. lastPostId",  i.lastPostId, 3);
        a.checkEqual("35. isSticky",    i.isSticky, false);

        // - error case
        AFL_CHECK_THROWS(a("41. getInfo"), TalkThread(userSession, root).getInfo(99), std::exception);
    }

    // getInfo multiple
    {
        // - ok case
        static const int32_t threadIds[] = {2,9,1};
        afl::container::PtrVector<TalkThread::Info> result;
        AFL_CHECK_SUCCEEDS(a("51. getInfo"), TalkThread(userSession, root).getInfo(threadIds, result));

        a.checkEqual  ("52. size", result.size(), 3U);
        a.checkNonNull("53. result",  result[0]);
        a.checkNull   ("54. result",  result[1]);
        a.checkNonNull("55. result",  result[2]);
        a.checkEqual  ("56. subject", result[0]->subject, "subj2");
        a.checkEqual  ("57. subject", result[2]->subject, "subj");
    }
    {
        // - boundary case
        afl::container::PtrVector<TalkThread::Info> result;
        AFL_CHECK_SUCCEEDS(a("58. getInfo"), TalkThread(userSession, root).getInfo(afl::base::Nothing, result));
        a.checkEqual("59. size", result.size(), 0U);
    }

    // getPosts
    {
        std::auto_ptr<afl::data::Value> p(TalkThread(userSession, root).getPosts(2, TalkThread::ListParameters()));
        a.checkEqual("61. size", Access(p).getArraySize(), 3U);
        a.checkEqual("62. result", Access(p)[0].toInteger(), 4);
        a.checkEqual("63. result", Access(p)[1].toInteger(), 5);
        a.checkEqual("64. result", Access(p)[2].toInteger(), 6);
    }

    // Stickyness
    {
        // Error case: user a does not have permission
        AFL_CHECK_THROWS(a("71. setSticky"), TalkThread(userSession, root).setSticky(1, true), std::exception);

        // Error case: nonexistant thread
        AFL_CHECK_THROWS(a("81. setSticky"), TalkThread(userSession, root).setSticky(3, true), std::exception);
        AFL_CHECK_THROWS(a("82. setSticky"), TalkThread(rootSession, root).setSticky(3, true), std::exception);

        // Success case: root can do it [repeatedly]
        AFL_CHECK_SUCCEEDS(a("91. setSticky"), TalkThread(rootSession, root).setSticky(1, true));
        AFL_CHECK_SUCCEEDS(a("92. setSticky"), TalkThread(rootSession, root).setSticky(1, true));

        // Verify
        TalkForum::ListParameters lp;
        lp.mode = lp.WantMemberCheck;
        lp.item = 1;
        std::auto_ptr<afl::data::Value> p(TalkForum(rootSession, root).getStickyThreads(1, lp));
        a.checkEqual("101. getStickyThreads", Access(p).toInteger(), 1);

        // Success case: b can do it
        AFL_CHECK_SUCCEEDS(a("111. setSticky"), TalkThread(otherSession, root).setSticky(1, false));
        AFL_CHECK_SUCCEEDS(a("112. setSticky"), TalkThread(otherSession, root).setSticky(1, false));

        p.reset(TalkForum(rootSession, root).getStickyThreads(1, lp));
        a.checkEqual("121. getStickyThreads", Access(p).toInteger(), 0);
    }

    // Get permissions
    {
        const String_t perms[] = {"write", "delete"};

        // root can do anything
        a.checkEqual("131. getPermissions", TalkThread(rootSession, root).getPermissions(1, perms), 3);

        // a can write but not delete
        a.checkEqual("141. getPermissions", TalkThread(userSession, root).getPermissions(1, perms), 1);

        // b can write and delete
        a.checkEqual("151. getPermissions", TalkThread(otherSession, root).getPermissions(1, perms), 3);
    }

    // Move
    {
        // - Error cases: users cannot do this due to missing permissions
        AFL_CHECK_THROWS(a("161. moveToForum"), TalkThread(userSession, root).moveToForum(1, 2), std::exception);
        AFL_CHECK_THROWS(a("162. moveToForum"), TalkThread(otherSession, root).moveToForum(1, 2), std::exception);

        // - Error case: bad Ids
        AFL_CHECK_THROWS(a("171. moveToForum"), TalkThread(rootSession, root).moveToForum(55, 2), std::exception);
        AFL_CHECK_THROWS(a("172. moveToForum"), TalkThread(rootSession, root).moveToForum(1, 55), std::exception);

        // - OK case, null operation
        AFL_CHECK_SUCCEEDS(a("181. moveToForum"), TalkThread(userSession, root).moveToForum(1, 1));
        AFL_CHECK_SUCCEEDS(a("182. moveToForum"), TalkThread(otherSession, root).moveToForum(1, 1));

        // - OK case
        AFL_CHECK_SUCCEEDS(a("191. moveToForum"), TalkThread(rootSession, root).moveToForum(1, 2));

        // - Verify
        a.checkEqual("201. forumId", TalkThread(userSession, root).getInfo(1).forumId, 2);
    }

    // Remove
    {
        // - Error case: a cannot remove
        AFL_CHECK_THROWS(a("211. remove"), TalkThread(userSession, root).remove(1), std::exception);
        AFL_CHECK_THROWS(a("212. remove"), TalkThread(userSession, root).remove(2), std::exception);

        // - Error case: b cannot remove #1 from forum #2
        AFL_CHECK_THROWS(a("221. remove"), TalkThread(otherSession, root).remove(1), std::exception);

        // - Not-quite-error case: does not exist
        a.checkEqual("231. remove", TalkThread(userSession, root).remove(99), false);
        a.checkEqual("232. remove", TalkThread(otherSession, root).remove(99), false);
        a.checkEqual("233. remove", TalkThread(rootSession, root).remove(99), false);

        // - Success case: root can remove thread #1 from forum #2
        a.checkEqual("241. remove", TalkThread(rootSession, root).remove(1), true);
        a.checkEqual("242. remove", TalkThread(rootSession, root).remove(1), false);

        // - Success case: b can remove thread #2 from forum #1
        a.checkEqual("251. remove", TalkThread(otherSession, root).remove(2), true);
        a.checkEqual("252. remove", TalkThread(otherSession, root).remove(2), false);
    }
}
