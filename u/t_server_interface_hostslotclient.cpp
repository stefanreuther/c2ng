/**
  *  \file u/t_server_interface_hostslotclient.cpp
  *  \brief Test for server::interface::HostSlotClient
  */

#include "server/interface/hostslotclient.hpp"

#include "t_server_interface.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

/** General command test. */
void
TestServerInterfaceHostSlotClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::HostSlotClient testee(mock);

    // add
    mock.expectCall("SLOTADD, 9");
    mock.provideNewResult(0);
    testee.add(9, afl::base::Nothing);

    static const int32_t slotsToAdd[] = {3,6,9};
    mock.expectCall("SLOTADD, 42, 3, 6, 9");
    mock.provideNewResult(0);
    testee.add(42, slotsToAdd);

    // remove
    mock.expectCall("SLOTRM, 7");
    mock.provideNewResult(0);
    testee.remove(7, afl::base::Nothing);

    static const int32_t slotsToRemove[] = {2,4,8,16};
    mock.expectCall("SLOTRM, 77, 2, 4, 8, 16");
    mock.provideNewResult(0);
    testee.remove(77, slotsToRemove);

    // getAll
    afl::data::Vector::Ref_t v = afl::data::Vector::create();
    v->pushBackInteger(5);
    v->pushBackInteger(7);
    mock.expectCall("SLOTLS, 12");
    mock.provideNewResult(new afl::data::VectorValue(v));

    afl::data::IntegerList_t result;
    testee.getAll(12, result);
    TS_ASSERT_EQUALS(result.size(), 2U);
    TS_ASSERT_EQUALS(result[0], 5);
    TS_ASSERT_EQUALS(result[1], 7);

    mock.checkFinish();
}
