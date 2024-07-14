/**
  *  \file test/server/talk/talkforumtest.cpp
  *  \brief Test for server::talk::TalkForum
  */

#include "server/talk/talkforum.hpp"

#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/value.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/sortoperation.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/sorter.hpp"
#include "server/talk/talkgroup.hpp"
#include "server/talk/talkpost.hpp"
#include "server/types.hpp"
#include <memory>

using afl::data::Access;
using afl::data::Segment;
using afl::data::Value;
using afl::data::Vector;
using afl::data::VectorValue;

/** Test executeListOperation(). */
AFL_TEST("server.talk.TalkForum:executeListOperation", a)
{
    class TestSorter : public server::talk::Sorter {
     public:
        virtual void applySortKey(afl::net::redis::SortOperation& op, const String_t& keyName) const
            {
                if (keyName == "boom") {
                    throw std::runtime_error("boom");
                } else {
                    op.by("*->" + keyName);
                }
            }
    };

    afl::test::CommandHandler mock(a);
    afl::net::redis::IntegerSetKey key(mock, "key");
    TestSorter sorter;

    // Default (=WantAll)
    {
        mock.expectCall("SORT, key");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackInteger(9))));

        server::interface::TalkForum::ListParameters p;
        std::auto_ptr<afl::data::Value> result(server::talk::TalkForum::executeListOperation(p, key, sorter));
        a.checkEqual("01", Access(result).getArraySize(), 2U);
        a.checkEqual("02", Access(result)[0].toInteger(), 1);
        a.checkEqual("03", Access(result)[1].toInteger(), 9);
    }

    // Part (=WantRange)
    {
        mock.expectCall("SORT, key, LIMIT, 3, 7");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackInteger(9).pushBackInteger(12))));

        server::interface::TalkForum::ListParameters p;
        p.mode = p.WantRange;
        p.start = 3;
        p.count = 7;
        std::auto_ptr<afl::data::Value> result(server::talk::TalkForum::executeListOperation(p, key, sorter));
        a.checkEqual("11", Access(result).getArraySize(), 3U);
        a.checkEqual("12", Access(result)[0].toInteger(), 1);
        a.checkEqual("13", Access(result)[1].toInteger(), 9);
        a.checkEqual("14", Access(result)[2].toInteger(), 12);
    }

    // Sorted
    {
        mock.expectCall("SORT, key, BY, *->field");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(9).pushBackInteger(1))));

        server::interface::TalkForum::ListParameters p;
        p.sortKey = "field";
        std::auto_ptr<afl::data::Value> result(server::talk::TalkForum::executeListOperation(p, key, sorter));
        a.checkEqual("21", Access(result).getArraySize(), 2U);
        a.checkEqual("22", Access(result)[0].toInteger(), 9);
        a.checkEqual("23", Access(result)[1].toInteger(), 1);
    }

    // Sorted by invalid key
    {
        server::interface::TalkForum::ListParameters p;
        p.sortKey = "boom";
        AFL_CHECK_THROWS(a("31. sort by invalid key"), server::talk::TalkForum::executeListOperation(p, key, sorter), std::exception);
    }

    // Member check
    {
        mock.expectCall("SISMEMBER, key, 42");
        mock.provideNewResult(server::makeIntegerValue(1));

        server::interface::TalkForum::ListParameters p;
        p.mode = p.WantMemberCheck;
        p.item = 42;
        std::auto_ptr<afl::data::Value> result(server::talk::TalkForum::executeListOperation(p, key, sorter));
        a.checkEqual("41", Access(result).toInteger(), 1);
    }

    // Size
    {
        mock.expectCall("SCARD, key");
        mock.provideNewResult(server::makeIntegerValue(6));

        server::interface::TalkForum::ListParameters p;
        p.mode = p.WantSize;
        std::auto_ptr<afl::data::Value> result(server::talk::TalkForum::executeListOperation(p, key, sorter));
        a.checkEqual("51", Access(result).toInteger(), 6);
    }

    mock.checkFinish();
}

