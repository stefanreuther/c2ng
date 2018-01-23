/**
  *  \file u/t_server_interface_hosttoolclient.cpp
  *  \brief Test for server::interface::HostToolClient
  */

#include "server/interface/hosttoolclient.hpp"

#include "t_server_interface.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

/** Test all commands. */
void
TestServerInterfaceHostToolClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::HostToolClient testee(mock, server::interface::HostTool::Master);

    // add
    {
        mock.expectCall("MASTERADD, i, p, x, k");
        mock.provideNewResult(0);
        testee.add("i", "p", "x", "k");
    }

    // set
    {
        mock.expectCall("MASTERSET, ii, kk, vv");
        mock.provideNewResult(0);
        testee.set("ii", "kk", "vv");
    }

    // set
    {
        mock.expectCall("MASTERGET, ii, kk");
        mock.provideNewResult(server::makeStringValue("answer"));
        TS_ASSERT_EQUALS(testee.get("ii", "kk"), "answer");
    }

    // remove
    {
        mock.expectCall("MASTERRM, old");
        mock.provideNewResult(server::makeIntegerValue(1));
        TS_ASSERT(testee.remove("old"));
    }

    // getAll
    {
        mock.expectCall("MASTERLS");
        {
            afl::data::Vector::Ref_t vec = afl::data::Vector::create();

            afl::data::Hash::Ref_t a = afl::data::Hash::create();
            a->setNew("id",          server::makeStringValue("9"));
            a->setNew("description", server::makeStringValue("desc 9"));
            a->setNew("kind",        server::makeStringValue("kind 9"));
            a->setNew("default",     server::makeIntegerValue(0));
            vec->pushBackNew(new afl::data::HashValue(a));

            afl::data::Hash::Ref_t b = afl::data::Hash::create();
            b->setNew("id",          server::makeStringValue("16"));
            b->setNew("description", server::makeStringValue("desc 16"));
            b->setNew("kind",        server::makeStringValue("kind 16"));
            b->setNew("default",     server::makeIntegerValue(1));
            vec->pushBackNew(new afl::data::HashValue(b));
            mock.provideNewResult(new afl::data::VectorValue(vec));
        }

        std::vector<server::interface::HostTool::Info> infos;
        testee.getAll(infos);

        TS_ASSERT_EQUALS(infos.size(), 2U);
        TS_ASSERT_EQUALS(infos[0].id,          "9");
        TS_ASSERT_EQUALS(infos[0].description, "desc 9");
        TS_ASSERT_EQUALS(infos[0].kind,        "kind 9");
        TS_ASSERT_EQUALS(infos[0].isDefault,   false);
        TS_ASSERT_EQUALS(infos[1].id,          "16");
        TS_ASSERT_EQUALS(infos[1].description, "desc 16");
        TS_ASSERT_EQUALS(infos[1].kind,        "kind 16");
        TS_ASSERT_EQUALS(infos[1].isDefault,   true);
    }

    // copy
    {
        mock.expectCall("MASTERCP, orig, clone");
        mock.provideNewResult(0);
        testee.copy("orig", "clone");
    }

    // setDefault
    {
        mock.expectCall("MASTERDEFAULT, d");
        mock.provideNewResult(0);
        testee.setDefault("d");
    }

    // getDifficulty
    {
        mock.expectCall("MASTERRATING, tool, GET");
        mock.provideNewResult(server::makeIntegerValue(182));
        TS_ASSERT_EQUALS(testee.getDifficulty("tool"), 182);
    }

    // clearDifficulty
    {
        mock.expectCall("MASTERRATING, tool, NONE");
        mock.provideNewResult(0);
        testee.clearDifficulty("tool");
    }

    // setDifficulty
    {
        mock.expectCall("MASTERRATING, t, AUTO, USE");
        mock.provideNewResult(server::makeIntegerValue(130));
        TS_ASSERT_EQUALS(testee.setDifficulty("t", afl::base::Nothing, true), 130);
    }
    {
        mock.expectCall("MASTERRATING, s, SET, 3, SHOW");
        mock.provideNewResult(server::makeIntegerValue(3));
        TS_ASSERT_EQUALS(testee.setDifficulty("s", 3, false), 3);
    }
    {
        mock.expectCall("MASTERRATING, s, SET, 17, USE");
        mock.provideNewResult(server::makeIntegerValue(17));
        TS_ASSERT_EQUALS(testee.setDifficulty("s", 17, true), 17);
    }

    mock.checkFinish();
}

/** Test all modes. */
void
TestServerInterfaceHostToolClient::testModes()
{
    using server::interface::HostToolClient;
    afl::test::CommandHandler mock("testModes");

    {
        mock.expectCall("HOSTGET, k, v");
        mock.provideNewResult(server::makeStringValue("s"));
        TS_ASSERT_EQUALS(HostToolClient(mock, HostToolClient::Host).get("k", "v"), "s");
    }
    {
        mock.expectCall("MASTERGET, mk, mv");
        mock.provideNewResult(server::makeStringValue("ms"));
        TS_ASSERT_EQUALS(HostToolClient(mock, HostToolClient::Master).get("mk", "mv"), "ms");
    }
    {
        mock.expectCall("SHIPLISTGET, x, y");
        mock.provideNewResult(server::makeStringValue("z"));
        TS_ASSERT_EQUALS(HostToolClient(mock, HostToolClient::ShipList).get("x", "y"), "z");
    }
    {
        mock.expectCall("TOOLGET, t, s");
        mock.provideNewResult(server::makeStringValue("v"));
        TS_ASSERT_EQUALS(HostToolClient(mock, HostToolClient::Tool).get("t", "s"), "v");
    }
    mock.checkFinish();
}

