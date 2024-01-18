/**
  *  \file test/server/interface/hostscheduletest.cpp
  *  \brief Test for server::interface::HostSchedule
  */

#include "server/interface/hostschedule.hpp"
#include "afl/test/testrunner.hpp"

using server::interface::HostSchedule;

/** Interface test. */
AFL_TEST_NOARG("server.interface.HostSchedule:interface")
{
    class Tester : public HostSchedule {
     public:
        virtual void add(int32_t /*gameId*/, const Schedule& /*sched*/)
            { }
        virtual void replace(int32_t /*gameId*/, const Schedule& /*sched*/)
            { }
        virtual void modify(int32_t /*gameId*/, const Schedule& /*sched*/)
            { }
        virtual void getAll(int32_t /*gameId*/, std::vector<Schedule>& /*result*/)
            { }
        virtual void drop(int32_t /*gameId*/)
            { }
        virtual void preview(int32_t /*gameId*/, afl::base::Optional<server::Time_t> /*timeLimit*/, afl::base::Optional<int32_t> /*turnLimit*/, afl::data::IntegerList_t& /*result*/)
            { }
    };
    Tester t;
}

/** Test format functions. */
AFL_TEST("server.interface.HostSchedule:formatType", a)
{
    a.checkEqual("01", HostSchedule::formatType(HostSchedule::Stopped), 0);
    a.checkEqual("02", HostSchedule::formatType(HostSchedule::Weekly), 1);
    a.checkEqual("03", HostSchedule::formatType(HostSchedule::Daily), 2);
    a.checkEqual("04", HostSchedule::formatType(HostSchedule::Quick), 3);
    a.checkEqual("05", HostSchedule::formatType(HostSchedule::Manual), 4);
}

AFL_TEST("server.interface.HostSchedule:formatCondition", a)
{
    a.checkEqual("01", HostSchedule::formatCondition(HostSchedule::None), 0);
    a.checkEqual("02", HostSchedule::formatCondition(HostSchedule::Turn), 1);
    a.checkEqual("03", HostSchedule::formatCondition(HostSchedule::Time), 2);
}

/** Test parse functions. */
AFL_TEST("server.interface.HostSchedule:parseType", a)
{
    HostSchedule::Type type;
    a.check("01", HostSchedule::parseType(0, type));
    a.checkEqual("02", type, HostSchedule::Stopped);
    a.check("03", HostSchedule::parseType(1, type));
    a.checkEqual("04", type, HostSchedule::Weekly);
    a.check("05", HostSchedule::parseType(2, type));
    a.checkEqual("06", type, HostSchedule::Daily);
    a.check("07", HostSchedule::parseType(3, type));
    a.checkEqual("08", type, HostSchedule::Quick);
    a.check("09", HostSchedule::parseType(4, type));
    a.checkEqual("10", type, HostSchedule::Manual);

    a.check("11", !HostSchedule::parseType(-1, type));
    a.check("12", !HostSchedule::parseType(5, type));
    a.check("13", !HostSchedule::parseType(999999, type));
}

AFL_TEST("server.interface.HostSchedule:parseCondition", a)
{
    HostSchedule::Condition condition;
    a.check("01", HostSchedule::parseCondition(0, condition));
    a.checkEqual("02", condition, HostSchedule::None);
    a.check("03", HostSchedule::parseCondition(1, condition));
    a.checkEqual("04", condition, HostSchedule::Turn);
    a.check("05", HostSchedule::parseCondition(2, condition));
    a.checkEqual("06", condition, HostSchedule::Time);

    a.check("11", !HostSchedule::parseCondition(-1, condition));
    a.check("12", !HostSchedule::parseCondition(3, condition));
    a.check("13", !HostSchedule::parseCondition(999999, condition));
}
