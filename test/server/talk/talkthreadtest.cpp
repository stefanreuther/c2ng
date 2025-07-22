/**
  *  \file test/server/talk/talkthreadtest.cpp
  *  \brief Test for server::talk::TalkThread
  */

#include "server/talk/talkthread.hpp"

#include <memory>
#include "afl/data/access.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/talkforum.hpp"
#include "server/talk/talkpost.hpp"
#include "util/string.hpp"

using afl::data::Access;
using afl::net::redis::InternalDatabase;
using server::talk::Configuration;
using server::talk::Forum;
using server::talk::Message;
using server::talk::Root;
using server::talk::Session;
using server::talk::Topic;
using server::talk::TalkForum;
using server::talk::TalkPost;
using server::talk::TalkThread;

namespace {
    String_t getSet(afl::net::redis::IntegerSetKey k)
    {
        afl::data::IntegerList_t values;
        k.sort().getResult(values);

        String_t result;
        for (size_t i = 0; i < values.size(); ++i) {
            if (i != 0) {
                result += ',';
            }
            result += afl::string::Format("%d", values[i]);
        }
        return result;
    }
}

/** Simple tests. */
AFL_TEST("server.talk.TalkThread", a)
{
    // Infrastructure
    InternalDatabase db;
    Configuration config;
    config.rateCostPerPost = 0;
    Root root(db, config);

    // Create some forums
    {
        const String_t f1[] = {"name", "forum1", "readperm", "all", "deleteperm", "u:b", "writeperm", "all"};
        const String_t f2[] = {"name", "forum2", "readperm", "all"};
        Session s;
        a.checkEqual("01. add", TalkForum(s, root).add(f1), 1);
        a.checkEqual("02. add", TalkForum(s, root).add(f2), 2);
    }

    // Create messages by posting stuff
    {
        Session s;
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

    Session rootSession;
    Session userSession; userSession.setUser("a");
    Session otherSession; otherSession.setUser("b");

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

/** Test moving a cross-posted thread.
    Also tests sequence numbers in cross-post in general. */
AFL_TEST("server.talk.TalkThread:move-crossposted", a)
{
    // Infrastructure
    InternalDatabase db;
    Configuration config;
    config.rateCostPerPost = 0;
    Root root(db, config);
    Session s;

    // Create some forums
    const String_t forumParams[] = {"name", "forum1", "readperm", "all", "writeperm", "all", "deleteperm", "all"};
    for (int i = 1; i <= 10; ++i) {
        a.checkEqual("01. add", TalkForum(s, root).add(forumParams), i);
    }

    // Post
    TalkPost::CreateOptions opts;
    opts.alsoPostTo.push_back(5);
    opts.alsoPostTo.push_back(1);
    opts.userId = "u";
    a.checkEqual("11. post", TalkPost(s, root).create(3, "sub", "forum:text", opts), 1);

    TalkPost::CreateOptions opts2;
    opts2.userId = "u";
    a.checkEqual("12. post", TalkPost(s, root).create(5, "other sub", "forum:text", opts2), 2);

    //            forum 1    forum 3   forum 5
    // Message 1   seq 1     *seq 1     seq 1
    // Message 2                       *seq 2

    // Reply
    TalkPost::ReplyOptions ropts;
    ropts.userId = "v";
    a.checkEqual("21. reply", TalkPost(s, root).reply(1, "reply 1", "forum:text", ropts), 3);
    a.checkEqual("22. reply", TalkPost(s, root).reply(1, "reply 2", "forum:text", ropts), 4);

    //            forum 1    forum 3   forum 5
    // Message 3             *seq 2
    // Message 4             *seq 3

    a.checkEqual("31. seq", Message(root, 1).sequenceNumber().get(), 1);
    a.checkEqual("32. seq", Message(root, 2).sequenceNumber().get(), 2);
    a.checkEqual("33. seq", Message(root, 3).sequenceNumber().get(), 2);
    a.checkEqual("34. seq", Message(root, 4).sequenceNumber().get(), 3);

    // Edit, to exercise sequence numbers
    TalkPost(s, root).edit(4, "new reply 2", "forum:text");
    TalkPost(s, root).edit(1, "new sub",     "forum:text");
    TalkPost(s, root).edit(2, "new other",   "forum:text");
    TalkPost(s, root).edit(3, "new reply 1", "forum:text");

    //            forum 1    forum 3   forum 5
    // Message 1   seq 2     *seq 5     seq 3
    // Message 2                       *seq 4
    // Message 3             *seq 6
    // Message 4             *seq 4

    // Verify
    a.checkEqual("41. seq", Message(root, 1).sequenceNumber().get(), 5);
    a.checkEqual("42. seq", Message(root, 2).sequenceNumber().get(), 4);
    a.checkEqual("43. seq", Message(root, 3).sequenceNumber().get(), 6);
    a.checkEqual("44. seq", Message(root, 4).sequenceNumber().get(), 4);
    a.checkEqual("45. seq", Message(root, 1).sequenceNumberIn(1).get(), 2);
    a.checkEqual("46. seq", Message(root, 1).sequenceNumberIn(3).get(), 0);   // Main sequence number, not in "in" branch
    a.checkEqual("47. seq", Message(root, 1).sequenceNumberIn(5).get(), 3);
    a.checkEqual("48. seq", Message(root, 2).sequenceNumberIn(5).get(), 0);   // Not cross-posted
    a.checkEqual("49. seq", Message(root, 3).sequenceNumberIn(1).get(), 0);   // Not cross-posted
    a.checkEqual("4a. seq", Message(root, 3).sequenceNumberIn(3).get(), 0);   // Not cross-posted

    // Placement in sets
    a.checkEqual("51. messages", getSet(Forum(root, 1).messages()), "1");
    a.checkEqual("52. messages", getSet(Forum(root, 3).messages()), "1,3,4");
    a.checkEqual("53. messages", getSet(Forum(root, 5).messages()), "1,2");
    a.checkEqual("54. crosspost", getSet(Topic(root, 1).alsoPostedTo()), "1,5");
    a.checkEqual("55. topics",    getSet(Forum(root, 1).topics()), "1");
    a.checkEqual("56. topics",    getSet(Forum(root, 3).topics()), "1");
    a.checkEqual("57. topics",    getSet(Forum(root, 5).topics()), "1,2");

    // - move the cross-posted thread -
    TalkThread(s, root).moveToForum(1, 1);

    //            forum 1    forum 3   forum 5
    // Message 1  *seq 3                seq 5
    // Message 2                       *seq 4
    // Message 3  *seq 4
    // Message 4  *seq 5

    // Verify
    a.checkEqual("61. forum", Topic(root, 1).forumId().get(), 1);
    a.checkEqual("62. seq", Message(root, 1).sequenceNumber().get(), 3);
    a.checkEqual("63. seq", Message(root, 2).sequenceNumber().get(), 4);
    a.checkEqual("64. seq", Message(root, 3).sequenceNumber().get(), 4);
    a.checkEqual("65. seq", Message(root, 4).sequenceNumber().get(), 5);
    a.checkEqual("65. seq", Message(root, 1).sequenceNumberIn(1).get(), 0);   // Main sequence number, no in "in" branch
    a.checkEqual("66. seq", Message(root, 1).sequenceNumberIn(3).get(), 0);   // Removed
    a.checkEqual("67. seq", Message(root, 1).sequenceNumberIn(5).get(), 5);
    a.checkEqual("68. seq", Message(root, 2).sequenceNumberIn(5).get(), 0);   // Not cross-posted
    a.checkEqual("69. seq", Message(root, 3).sequenceNumberIn(1).get(), 0);   // Not cross-posted
    a.checkEqual("6a. seq", Message(root, 3).sequenceNumberIn(3).get(), 0);   // Not cross-posted

    // Placement in sets
    a.checkEqual("71. messages",  getSet(Forum(root, 1).messages()), "1,3,4");
    a.checkEqual("72. messages",  getSet(Forum(root, 3).messages()), "");
    a.checkEqual("73. messages",  getSet(Forum(root, 5).messages()), "1,2");
    a.checkEqual("74. crosspost", getSet(Topic(root, 1).alsoPostedTo()), "5");
    a.checkEqual("75. topics",    getSet(Forum(root, 1).topics()), "1");
    a.checkEqual("76. topics",    getSet(Forum(root, 3).topics()), "");
    a.checkEqual("77. topics",    getSet(Forum(root, 5).topics()), "1,2");
}

/** Test moving a cross-posted thread.
    Move the thread into one it is not cross-posted in. */
AFL_TEST("server.talk.TalkThread:move-crossposted-to-new", a)
{
    // Infrastructure
    InternalDatabase db;
    Configuration config;
    config.rateCostPerPost = 0;
    Root root(db, config);
    Session s;

    // Create some forums
    const String_t forumParams[] = {"name", "forum1", "readperm", "all", "writeperm", "all", "deleteperm", "all"};
    for (int i = 1; i <= 10; ++i) {
        a.checkEqual("01. add", TalkForum(s, root).add(forumParams), i);
    }

    // Post
    TalkPost::CreateOptions opts;
    opts.alsoPostTo.push_back(5);
    opts.alsoPostTo.push_back(1);
    opts.userId = "u";
    a.checkEqual("11. post", TalkPost(s, root).create(3, "sub", "forum:text", opts), 1);

    TalkPost::CreateOptions opts2;
    opts2.userId = "u";
    a.checkEqual("12. post", TalkPost(s, root).create(5, "other sub", "forum:text", opts2), 2);

    // Reply
    TalkPost::ReplyOptions ropts;
    ropts.userId = "v";
    a.checkEqual("21. reply", TalkPost(s, root).reply(1, "reply 1", "forum:text", ropts), 3);
    a.checkEqual("22. reply", TalkPost(s, root).reply(1, "reply 2", "forum:text", ropts), 4);

    // Edit, to exercise sequence numbers
    TalkPost(s, root).edit(4, "new reply 2", "forum:text");
    TalkPost(s, root).edit(1, "new sub",     "forum:text");
    TalkPost(s, root).edit(2, "new other",   "forum:text");
    TalkPost(s, root).edit(3, "new reply 1", "forum:text");

    // Up to here, same sequence as in previous test.

    //            forum 1    forum 3   forum 5
    // Message 1   seq 2     *seq 5     seq 3
    // Message 2                       *seq 4
    // Message 3             *seq 6
    // Message 4             *seq 4

    // - move the cross-posted thread -
    TalkThread(s, root).moveToForum(1, 7);

    //            forum 1    forum 3   forum 5   forum 7
    // Message 1   seq 3                seq 5    *seq 2 -- not 1, that is its previousSequenceNumber()
    // Message 2                       *seq 4
    // Message 3                                 *seq 3
    // Message 4                                 *seq 5 -- not 4, that is its sequenceNumber()

    // Verify
    a.checkEqual("61. forum", Topic(root, 1).forumId().get(), 7);
    a.checkEqual("62. seq", Message(root, 1).sequenceNumber().get(), 2);
    a.checkEqual("63. seq", Message(root, 2).sequenceNumber().get(), 4);
    a.checkEqual("64. seq", Message(root, 3).sequenceNumber().get(), 3);
    a.checkEqual("65. seq", Message(root, 4).sequenceNumber().get(), 5);
    a.checkEqual("65. seq", Message(root, 1).sequenceNumberIn(1).get(), 3);
    a.checkEqual("66. seq", Message(root, 1).sequenceNumberIn(3).get(), 0);   // Removed
    a.checkEqual("67. seq", Message(root, 1).sequenceNumberIn(5).get(), 5);
    a.checkEqual("67a. seq", Message(root, 1).sequenceNumberIn(7).get(), 0);  // Main sequence number, no in "in" branch
    a.checkEqual("68. seq", Message(root, 2).sequenceNumberIn(5).get(), 0);   // Not cross-posted
    a.checkEqual("69. seq", Message(root, 3).sequenceNumberIn(1).get(), 0);   // Not cross-posted
    a.checkEqual("6a. seq", Message(root, 3).sequenceNumberIn(3).get(), 0);   // Not cross-posted

    // Placement in sets
    a.checkEqual("71. messages", getSet(Forum(root, 1).messages()), "1");
    a.checkEqual("72. messages", getSet(Forum(root, 3).messages()), "");
    a.checkEqual("73. messages", getSet(Forum(root, 5).messages()), "1,2");
    a.checkEqual("74. messages", getSet(Forum(root, 7).messages()), "1,3,4");
    a.checkEqual("75. crosspost", getSet(Topic(root, 1).alsoPostedTo()), "1,5");
    a.checkEqual("75. topics",    getSet(Forum(root, 1).topics()), "1");
    a.checkEqual("76. topics",    getSet(Forum(root, 3).topics()), "");
    a.checkEqual("77. topics",    getSet(Forum(root, 5).topics()), "1,2");
    a.checkEqual("78. topics",    getSet(Forum(root, 7).topics()), "1");
}
