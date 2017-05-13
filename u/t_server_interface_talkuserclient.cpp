/**
  *  \file u/t_server_interface_talkuserclient.cpp
  *  \brief Test for server::interface::TalkUserClient
  */

#include <memory>
#include "server/interface/talkuserclient.hpp"

#include "t_server_interface.hpp"
#include "u/helper/commandhandlermock.hpp"
#include "server/types.hpp"

/** Test it. */
void
TestServerInterfaceTalkUserClient::testIt()
{
    using server::interface::TalkUser;

    CommandHandlerMock mock;
    server::interface::TalkUserClient testee(mock);

    // USERNEWSRC
    // - trivial/no-ops
    mock.expectCall("USERNEWSRC");
    mock.provideReturnValue(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::NoResult, afl::base::Nothing, afl::base::Nothing);

    static const int32_t posts[] = {5,7,9};
    mock.expectCall("USERNEWSRC|POST|5|7|9");
    mock.provideReturnValue(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::NoResult, afl::base::Nothing, posts);

    // - results
    mock.expectCall("USERNEWSRC|GET|POST|5|7|9");
    mock.provideReturnValue(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, posts);

    mock.expectCall("USERNEWSRC|ANY|POST|5|7|9");
    mock.provideReturnValue(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAnyRead, afl::base::Nothing, posts);

    mock.expectCall("USERNEWSRC|ALL|POST|5|7|9");
    mock.provideReturnValue(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAllRead, afl::base::Nothing, posts);

    mock.expectCall("USERNEWSRC|FIRSTSET|POST|5|7|9");
    mock.provideReturnValue(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstRead, afl::base::Nothing, posts);

    mock.expectCall("USERNEWSRC|FIRSTCLEAR|POST|5|7|9");
    mock.provideReturnValue(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstUnread, afl::base::Nothing, posts);

    // - modifications
    mock.expectCall("USERNEWSRC|SET|POST|5|7|9");
    mock.provideReturnValue(0);
    testee.accessNewsrc(TalkUser::MarkRead, TalkUser::NoResult, afl::base::Nothing, posts);

    mock.expectCall("USERNEWSRC|CLEAR|POST|5|7|9");
    mock.provideReturnValue(0);
    testee.accessNewsrc(TalkUser::MarkUnread, TalkUser::NoResult, afl::base::Nothing, posts);

    // - combinations
    mock.expectCall("USERNEWSRC|SET|GET|POST|5|7|9");
    mock.provideReturnValue(0);
    testee.accessNewsrc(TalkUser::MarkRead, TalkUser::GetAll, afl::base::Nothing, posts);

    // - selection
    {
        static const TalkUser::Selection sel[] = {
            { TalkUser::ForumScope, 3, 0 },
            { TalkUser::ThreadScope, 8, 0 },
            { TalkUser::RangeScope, 10, 15 },
            { TalkUser::ForumScope, 9, 0 },
        };
        mock.expectCall("USERNEWSRC|GET|FORUM|3|THREAD|8|RANGE|10|15|FORUM|9");
        mock.provideReturnValue(0);
        testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, sel, afl::base::Nothing);

        mock.expectCall("USERNEWSRC|GET|FORUM|3|THREAD|8|RANGE|10|15|FORUM|9|POST|5|7|9");
        mock.provideReturnValue(0);
        testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, sel, posts);
    }

    // - return value
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("USERNEWSRC|ANY|POST|5|7|9");
        mock.provideReturnValue(server::makeStringValue("111"));
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAnyRead, afl::base::Nothing, posts));

        // This command relies on being able to interpret a result as number or string as needed.
        TS_ASSERT_EQUALS(server::toString(p.get()), "111");
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 111);
    }

    // USERWATCH/USERUNWATCH/USERMARKSEEN
    mock.expectCall("USERWATCH");
    mock.provideReturnValue(0);
    testee.watch(afl::base::Nothing);

    mock.expectCall("USERUNWATCH");
    mock.provideReturnValue(0);
    testee.unwatch(afl::base::Nothing);

    mock.expectCall("USERMARKSEEN");
    mock.provideReturnValue(0);
    testee.markSeen(afl::base::Nothing);

    {
        static const TalkUser::Selection sel[] = {
            { TalkUser::ForumScope, 3, 0 },
            { TalkUser::ThreadScope, 8, 0 },
        };
        mock.expectCall("USERWATCH|FORUM|3|THREAD|8");
        mock.provideReturnValue(0);
        testee.watch(sel);

        mock.expectCall("USERUNWATCH|FORUM|3|THREAD|8");
        mock.provideReturnValue(0);
        testee.unwatch(sel);

        mock.expectCall("USERMARKSEEN|FORUM|3|THREAD|8");
        mock.provideReturnValue(0);
        testee.markSeen(sel);
    }

    // USERLSWATCHEDTHREADS/USERLSWATCHEDFORUMS/USERLSPOSTED
    mock.expectCall("USERLSWATCHEDTHREADS");
    mock.provideReturnValue(0);
    testee.getWatchedThreads(TalkUser::ListParameters());

    mock.expectCall("USERLSWATCHEDFORUMS");
    mock.provideReturnValue(0);
    testee.getWatchedForums(TalkUser::ListParameters());

    mock.expectCall("USERLSPOSTED|fred");
    mock.provideReturnValue(0);
    testee.getPostedMessages("fred", TalkUser::ListParameters());

    {
        TalkUser::ListParameters params;
        params.mode = TalkUser::ListParameters::WantRange;
        params.start = 20;
        params.count = 10;
        params.sortKey = "name";

        mock.expectCall("USERLSWATCHEDTHREADS|LIMIT|20|10|SORT|name");
        mock.provideReturnValue(0);
        testee.getWatchedThreads(params);

        mock.expectCall("USERLSWATCHEDFORUMS|LIMIT|20|10|SORT|name");
        mock.provideReturnValue(0);
        testee.getWatchedForums(params);

        mock.expectCall("USERLSPOSTED|wilma|LIMIT|20|10|SORT|name");
        mock.provideReturnValue(0);
        testee.getPostedMessages("wilma", params);
    }

    // Return value passing
    {
        mock.expectCall("USERNEWSRC|GET|POST|5|7|9");
        mock.provideReturnValue(server::makeIntegerValue(4711));
        std::auto_ptr<afl::data::Value> p(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, posts));
        TS_ASSERT(p.get() != 0);
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 4711);
    }
    {
        mock.expectCall("USERLSWATCHEDTHREADS");
        mock.provideReturnValue(server::makeIntegerValue(7));
        std::auto_ptr<afl::data::Value> p(testee.getWatchedThreads(TalkUser::ListParameters()));
        TS_ASSERT(p.get() != 0);
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 7);
    }

    mock.checkFinish();
}

