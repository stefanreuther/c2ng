/**
  *  \file test/server/interface/hostspecificationclienttest.cpp
  *  \brief Test for server::interface::HostSpecificationClient
  */

#include "server/interface/hostspecificationclient.hpp"

#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"
#include <memory>

AFL_TEST("server.interface.HostSpecificationClient", a)
{
    afl::test::CommandHandler cc(a);
    server::interface::HostSpecificationClient testee(cc);

    // SPECSHIPLIST
    cc.expectCall("SPECSHIPLIST, booh, json, beamspec");
    cc.provideNewResult(server::makeStringValue("{}"));
    {
        afl::data::StringList_t sv;
        sv.push_back("beamspec");
        std::auto_ptr<afl::data::Value> val(testee.getShiplistData("booh", server::interface::HostSpecificationClient::JsonString, sv));
        a.checkEqual("01. getShiplistData", server::toString(val.get()), "{}");
    }

    // SPECGAME
    cc.expectCall("SPECGAME, 3, direct, hullspec, torpspec");
    cc.provideNewResult(server::makeIntegerValue(42));
    {
        afl::data::StringList_t sv;
        sv.push_back("hullspec");
        sv.push_back("torpspec");
        std::auto_ptr<afl::data::Value> val(testee.getGameData(3, server::interface::HostSpecificationClient::Direct, sv));
        a.checkEqual("11. getGameData", server::toInteger(val.get()), 42);
    }
}
