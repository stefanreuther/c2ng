/**
  *  \file u/t_server_interface_hostschedule.cpp
  *  \brief Test for server::interface::HostSchedule
  */

#include "server/interface/hostschedule.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceHostSchedule::testInterface()
{
    class Tester : public server::interface::HostSchedule {
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
void
TestServerInterfaceHostSchedule::testFormat()
{
    using server::interface::HostSchedule;
    TS_ASSERT_EQUALS(HostSchedule::formatType(HostSchedule::Stopped), 0);
    TS_ASSERT_EQUALS(HostSchedule::formatType(HostSchedule::Weekly), 1);
    TS_ASSERT_EQUALS(HostSchedule::formatType(HostSchedule::Daily), 2);
    TS_ASSERT_EQUALS(HostSchedule::formatType(HostSchedule::Quick), 3);
    TS_ASSERT_EQUALS(HostSchedule::formatType(HostSchedule::Manual), 4);
    TS_ASSERT_EQUALS(HostSchedule::formatCondition(HostSchedule::None), 0);
    TS_ASSERT_EQUALS(HostSchedule::formatCondition(HostSchedule::Turn), 1);
    TS_ASSERT_EQUALS(HostSchedule::formatCondition(HostSchedule::Time), 2);
}

/** Test parse functions. */
void
TestServerInterfaceHostSchedule::testParse()
{
    using server::interface::HostSchedule;

    HostSchedule::Type type;
    TS_ASSERT(HostSchedule::parseType(0, type));
    TS_ASSERT_EQUALS(type, HostSchedule::Stopped);
    TS_ASSERT(HostSchedule::parseType(1, type));
    TS_ASSERT_EQUALS(type, HostSchedule::Weekly);
    TS_ASSERT(HostSchedule::parseType(2, type));
    TS_ASSERT_EQUALS(type, HostSchedule::Daily);
    TS_ASSERT(HostSchedule::parseType(3, type));
    TS_ASSERT_EQUALS(type, HostSchedule::Quick);
    TS_ASSERT(HostSchedule::parseType(4, type));
    TS_ASSERT_EQUALS(type, HostSchedule::Manual);

    TS_ASSERT(!HostSchedule::parseType(-1, type));
    TS_ASSERT(!HostSchedule::parseType(5, type));
    TS_ASSERT(!HostSchedule::parseType(999999, type));

    HostSchedule::Condition condition;
    TS_ASSERT(HostSchedule::parseCondition(0, condition));
    TS_ASSERT_EQUALS(condition, HostSchedule::None);
    TS_ASSERT(HostSchedule::parseCondition(1, condition));
    TS_ASSERT_EQUALS(condition, HostSchedule::Turn);
    TS_ASSERT(HostSchedule::parseCondition(2, condition));
    TS_ASSERT_EQUALS(condition, HostSchedule::Time);

    TS_ASSERT(!HostSchedule::parseCondition(-1, condition));
    TS_ASSERT(!HostSchedule::parseCondition(3, condition));
    TS_ASSERT(!HostSchedule::parseCondition(999999, condition));
}