/** Test commands. */
AFL_TEST("server.talk.TalkForum:basics", a)
{
    using server::talk::TalkPost;
    using server::talk::TalkForum;
    using server::talk::TalkGroup;
    using afl::data::Access;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, server::talk::Configuration());
    server::talk::Session rootSession;
    server::talk::Session userSession;
    userSession.setUser("a");

    // Create two groups [for testing]
    TalkGroup(rootSession, root).add("g1", TalkGroup::Description());
    TalkGroup(rootSession, root).add("g2", TalkGroup::Description());

    // Create two forums
    {
        // First forum
        const String_t config1[] = { "name", "First",
                                     "parent", "g1",
                                     "newsgroup", "ng.first",
                                     "readperm", "all",
                                     "writeperm", "u:b" };
        int32_t id = TalkForum(rootSession, root).add(config1);
        a.checkEqual("01. add", id, 1);
    }
    {
        // Try to create as user, must fail
        const String_t config2a[] = { "name", "Second" };
        AFL_CHECK_THROWS(a("02. add as user"), TalkForum(userSession, root).add(config2a), std::exception);
    }
    {
        // Second forum
        const String_t config2[] = { "name", "Second",
                                     "readperm", "all",
                                     "writeperm", "all" };
        int32_t id = TalkForum(rootSession, root).add(config2);
        a.checkEqual("03. add", id, 2);
    }
    {
        // Verify group content
        afl::data::IntegerList_t forums;
        afl::data::StringList_t groups;
        TalkGroup(userSession, root).list("g1", groups, forums);
        a.checkEqual("04. size", forums.size(), 1U);
        a.checkEqual("05. result", forums[0], 1);
    }

    // Configure forums
    {
        // - change config
        const String_t reconfig1[] = { "parent", "g2" };
        TalkForum(rootSession, root).configure(1, reconfig1);
    }
    {
        // - verify
        afl::data::IntegerList_t forums;
        afl::data::StringList_t groups;
        TalkGroup(userSession, root).list("g1", groups, forums);
        a.checkEqual("11. size", forums.size(), 0U);
    }
    {
        // - verify
        afl::data::IntegerList_t forums;
        afl::data::StringList_t groups;
        TalkGroup(userSession, root).list("g2", groups, forums);
        a.checkEqual("12. size", forums.size(), 1U);
        a.checkEqual("13. result", forums[0], 1);
    }
    {
        // - nonexistant
        const String_t reconfig1[] = { "parent", "g2" };
        AFL_CHECK_THROWS(a("14. nonexistant"), TalkForum(rootSession, root).configure(5, reconfig1), std::exception);
    }
    {
        // - permission denied
        const String_t reconfig1[] = { "parent", "g2" };
        AFL_CHECK_THROWS(a("15. permission"), TalkForum(userSession, root).configure(1, reconfig1), std::exception);
    }
    {
        // - syntax error
        const String_t reconfig1[] = { "parent" };
        AFL_CHECK_THROWS(a("16. syntax error"), TalkForum(rootSession, root).configure(1, reconfig1), std::exception);
    }

    // Get configuration
    {
        // - ok
        std::auto_ptr<afl::data::Value> p(TalkForum(rootSession, root).getValue(2, "readperm"));
        a.checkEqual("21. getValue", server::toString(p.get()), "all");
    }
    {
        // - nonexistant
        AFL_CHECK_THROWS(a("22. getValue nonexistant"), TalkForum(rootSession, root).getValue(9, "readperm"), std::exception);
    }

    // Get information
    {
        // - ok, ask first as user
        TalkForum::Info i = TalkForum(userSession, root).getInfo(1);
        a.checkEqual("31. name",          i.name, "First");
        a.checkEqual("32. parentGroup",   i.parentGroup, "g2");
        a.checkEqual("33. description",   i.description, "");
        a.checkEqual("34. newsgroupName", i.newsgroupName, "ng.first");
    }
    {
        // - ok, ask second as root
        TalkForum::Info i = TalkForum(rootSession, root).getInfo(2);
        a.checkEqual("35. name",          i.name, "Second");
        a.checkEqual("36. parentGroup",   i.parentGroup, "");
        a.checkEqual("37. description",   i.description, "");
        a.checkEqual("38. newsgroupName", i.newsgroupName, "");
    }
    {
        // - error case
        AFL_CHECK_THROWS(a("39. getInfo nonexistant"), TalkForum(userSession, root).getInfo(10), std::exception);
    }
    {
        // - ask multiple
        static const int32_t fids[] = { 1, 2 };
        afl::container::PtrVector<TalkForum::Info> is;
        AFL_CHECK_SUCCEEDS(a("40. getInfo"), TalkForum(userSession, root).getInfo(fids, is));
        a.checkEqual  ("41. size",   is.size(), 2U);
        a.checkNonNull("42. result", is[0]);
        a.checkNonNull("43. result", is[1]);
        a.checkEqual  ("44. name",   is[0]->name, "First");
        a.checkEqual  ("45. name",   is[1]->name, "Second");
    }
    {
        // - ask multiple, including invalid
        // FIXME: this is consistent with PCC2, but inconsistent with other get-multiple commands that return a null pointer for failing items
        static const int32_t fids[] = { 1, 10, 2 };
        afl::container::PtrVector<TalkForum::Info> is;
        AFL_CHECK_THROWS(a("46. getInfo"), TalkForum(userSession, root).getInfo(fids, is), std::exception);
    }

    // Get permissions
    {
        const String_t perms[] = { "write", "read" };
        a.checkEqual("51. getPermissions", TalkForum(rootSession, root).getPermissions(1, perms), 3);
        a.checkEqual("52. getPermissions", TalkForum(userSession, root).getPermissions(1, perms), 2);

        AFL_CHECK_THROWS(a("61. getPermissions"), TalkForum(userSession, root).getPermissions(10, perms), std::exception);
    }

    // Get size
    {
        // - initially empty
        TalkForum::Size sz = TalkForum(userSession, root).getSize(2);
        a.checkEqual("71. numThreads",       sz.numThreads, 0);
        a.checkEqual("72. numStickyThreads", sz.numStickyThreads, 0);
        a.checkEqual("73. numMessages",      sz.numMessages, 0);
    }
    {
        // - create one topic with two posts
        int32_t postId = TalkPost(userSession, root).create(2, "subj", "text:text", TalkPost::CreateOptions());
        a.checkEqual("74. creat", postId, 1);

        int32_t replyId = TalkPost(userSession, root).reply(postId, "Re: subj", "text:witty reply", TalkPost::ReplyOptions());
        a.checkEqual("81. reply", replyId, 2);
    }
    {
        // - no longer empty
        TalkForum::Size sz = TalkForum(userSession, root).getSize(2);
        a.checkEqual("82. numThreads",       sz.numThreads, 1);
        a.checkEqual("83. numStickyThreads", sz.numStickyThreads, 0);
        a.checkEqual("84. numMessages",      sz.numMessages, 2);
    }
    {
        // - error case
        AFL_CHECK_THROWS(a("85. getSize"), TalkForum(userSession, root).getSize(9), std::exception);
    }

    // Get content. Let's keep this simple.
    {
        std::auto_ptr<afl::data::Value> p(TalkForum(userSession, root).getThreads(2, TalkForum::ListParameters()));
        a.checkEqual("91. getThreads", Access(p).getArraySize(), 1U);
        a.checkEqual("92. getThreads", Access(p)[0].toInteger(), 1);
    }
    {
        std::auto_ptr<afl::data::Value> p(TalkForum(userSession, root).getStickyThreads(2, TalkForum::ListParameters()));
        a.checkEqual("93. getStickyThreads", Access(p).getArraySize(), 0U);
    }
    {
        std::auto_ptr<afl::data::Value> p(TalkForum(userSession, root).getPosts(2, TalkForum::ListParameters()));
        a.checkEqual("94. getPosts", Access(p).getArraySize(), 2U);
        a.checkEqual("95. getPosts", Access(p)[0].toInteger(), 1);
        a.checkEqual("96. getPosts", Access(p)[1].toInteger(), 2);
    }
    {
        // - error cases
        AFL_CHECK_THROWS(a("97. bad id"), TalkForum(userSession, root).getThreads(7, TalkForum::ListParameters()), std::exception);
        AFL_CHECK_THROWS(a("98. bad id"), TalkForum(userSession, root).getStickyThreads(7, TalkForum::ListParameters()), std::exception);
        AFL_CHECK_THROWS(a("99. bad id"), TalkForum(userSession, root).getPosts(7, TalkForum::ListParameters()), std::exception);
    }
}

/** Test findForum(). */
AFL_TEST("server.talk.TalkForum:findForum", a)
{
    using server::talk::TalkPost;
    using server::talk::TalkForum;
    using server::talk::TalkGroup;
    using afl::data::Access;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, server::talk::Configuration());
    server::talk::Session rootSession;
    server::talk::Session userSession;
    userSession.setUser("a");

    // Create a bunch of forums
    const String_t config[] = { "name", "f" };
    for (int i = 0; i < 10; ++i) {
        TalkForum(rootSession, root).add(config);
    }
    int32_t fid = TalkForum(rootSession, root).add(config);
    a.checkEqual("01", fid, 11);

    // Configure
    afl::net::redis::HashKey(db, "forum:byname").intField("news").set(fid);

    // Test
    a.checkEqual("11. find news", TalkForum(rootSession, root).findForum("news"), fid);
    a.checkEqual("12. find news", TalkForum(userSession, root).findForum("news"), fid);

    a.checkEqual("21. find other", TalkForum(rootSession, root).findForum("other"), 0);
    a.checkEqual("22. find other", TalkForum(userSession, root).findForum("other"), 0);
}
