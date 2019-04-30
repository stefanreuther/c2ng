/**
  *  \file u/t_server_interface_usermanagementserver.cpp
  *  \brief Test for server::interface::UserManagementServer
  */

#include <stdexcept>
#include "server/interface/usermanagementserver.hpp"

#include "t_server_interface.hpp"
#include "server/interface/usermanagement.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/string/format.hpp"
#include "afl/data/access.hpp"
#include "server/interface/usermanagementclient.hpp"

using afl::string::Format;
using afl::data::Segment;
using server::interface::UserManagement;
using server::Value_t;

namespace {
    class UserManagementMock : public afl::test::CallReceiver, public UserManagement {
     public:
        UserManagementMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual String_t add(String_t userName, String_t password, afl::base::Memory<const String_t> config)
            {
                String_t check = Format("add(%s,%s", userName, password);
                while (const String_t* p = config.eat()) {
                    check += ",";
                    check += *p;
                }
                check += ")";
                checkCall(check);
                return consumeReturnValue<String_t>();
            }
        virtual String_t login(String_t userName, String_t password)
            {
                checkCall(Format("login(%s,%s)", userName, password));
                return consumeReturnValue<String_t>();
            }
        virtual String_t getUserIdByName(String_t userName)
            {
                checkCall(Format("getUserIdByName(%s)", userName));
                return consumeReturnValue<String_t>();
            }
        virtual String_t getNameByUserId(String_t userId)
            {
                checkCall(Format("getNameByUserId(%s)", userId));
                return consumeReturnValue<String_t>();
            }
        virtual void getNamesByUserId(afl::base::Memory<const String_t> userIds, afl::data::StringList_t& userNames)
            {
                String_t check;
                while (const String_t* p = userIds.eat()) {
                    if (!check.empty()) {
                        check += ",";
                    }
                    check += *p;
                }
                checkCall(Format("getNamesByUserId(%s)", check));

                size_t n = consumeReturnValue<size_t>();
                while (n > 0) {
                    userNames.push_back(consumeReturnValue<String_t>());
                    --n;
                }
            }
        virtual Value_t* getProfileRaw(String_t userId, String_t key)
            {
                checkCall(Format("getProfileRaw(%s,%s)", userId, key));
                return consumeReturnValue<Value_t*>();
            }
        virtual Value_t* getProfileRaw(String_t userId, afl::base::Memory<const String_t> keys)
            {
                String_t check = Format("getProfileRaw(List)(%s", userId);
                while (const String_t* p = keys.eat()) {
                    check += ",";
                    check += *p;
                }
                check += ")";
                checkCall(check);
                return consumeReturnValue<Value_t*>();
            }
        virtual void setProfile(String_t userId, afl::base::Memory<const String_t> config)
            {
                String_t check = Format("setProfile(%s", userId);
                while (const String_t* p = config.eat()) {
                    check += ",";
                    check += *p;
                }
                check += ")";
                checkCall(check);
            }
        virtual void setPassword(String_t userId, String_t password)
            {
                checkCall(Format("setPassword(%s,%s)", userId, password));
            }

    };
}

/** Test regular server calls. */
void
TestServerInterfaceUserManagementServer::testIt()
{
    UserManagementMock mock("testIt");
    server::interface::UserManagementServer testee(mock);

    // add
    mock.expectCall("add(uu,pp,kk,vv,kkk,vvv)");
    mock.provideReturnValue(String_t("id"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("ADDUSER").pushBackString("uu").pushBackString("pp") .pushBackString("kk").pushBackString("vv") .pushBackString("kkk").pushBackString("vvv")), "id");

    mock.expectCall("add(uu,pp)");
    mock.provideReturnValue(String_t("id2"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("ADDUSER").pushBackString("uu").pushBackString("pp")), "id2");

    // login
    mock.expectCall("login(nn,gg)");
    mock.provideReturnValue(String_t("qq"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("LOGIN").pushBackString("nn").pushBackString("gg")), "qq");

    // getUserIdByName
    mock.expectCall("getUserIdByName(who)");
    mock.provideReturnValue(String_t("ss"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("LOOKUP").pushBackString("who")), "ss");

    // getNameByUserId
    mock.expectCall("getNameByUserId(ss)");
    mock.provideReturnValue(String_t("who"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("NAME").pushBackString("ss")), "who");

    // getNamesByUserId
    {
        mock.expectCall("getNamesByUserId(alpha,bravo)");
        mock.provideReturnValue(size_t(2));
        mock.provideReturnValue(String_t("charlie"));
        mock.provideReturnValue(String_t("kilo"));

        std::auto_ptr<Value_t> result(testee.call(Segment().pushBackString("MNAME").pushBackString("alpha").pushBackString("bravo")));
        afl::data::Access a(result);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0].toString(), "charlie");
        TS_ASSERT_EQUALS(a[1].toString(), "kilo");
    }

    // getProfileRaw
    {
        mock.expectCall("getProfileRaw(uu,ky)");
        mock.provideReturnValue(server::makeIntegerValue(42));

        std::auto_ptr<Value_t> result(testee.call(Segment().pushBackString("GET").pushBackString("uu").pushBackString("ky")));
        afl::data::Access a(result);
        TS_ASSERT_EQUALS(a.toInteger(), 42);
    }
    {
        // Check that we can pass null
        mock.expectCall("getProfileRaw(uu,kn)");
        mock.provideReturnValue(static_cast<Value_t*>(0));

        std::auto_ptr<Value_t> result(testee.call(Segment().pushBackString("GET").pushBackString("uu").pushBackString("kn")));
        TS_ASSERT(result.get() == 0);
    }

    // getProfileRaw
    {
        // FIXME: For now we are passing the raw result; nothing yet enforces that the result should be an array.
        // Thus we only check that the value is properly passed back.
        mock.expectCall("getProfileRaw(List)(uu,k1,k2)");
        mock.provideReturnValue(server::makeIntegerValue(77));

        std::auto_ptr<Value_t> result(testee.call(Segment().pushBackString("MGET").pushBackString("uu").pushBackString("k1").pushBackString("k2")));
        afl::data::Access a(result);
        TS_ASSERT_EQUALS(a.toInteger(), 77);
    }

    // setProfile
    mock.expectCall("setProfile(u,k,v)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("SET").pushBackString("u").pushBackString("k").pushBackString("v")));

    // setPassword
    mock.expectCall("setPassword(u,s3cr3t)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("PASSWD").pushBackString("u").pushBackString("s3cr3t")));

    // Variant
    mock.expectCall("setPassword(u,q)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("passwd").pushBackString("u").pushBackString("q")));

    mock.checkFinish();
}

/** Test erroneous calls. */
void
TestServerInterfaceUserManagementServer::testErrors()
{
    UserManagementMock mock("testErrors");
    server::interface::UserManagementServer testee(mock);

    // Too short
    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);

    // Wrong verb
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("hi")), std::exception);

    // Wrong parameter count
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PASSWD")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PASSWD").pushBackString("a")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PASSWD").pushBackString("a").pushBackString("a").pushBackString("a")), std::exception);

    // Not detected: add() or setProfile() with an odd number of k,v arguments
}

