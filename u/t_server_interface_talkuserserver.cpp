/**
  *  \file u/t_server_interface_talkuserserver.cpp
  *  \brief Test for server::interface::TalkUserServer
  */

#include <stdexcept>
#include <memory>
#include "server/interface/talkuserserver.hpp"

#include "t_server_interface.hpp"
#include "u/helper/callreceiver.hpp"
#include "afl/string/format.hpp"
#include "server/types.hpp"
#include "server/interface/talkuser.hpp"
#include "server/interface/talkuserclient.hpp"

using afl::data::Segment;
using afl::string::Format;

namespace {
    class TalkUserMock : public server::interface::TalkUser, public CallReceiver {
     public:
        virtual afl::data::Value* accessNewsrc(Modification modif, Result res, afl::base::Memory<const Selection> selections, afl::base::Memory<const int32_t> posts)
            {
                String_t modifStr = "?";
                switch (modif) {
                 case NoModification: modifStr = "no";         break;
                 case MarkRead:       modifStr = "markRead";   break;
                 case MarkUnread:     modifStr = "markUnread"; break;
                }
                String_t resultStr = "?";
                switch (res) {
                 case NoResult:       resultStr = "no";          break;
                 case GetAll:         resultStr = "getAll";      break;
                 case CheckIfAnyRead: resultStr = "anyRead";     break;
                 case CheckIfAllRead: resultStr = "allRead";     break;
                 case GetFirstRead:   resultStr = "firstRead";   break;
                 case GetFirstUnread: resultStr = "firstUnread"; break;
                }
                checkCall(Format("accessNewsrc(%s,%s,%s,%s)", modifStr, resultStr, formatSelections(selections), formatPosts(posts)));
                return consumeReturnValue<afl::data::Value*>();
            }
        virtual void watch(afl::base::Memory<const Selection> selections)
            {
                checkCall(Format("watch(%s)", formatSelections(selections)));
            }
        virtual void unwatch(afl::base::Memory<const Selection> selections)
            {
                checkCall(Format("unwatch(%s)", formatSelections(selections)));
            }
        virtual void markSeen(afl::base::Memory<const Selection> selections)
            {
                checkCall(Format("markSeen(%s)", formatSelections(selections)));
            }
        virtual afl::data::Value* getWatchedThreads(const ListParameters& params)
            {
                checkCall(Format("getWatchedThreads(%s)", formatListParameters(params)));
                return consumeReturnValue<afl::data::Value*>();
            }
        virtual afl::data::Value* getWatchedForums(const ListParameters& params)
            {
                checkCall(Format("getWatchedForums(%s)", formatListParameters(params)));
                return consumeReturnValue<afl::data::Value*>();
            }
        virtual afl::data::Value* getPostedMessages(String_t user, const ListParameters& params)
            {
                checkCall(Format("getPostedMessages(%s,%s)", user, formatListParameters(params)));
                return consumeReturnValue<afl::data::Value*>();
            }

        // Maks this function as we only have one return type
        void provideReturnValue(afl::data::Value* p)
            { CallReceiver::provideReturnValue(p); }

        // FIXME: copy from TalkForumMock
        static String_t formatListParameters(const ListParameters& params)
            {
                String_t result;
                switch (params.mode) {
                 case ListParameters::WantAll:
                    result = "all";
                    break;
                 case ListParameters::WantRange:
                    result = Format("range(%d,%d)", params.start, params.count);
                    break;
                 case ListParameters::WantSize:
                    result = "size";
                    break;
                 case ListParameters::WantMemberCheck:
                    result = Format("member(%d)", params.item);
                    break;
                }
                if (const String_t* p = params.sortKey.get()) {
                    result += Format(",sort(%s)", *p);
                }
                return result;
            }
        static String_t formatSelections(afl::base::Memory<const Selection> selections)
            {
                String_t result = "sel(";
                while (const Selection* p = selections.eat()) {
                    switch (p->scope) {
                     case ForumScope:  result += Format("forum(%d)",    p->id);            break;
                     case ThreadScope: result += Format("thread(%d)",   p->id);            break;
                     case RangeScope:  result += Format("range(%d,%d)", p->id, p->lastId); break;
                    }
                    if (!selections.empty()) {
                        result += ",";
                    }
                }
                return result + ")";
            }
        static String_t formatPosts(afl::base::Memory<const int32_t> posts)
            {
                String_t result = "post(";
                while (const int32_t* p = posts.eat()) {
                    result += Format("%d", *p);
                    if (!posts.empty()) {
                        result += ",";
                    }
                }
                return result + ")";
            }
    };
}

