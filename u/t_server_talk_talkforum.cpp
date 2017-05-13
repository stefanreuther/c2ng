/**
  *  \file u/t_server_talk_talkforum.cpp
  *  \brief Test for server::talk::TalkForum
  */

#include <memory>
#include "server/talk/talkforum.hpp"

#include "t_server_talk.hpp"
#include "u/helper/commandhandlermock.hpp"
#include "server/talk/sorter.hpp"
#include "afl/net/redis/sortoperation.hpp"
#include "afl/data/value.hpp"
#include "afl/data/access.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/segment.hpp"
#include "server/types.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/session.hpp"
#include "server/talk/root.hpp"
#include "server/talk/talkgroup.hpp"
#include "server/talk/talkpost.hpp"

using afl::data::Access;
using afl::data::Segment;
using afl::data::Value;
using afl::data::Vector;
using afl::data::VectorValue;

/** Test executeListOperation(). */
void
TestServerTalkTalkForum::testListOperation()
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

    CommandHandlerMock mock;
    afl::net::redis::IntegerSetKey key(mock, "key");
    TestSorter sorter;

    // Default (=WantAll)
    {
        mock.expectCall("SORT|key");
        mock.provideReturnValue(new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackInteger(9))));

        server::interface::TalkForum::ListParameters p;
        std::auto_ptr<afl::data::Value> result(server::talk::TalkForum::executeListOperation(p, key, sorter));
        TS_ASSERT_EQUALS(Access(result).getArraySize(), 2U);
        TS_ASSERT_EQUALS(Access(result)[0].toInteger(), 1);
        TS_ASSERT_EQUALS(Access(result)[1].toInteger(), 9);
    }

    // Part (=WantRange)
    {
        mock.expectCall("SORT|key|LIMIT|3|7");
        mock.provideReturnValue(new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackInteger(9).pushBackInteger(12))));

        server::interface::TalkForum::ListParameters p;
        p.mode = p.WantRange;
        p.start = 3;
        p.count = 7;
        std::auto_ptr<afl::data::Value> result(server::talk::TalkForum::executeListOperation(p, key, sorter));
        TS_ASSERT_EQUALS(Access(result).getArraySize(), 3U);
        TS_ASSERT_EQUALS(Access(result)[0].toInteger(), 1);
        TS_ASSERT_EQUALS(Access(result)[1].toInteger(), 9);
        TS_ASSERT_EQUALS(Access(result)[2].toInteger(), 12);
    }

    // Sorted
    {
        mock.expectCall("SORT|key|BY|*->field");
        mock.provideReturnValue(new VectorValue(Vector::create(Segment().pushBackInteger(9).pushBackInteger(1))));

        server::interface::TalkForum::ListParameters p;
        p.sortKey = "field";
        std::auto_ptr<afl::data::Value> result(server::talk::TalkForum::executeListOperation(p, key, sorter));
        TS_ASSERT_EQUALS(Access(result).getArraySize(), 2U);
        TS_ASSERT_EQUALS(Access(result)[0].toInteger(), 9);
        TS_ASSERT_EQUALS(Access(result)[1].toInteger(), 1);
    }

    // Sorted by invalid key
    {
        server::interface::TalkForum::ListParameters p;
        p.sortKey = "boom";
        TS_ASSERT_THROWS(server::talk::TalkForum::executeListOperation(p, key, sorter), std::exception);
    }

    // Member check
    {
        mock.expectCall("SISMEMBER|key|42");
        mock.provideReturnValue(server::makeIntegerValue(1));

        server::interface::TalkForum::ListParameters p;
        p.mode = p.WantMemberCheck;
        p.item = 42;
        std::auto_ptr<afl::data::Value> result(server::talk::TalkForum::executeListOperation(p, key, sorter));
        TS_ASSERT_EQUALS(Access(result).toInteger(), 1);
    }

    // Size
    {
        mock.expectCall("SCARD|key");
        mock.provideReturnValue(server::makeIntegerValue(6));

        server::interface::TalkForum::ListParameters p;
        p.mode = p.WantSize;
        std::auto_ptr<afl::data::Value> result(server::talk::TalkForum::executeListOperation(p, key, sorter));
        TS_ASSERT_EQUALS(Access(result).toInteger(), 6);
    }

    mock.checkFinish();
}

