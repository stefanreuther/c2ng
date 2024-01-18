/**
  *  \file test/server/interface/usermanagementclienttest.cpp
  *  \brief Test for server::interface::UserManagementClient
  */

#include "server/interface/usermanagementclient.hpp"

#include "afl/data/access.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"
#include <memory>

/** Test regular client operation. */
AFL_TEST("server.interface.UserManagementClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::UserManagementClient testee(mock);

    // add
    mock.expectCall("ADDUSER, u, p");
    mock.provideNewResult(server::makeStringValue("i"));
    a.checkEqual("01. add", testee.add("u", "p", afl::base::Nothing), "i");

    {
        const String_t kv[] = { "kk", "vv" };
        mock.expectCall("ADDUSER, u2, p2, kk, vv");
        mock.provideNewResult(server::makeStringValue("i2"));
        a.checkEqual("11. add", testee.add("u2", "p2", kv), "i2");
    }

    // remove
    mock.expectCall("DELUSER, kk");
    mock.provideNewResult(server::makeStringValue("OK"));
    AFL_CHECK_SUCCEEDS(a("21. remove"), testee.remove("kk"));

    // login
    mock.expectCall("LOGIN, n, pw");
    mock.provideNewResult(server::makeStringValue("id"));
    a.checkEqual("31. login", testee.login("n", "pw"), "id");

    // getUserIdByName
    mock.expectCall("LOOKUP, ww");
    mock.provideNewResult(server::makeStringValue("nn"));
    a.checkEqual("41. getUserIdByName", testee.getUserIdByName("ww"), "nn");

    // getNameByUserId
    mock.expectCall("NAME, qq");
    mock.provideNewResult(server::makeStringValue("rr"));
    a.checkEqual("51. getNameByUserId", testee.getNameByUserId("qq"), "rr");

    // getNamesByUserId
    {
        const String_t ids[] = { "one", "two", "three" };
        mock.expectCall("MNAME, one, two, three");

        afl::data::Vector::Ref_t vec = afl::data::Vector::create();
        vec->pushBackString("11");
        vec->pushBackString("22");
        vec->pushBackString("33");
        mock.provideNewResult(new afl::data::VectorValue(vec));

        afl::data::StringList_t names;
        AFL_CHECK_SUCCEEDS(a("61. getNamesByUserId"), testee.getNamesByUserId(ids, names));
        a.checkEqual("62. size", names.size(), 3U);
        a.checkEqual("63. result", names[0], "11");
        a.checkEqual("64. result", names[1], "22");
        a.checkEqual("65. result", names[2], "33");
    }

    // getProfileRaw
    {
        mock.expectCall("GET, uz, kk");
        mock.provideNewResult(server::makeStringValue("The Value"));
        std::auto_ptr<server::Value_t> v(testee.getProfileRaw("uz", "kk"));
        a.checkEqual("71. getProfileRaw", server::toString(v.get()), "The Value");
    }
    {
        // Make sure the protocol passes null values correctly
        mock.expectCall("GET, uz, kk");
        mock.provideNewResult(0);
        std::auto_ptr<server::Value_t> v(testee.getProfileRaw("uz", "kk"));
        a.checkNull("72. getProfileRaw", v.get());
    }

    // getProfileRaw (2)
    {
        const String_t keys[] = { "a", "b", "cc" };
        mock.expectCall("MGET, id, a, b, cc");

        afl::data::Vector::Ref_t vec = afl::data::Vector::create();
        vec->pushBackString("vv");
        vec->pushBackString("ww");
        vec->pushBackString("xxx");
        mock.provideNewResult(new afl::data::VectorValue(vec));

        std::auto_ptr<server::Value_t> result(testee.getProfileRaw("id", keys));
        afl::data::Access ap(result);
        a.checkEqual("81. getArraySize", ap.getArraySize(), 3U);
        a.checkEqual("82. result", ap[0].toString(), "vv");
        a.checkEqual("83. result", ap[1].toString(), "ww");
        a.checkEqual("84. result", ap[2].toString(), "xxx");
    }

    // setProfile
    {
        const String_t kv[] = { "kk", "vv" };
        mock.expectCall("SET, uu, kk, vv");
        mock.provideNewResult(0);
        AFL_CHECK_SUCCEEDS(a("91. setProfile"), testee.setProfile("uu", kv));
    }

    // setPassword
    mock.expectCall("PASSWD, u, secret");
    mock.provideNewResult(0);
    AFL_CHECK_SUCCEEDS(a("101. setPassword"), testee.setPassword("u", "secret"));

    mock.checkFinish();
}
