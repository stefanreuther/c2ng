/**
  *  \file test/server/interface/hostscheduleservertest.cpp
  *  \brief Test for server::interface::HostScheduleServer
  */

#include "server/interface/hostscheduleserver.hpp"

#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/hostscheduleclient.hpp"
#include <stdexcept>

using afl::data::Access;
using afl::data::Segment;
using afl::string::Format;
using server::Value_t;
using server::interface::HostSchedule;

namespace {
    class HostScheduleMock : public afl::test::CallReceiver, public HostSchedule {
     public:
        HostScheduleMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual void add(int32_t gameId, const Schedule& sched)
            { checkCall(Format("add(%d,%s)", gameId, formatSchedule(sched))); }
        virtual void replace(int32_t gameId, const Schedule& sched)
            { checkCall(Format("replace(%d,%s)", gameId, formatSchedule(sched))); }
        virtual void modify(int32_t gameId, const Schedule& sched)
            { checkCall(Format("modify(%d,%s)", gameId, formatSchedule(sched))); }
        virtual void getAll(int32_t gameId, std::vector<Schedule>& result)
            {
                checkCall(Format("getAll(%d)", gameId));
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    result.push_back(consumeReturnValue<Schedule>());
                }
            }
        virtual void drop(int32_t gameId)
            { checkCall(Format("drop(%d)", gameId)); }
        virtual void preview(int32_t gameId,
                             afl::base::Optional<server::Time_t> timeLimit,
                             afl::base::Optional<int32_t> turnLimit,
                             afl::data::IntegerList_t& result)
            {
                checkCall(Format("preview(%d,%d,%d)", gameId, timeLimit.orElse(-1), turnLimit.orElse(-1)));
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    result.push_back(consumeReturnValue<int>());
                }
            }

        static String_t formatSchedule(const Schedule& sch)
            {
                String_t result = "[";
                addProperty(result, "type", sch.type);
                addProperty(result, "weekdays", sch.weekdays);
                addProperty(result, "interval", sch.interval);
                addProperty(result, "daytime", sch.daytime);
                addProperty(result, "hostEarly", sch.hostEarly);
                addProperty(result, "hostDelay", sch.hostDelay);
                addProperty(result, "hostLimit", sch.hostLimit);
                addProperty(result, "condition", sch.condition);
                addProperty(result, "conditionTurn", sch.conditionTurn);
                addProperty(result, "conditionTime", sch.conditionTime);
                result += "]";
                return result;
            }

        template<typename T> static void addProperty(String_t& out, const char* name, afl::base::Optional<T> value)
            {
                if (const T* p = value.get()) {
                    if (out.size() > 1) {
                        out += ",";
                    }
                    out += name;
                    out += "=";
                    out += formatValue(*p);
                }
            }

        static String_t formatValue(int32_t i)
            { return Format("%d", i); }
        static String_t formatValue(bool b)
            { return b ? "t" : "f"; }
        static String_t formatValue(Type t)
            {
                switch (t) {
                 case Stopped: return "stopped";
                 case Weekly:  return "weekly";
                 case Daily:   return "daily";
                 case Quick:   return "asap";
                 case Manual:  return "manual";
                }
                return "?";
            }
        static String_t formatValue(Condition c)
            {
                switch (c) {
                 case None: return "none";
                 case Turn: return "turn";
                 case Time: return "time";
                }
                return "?";
            }
    };
}

