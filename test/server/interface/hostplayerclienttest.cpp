/**
  *  \file test/server/interface/hostplayerclienttest.cpp
  *  \brief Test for server::interface::HostPlayerClient
  */

#include "server/interface/hostplayerclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"
#include <stdexcept>

using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Hash;
using afl::data::HashValue;
using server::interface::HostPlayer;

/** Simple tests. */
AFL_TEST("server.interface.HostPlayerClient:commands", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::HostPlayerClient testee(mock);

    // join
    mock.expectCall("PLAYERJOIN, 42, 3, uu");
    mock.provideNewResult(0);
    AFL_CHECK_SUCCEEDS(a("01. join"), testee.join(42, 3, "uu"));

    // substitute
    mock.expectCall("PLAYERSUBST, 56, 1, zz");
    mock.provideNewResult(0);
    AFL_CHECK_SUCCEEDS(a("11. substitute"), testee.substitute(56, 1, "zz"));

    // resign
    mock.expectCall("PLAYERRESIGN, 23, 3, a");
    mock.provideNewResult(0);
    AFL_CHECK_SUCCEEDS(a("21. resign"), testee.resign(23, 3, "a"));

    // add
    mock.expectCall("PLAYERADD, 93, pp");
    mock.provideNewResult(0);
    AFL_CHECK_SUCCEEDS(a("31. add"), testee.add(93, "pp"));

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
        a.checkEqual("41. longName",      i.longName, "Long");
        a.checkEqual("42. shortName",     i.shortName, "Short");
        a.checkEqual("43. adjectiveName", i.adjectiveName, "Adjective");
        a.checkEqual("44. userIds",       i.userIds.size(), 3U);
        a.checkEqual("45. userIds",       i.userIds[0], "fred");
        a.checkEqual("46. userIds",       i.userIds[1], "barney");
        a.checkEqual("47. userIds",       i.userIds[2], "wilma");
        a.checkEqual("48. numEditable",   i.numEditable, 2);
        a.checkEqual("49. joinable",      i.joinable, true);
    }
    // - no response, deserialized as default
    {
        mock.expectCall("PLAYERSTAT, 17, 3");
        mock.provideNewResult(0);

        HostPlayer::Info i = testee.getInfo(17, 3);
        a.checkEqual("51. longName",      i.longName, "");
        a.checkEqual("52. shortName",     i.shortName, "");
        a.checkEqual("53. adjectiveName", i.adjectiveName, "");
        a.checkEqual("54. userIds",       i.userIds.size(), 0U);
        a.checkEqual("55. numEditable",   i.numEditable, 0);
        a.checkEqual("56. joinable",      i.joinable, false);
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
        AFL_CHECK_SUCCEEDS(a("61. list"), testee.list(7, false, result));

        a.checkEqual("71. size",     result.size(), 2U);
        a.checkEqual("72. longName", result[2].longName, "h1");
        a.checkEqual("73. longName", result[5].longName, "h2");
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
    //     AFL_CHECK_SUCCEEDS(a("81. list"), testee.list(7, false, result));

    //     a.checkEqual("91. size", result.size(), 2U);
    //     a.checkEqual("92. longName", result[2].longName, "h1");
    //     a.checkEqual("93. longName", result[5].longName, "h2");
    // }

    // - null answer
    {
        mock.expectCall("PLAYERLS, 3, ALL");
        mock.provideNewResult(0);
        std::map<int, HostPlayer::Info> result;
        AFL_CHECK_SUCCEEDS(a("101. list"), testee.list(3, true, result));
        a.checkEqual("102. size", result.size(), 0U);
    }

    // setDirectory
    mock.expectCall("PLAYERSETDIR, 8, ux, d/i/r");
    mock.provideNewResult(0);
    AFL_CHECK_SUCCEEDS(a("111. setDirectory"), testee.setDirectory(8, "ux", "d/i/r"));

    // getDirectory
    mock.expectCall("PLAYERGETDIR, 32, uz");
    mock.provideNewResult(server::makeStringValue("dd"));
    a.checkEqual("121. getDirectory", testee.getDirectory(32, "uz"), "dd");

    // checkFile
    mock.expectCall("PLAYERCHECKFILE, 5, uid, file.dat");
    mock.provideNewResult(server::makeStringValue("allow"));
    a.checkEqual("131. checkFile", testee.checkFile(5, "uid", "file.dat", afl::base::Nothing), HostPlayer::Allow);

    mock.expectCall("PLAYERCHECKFILE, 5, uid, file.dat, DIR, d");
    mock.provideNewResult(server::makeStringValue("refuse"));
    a.checkEqual("141. checkFile", testee.checkFile(5, "uid", "file.dat", String_t("d")), HostPlayer::Refuse);

    // get
    mock.expectCall("PLAYERGET, 17, uu, kk");
    mock.provideNewResult(server::makeStringValue("the value"));
    a.checkEqual("151. get", testee.get(17, "uu", "kk"), "the value");

    // set
    mock.expectCall("PLAYERSET, 32, mm, nn, oo");
    mock.provideNewResult(server::makeStringValue("OK"));
    AFL_CHECK_SUCCEEDS(a("161. set"), testee.set(32, "mm", "nn", "oo"));

    mock.checkFinish();
}

/** Test failure in return value. */
AFL_TEST("server.interface.HostPlayerClient:error", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::HostPlayerClient testee(mock);

    mock.expectCall("PLAYERCHECKFILE, 5, uid, file.dat");
    mock.provideNewResult(server::makeStringValue("whatever"));
    AFL_CHECK_THROWS(a("01. checkFile"), testee.checkFile(5, "uid", "file.dat", afl::base::Nothing), std::exception);
}