/** Function tests. */
void
TestServerInterfaceTalkUserServer::testIt()
{
    TalkUserMock mock;
    server::interface::TalkUserServer testee(mock);

    // accessNewsrc
    // - individual action keywords
    mock.expectCall("accessNewsrc(no,no,sel(),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC"));

    mock.expectCall("accessNewsrc(markRead,no,sel(),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC").pushBackString("SET"));

    mock.expectCall("accessNewsrc(markUnread,no,sel(),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC").pushBackString("CLEAR"));

    mock.expectCall("accessNewsrc(no,getAll,sel(),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC").pushBackString("GET"));

    mock.expectCall("accessNewsrc(no,anyRead,sel(),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC").pushBackString("ANY"));

    mock.expectCall("accessNewsrc(no,allRead,sel(),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC").pushBackString("ALL"));

    mock.expectCall("accessNewsrc(no,firstRead,sel(),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC").pushBackString("FIRSTSET"));

    mock.expectCall("accessNewsrc(no,firstUnread,sel(),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC").pushBackString("FIRSTCLEAR"));

    // - combinations of action keywords
    // -- last action wins (no combinations)
    mock.expectCall("accessNewsrc(no,firstUnread,sel(),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC").pushBackString("ALL").pushBackString("FIRSTCLEAR"));

    // -- action + return
    mock.expectCall("accessNewsrc(markRead,firstUnread,sel(),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC").pushBackString("SET").pushBackString("FIRSTCLEAR"));

    // -- action + return
    mock.expectCall("accessNewsrc(markRead,firstUnread,sel(),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC").pushBackString("FIRSTCLEAR").pushBackString("SET"));

    // - scope selection
    mock.expectCall("accessNewsrc(no,no,sel(forum(3),thread(9),range(3,4)),post(1,2,3,4))");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC")
                    .pushBackString("FORUM").pushBackInteger(3)
                    .pushBackString("THREAD").pushBackInteger(9)
                    .pushBackString("RANGE").pushBackInteger(3).pushBackInteger(4)
                    .pushBackString("POST").pushBackInteger(1).pushBackInteger(2).pushBackInteger(3).pushBackInteger(4));

    mock.expectCall("accessNewsrc(markRead,getAll,sel(forum(3),thread(9),range(3,4)),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERNEWSRC")
                    .pushBackString("FORUM").pushBackInteger(3)
                    .pushBackString("GET")
                    .pushBackString("THREAD").pushBackInteger(9)
                    .pushBackString("SET")
                    .pushBackString("RANGE").pushBackInteger(3).pushBackInteger(4));

    // -- case variation
    mock.expectCall("accessNewsrc(markRead,getAll,sel(forum(3),thread(9),range(3,4)),post())");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("usernewsrc")
                    .pushBackString("forum").pushBackInteger(3)
                    .pushBackString("get")
                    .pushBackString("thread").pushBackInteger(9)
                    .pushBackString("set")
                    .pushBackString("range").pushBackInteger(3).pushBackInteger(4));

    // - result passing
    {
        mock.expectCall("accessNewsrc(no,getAll,sel(),post(3))");
        mock.provideReturnValue(server::makeIntegerValue(1));
        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("USERNEWSRC").pushBackString("GET").pushBackString("POST").pushBackInteger(3)));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 1);
    }

    // watch/unwatch/markseen
    mock.expectCall("watch(sel())");
    testee.callVoid(Segment().pushBackString("USERWATCH"));
    mock.expectCall("watch(sel(forum(3)))");
    testee.callVoid(Segment().pushBackString("USERWATCH").pushBackString("FORUM").pushBackInteger(3));
    mock.expectCall("watch(sel(thread(9),forum(3)))");
    testee.callVoid(Segment().pushBackString("USERWATCH").pushBackString("THREAD").pushBackInteger(9).pushBackString("FORUM").pushBackInteger(3));

    mock.expectCall("unwatch(sel())");
    testee.callVoid(Segment().pushBackString("USERUNWATCH"));
    mock.expectCall("unwatch(sel(forum(3)))");
    testee.callVoid(Segment().pushBackString("USERUNWATCH").pushBackString("FORUM").pushBackInteger(3));
    mock.expectCall("unwatch(sel(thread(9),forum(3)))");
    testee.callVoid(Segment().pushBackString("USERUNWATCH").pushBackString("THREAD").pushBackInteger(9).pushBackString("FORUM").pushBackInteger(3));

    mock.expectCall("markSeen(sel())");
    testee.callVoid(Segment().pushBackString("USERMARKSEEN"));
    mock.expectCall("markSeen(sel(forum(3)))");
    testee.callVoid(Segment().pushBackString("USERMARKSEEN").pushBackString("FORUM").pushBackInteger(3));
    mock.expectCall("markSeen(sel(thread(9),forum(3)))");
    testee.callVoid(Segment().pushBackString("USERMARKSEEN").pushBackString("THREAD").pushBackInteger(9).pushBackString("FORUM").pushBackInteger(3));

    // lswatched/lspostsd
    mock.expectCall("getWatchedThreads(all)");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERLSWATCHEDTHREADS"));

    mock.expectCall("getWatchedThreads(range(5,3))");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERLSWATCHEDTHREADS").pushBackString("LIMIT").pushBackInteger(5).pushBackInteger(3));

    {
        mock.expectCall("getWatchedThreads(size)");
        mock.provideReturnValue(server::makeIntegerValue(27));
        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("USERLSWATCHEDTHREADS").pushBackString("SIZE")));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 27);
    }

    mock.expectCall("getWatchedForums(all)");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERLSWATCHEDFORUMS"));

    mock.expectCall("getWatchedForums(range(5,3))");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERLSWATCHEDFORUMS").pushBackString("LIMIT").pushBackInteger(5).pushBackInteger(3));

    {
        mock.expectCall("getWatchedForums(size)");
        mock.provideReturnValue(server::makeIntegerValue(27));
        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("USERLSWATCHEDFORUMS").pushBackString("SIZE")));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 27);
    }

    mock.expectCall("getPostedMessages(uid,all)");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERLSPOSTED").pushBackString("uid"));

    mock.expectCall("getPostedMessages(uid2,range(5,3))");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("USERLSPOSTED").pushBackString("uid2").pushBackString("LIMIT").pushBackInteger(5).pushBackInteger(3));

    {
        mock.expectCall("getPostedMessages(uid3,size)");
        mock.provideReturnValue(server::makeIntegerValue(97));
        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("USERLSPOSTED").pushBackString("uid3").pushBackString("SIZE")));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 97);
    }

    mock.checkFinish();
}

