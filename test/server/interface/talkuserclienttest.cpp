/**
  *  \file test/server/interface/talkuserclienttest.cpp
  *  \brief Test for server::interface::TalkUserClient
  */

#include "server/interface/talkuserclient.hpp"

#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"
#include <memory>

/** Test it. */
AFL_TEST("server.interface.TalkUserClient", a)
{
    using server::interface::TalkUser;

    afl::test::CommandHandler mock(a);
    server::interface::TalkUserClient testee(mock);

    // USERNEWSRC
    // - trivial/no-ops
    mock.expectCall("USERNEWSRC");
    mock.provideNewResult(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::NoResult, afl::base::Nothing, afl::base::Nothing);

    static const int32_t posts[] = {5,7,9};
    mock.expectCall("USERNEWSRC, POST, 5, 7, 9");
    mock.provideNewResult(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::NoResult, afl::base::Nothing, posts);

    // - results
    mock.expectCall("USERNEWSRC, GET, POST, 5, 7, 9");
    mock.provideNewResult(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, posts);

    mock.expectCall("USERNEWSRC, ANY, POST, 5, 7, 9");
    mock.provideNewResult(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAnyRead, afl::base::Nothing, posts);

    mock.expectCall("USERNEWSRC, ALL, POST, 5, 7, 9");
    mock.provideNewResult(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAllRead, afl::base::Nothing, posts);

    mock.expectCall("USERNEWSRC, FIRSTSET, POST, 5, 7, 9");
    mock.provideNewResult(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstRead, afl::base::Nothing, posts);

    mock.expectCall("USERNEWSRC, FIRSTCLEAR, POST, 5, 7, 9");
    mock.provideNewResult(0);
    testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstUnread, afl::base::Nothing, posts);

    // - modifications
    mock.expectCall("USERNEWSRC, SET, POST, 5, 7, 9");
    mock.provideNewResult(0);
    testee.accessNewsrc(TalkUser::MarkRead, TalkUser::NoResult, afl::base::Nothing, posts);

    mock.expectCall("USERNEWSRC, CLEAR, POST, 5, 7, 9");
    mock.provideNewResult(0);
    testee.accessNewsrc(TalkUser::MarkUnread, TalkUser::NoResult, afl::base::Nothing, posts);

    // - combinations
    mock.expectCall("USERNEWSRC, SET, GET, POST, 5, 7, 9");
    mock.provideNewResult(0);
    testee.accessNewsrc(TalkUser::MarkRead, TalkUser::GetAll, afl::base::Nothing, posts);

    // - selection
    {
        static const TalkUser::Selection sel[] = {
            { TalkUser::ForumScope, 3, 0 },
            { TalkUser::ThreadScope, 8, 0 },
            { TalkUser::RangeScope, 10, 15 },
            { TalkUser::ForumScope, 9, 0 },
        };
        mock.expectCall("USERNEWSRC, GET, FORUM, 3, THREAD, 8, RANGE, 10, 15, FORUM, 9");
        mock.provideNewResult(0);
        testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, sel, afl::base::Nothing);

        mock.expectCall("USERNEWSRC, GET, FORUM, 3, THREAD, 8, RANGE, 10, 15, FORUM, 9, POST, 5, 7, 9");
        mock.provideNewResult(0);
        testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, sel, posts);
    }

    // - return value
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("USERNEWSRC, ANY, POST, 5, 7, 9");
        mock.provideNewResult(server::makeStringValue("111"));
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAnyRead, afl::base::Nothing, posts));

        // This command relies on being able to interpret a result as number or string as needed.
        a.checkEqual("01. result", server::toString(p.get()), "111");
        a.checkEqual("02. result", server::toInteger(p.get()), 111);
    }

    // USERWATCH/USERUNWATCH/USERMARKSEEN
    mock.expectCall("USERWATCH");
    mock.provideNewResult(0);
    testee.watch(afl::base::Nothing);

    mock.expectCall("USERUNWATCH");
    mock.provideNewResult(0);
    testee.unwatch(afl::base::Nothing);

    mock.expectCall("USERMARKSEEN");
    mock.provideNewResult(0);
    testee.markSeen(afl::base::Nothing);

    {
        static const TalkUser::Selection sel[] = {
            { TalkUser::ForumScope, 3, 0 },
            { TalkUser::ThreadScope, 8, 0 },
        };
        mock.expectCall("USERWATCH, FORUM, 3, THREAD, 8");
        mock.provideNewResult(0);
        testee.watch(sel);

        mock.expectCall("USERUNWATCH, FORUM, 3, THREAD, 8");
        mock.provideNewResult(0);
        testee.unwatch(sel);

        mock.expectCall("USERMARKSEEN, FORUM, 3, THREAD, 8");
        mock.provideNewResult(0);
        testee.markSeen(sel);
    }

    // USERLSWATCHEDTHREADS/USERLSWATCHEDFORUMS/USERLSPOSTED
    mock.expectCall("USERLSWATCHEDTHREADS");
    mock.provideNewResult(0);
    testee.getWatchedThreads(TalkUser::ListParameters());

    mock.expectCall("USERLSWATCHEDFORUMS");
    mock.provideNewResult(0);
    testee.getWatchedForums(TalkUser::ListParameters());

    mock.expectCall("USERLSPOSTED, fred");
    mock.provideNewResult(0);
    testee.getPostedMessages("fred", TalkUser::ListParameters());

    {
        TalkUser::ListParameters params;
        params.mode = TalkUser::ListParameters::WantRange;
        params.start = 20;
        params.count = 10;
        params.sortKey = "name";

        mock.expectCall("USERLSWATCHEDTHREADS, LIMIT, 20, 10, SORT, name");
        mock.provideNewResult(0);
        testee.getWatchedThreads(params);

        mock.expectCall("USERLSWATCHEDFORUMS, LIMIT, 20, 10, SORT, name");
        mock.provideNewResult(0);
        testee.getWatchedForums(params);

        mock.expectCall("USERLSPOSTED, wilma, LIMIT, 20, 10, SORT, name");
        mock.provideNewResult(0);
        testee.getPostedMessages("wilma", params);
    }

    // USERLSCROSS
    mock.expectCall("USERLSCROSS");
    mock.provideNewResult(0);
    testee.getCrosspostToGameCandidates(TalkUser::ListParameters());

    {
        TalkUser::ListParameters params;
        params.mode = TalkUser::ListParameters::WantRange;
        params.start = 20;
        params.count = 10;
        params.sortKey = "key";

        mock.expectCall("USERLSCROSS, LIMIT, 20, 10, SORT, key");
        mock.provideNewResult(0);
        testee.getCrosspostToGameCandidates(params);
    }

    // Return value passing
    {
        mock.expectCall("USERNEWSRC, GET, POST, 5, 7, 9");
        mock.provideNewResult(server::makeIntegerValue(4711));
        std::auto_ptr<afl::data::Value> p(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, posts));
        a.checkNonNull("11. result", p.get());
        a.checkEqual("12. result", server::toInteger(p.get()), 4711);
    }
    {
        mock.expectCall("USERLSWATCHEDTHREADS");
        mock.provideNewResult(server::makeIntegerValue(7));
        std::auto_ptr<afl::data::Value> p(testee.getWatchedThreads(TalkUser::ListParameters()));
        a.checkNonNull("13. result", p.get());
        a.checkEqual("14. result", server::toInteger(p.get()), 7);
    }

    mock.checkFinish();
}
