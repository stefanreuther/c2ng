/**
  *  \file u/t_server_interface_hostplayerclient.cpp
  *  \brief Test for server::interface::HostPlayerClient
  */

#include "server/interface/hostplayerclient.hpp"

#include <stdexcept>
#include "t_server_interface.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Hash;
using afl::data::HashValue;
using server::interface::HostPlayer;

/** Simple tests. */
void
TestServerInterfaceHostPlayerClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::HostPlayerClient testee(mock);

    // join
    mock.expectCall("PLAYERJOIN, 42, 3, uu");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS_NOTHING(testee.join(42, 3, "uu"));

    // substitute
    mock.expectCall("PLAYERSUBST, 56, 1, zz");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS_NOTHING(testee.substitute(56, 1, "zz"));

    // resign
    mock.expectCall("PLAYERRESIGN, 23, 3, a");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS_NOTHING(testee.resign(23, 3, "a"));

    // add
    mock.expectCall("PLAYERADD, 93, pp");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS_NOTHING(testee.add(93, "pp"));

    // getInfo
    // - full response
    {
        Vector::Ref_t v = Vector::create();
        v->pushBackString("fred");
        v->pushBackString("barney");
        v->pushBackString("wilma");

        Hash::Ref_t h = Hash::create();
        h->setNew("long", server::makeStringValue("Long"));
        h->setNew("short", server::makeStringValue("Short"));
        h->setNew("adj", server::makeStringValue("Adjective"));
        h->setNew("users", new VectorValue(v));
        h->setNew("editable", server::makeIntegerValue(2));
        h->setNew("joinable", server::makeIntegerValue(1));

        mock.expectCall("PLAYERSTAT, 17, 3");
        mock.provideNewResult(new HashValue(h));

        HostPlayer::Info i = testee.getInfo(17, 3);
        TS_ASSERT_EQUALS(i.longName, "Long");
        TS_ASSERT_EQUALS(i.shortName, "Short");
        TS_ASSERT_EQUALS(i.adjectiveName, "Adjective");
        TS_ASSERT_EQUALS(i.userIds.size(), 3U);
        TS_ASSERT_EQUALS(i.userIds[0], "fred");
        TS_ASSERT_EQUALS(i.userIds[1], "barney");
        TS_ASSERT_EQUALS(i.userIds[2], "wilma");
        TS_ASSERT_EQUALS(i.numEditable, 2);
        TS_ASSERT_EQUALS(i.joinable, true);
    }
    // - no response, deserialized as default
    {
        mock.expectCall("PLAYERSTAT, 17, 3");
        mock.provideNewResult(0);

        HostPlayer::Info i = testee.getInfo(17, 3);
        TS_ASSERT_EQUALS(i.longName, "");
        TS_ASSERT_EQUALS(i.shortName, "");
        TS_ASSERT_EQUALS(i.adjectiveName, "");
        TS_ASSERT_EQUALS(i.userIds.size(), 0U);
        TS_ASSERT_EQUALS(i.numEditable, 0);
        TS_ASSERT_EQUALS(i.joinable, false);
    }

    // list
    // - answer is array of items
    {
        Hash::Ref_t h1 = Hash::create();
        h1->setNew("long", server::makeStringValue("h1"));

        Hash::Ref_t h2 = Hash::create();
        h2->setNew("long", server::makeStringValue("h2"));

        Vector::Ref_t v = Vector::create();
        v->pushBackInteger(2);
        v->pushBackNew(new HashValue(h1));
        v->pushBackInteger(5);
        v->pushBackNew(new HashValue(h2));

        mock.expectCall("PLAYERLS, 7");
        mock.provideNewResult(new VectorValue(v));

        std::map<int, HostPlayer::Info> result;
        TS_ASSERT_THROWS_NOTHING(testee.list(7, false, result));

        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[2].longName, "h1");
        TS_ASSERT_EQUALS(result[5].longName, "h2");
    }
    // - answer is native hash
    /* FIXME: as of 20170807, we do not support this scheme.
       Although it sounds somehow natural, it adds additional work for little/no gain.
       (a) hashes cannot be passed through RESP and will be flattened to key/value-pair arrays anyway.
       (b) key/value-pair arrays deal much better with keys being integers; we do have integer conversion
           primitives for scalars read from a vector, but none for hash keys.
       (c) the knowledge only matters in the HostPlayerClient/HostPlayerServer combo. Everyone else
           uses the C++ interface using a std::map. */
    // {
    //     Hash::Ref_t h1 = Hash::create();
    //     h1->setNew("long", server::makeStringValue("h1"));

    //     Hash::Ref_t h2 = Hash::create();
    //     h2->setNew("long", server::makeStringValue("h2"));

    //     Hash::Ref_t h = Hash::create();
    //     h->setNew("2", new HashValue(h1));
    //     h->setNew("5", new HashValue(h2));

    //     mock.expectCall("PLAYERLS, 7");
    //     mock.provideNewResult(new HashValue(h));

    //     std::map<int, HostPlayer::Info> result;
    //     TS_ASSERT_THROWS_NOTHING(testee.list(7, false, result));

    //     TS_ASSERT_EQUALS(result.size(), 2U);
    //     TS_ASSERT_EQUALS(result[2].longName, "h1");
    //     TS_ASSERT_EQUALS(result[5].longName, "h2");
    // }

    // - null answer
    {
        mock.expectCall("PLAYERLS, 3, ALL");
        mock.provideNewResult(0);
        std::map<int, HostPlayer::Info> result;
        TS_ASSERT_THROWS_NOTHING(testee.list(3, true, result));
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // setDirectory
    mock.expectCall("PLAYERSETDIR, 8, ux, d/i/r");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS_NOTHING(testee.setDirectory(8, "ux", "d/i/r"));

    // getDirectory
    mock.expectCall("PLAYERGETDIR, 32, uz");
    mock.provideNewResult(server::makeStringValue("dd"));
    TS_ASSERT_EQUALS(testee.getDirectory(32, "uz"), "dd");

    // checkFile
    mock.expectCall("PLAYERCHECKFILE, 5, uid, file.dat");
    mock.provideNewResult(server::makeStringValue("allow"));
    TS_ASSERT_EQUALS(testee.checkFile(5, "uid", "file.dat", afl::base::Nothing), HostPlayer::Allow);

    mock.expectCall("PLAYERCHECKFILE, 5, uid, file.dat, DIR, d");
    mock.provideNewResult(server::makeStringValue("refuse"));
    TS_ASSERT_EQUALS(testee.checkFile(5, "uid", "file.dat", String_t("d")), HostPlayer::Refuse);

    mock.checkFinish();
}

/** Test failure in return value. */
void
TestServerInterfaceHostPlayerClient::testFail()
{
    afl::test::CommandHandler mock("testFail");
    server::interface::HostPlayerClient testee(mock);

    mock.expectCall("PLAYERCHECKFILE, 5, uid, file.dat");
    mock.provideNewResult(server::makeStringValue("whatever"));
    TS_ASSERT_THROWS(testee.checkFile(5, "uid", "file.dat", afl::base::Nothing), std::exception);
}