/** Error tests. */
void
TestServerInterfaceTalkUserServer::testErrors()
{
    TalkUserMock mock;
    server::interface::TalkUserServer testee(mock);

    // Invalid command
    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("foo")), std::exception);

    // Invalid USERNEWSRC
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("USERNEWSRC")
                                     .pushBackString("POST").pushBackInteger(1).pushBackInteger(2).pushBackInteger(3).pushBackInteger(4)
                                     .pushBackString("SET")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("USERNEWSRC").pushBackString("THREAD")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("USERNEWSRC").pushBackString("FOO").pushBackInteger(1)), std::exception);

    // ComposableCommandHandler personality
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_EQUALS(testee.handleCommand("huhu", args, p), false);
}

/** Roundtrip tests. */
void
TestServerInterfaceTalkUserServer::testRoundtrip()
{
    using server::interface::TalkUser;
    TalkUserMock mock;
    server::interface::TalkUserServer level1(mock);
    server::interface::TalkUserClient level2(level1);
    server::interface::TalkUserServer level3(level2);
    server::interface::TalkUserClient level4(level3);

    // accessNewsrc
    // - individual action keywords
    mock.expectCall("accessNewsrc(no,no,sel(),post())");
    mock.provideReturnValue(0);
    level4.accessNewsrc(TalkUser::NoModification, TalkUser::NoResult, afl::base::Nothing, afl::base::Nothing);

    mock.expectCall("accessNewsrc(markRead,no,sel(),post())");
    mock.provideReturnValue(0);
    level4.accessNewsrc(TalkUser::MarkRead, TalkUser::NoResult, afl::base::Nothing, afl::base::Nothing);

    mock.expectCall("accessNewsrc(markUnread,no,sel(),post())");
    mock.provideReturnValue(0);
    level4.accessNewsrc(TalkUser::MarkUnread, TalkUser::NoResult, afl::base::Nothing, afl::base::Nothing);

    mock.expectCall("accessNewsrc(no,getAll,sel(),post())");
    mock.provideReturnValue(0);
    level4.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, afl::base::Nothing);

    mock.expectCall("accessNewsrc(no,anyRead,sel(),post())");
    mock.provideReturnValue(0);
    level4.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAnyRead, afl::base::Nothing, afl::base::Nothing);

    mock.expectCall("accessNewsrc(no,allRead,sel(),post())");
    mock.provideReturnValue(0);
    level4.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAllRead, afl::base::Nothing, afl::base::Nothing);

    mock.expectCall("accessNewsrc(no,firstRead,sel(),post())");
    mock.provideReturnValue(0);
    level4.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstRead, afl::base::Nothing, afl::base::Nothing);

    mock.expectCall("accessNewsrc(no,firstUnread,sel(),post())");
    mock.provideReturnValue(0);
    level4.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstUnread, afl::base::Nothing, afl::base::Nothing);

    // - combinations of action keywords
    mock.expectCall("accessNewsrc(markRead,firstUnread,sel(),post())");
    mock.provideReturnValue(0);
    level4.accessNewsrc(TalkUser::MarkRead, TalkUser::GetFirstUnread, afl::base::Nothing, afl::base::Nothing);

    // - scope selection
    {
        static const TalkUser::Selection sel[] = {
            { TalkUser::ForumScope, 3, 0 },
            { TalkUser::ThreadScope, 9, 0 },
            { TalkUser::RangeScope, 3, 4 },
        };
        static const int32_t posts[] = { 1, 2, 3, 4 };

        mock.expectCall("accessNewsrc(no,no,sel(forum(3),thread(9),range(3,4)),post(1,2,3,4))");
        mock.provideReturnValue(0);
        level4.accessNewsrc(TalkUser::NoModification, TalkUser::NoResult, sel, posts);

        mock.expectCall("accessNewsrc(markRead,getAll,sel(forum(3),thread(9),range(3,4)),post())");
        mock.provideReturnValue(0);
        level4.accessNewsrc(TalkUser::MarkRead, TalkUser::GetAll, sel, afl::base::Nothing);
    }

    // - result passing
    {
        static const int32_t posts[] = { 3 };
        mock.expectCall("accessNewsrc(no,getAll,sel(),post(3))");
        mock.provideReturnValue(server::makeIntegerValue(1));
        std::auto_ptr<afl::data::Value> p(level4.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, posts));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 1);
    }

    // watch/unwatch/markseen
    {
        static const TalkUser::Selection sel[] = {
            { TalkUser::ThreadScope, 10, 0 },
            { TalkUser::ForumScope, 2, 0 },
        };
        mock.expectCall("watch(sel())");
        level4.watch(afl::base::Nothing);
        mock.expectCall("watch(sel(thread(10),forum(2)))");
        level4.watch(sel);

        mock.expectCall("unwatch(sel())");
        level4.unwatch(afl::base::Nothing);
        mock.expectCall("unwatch(sel(thread(10),forum(2)))");
        level4.unwatch(sel);

        mock.expectCall("markSeen(sel())");
        level4.markSeen(afl::base::Nothing);
        mock.expectCall("markSeen(sel(thread(10),forum(2)))");
        level4.markSeen(sel);
    }

    // lswatched/lsposted
    TalkUser::ListParameters lpLimit;
    lpLimit.mode = lpLimit.WantRange;
    lpLimit.start = 5;
    lpLimit.count = 3;

    TalkUser::ListParameters lpSize;
    lpSize.mode = lpSize.WantSize;

    mock.expectCall("getWatchedThreads(all)");
    mock.provideReturnValue(0);
    level4.getWatchedThreads(TalkUser::ListParameters());

    mock.expectCall("getWatchedThreads(range(5,3))");
    mock.provideReturnValue(0);
    level4.getWatchedThreads(lpLimit);

    {
        mock.expectCall("getWatchedThreads(size)");
        mock.provideReturnValue(server::makeIntegerValue(27));
        std::auto_ptr<afl::data::Value> p(level4.getWatchedThreads(lpSize));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 27);
    }

    mock.expectCall("getWatchedForums(all)");
    mock.provideReturnValue(0);
    level4.getWatchedForums(TalkUser::ListParameters());

    mock.expectCall("getWatchedForums(range(5,3))");
    mock.provideReturnValue(0);
    level4.getWatchedForums(lpLimit);

    {
        mock.expectCall("getWatchedForums(size)");
        mock.provideReturnValue(server::makeIntegerValue(27));
        std::auto_ptr<afl::data::Value> p(level4.getWatchedForums(lpSize));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 27);
    }

    mock.expectCall("getPostedMessages(a,all)");
    mock.provideReturnValue(0);
    level4.getPostedMessages("a", TalkUser::ListParameters());

    mock.expectCall("getPostedMessages(b,range(5,3))");
    mock.provideReturnValue(0);
    level4.getPostedMessages("b", lpLimit);

    {
        mock.expectCall("getPostedMessages(c,size)");
        mock.provideReturnValue(server::makeIntegerValue(99));
        std::auto_ptr<afl::data::Value> p(level4.getPostedMessages("c", lpSize));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 99);
    }

    mock.checkFinish();
}