/** Test general cases. */
AFL_TEST("server.interface.HostScheduleServer:commands", a)
{
    HostScheduleMock mock(a);
    server::interface::HostScheduleServer testee(mock);

    // SCHEDULEADD [testing all keywords]
    mock.expectCall("add(9,[])");
    AFL_CHECK_SUCCEEDS(a("01. scheduleadd"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(9)));

    mock.expectCall("add(3,[type=weekly,weekdays=17,hostEarly=t])");
    AFL_CHECK_SUCCEEDS(a("11. scheduleadd"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(3).pushBackString("WEEKLY").pushBackInteger(17).pushBackString("EARLY")));

    mock.expectCall("add(42,[type=stopped,condition=time,conditionTime=1900000])");
    AFL_CHECK_SUCCEEDS(a("21. scheduleadd"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(42).pushBackString("STOP").pushBackString("UNTILTIME").pushBackInteger(1900000)));

    mock.expectCall("add(42,[type=stopped,condition=time,conditionTime=1900000])");
    AFL_CHECK_SUCCEEDS(a("31. scheduleadd"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(42).pushBackString("UNTILTIME").pushBackInteger(1900000).pushBackString("STOP")));

    mock.expectCall("add(17,[type=daily,interval=3,hostDelay=90,hostLimit=100])");
    AFL_CHECK_SUCCEEDS(a("41. scheduleadd"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(17).pushBackString("DAILY").pushBackInteger(3).
                                                             pushBackString("LIMIT").pushBackInteger(100).pushBackString("DELAY").pushBackInteger(90)));

    mock.expectCall("add(6,[type=asap,condition=turn,conditionTurn=20])");
    AFL_CHECK_SUCCEEDS(a("51. scheduleadd"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(6).pushBackString("UNTILTURN").pushBackInteger(20).pushBackString("ASAP")));

    mock.expectCall("add(12,[type=manual,hostEarly=f,condition=none])");
    AFL_CHECK_SUCCEEDS(a("61. scheduleadd"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(12).pushBackString("NOEARLY").pushBackString("MANUAL").pushBackString("FOREVER")));

    mock.expectCall("add(3,[type=weekly,weekdays=31,daytime=1000])");
    AFL_CHECK_SUCCEEDS(a("71. scheduleadd"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(3).pushBackString("WEEKLY").pushBackInteger(31).
                                                             pushBackString("DAYTIME").pushBackInteger(1000)));

    // SCHEDULESET [parts]
    mock.expectCall("replace(7,[])");
    AFL_CHECK_SUCCEEDS(a("81. scheduleset"), testee.callVoid(Segment().pushBackString("SCHEDULESET").pushBackInteger(7)));

    mock.expectCall("replace(2,[type=weekly,weekdays=9])");
    AFL_CHECK_SUCCEEDS(a("91. scheduleset"), testee.callVoid(Segment().pushBackString("SCHEDULESET").pushBackInteger(2).pushBackString("WEEKLY").pushBackInteger(9)));

    // SCHEDULEMOD [parts]
    mock.expectCall("modify(7,[])");
    AFL_CHECK_SUCCEEDS(a("101. schedulemod"), testee.callVoid(Segment().pushBackString("SCHEDULEMOD").pushBackInteger(7)));

    mock.expectCall("modify(2,[type=weekly,weekdays=9])");
    AFL_CHECK_SUCCEEDS(a("111. schedulemod"), testee.callVoid(Segment().pushBackString("SCHEDULEMOD").pushBackInteger(2).pushBackString("WEEKLY").pushBackInteger(9)));

    // SCHEDULELIST
    // - empty
    {
        mock.expectCall("getAll(12)");
        mock.provideReturnValue(0);
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("SCHEDULELIST").pushBackInteger(12)));
        a.checkEqual("121. schedulelist", Access(p).getArraySize(), 0U);
    }
    // - nonempty
    {
        HostSchedule::Schedule sa;
        sa.type = HostSchedule::Weekly;
        sa.weekdays = 24;
        sa.daytime = 300;
        sa.condition = HostSchedule::Turn;
        sa.conditionTurn = 20;

        HostSchedule::Schedule sb;
        sb.type = HostSchedule::Daily;
        sb.interval = 2;
        sb.daytime = 240;
        sb.condition = HostSchedule::None;

        mock.expectCall("getAll(4)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(sa);
        mock.provideReturnValue(sb);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("SCHEDULELIST").pushBackInteger(4)));
        Access ap(p);

        a.checkEqual("131. getArraySize", ap.getArraySize(), 2U);
        a.checkEqual("132. type",         ap[0]("type").toInteger(), 1);
        a.checkEqual("133. weekdays",     ap[0]("weekdays").toInteger(), 24);
        a.checkEqual("134. daytime",      ap[0]("daytime").toInteger(), 300);
        a.checkEqual("135. condition",    ap[0]("condition").toInteger(), 1);
        a.checkEqual("136. condturn",     ap[0]("condTurn").toInteger(), 20);
        a.checkNull ("137. interval",     ap[0]("interval").getValue());
        a.checkNull ("138. hostearly",    ap[0]("hostEarly").getValue());
        a.checkNull ("139. hostdelay",    ap[0]("hostDelay").getValue());
        a.checkEqual("140. type",         ap[1]("type").toInteger(), 2);
        a.checkEqual("141. interval",     ap[1]("interval").toInteger(), 2);
        a.checkEqual("142. daytime",      ap[1]("daytime").toInteger(), 240);
        a.checkEqual("143. condition",    ap[1]("condition").toInteger(), 0);
        a.checkNull ("144. weekdays",     ap[1]("weekdays").getValue());
        a.checkNull ("145. hostearly",    ap[1]("hostEarly").getValue());
        a.checkNull ("146. hostdelay",    ap[1]("hostDelay").getValue());
    }

    // SCHEDULEDROP
    mock.expectCall("drop(92)");
    AFL_CHECK_SUCCEEDS(a("151. scheduledrop"), testee.callVoid(Segment().pushBackString("SCHEDULEDROP").pushBackInteger(92)));

    // SCHEDULESHOW
    // - return
    {
        mock.expectCall("preview(32,-1,-1)");
        mock.provideReturnValue(3);
        mock.provideReturnValue(1932000);
        mock.provideReturnValue(1943000);
        mock.provideReturnValue(1954000);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("SCHEDULESHOW").pushBackInteger(32)));
        Access ap(p);
        a.checkEqual("161. getArraySize", ap.getArraySize(), 3U);
        a.checkEqual("162. result", ap[0].toInteger(), 1932000);
        a.checkEqual("163. result", ap[1].toInteger(), 1943000);
        a.checkEqual("164. result", ap[2].toInteger(), 1954000);
    }

    // - variations
    mock.expectCall("preview(15,77777,88)");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("SCHEDULESHOW").pushBackInteger(15).pushBackString("TURNLIMIT").pushBackInteger(88).pushBackString("TIMELIMIT").pushBackInteger(77777));

    mock.expectCall("preview(15,-1,55)");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("SCHEDULESHOW").pushBackInteger(15).pushBackString("TURNLIMIT").pushBackInteger(55));

    // Variations
    // - case
    mock.expectCall("add(42,[type=stopped,condition=time,conditionTime=1900000])");
    AFL_CHECK_SUCCEEDS(a("171. scheduleadd"), testee.callVoid(Segment().pushBackString("scheduleadd").pushBackInteger(42).pushBackString("stop").pushBackString("untiltime").pushBackInteger(1900000)));

    mock.expectCall("preview(15,-1,55)");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("scheduleshow").pushBackInteger(15).pushBackString("turnlimit").pushBackInteger(55));

    // - cancelling options
    mock.expectCall("add(42,[type=stopped,condition=time,conditionTime=1900000])");
    AFL_CHECK_SUCCEEDS(a("181. scheduleadd"), testee.callVoid(Segment().pushBackString("scheduleadd").pushBackInteger(42).pushBackString("manual").pushBackString("forever").
                                                              pushBackString("stop").pushBackString("untiltime").pushBackInteger(1900000)));

    mock.expectCall("preview(15,-1,12)");
    mock.provideReturnValue(0);
    testee.callVoid(Segment().pushBackString("scheduleshow").pushBackInteger(15).pushBackString("turnlimit").pushBackInteger(55).pushBackString("turnlimit").pushBackInteger(12));

    mock.checkFinish();
}

