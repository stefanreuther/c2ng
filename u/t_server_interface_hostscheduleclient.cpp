/**
  *  \file u/t_server_interface_hostscheduleclient.cpp
  *  \brief Test for server::interface::HostScheduleClient
  */

#include "server/interface/hostscheduleclient.hpp"

#include "t_server_interface.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/except/invaliddataexception.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Segment;

void
TestServerInterfaceHostScheduleClient::testModify()
{
    using server::interface::HostSchedule;
    afl::test::CommandHandler mock("testModify");
    server::interface::HostScheduleClient testee(mock);

    // add/replace/modify
    // - pathological cases
    mock.expectCall("SCHEDULEADD, 3");
    mock.provideNewResult(0);
    testee.add(3, HostSchedule::Schedule());

    mock.expectCall("SCHEDULESET, 7");
    mock.provideNewResult(0);
    testee.replace(7, HostSchedule::Schedule());

    mock.expectCall("SCHEDULEMOD, 9");
    mock.provideNewResult(0);
    testee.modify(9, HostSchedule::Schedule());

    // - types
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Stopped;
        mock.expectCall("SCHEDULEADD, 12, STOP");
        mock.provideNewResult(0);
        testee.add(12, sch);
    }
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Weekly;
        sch.weekdays = 9;
        mock.expectCall("SCHEDULEMOD, 7, WEEKLY, 9");
        mock.provideNewResult(0);
        testee.modify(7, sch);
    }
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Daily;
        sch.interval = 4;
        mock.expectCall("SCHEDULESET, 1, DAILY, 4");
        mock.provideNewResult(0);
        testee.replace(1, sch);
    }
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Quick;
        mock.expectCall("SCHEDULESET, 75, ASAP");
        mock.provideNewResult(0);
        testee.replace(75, sch);
    }
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Manual;
        mock.expectCall("SCHEDULEADD, 6, MANUAL");
        mock.provideNewResult(0);
        testee.add(6, sch);
    }

    // - daytime
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Weekly;  // not setting weekdays means it's "WEEKLY 0"
        sch.daytime = 360;                // 6:00
        mock.expectCall("SCHEDULEADD, 2, WEEKLY, 0, DAYTIME, 360");
        mock.provideNewResult(0);
        testee.add(2, sch);
    }

    // - early/noearly
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Stopped;
        sch.hostEarly = true;
        mock.expectCall("SCHEDULEADD, 8, STOP, EARLY");
        mock.provideNewResult(0);
        testee.add(8, sch);
    }
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Daily;  // not setting interval means it's "DAILY 0"
        sch.hostEarly = false;
        mock.expectCall("SCHEDULEADD, 8, DAILY, 0, NOEARLY");
        mock.provideNewResult(0);
        testee.add(8, sch);
    }

    // - hostDelay
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Weekly;
        sch.weekdays = 19;
        sch.hostDelay = 20;
        mock.expectCall("SCHEDULEMOD, 7, WEEKLY, 19, DELAY, 20");
        mock.provideNewResult(0);
        testee.modify(7, sch);
    }

    // - hostLimit
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Daily;
        sch.interval = 3;
        sch.hostLimit = 300;
        mock.expectCall("SCHEDULEMOD, 7, DAILY, 3, LIMIT, 300");
        mock.provideNewResult(0);
        testee.modify(7, sch);
    }

    // - conditions
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Weekly;
        sch.weekdays = 0;
        sch.condition = HostSchedule::None;
        mock.expectCall("SCHEDULEMOD, 7, WEEKLY, 0, FOREVER");
        mock.provideNewResult(0);
        testee.modify(7, sch);
    }
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Stopped;
        sch.condition = HostSchedule::Time;
        sch.conditionTime = 140000;
        mock.expectCall("SCHEDULEADD, 4, STOP, UNTILTIME, 140000");
        mock.provideNewResult(0);
        testee.add(4, sch);
    }
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Stopped;
        sch.condition = HostSchedule::Turn;
        sch.conditionTurn = 2;
        mock.expectCall("SCHEDULEADD, 4, STOP, UNTILTURN, 2");
        mock.provideNewResult(0);
        testee.add(4, sch);
    }

    // - combination
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Weekly;
        sch.weekdays = 3;
        sch.interval = 2;
        sch.daytime = 900;
        sch.hostEarly = true;
        sch.hostDelay = 15;
        sch.hostLimit = 200;
        sch.condition = HostSchedule::Turn;
        sch.conditionTurn = 42;
        sch.conditionTime = 14141414;
        mock.expectCall("SCHEDULEADD, 16, WEEKLY, 3, DAYTIME, 900, EARLY, DELAY, 15, LIMIT, 200, UNTILTURN, 42");
        mock.provideNewResult(0);
        testee.add(16, sch);
    }
    mock.checkFinish();
}

