/**
  *  \file test/server/interface/hostslotservertest.cpp
  *  \brief Test for server::interface::HostSlotServer
  */

#include "server/interface/hostslotserver.hpp"

#include "afl/data/access.hpp"
#include "afl/data/value.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/hostslot.hpp"
#include "server/interface/hostslotclient.hpp"

using afl::string::Format;
using afl::data::Segment;
using afl::data::Value;
using afl::data::Access;

namespace {
    class HostSlotMock : public afl::test::CallReceiver, public server::interface::HostSlot {
     public:
        HostSlotMock(afl::test::Assert a)
            : CallReceiver(a)
            { }

        virtual void add(int32_t gameId, afl::base::Memory<const int32_t> slotNrs)
            {
                String_t slots;
                while (const int32_t* p = slotNrs.eat()) {
                    slots += Format(",%d", *p);
                }
                checkCall(Format("add(%d%s)", gameId, slots));
            }

        virtual void remove(int32_t gameId, afl::base::Memory<const int32_t> slotNrs)
            {
                String_t slots;
                while (const int32_t* p = slotNrs.eat()) {
                    slots += Format(",%d", *p);
                }
                checkCall(Format("remove(%d%s)", gameId, slots));
            }

        virtual void getAll(int32_t gameId, afl::data::IntegerList_t& result)
            {
                checkCall(Format("getAll(%d)", gameId));

                int n = consumeReturnValue<int>();
                for (int i = 0; i < n; ++i) {
                    result.push_back(consumeReturnValue<int>());
                }
            }
    };
}


/** Test server operations.
    Generate some standard commands and check that they are correctly passed. */
AFL_TEST("server.interface.HostSlotServer:commands", a)
{
    HostSlotMock mock(a);
    server::interface::HostSlotServer testee(mock);

    // add
    mock.expectCall("add(7,2,3,4)");
    testee.callVoid(Segment().pushBackString("SLOTADD").pushBackInteger(7).pushBackInteger(2).pushBackInteger(3).pushBackInteger(4));

    mock.expectCall("add(12)");
    testee.callVoid(Segment().pushBackString("SLOTADD").pushBackInteger(12));

    // remove
    mock.expectCall("remove(9,12)");
    testee.callVoid(Segment().pushBackString("SLOTRM").pushBackInteger(9).pushBackInteger(12));

    mock.expectCall("remove(777)");
    testee.callVoid(Segment().pushBackString("SLOTRM").pushBackInteger(777));

    mock.expectCall("remove(99)");
    testee.callVoid(Segment().pushBackString("slotrm").pushBackInteger(99));

    // getAll
    mock.expectCall("getAll(11)");
    mock.provideReturnValue(2);
    mock.provideReturnValue(42);
    mock.provideReturnValue(23);
    std::auto_ptr<Value> p(testee.call(Segment().pushBackString("SLOTLS").pushBackInteger(11)));
    a.checkEqual("01. getArraySize", Access(p).getArraySize(), 2U);
    a.checkEqual("02. result 0", Access(p)[0].toInteger(), 42);
    a.checkEqual("03. result 1", Access(p)[1].toInteger(), 23);

    mock.checkFinish();
}

/** Test errors. */
AFL_TEST("server.interface.HostSlotServer:error", a)
{
    HostSlotMock mock(a);
    server::interface::HostSlotServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    AFL_CHECK_THROWS(a("01. empty"),         testee.call(empty), std::exception);
    AFL_CHECK_THROWS(a("02. bad verb"),      testee.call(Segment().pushBackString("wut")), std::exception);
    AFL_CHECK_THROWS(a("03. missing arg"),   testee.call(Segment().pushBackString("SLOTADD")), std::exception);
    AFL_CHECK_THROWS(a("04. too many args"), testee.call(Segment().pushBackString("SLOTLS").pushBackInteger(2).pushBackInteger(3)), std::exception);
    AFL_CHECK_THROWS(a("05. bad type"),      testee.call(Segment().pushBackString("SLOTLS").pushBackString("X")), std::exception);
}

/** Test roundtrip operation. */
AFL_TEST("server.interface.HostSlotServer:roundtrip", a)
{
    HostSlotMock mock(a);
    server::interface::HostSlotServer level1(mock);
    server::interface::HostSlotClient level2(level1);
    server::interface::HostSlotServer level3(level2);
    server::interface::HostSlotClient level4(level3);

    // add, remove
    static const int32_t slots[] = {9,10,11};
    mock.expectCall("add(145,9,10,11)");
    level4.add(145, slots);

    mock.expectCall("remove(998,9,10,11)");
    level4.remove(998, slots);

    // getAll
    mock.expectCall("getAll(42)");
    mock.provideReturnValue(3);
    mock.provideReturnValue(32);
    mock.provideReturnValue(16);
    mock.provideReturnValue(8);
    afl::data::IntegerList_t result;
    level4.getAll(42, result);
    a.checkEqual("01. size", result.size(), 3U);
    a.checkEqual("02. result", result[0], 32);
    a.checkEqual("03. result", result[1], 16);
    a.checkEqual("04. result", result[2], 8);

    mock.checkFinish();
}
