/**
  *  \file u/t_server_interface_usermanagementclient.cpp
  *  \brief Test for server::interface::UserManagementClient
  */

#include <memory>
#include "server/interface/usermanagementclient.hpp"

#include "t_server_interface.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/access.hpp"

/** Test regular client operation. */
void
TestServerInterfaceUserManagementClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::UserManagementClient testee(mock);

    // add
    mock.expectCall("ADDUSER, u, p");
    mock.provideNewResult(server::makeStringValue("i"));
    TS_ASSERT_EQUALS(testee.add("u", "p", afl::base::Nothing), "i");

    {
        const String_t kv[] = { "kk", "vv" };
        mock.expectCall("ADDUSER, u2, p2, kk, vv");
        mock.provideNewResult(server::makeStringValue("i2"));
        TS_ASSERT_EQUALS(testee.add("u2", "p2", kv), "i2");
    }

    // remove
    mock.expectCall("DELUSER, kk");
    mock.provideNewResult(server::makeStringValue("OK"));
    TS_ASSERT_THROWS_NOTHING(testee.remove("kk"));

    // login
    mock.expectCall("LOGIN, n, pw");
    mock.provideNewResult(server::makeStringValue("id"));
    TS_ASSERT_EQUALS(testee.login("n", "pw"), "id");

    // getUserIdByName
    mock.expectCall("LOOKUP, ww");
    mock.provideNewResult(server::makeStringValue("nn"));
    TS_ASSERT_EQUALS(testee.getUserIdByName("ww"), "nn");

    // getNameByUserId
    mock.expectCall("NAME, qq");
    mock.provideNewResult(server::makeStringValue("rr"));
    TS_ASSERT_EQUALS(testee.getNameByUserId("qq"), "rr");

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
        TS_ASSERT_THROWS_NOTHING(testee.getNamesByUserId(ids, names));
        TS_ASSERT_EQUALS(names.size(), 3U);
        TS_ASSERT_EQUALS(names[0], "11");
        TS_ASSERT_EQUALS(names[1], "22");
        TS_ASSERT_EQUALS(names[2], "33");
    }

    // getProfileRaw
    {
        mock.expectCall("GET, uz, kk");
        mock.provideNewResult(server::makeStringValue("The Value"));
        std::auto_ptr<server::Value_t> v(testee.getProfileRaw("uz", "kk"));
        TS_ASSERT_EQUALS(server::toString(v.get()), "The Value");
    }
    {
        // Make sure the protocol passes null values correctly
        mock.expectCall("GET, uz, kk");
        mock.provideNewResult(0);
        std::auto_ptr<server::Value_t> v(testee.getProfileRaw("uz", "kk"));
        TS_ASSERT(v.get() == 0);
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
        afl::data::Access a(result);
        TS_ASSERT_EQUALS(a.getArraySize(), 3U);
        TS_ASSERT_EQUALS(a[0].toString(), "vv");
        TS_ASSERT_EQUALS(a[1].toString(), "ww");
        TS_ASSERT_EQUALS(a[2].toString(), "xxx");
    }

    // setProfile
    {
        const String_t kv[] = { "kk", "vv" };
        mock.expectCall("SET, uu, kk, vv");
        mock.provideNewResult(0);
        TS_ASSERT_THROWS_NOTHING(testee.setProfile("uu", kv));
    }

    // setPassword
    mock.expectCall("PASSWD, u, secret");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS_NOTHING(testee.setPassword("u", "secret"));

    mock.checkFinish();
}