/** Test roundtrip operation with UserManagementClient. */
void
TestServerInterfaceUserManagementServer::testRoundtrip()
{
    UserManagementMock mock("testRoundtrip");
    server::interface::UserManagementServer level1(mock);
    server::interface::UserManagementClient level2(level1);
    server::interface::UserManagementServer level3(level2);
    server::interface::UserManagementClient level4(level3);

    // add
    {
        const String_t kvs[] = { "kk", "vv", "kkk", "vvv" };
        mock.expectCall("add(uu,pp,kk,vv,kkk,vvv)");
        mock.provideReturnValue(String_t("id"));
        TS_ASSERT_EQUALS(level4.add("uu", "pp", kvs), "id");
    }
    {
        mock.expectCall("add(uu,pp)");
        mock.provideReturnValue(String_t("id2"));
        TS_ASSERT_EQUALS(level4.add("uu", "pp", afl::base::Nothing), "id2");
    }

    // login
    mock.expectCall("login(nn,gg)");
    mock.provideReturnValue(String_t("qq"));
    TS_ASSERT_EQUALS(level4.login("nn", "gg"), "qq");

    // getUserIdByName
    mock.expectCall("getUserIdByName(who)");
    mock.provideReturnValue(String_t("ss"));
    TS_ASSERT_EQUALS(level4.getUserIdByName("who"), "ss");

    // getNameByUserId
    mock.expectCall("getNameByUserId(ss)");
    mock.provideReturnValue(String_t("who"));
    TS_ASSERT_EQUALS(level4.getNameByUserId("ss"), "who");

    // getNamesByUserId
    {
        mock.expectCall("getNamesByUserId(alpha,bravo)");
        mock.provideReturnValue(size_t(2));
        mock.provideReturnValue(String_t("charlie"));
        mock.provideReturnValue(String_t("kilo"));

        const String_t names[] = {"alpha", "bravo"};

        afl::data::StringList_t result;
        level4.getNamesByUserId(names, result);
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0], "charlie");
        TS_ASSERT_EQUALS(result[1], "kilo");
    }

    // getProfileRaw
    {
        mock.expectCall("getProfileRaw(uu,ky)");
        mock.provideReturnValue(server::makeIntegerValue(42));

        std::auto_ptr<Value_t> result(level4.getProfileRaw("uu", "ky"));
        afl::data::Access a(result);
        TS_ASSERT_EQUALS(a.toInteger(), 42);
    }

    // getProfileRaw
    {
        // FIXME: For now we are passing the raw result; nothing yet enforces that the result should be an array.
        // Thus we only check that the value is properly passed back.
        mock.expectCall("getProfileRaw(List)(uu,k1,k2)");
        mock.provideReturnValue(server::makeIntegerValue(77));

        const String_t ks[] = {"k1", "k2"};
        std::auto_ptr<Value_t> result(level4.getProfileRaw("uu", ks));
        afl::data::Access a(result);
        TS_ASSERT_EQUALS(a.toInteger(), 77);
    }

    // setProfile
    {
        const String_t kvs[] = {"k", "v"};
        mock.expectCall("setProfile(u,k,v)");
        TS_ASSERT_THROWS_NOTHING(level4.setProfile("u", kvs));
    }

    // setPassword
    mock.expectCall("setPassword(u,s3cr3t)");
    TS_ASSERT_THROWS_NOTHING(level4.setPassword("u", "s3cr3t"));

    mock.checkFinish();
}

