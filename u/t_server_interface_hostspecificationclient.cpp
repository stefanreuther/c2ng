/**
  *  \file u/t_server_interface_hostspecificationclient.cpp
  *  \brief Test for server::interface::HostSpecificationClient
  */

#include <memory>
#include "server/interface/hostspecificationclient.hpp"
#include "afl/test/commandhandler.hpp"

#include "t_server_interface.hpp"
#include "server/types.hpp"

void
TestServerInterfaceHostSpecificationClient::testIt()
{
    afl::test::CommandHandler cc("testIt");
    server::interface::HostSpecificationClient testee(cc);

    // SPECSHIPLIST
    cc.expectCall("SPECSHIPLIST, booh, json, beamspec");
    cc.provideNewResult(server::makeStringValue("{}"));
    {
        afl::data::StringList_t sv;
        sv.push_back("beamspec");
        std::auto_ptr<afl::data::Value> val(testee.getShiplistData("booh", server::interface::HostSpecificationClient::JsonString, sv));
        TS_ASSERT_EQUALS(server::toString(val.get()), "{}");
    }

    // SPECGAME
    cc.expectCall("SPECGAME, 3, direct, hullspec, torpspec");
    cc.provideNewResult(server::makeIntegerValue(42));
    {
        afl::data::StringList_t sv;
        sv.push_back("hullspec");
        sv.push_back("torpspec");
        std::auto_ptr<afl::data::Value> val(testee.getGameData(3, server::interface::HostSpecificationClient::Direct, sv));
        TS_ASSERT_EQUALS(server::toInteger(val.get()), 42);
    }
}