/** Test errors. */
AFL_TEST("server.interface.HostScheduleServer:errors", a)
{
    HostScheduleMock mock(a);
    server::interface::HostScheduleServer testee(mock);

    // Missing parameters
    Segment empty;
    AFL_CHECK_THROWS(a("01. no verb"), testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. missing arg"), testee.callVoid(Segment().pushBackString("SCHEDULESHOW")), std::exception);
    AFL_CHECK_THROWS(a("03. missing option"), testee.callVoid(Segment().pushBackString("SCHEDULESHOW").pushBackInteger(9).pushBackString("TURNLIMIT")), std::exception);
    AFL_CHECK_THROWS(a("04. missing option"), testee.callVoid(Segment().pushBackString("SCHEDULESHOW").pushBackInteger(9).pushBackString("TIMELIMIT")), std::exception);
    AFL_CHECK_THROWS(a("05. missing option"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(42).pushBackString("UNTILTIME")), std::exception);
    AFL_CHECK_THROWS(a("06. missing option"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(42).pushBackString("UNTILTURN")), std::exception);
    AFL_CHECK_THROWS(a("07. missing option"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(42).pushBackString("UNTILTURN")), std::exception);
    AFL_CHECK_THROWS(a("08. missing option"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(42).pushBackString("DAILY")), std::exception);
    AFL_CHECK_THROWS(a("09. missing option"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(42).pushBackString("WEEKLY")), std::exception);

    // Bad commands
    AFL_CHECK_THROWS(a("11. bad verb"), testee.callVoid(Segment().pushBackString("")), std::exception);
    AFL_CHECK_THROWS(a("12. bad verb"), testee.callVoid(Segment().pushBackString("SCHEDULEVIEW")), std::exception);
    AFL_CHECK_THROWS(a("13. bad verb"), testee.callVoid(Segment().pushBackString("GET")), std::exception);

    // Bad keywords
    AFL_CHECK_THROWS(a("21. bad option"), testee.callVoid(Segment().pushBackString("SCHEDULESHOW").pushBackInteger(9).pushBackString("UNTILTIME").pushBackInteger(99)), std::exception);
    AFL_CHECK_THROWS(a("22. bad option"), testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(42).pushBackString("MOO")), std::exception);
}

