/**
  *  \file u/t_server_talk_talkthread.cpp
  *  \brief Test for server::talk::TalkThread
  */

#include <memory>
#include "server/talk/talkthread.hpp"

#include "t_server_talk.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "server/talk/root.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/talkpost.hpp"
#include "server/talk/talkforum.hpp"
#include "server/talk/session.hpp"
#include "afl/data/access.hpp"

/** Simple tests. */
void
TestServerTalkTalkThread::testIt()
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
        TS_ASSERT_EQUALS(TalkForum(s, root).add(f1), 1);
        TS_ASSERT_EQUALS(TalkForum(s, root).add(f2), 2);
    }

    // Create messages by posting stuff
    {
        server::talk::Session s;
        s.setUser("a");

        // One thread
        TS_ASSERT_EQUALS(TalkPost(s, root).create(1, "subj", "text:content", TalkPost::CreateOptions()), 1);
        TS_ASSERT_EQUALS(TalkPost(s, root).reply(1, "re: subj", "text:more", TalkPost::ReplyOptions()), 2);
        TS_ASSERT_EQUALS(TalkPost(s, root).reply(1, "re: subj", "text:more", TalkPost::ReplyOptions()), 3);
        TS_ASSERT_EQUALS(TalkPost(s, root).getInfo(2).threadId, 1);

        // Another
        TS_ASSERT_EQUALS(TalkPost(s, root).create(1, "subj2", "text:content", TalkPost::CreateOptions()), 4);
        TS_ASSERT_EQUALS(TalkPost(s, root).reply(4, "re: subj2", "text:more", TalkPost::ReplyOptions()), 5);
        TS_ASSERT_EQUALS(TalkPost(s, root).reply(5, "re: re: subj2", "text:more", TalkPost::ReplyOptions()), 6);
        TS_ASSERT_EQUALS(TalkPost(s, root).getInfo(4).threadId, 2);
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
        TS_ASSERT_EQUALS(i.subject, "subj");
        TS_ASSERT_EQUALS(i.forumId, 1);
        TS_ASSERT_EQUALS(i.firstPostId, 1);
        TS_ASSERT_EQUALS(i.lastPostId, 3);
        TS_ASSERT_EQUALS(i.isSticky, false);

        // - error case
        TS_ASSERT_THROWS(TalkThread(userSession, root).getInfo(99), std::exception);
    }

    // getInfo multiple
    {
        // - ok case
        static const int32_t threadIds[] = {2,9,1};
        afl::container::PtrVector<TalkThread::Info> result;
        TS_ASSERT_THROWS_NOTHING(TalkThread(userSession, root).getInfo(threadIds, result));
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT(result[1] == 0);
        TS_ASSERT(result[2] != 0);
        TS_ASSERT_EQUALS(result[0]->subject, "subj2");
        TS_ASSERT_EQUALS(result[2]->subject, "subj");
    }
    {
        // - boundary case
        afl::container::PtrVector<TalkThread::Info> result;
        TS_ASSERT_THROWS_NOTHING(TalkThread(userSession, root).getInfo(afl::base::Nothing, result));
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // getPosts
    {
        std::auto_ptr<afl::data::Value> p(TalkThread(userSession, root).getPosts(2, TalkThread::ListParameters()));
        TS_ASSERT_EQUALS(Access(p).getArraySize(), 3U);
        TS_ASSERT_EQUALS(Access(p)[0].toInteger(), 4);
        TS_ASSERT_EQUALS(Access(p)[1].toInteger(), 5);
        TS_ASSERT_EQUALS(Access(p)[2].toInteger(), 6);
    }

    // Stickyness
    {
        // Error case: user a does not have permission
        TS_ASSERT_THROWS(TalkThread(userSession, root).setSticky(1, true), std::exception);

        // Error case: nonexistant thread
        TS_ASSERT_THROWS(TalkThread(userSession, root).setSticky(3, true), std::exception);
        TS_ASSERT_THROWS(TalkThread(rootSession, root).setSticky(3, true), std::exception);

        // Success case: root can do it [repeatedly]
        TS_ASSERT_THROWS_NOTHING(TalkThread(rootSession, root).setSticky(1, true));
        TS_ASSERT_THROWS_NOTHING(TalkThread(rootSession, root).setSticky(1, true));

        // Verify
        TalkForum::ListParameters lp;
        lp.mode = lp.WantMemberCheck;
        lp.item = 1;
        std::auto_ptr<afl::data::Value> p(TalkForum(rootSession, root).getStickyThreads(1, lp));
        TS_ASSERT_EQUALS(Access(p).toInteger(), 1);

        // Success case: b can do it
        TS_ASSERT_THROWS_NOTHING(TalkThread(otherSession, root).setSticky(1, false));
        TS_ASSERT_THROWS_NOTHING(TalkThread(otherSession, root).setSticky(1, false));

        p.reset(TalkForum(rootSession, root).getStickyThreads(1, lp));
        TS_ASSERT_EQUALS(Access(p).toInteger(), 0);
    }

    // Get permissions
    {
        const String_t perms[] = {"write", "delete"};

        // root can do anything
        TS_ASSERT_EQUALS(TalkThread(rootSession, root).getPermissions(1, perms), 3);

        // a can write but not delete
        TS_ASSERT_EQUALS(TalkThread(userSession, root).getPermissions(1, perms), 1);

        // b can write and delete
        TS_ASSERT_EQUALS(TalkThread(otherSession, root).getPermissions(1, perms), 3);
    }

    // Move
    {
        // - Error cases: users cannot do this due to missing permissions
        TS_ASSERT_THROWS(TalkThread(userSession, root).moveToForum(1, 2), std::exception);
        TS_ASSERT_THROWS(TalkThread(otherSession, root).moveToForum(1, 2), std::exception);

        // - Error case: bad Ids
        TS_ASSERT_THROWS(TalkThread(rootSession, root).moveToForum(55, 2), std::exception);
        TS_ASSERT_THROWS(TalkThread(rootSession, root).moveToForum(1, 55), std::exception);

        // - OK case, null operation
        TS_ASSERT_THROWS_NOTHING(TalkThread(userSession, root).moveToForum(1, 1));
        TS_ASSERT_THROWS_NOTHING(TalkThread(otherSession, root).moveToForum(1, 1));

        // - OK case
        TS_ASSERT_THROWS_NOTHING(TalkThread(rootSession, root).moveToForum(1, 2));

        // - Verify
        TS_ASSERT_EQUALS(TalkThread(userSession, root).getInfo(1).forumId, 2);
    }

    // Remove
    {
        // - Error case: a cannot remove
        TS_ASSERT_THROWS(TalkThread(userSession, root).remove(1), std::exception);
        TS_ASSERT_THROWS(TalkThread(userSession, root).remove(2), std::exception);

        // - Error case: b cannot remove #1 from forum #2
        TS_ASSERT_THROWS(TalkThread(otherSession, root).remove(1), std::exception);

        // - Not-quite-error case: does not exist
        TS_ASSERT_EQUALS(TalkThread(userSession, root).remove(99), false);
        TS_ASSERT_EQUALS(TalkThread(otherSession, root).remove(99), false);
        TS_ASSERT_EQUALS(TalkThread(rootSession, root).remove(99), false);

        // - Success case: root can remove thread #1 it from forum #2
        TS_ASSERT_EQUALS(TalkThread(rootSession, root).remove(1), true);
        TS_ASSERT_EQUALS(TalkThread(rootSession, root).remove(1), false);

        // - Success case: b can remove thread #2 from forum #1
        TS_ASSERT_EQUALS(TalkThread(otherSession, root).remove(2), true);
        TS_ASSERT_EQUALS(TalkThread(otherSession, root).remove(2), false);
    }
}

