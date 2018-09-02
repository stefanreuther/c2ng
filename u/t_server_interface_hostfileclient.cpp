/**
  *  \file u/t_server_interface_hostfileclient.cpp
  *  \brief Test for server::interface::HostFileClient
  */

#include "server/interface/hostfileclient.hpp"

#include "t_server_interface.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;
using server::interface::HostFile;
using server::interface::HostFileClient;
using server::makeIntegerValue;
using server::makeStringValue;

/** Test HostFileClient interface methods. */
void
TestServerInterfaceHostFileClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    HostFileClient testee(mock);

    // getFile
    mock.expectCall("GET, game/4/3/f.txt");
    mock.provideNewResult(makeStringValue("c.."));
    TS_ASSERT_EQUALS(testee.getFile("game/4/3/f.txt"), "c..");

    // getDirectoryContent
    {
        Vector::Ref_t v = Vector::create();
        Hash::Ref_t h1 = Hash::create();
        v->pushBackNew(makeStringValue("f1"));
        h1->setNew("size", makeIntegerValue(111));
        v->pushBackNew(new HashValue(h1));

        Hash::Ref_t h2 = Hash::create();
        v->pushBackNew(makeStringValue("f2"));
        h2->setNew("size", makeIntegerValue(222));
        v->pushBackNew(new HashValue(h2));

        mock.expectCall("LS, game/9");
        mock.provideNewResult(new VectorValue(v));

        HostFile::InfoVector_t result;
        TS_ASSERT_THROWS_NOTHING(testee.getDirectoryContent("game/9", result));

        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0].name, "f1");
        TS_ASSERT_EQUALS(result[0].size.orElse(0), 111);
        TS_ASSERT_EQUALS(result[1].name, "f2");
        TS_ASSERT_EQUALS(result[1].size.orElse(0), 222);
    }

    // getFileInformation
    {
        Hash::Ref_t h1 = Hash::create();
        h1->setNew("name", makeStringValue("y.dat"));
        h1->setNew("size", makeIntegerValue(42));
        mock.provideNewResult(new HashValue(h1));
        mock.expectCall("STAT, a/y.dat");

        HostFile::Info result = testee.getFileInformation("a/y.dat");
        TS_ASSERT_EQUALS(result.name, "y.dat");
        TS_ASSERT_EQUALS(result.size.orElse(0), 42);
    }

    // getPathDescription
    {
        Vector::Ref_t v = Vector::create();
        Hash::Ref_t h1 = Hash::create();
        v->pushBackNew(makeStringValue("g"));
        h1->setNew("size", makeIntegerValue(77));
        v->pushBackNew(new HashValue(h1));

        Hash::Ref_t h2 = Hash::create();
        v->pushBackNew(makeStringValue("1"));
        h2->setNew("size", makeIntegerValue(66));
        v->pushBackNew(new HashValue(h2));

        mock.expectCall("PSTAT, g/1");
        mock.provideNewResult(new VectorValue(v));

        HostFile::InfoVector_t result;
        TS_ASSERT_THROWS_NOTHING(testee.getPathDescription("g/1", result));

        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0].name, "g");
        TS_ASSERT_EQUALS(result[0].size.orElse(0), 77);
        TS_ASSERT_EQUALS(result[1].name, "1");
        TS_ASSERT_EQUALS(result[1].size.orElse(0), 66);
    }
}

/** Test HostFileClient::unpackInfo(). */
void
TestServerInterfaceHostFileClient::testUnpack()
{
    // Empty
    HostFileClient::Info i = HostFileClient::unpackInfo(0);
    TS_ASSERT_EQUALS(i.name, "");
    TS_ASSERT_EQUALS(i.label, HostFileClient::NameLabel);
    TS_ASSERT_EQUALS(i.type, server::interface::FileBase::IsUnknown);
    TS_ASSERT_EQUALS(i.size.isValid(), false);
    TS_ASSERT_EQUALS(i.visibility.isValid(), false);
    TS_ASSERT_EQUALS(i.contentId.isValid(), false);
    TS_ASSERT_EQUALS(i.turnNumber.isValid(), false);
    TS_ASSERT_EQUALS(i.slotId.isValid(), false);
    TS_ASSERT_EQUALS(i.slotName.isValid(), false);
    TS_ASSERT_EQUALS(i.gameId.isValid(), false);
    TS_ASSERT_EQUALS(i.gameName.isValid(), false);
    TS_ASSERT_EQUALS(i.toolName.isValid(), false);

    // Fully-populated
    Hash::Ref_t h = Hash::create();
    h->setNew("name",       makeStringValue("player7.rst"));
    h->setNew("type",       makeStringValue("file"));
    h->setNew("label",      makeStringValue("turn"));
    h->setNew("size",       makeIntegerValue(123456));
    h->setNew("visibility", makeIntegerValue(2));
    h->setNew("id",         makeStringValue("adc83b19e793491b1c6ea0fd8b46cd9f32e592fc"));
    h->setNew("turn",       makeIntegerValue(12));
    h->setNew("slot",       makeIntegerValue(7));
    h->setNew("slotname",   makeStringValue("The Tholians"));
    h->setNew("game",       makeIntegerValue(42));
    h->setNew("gamename",   makeStringValue("Battle 357"));
    h->setNew("toolname",   makeStringValue("MOY"));
    HashValue hv(h);
    i = HostFileClient::unpackInfo(&hv);

    TS_ASSERT_EQUALS(i.name, "player7.rst");
    TS_ASSERT_EQUALS(i.label, HostFileClient::TurnLabel);
    TS_ASSERT_EQUALS(i.type, server::interface::FileBase::IsFile);
    TS_ASSERT_EQUALS(i.size.orElse(0), 123456);
    TS_ASSERT_EQUALS(i.visibility.orElse(0), 2);
    TS_ASSERT_EQUALS(i.contentId.orElse(""), "adc83b19e793491b1c6ea0fd8b46cd9f32e592fc");
    TS_ASSERT_EQUALS(i.turnNumber.orElse(0), 12);
    TS_ASSERT_EQUALS(i.slotId.orElse(0), 7);
    TS_ASSERT_EQUALS(i.slotName.orElse(""), "The Tholians");
    TS_ASSERT_EQUALS(i.gameId.orElse(0), 42);
    TS_ASSERT_EQUALS(i.gameName.orElse(""), "Battle 357");
    TS_ASSERT_EQUALS(i.toolName.orElse(""), "MOY");
}