/** Test roundtrip with client. */
AFL_TEST("server.interface.HostScheduleServer:roundtrip", a)
{
    HostScheduleMock mock(a);
    server::interface::HostScheduleServer level1(mock);
    server::interface::HostScheduleClient level2(level1);
    server::interface::HostScheduleServer level3(level2);
    server::interface::HostScheduleClient level4(level3);

    // SCHEDULEADD [testing all variations]
    {
        mock.expectCall("add(9,[])");
        AFL_CHECK_SUCCEEDS(a("01. add"), level4.add(9, HostSchedule::Schedule()));
    }

    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Weekly;
        sch.weekdays = 17;
        sch.hostEarly = true;

        mock.expectCall("add(3,[type=weekly,weekdays=17,hostEarly=t])");
        AFL_CHECK_SUCCEEDS(a("11. add"), level4.add(3, sch));
    }

    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Stopped;
        sch.condition = HostSchedule::Time;
        sch.conditionTime = 1900000;
        mock.expectCall("add(42,[type=stopped,condition=time,conditionTime=1900000])");
        AFL_CHECK_SUCCEEDS(a("21. add"), level4.add(42, sch));
    }

    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Daily;
        sch.interval = 3;
        sch.hostDelay = 90;
        sch.hostLimit = 100;
        mock.expectCall("add(17,[type=daily,interval=3,hostDelay=90,hostLimit=100])");
        AFL_CHECK_SUCCEEDS(a("31. add"), level4.add(17, sch));
    }

    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Quick;
        sch.condition = HostSchedule::Turn;
        sch.conditionTurn = 20;
        mock.expectCall("add(6,[type=asap,condition=turn,conditionTurn=20])");
        AFL_CHECK_SUCCEEDS(a("41. add"), level4.add(6, sch));
    }

    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Manual;
        sch.hostEarly = false;
        sch.condition = HostSchedule::None;
        mock.expectCall("add(12,[type=manual,hostEarly=f,condition=none])");
        AFL_CHECK_SUCCEEDS(a("51. add"), level4.add(12, sch));
    }

    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Weekly;
        sch.weekdays = 31;
        sch.daytime = 1000;
        mock.expectCall("add(3,[type=weekly,weekdays=31,daytime=1000])");
        AFL_CHECK_SUCCEEDS(a("61. add"), level4.add(3, sch));
    }

    // SCHEDULESET [parts]
    mock.expectCall("replace(7,[])");
    level4.replace(7, HostSchedule::Schedule());

    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Weekly;
        sch.weekdays = 9;
        mock.expectCall("replace(2,[type=weekly,weekdays=9])");
        level4.replace(2, sch);
    }

    // SCHEDULEMOD [parts]
    mock.expectCall("modify(7,[])");
    level4.modify(7, HostSchedule::Schedule());

    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Weekly;
        sch.weekdays = 9;
        mock.expectCall("modify(2,[type=weekly,weekdays=9])");
        level4.modify(2, sch);
    }

    // SCHEDULELIST
    // - empty
    {
        mock.expectCall("getAll(12)");
        mock.provideReturnValue(0);
        std::vector<HostSchedule::Schedule> result;
        AFL_CHECK_SUCCEEDS(a("71. getAll"), level4.getAll(12, result));
        a.checkEqual("72. size", result.size(), 0U);
    }
    // - nonempty
    {
        HostSchedule::Schedule sa;
        sa.type = HostSchedule::Weekly;
        sa.weekdays = 24;
        sa.daytime = 300;
        sa.condition = HostSchedule::Turn;
        sa.conditionTurn = 20;

        HostSchedule::Schedule sb;
        sb.type = HostSchedule::Daily;
        sb.interval = 2;
        sb.daytime = 240;
        sb.condition = HostSchedule::None;

        mock.expectCall("getAll(4)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(sa);
        mock.provideReturnValue(sb);

        std::vector<HostSchedule::Schedule> result;
        AFL_CHECK_SUCCEEDS(a("81. getAll"), level4.getAll(4, result));

        a.checkEqual("91. size",     result.size(), 2U);
        a.check("92. type",          result[0].type.isSame(HostSchedule::Weekly));
        a.check("93. weekdays",      result[0].weekdays.isSame(24));
        a.check("94. daytime",       result[0].daytime.isSame(300));
        a.check("95. condition",     result[0].condition.isSame(HostSchedule::Turn));
        a.check("96. conditionTurn", result[0].conditionTurn.isSame(20));
        a.check("97. interval",     !result[0].interval.isValid());
        a.check("98. hostEarly",    !result[0].hostEarly.isValid());
        a.check("99. hostDelay",    !result[0].hostDelay.isValid());
        a.check("100. type",         result[1].type.isSame(HostSchedule::Daily));
        a.check("101. interval",     result[1].interval.isSame(2));
        a.check("102. daytime",      result[1].daytime.isSame(240));
        a.check("103. condition",    result[1].condition.isSame(HostSchedule::None));
        a.check("104. weekdays",    !result[1].weekdays.isValid());
        a.check("105. hostEarly",   !result[1].hostEarly.isValid());
        a.check("106. hostDelay",   !result[1].hostDelay.isValid());
    }
    // - one, complete
    {
        HostSchedule::Schedule sa;
        sa.type = HostSchedule::Weekly;
        sa.weekdays = 24;
        sa.daytime = 300;
        sa.condition = HostSchedule::Time;
        sa.conditionTime = 2017;
        sa.hostEarly = false;
        sa.hostDelay = 30;
        sa.hostLimit = 20;

        mock.expectCall("getAll(2)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(sa);

        std::vector<HostSchedule::Schedule> result;
        AFL_CHECK_SUCCEEDS(a("111. getAll"), level4.getAll(2, result));

        a.checkEqual("121. size",     result.size(), 1U);
        a.check("122. type",          result[0].type.isSame(HostSchedule::Weekly));
        a.check("123. weekdays",      result[0].weekdays.isSame(24));
        a.check("124. daytime",       result[0].daytime.isSame(300));
        a.check("125. condition",     result[0].condition.isSame(HostSchedule::Time));
        a.check("126. conditionTime", result[0].conditionTime.isSame(2017));
        a.check("127. interval",     !result[0].interval.isValid());
        a.check("128. hostEarly",     result[0].hostEarly.isSame(false));
        a.check("129. hostDelay",     result[0].hostDelay.isSame(30));
        a.check("130. hostLimit",     result[0].hostLimit.isSame(20));
    }

    // SCHEDULEDROP
    mock.expectCall("drop(92)");
    AFL_CHECK_SUCCEEDS(a("131. drop"), level4.drop(92));

    // SCHEDULESHOW
    // - return
    {
        mock.expectCall("preview(32,-1,-1)");
        mock.provideReturnValue(3);
        mock.provideReturnValue(1932000);
        mock.provideReturnValue(1943000);
        mock.provideReturnValue(1954000);

        afl::data::IntegerList_t result;
        AFL_CHECK_SUCCEEDS(a("141. preview"), level4.preview(32, afl::base::Nothing, afl::base::Nothing, result));

        a.checkEqual("151. size", result.size(), 3U);
        a.checkEqual("152. result", result[0], 1932000);
        a.checkEqual("153. result", result[1], 1943000);
        a.checkEqual("154. result", result[2], 1954000);
    }

    // - variations
    {
        mock.expectCall("preview(32,77777,88)");
        mock.provideReturnValue(0);
        afl::data::IntegerList_t result;
        AFL_CHECK_SUCCEEDS(a("161. preview"), level4.preview(32, 77777, 88, result));
        a.checkEqual("162. size", result.size(), 0U);
    }
    {
        mock.expectCall("preview(15,-1,55)");
        mock.provideReturnValue(0);
        afl::data::IntegerList_t result;
        AFL_CHECK_SUCCEEDS(a("163. preview"), level4.preview(15, afl::base::Nothing, 55, result));
        a.checkEqual("164. size", result.size(), 0U);
    }

    mock.checkFinish();
}