void
TestServerInterfaceHostScheduleClient::testOther()
{
    using server::interface::HostSchedule;
    afl::test::CommandHandler mock("testOther");
    server::interface::HostScheduleClient testee(mock);

    // getAll
    // - empty
    {
        std::vector<HostSchedule::Schedule> result;
        mock.expectCall("SCHEDULELIST, 5");
        mock.provideNewResult(0);
        testee.getAll(5, result);
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // - empty vector
    {
        std::vector<HostSchedule::Schedule> result;
        mock.expectCall("SCHEDULELIST, 6");
        mock.provideNewResult(new VectorValue(Vector::create()));
        testee.getAll(6, result);
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // - two elements
    {
        Hash::Ref_t a = Hash::create();
        a->setNew("type", server::makeIntegerValue(2));        // DAILY
        a->setNew("hostEarly", server::makeIntegerValue(0));

        Hash::Ref_t b = Hash::create();
        b->setNew("type",      server::makeIntegerValue(1));        // WEEKLY
        b->setNew("weekdays",  server::makeIntegerValue(19));
        b->setNew("interval",  server::makeIntegerValue(6));
        b->setNew("daytime",   server::makeIntegerValue(1400));
        b->setNew("hostEarly", server::makeIntegerValue(1));
        b->setNew("hostDelay", server::makeIntegerValue(25));
        b->setNew("hostLimit", server::makeIntegerValue(150));
        b->setNew("condition", server::makeIntegerValue(2));   // UNTILTIME
        b->setNew("condTurn",  server::makeIntegerValue(80));
        b->setNew("condTime",  server::makeIntegerValue(150000003));

        Vector::Ref_t v = Vector::create();
        v->pushBackNew(new HashValue(a));
        v->pushBackNew(new HashValue(b));

        std::vector<HostSchedule::Schedule> result;
        mock.expectCall("SCHEDULELIST, 250");
        mock.provideNewResult(new VectorValue(v));
        testee.getAll(250, result);
        TS_ASSERT_EQUALS(result.size(), 2U);

        TS_ASSERT(result[0].type.isSame(HostSchedule::Daily));
        TS_ASSERT(!result[0].weekdays.isValid());
        TS_ASSERT(!result[0].interval.isValid());
        TS_ASSERT(!result[0].daytime.isValid());
        TS_ASSERT(result[0].hostEarly.isSame(false));
        TS_ASSERT(!result[0].hostDelay.isValid());
        TS_ASSERT(!result[0].hostLimit.isValid());
        TS_ASSERT(!result[0].condition.isValid());
        TS_ASSERT(!result[0].conditionTurn.isValid());
        TS_ASSERT(!result[0].conditionTime.isValid());

        TS_ASSERT(result[1].type.isSame(HostSchedule::Weekly));
        TS_ASSERT(result[1].weekdays.isSame(19));
        TS_ASSERT(result[1].interval.isSame(6));
        TS_ASSERT(result[1].daytime.isSame(1400));
        TS_ASSERT(result[1].hostEarly.isSame(1));
        TS_ASSERT(result[1].hostDelay.isSame(25));
        TS_ASSERT(result[1].hostLimit.isSame(150));
        TS_ASSERT(result[1].condition.isSame(HostSchedule::Time));
        TS_ASSERT(result[1].conditionTurn.isSame(80));
        TS_ASSERT(result[1].conditionTime.isSame(150000003));
    }

    // drop
    mock.expectCall("SCHEDULEDROP, 13");
    mock.provideNewResult(0);
    testee.drop(13);

    // preview
    {
        afl::data::IntegerList_t list;
        mock.expectCall("SCHEDULESHOW, 8");
        mock.provideNewResult(new VectorValue(Vector::create(afl::data::Segment().pushBackInteger(1000).pushBackInteger(2000))));
        testee.preview(8, afl::base::Nothing, afl::base::Nothing, list);
        TS_ASSERT_EQUALS(list.size(), 2U);
        TS_ASSERT_EQUALS(list[0], 1000);
        TS_ASSERT_EQUALS(list[1], 2000);
    }
    {
        afl::data::IntegerList_t list;
        mock.expectCall("SCHEDULESHOW, 8, TIMELIMIT, 900000");
        mock.provideNewResult(new VectorValue(Vector::create(afl::data::Segment().pushBackInteger(800000))));
        testee.preview(8, 900000, afl::base::Nothing, list);
        TS_ASSERT_EQUALS(list.size(), 1U);
        TS_ASSERT_EQUALS(list[0], 800000);
    }
    {
        afl::data::IntegerList_t list;
        mock.expectCall("SCHEDULESHOW, 18, TURNLIMIT, 5");
        mock.provideNewResult(new VectorValue(Vector::create(afl::data::Segment().pushBackInteger(99))));
        testee.preview(18, afl::base::Nothing, 5, list);
        TS_ASSERT_EQUALS(list.size(), 1U);
        TS_ASSERT_EQUALS(list[0], 99);
    }
    {
        afl::data::IntegerList_t list;
        mock.expectCall("SCHEDULESHOW, 18, TIMELIMIT, 10, TURNLIMIT, 7");
        mock.provideNewResult(new VectorValue(Vector::create()));
        testee.preview(18, 10, 7, list);
        TS_ASSERT_EQUALS(list.size(), 0U);
    }

    mock.checkFinish();
}

void
TestServerInterfaceHostScheduleClient::testErrors()
{
    using server::interface::HostSchedule;
    afl::test::CommandHandler mock("testErrors");
    server::interface::HostScheduleClient testee(mock);

    // Bad type
    {
        Hash::Ref_t a = Hash::create();
        a->setNew("type", server::makeIntegerValue(99));
        mock.expectCall("SCHEDULELIST, 82");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackNew(new HashValue(a)))));

        std::vector<HostSchedule::Schedule> result;
        TS_ASSERT_THROWS(testee.getAll(82, result), afl::except::InvalidDataException);
    }

    // Bad condition
    {
        Hash::Ref_t a = Hash::create();
        a->setNew("condition", server::makeStringValue("meh"));
        mock.expectCall("SCHEDULELIST, 155");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackNew(new HashValue(a)))));

        std::vector<HostSchedule::Schedule> result;
        TS_ASSERT_THROWS(testee.getAll(155, result), afl::except::InvalidDataException);
    }
}