/** Test commands. */
void
TestServerTalkTalkForum::testIt()
{
    using server::talk::TalkPost;
    using server::talk::TalkForum;
    using server::talk::TalkGroup;
    using afl::data::Access;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());
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
        TS_ASSERT_EQUALS(id, 1);
    }
    {
        // Try to create as user, must fail
        const String_t config2a[] = { "name", "Second" };
        TS_ASSERT_THROWS(TalkForum(userSession, root).add(config2a), std::exception);
    }
    {
        // Second forum
        const String_t config2[] = { "name", "Second",
                                     "readperm", "all",
                                     "writeperm", "all" };
        int32_t id = TalkForum(rootSession, root).add(config2);
        TS_ASSERT_EQUALS(id, 2);
    }
    {
        // Verify group content
        afl::data::IntegerList_t forums;
        afl::data::StringList_t groups;
        TalkGroup(userSession, root).list("g1", groups, forums);
        TS_ASSERT_EQUALS(forums.size(), 1U);
        TS_ASSERT_EQUALS(forums[0], 1);
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
        TS_ASSERT_EQUALS(forums.size(), 0U);
    }
    {
        // - verify
        afl::data::IntegerList_t forums;
        afl::data::StringList_t groups;
        TalkGroup(userSession, root).list("g2", groups, forums);
        TS_ASSERT_EQUALS(forums.size(), 1U);
        TS_ASSERT_EQUALS(forums[0], 1);
    }
    {
        // - nonexistant
        const String_t reconfig1[] = { "parent", "g2" };
        TS_ASSERT_THROWS(TalkForum(rootSession, root).configure(5, reconfig1), std::exception);
    }
    {
        // - permission denied
        const String_t reconfig1[] = { "parent", "g2" };
        TS_ASSERT_THROWS(TalkForum(userSession, root).configure(1, reconfig1), std::exception);
    }
    {
        // - syntax error
        const String_t reconfig1[] = { "parent" };
        TS_ASSERT_THROWS(TalkForum(rootSession, root).configure(1, reconfig1), std::exception);
    }

    // Get configuration
    {
        // - ok
        std::auto_ptr<afl::data::Value> p(TalkForum(rootSession, root).getValue(2, "readperm"));
        TS_ASSERT_EQUALS(server::toString(p.get()), "all");
    }
    {
        // - nonexistant
        TS_ASSERT_THROWS(TalkForum(rootSession, root).getValue(9, "readperm"), std::exception);
    }

    // Get information
    {
        // - ok, ask first as user
        TalkForum::Info i = TalkForum(userSession, root).getInfo(1);
        TS_ASSERT_EQUALS(i.name, "First");
        TS_ASSERT_EQUALS(i.parentGroup, "g2");
        TS_ASSERT_EQUALS(i.description, "");
        TS_ASSERT_EQUALS(i.newsgroupName, "ng.first");
    }
    {
        // - ok, ask second as root
        TalkForum::Info i = TalkForum(rootSession, root).getInfo(2);
        TS_ASSERT_EQUALS(i.name, "Second");
        TS_ASSERT_EQUALS(i.parentGroup, "");
        TS_ASSERT_EQUALS(i.description, "");
        TS_ASSERT_EQUALS(i.newsgroupName, "");
    }
    {
        // - error case
        TS_ASSERT_THROWS(TalkForum(userSession, root).getInfo(10), std::exception);
    }
    {
        // - ask multiple
        static const int32_t fids[] = { 1, 2 };
        afl::container::PtrVector<TalkForum::Info> is;
        TS_ASSERT_THROWS_NOTHING(TalkForum(userSession, root).getInfo(fids, is));
        TS_ASSERT_EQUALS(is.size(), 2U);
        TS_ASSERT(is[0] != 0);
        TS_ASSERT(is[1] != 0);
        TS_ASSERT_EQUALS(is[0]->name, "First");
        TS_ASSERT_EQUALS(is[1]->name, "Second");
    }
    {
        // - ask multiple, including invalid
        // FIXME: this is consistent with PCC2, but inconsistent with other get-multiple commands that return a null pointer for failing items
        static const int32_t fids[] = { 1, 10, 2 };
        afl::container::PtrVector<TalkForum::Info> is;
        TS_ASSERT_THROWS(TalkForum(userSession, root).getInfo(fids, is), std::exception);
    }

    // Get permissions
    {
        const String_t perms[] = { "write", "read" };
        TS_ASSERT_EQUALS(TalkForum(rootSession, root).getPermissions(1, perms), 3);
        TS_ASSERT_EQUALS(TalkForum(userSession, root).getPermissions(1, perms), 2);

        TS_ASSERT_THROWS(TalkForum(userSession, root).getPermissions(10, perms), std::exception);
    }

    // Get size
    {
        // - initially empty
        TalkForum::Size sz = TalkForum(userSession, root).getSize(2);
        TS_ASSERT_EQUALS(sz.numThreads, 0);
        TS_ASSERT_EQUALS(sz.numStickyThreads, 0);
        TS_ASSERT_EQUALS(sz.numMessages, 0);
    }
    {
        // - create one topic with two posts
        int32_t postId = TalkPost(userSession, root).create(2, "subj", "text:text", TalkPost::CreateOptions());
        TS_ASSERT_EQUALS(postId, 1);

        int32_t replyId = TalkPost(userSession, root).reply(postId, "Re: subj", "text:witty reply", TalkPost::ReplyOptions());
        TS_ASSERT_EQUALS(replyId, 2);
    }
    {
        // - no longer empty
        TalkForum::Size sz = TalkForum(userSession, root).getSize(2);
        TS_ASSERT_EQUALS(sz.numThreads, 1);
        TS_ASSERT_EQUALS(sz.numStickyThreads, 0);
        TS_ASSERT_EQUALS(sz.numMessages, 2);
    }
    {
        // - error case
        TS_ASSERT_THROWS(TalkForum(userSession, root).getSize(9), std::exception);
    }

    // Get content. Let's keep this simple.
    {
        std::auto_ptr<afl::data::Value> p(TalkForum(userSession, root).getThreads(2, TalkForum::ListParameters()));
        TS_ASSERT_EQUALS(Access(p).getArraySize(), 1U);
        TS_ASSERT_EQUALS(Access(p)[0].toInteger(), 1);
    }
    {
        std::auto_ptr<afl::data::Value> p(TalkForum(userSession, root).getStickyThreads(2, TalkForum::ListParameters()));
        TS_ASSERT_EQUALS(Access(p).getArraySize(), 0U);
    }
    {
        std::auto_ptr<afl::data::Value> p(TalkForum(userSession, root).getPosts(2, TalkForum::ListParameters()));
        TS_ASSERT_EQUALS(Access(p).getArraySize(), 2U);
        TS_ASSERT_EQUALS(Access(p)[0].toInteger(), 1);
        TS_ASSERT_EQUALS(Access(p)[1].toInteger(), 2);
    }
    {
        // - error cases
        TS_ASSERT_THROWS(TalkForum(userSession, root).getThreads(7, TalkForum::ListParameters()), std::exception);
        TS_ASSERT_THROWS(TalkForum(userSession, root).getStickyThreads(7, TalkForum::ListParameters()), std::exception);
        TS_ASSERT_THROWS(TalkForum(userSession, root).getPosts(7, TalkForum::ListParameters()), std::exception);
    }
}

