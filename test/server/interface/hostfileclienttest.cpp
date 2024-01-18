/**
  *  \file test/server/interface/hostfileclienttest.cpp
  *  \brief Test for server::interface::HostFileClient
  */

#include "server/interface/hostfileclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("server.interface.HostFileClient:commands", a)
{
    afl::test::CommandHandler mock(a);
    HostFileClient testee(mock);

    // getFile
    mock.expectCall("GET, game/4/3/f.txt");
    mock.provideNewResult(makeStringValue("c.."));
    a.checkEqual("01. getFile", testee.getFile("game/4/3/f.txt"), "c..");

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
        AFL_CHECK_SUCCEEDS(a("11. getDirectoryContent"), testee.getDirectoryContent("game/9", result));

        a.checkEqual("21. size", result.size(), 2U);
        a.checkEqual("22. name", result[0].name, "f1");
        a.checkEqual("23. size", result[0].size.orElse(0), 111);
        a.checkEqual("24. name", result[1].name, "f2");
        a.checkEqual("25. size", result[1].size.orElse(0), 222);
    }

    // getFileInformation
    {
        Hash::Ref_t h1 = Hash::create();
        h1->setNew("name", makeStringValue("y.dat"));
        h1->setNew("size", makeIntegerValue(42));
        mock.provideNewResult(new HashValue(h1));
        mock.expectCall("STAT, a/y.dat");

        HostFile::Info result = testee.getFileInformation("a/y.dat");
        a.checkEqual("31. name", result.name, "y.dat");
        a.checkEqual("32. size", result.size.orElse(0), 42);
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
        AFL_CHECK_SUCCEEDS(a("41. getPathDescription"), testee.getPathDescription("g/1", result));

        a.checkEqual("51. size", result.size(), 2U);
        a.checkEqual("52. name", result[0].name, "g");
        a.checkEqual("53. size", result[0].size.orElse(0), 77);
        a.checkEqual("54. name", result[1].name, "1");
        a.checkEqual("55. size", result[1].size.orElse(0), 66);
    }
}

/** Test HostFileClient::unpackInfo(). */
AFL_TEST("server.interface.HostFileClient:unpackInfo", a)
{
    // Empty
    HostFileClient::Info i = HostFileClient::unpackInfo(0);
    a.checkEqual("01. name",       i.name, "");
    a.checkEqual("02. label",      i.label, HostFileClient::NameLabel);
    a.checkEqual("03. type",       i.type, server::interface::FileBase::IsUnknown);
    a.checkEqual("04. size",       i.size.isValid(), false);
    a.checkEqual("05. visibility", i.visibility.isValid(), false);
    a.checkEqual("06. contentId",  i.contentId.isValid(), false);
    a.checkEqual("07. turnNumber", i.turnNumber.isValid(), false);
    a.checkEqual("08. slotId",     i.slotId.isValid(), false);
    a.checkEqual("09. slotName",   i.slotName.isValid(), false);
    a.checkEqual("10. gameId",     i.gameId.isValid(), false);
    a.checkEqual("11. gameName",   i.gameName.isValid(), false);
    a.checkEqual("12. toolName",   i.toolName.isValid(), false);

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

    a.checkEqual("21. name",       i.name, "player7.rst");
    a.checkEqual("22. label",      i.label, HostFileClient::TurnLabel);
    a.checkEqual("23. type",       i.type, server::interface::FileBase::IsFile);
    a.checkEqual("24. size",       i.size.orElse(0), 123456);
    a.checkEqual("25. visibility", i.visibility.orElse(0), 2);
    a.checkEqual("26. contentId",  i.contentId.orElse(""), "adc83b19e793491b1c6ea0fd8b46cd9f32e592fc");
    a.checkEqual("27. turnNumber", i.turnNumber.orElse(0), 12);
    a.checkEqual("28. slotId",     i.slotId.orElse(0), 7);
    a.checkEqual("29. slotName",   i.slotName.orElse(""), "The Tholians");
    a.checkEqual("30. gameId",     i.gameId.orElse(0), 42);
    a.checkEqual("31. gameName",   i.gameName.orElse(""), "Battle 357");
    a.checkEqual("32. toolName",   i.toolName.orElse(""), "MOY");
}
