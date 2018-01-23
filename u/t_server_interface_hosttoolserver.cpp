/**
  *  \file u/t_server_interface_hosttoolserver.cpp
  *  \brief Test for server::interface::HostToolServer
  */

#include "server/interface/hosttoolserver.hpp"

#include <stdexcept>
#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/hosttoolclient.hpp"

using afl::string::Format;
using afl::data::Segment;
using server::interface::HostTool;

namespace {
    class HostToolMock : public HostTool, public afl::test::CallReceiver {
     public:
        HostToolMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual void add(String_t id, String_t path, String_t program, String_t kind)
            { checkCall(Format("add(%s,%s,%s,%s)", id, path, program, kind)); }
        virtual void set(String_t id, String_t key, String_t value)
            { checkCall(Format("set(%s,%s,%s)", id, key, value)); }
        virtual String_t get(String_t id, String_t key)
            {
                checkCall(Format("get(%s,%s)", id, key));
                return consumeReturnValue<String_t>();
            }
        virtual bool remove(String_t id)
            {
                checkCall(Format("remove(%s)", id));
                return consumeReturnValue<bool>();
            }
        virtual void getAll(std::vector<Info>& result)
            {
                checkCall("getAll()");
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    result.push_back(consumeReturnValue<Info>());
                }
            }
        virtual void copy(String_t sourceId, String_t destinationId)
            { checkCall(Format("copy(%s,%s)", sourceId, destinationId)); }
        virtual void setDefault(String_t id)
            { checkCall(Format("setDefault(%s)", id)); }
        virtual int32_t getDifficulty(String_t id)
            {
                checkCall(Format("getDifficulty(%s)", id));
                return consumeReturnValue<int>();
            }
        virtual void clearDifficulty(String_t id)
            { checkCall(Format("clearDifficulty(%s)", id)); }
        virtual int32_t setDifficulty(String_t id, afl::base::Optional<int32_t> value, bool use)
            {
                checkCall(Format("setDifficulty(%s,%d,%s)", id, value.orElse(-1), use?"true":"false"));
                return consumeReturnValue<int>();
            }
    };
}

/** Test HostToolServer. */
void
TestServerInterfaceHostToolServer::testIt()
{
    HostToolMock mock("testIt");
    server::interface::HostToolServer testee(mock, HostTool::Host);

    // add
    mock.expectCall("add(i,p,x,k)");
    testee.callVoid(Segment().pushBackString("HOSTADD").pushBackString("i").pushBackString("p").pushBackString("x").pushBackString("k"));

    // set
    mock.expectCall("set(id,key,val)");
    testee.callVoid(Segment().pushBackString("HOSTSET").pushBackString("id").pushBackString("key").pushBackString("val"));

    // get
    mock.expectCall("get(qi,qk)");
    mock.provideReturnValue(String_t("qr"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("HOSTGET").pushBackString("qi").pushBackString("qk")), "qr");

    // rm
    mock.expectCall("remove(x)");
    mock.provideReturnValue(true);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("HOSTRM").pushBackString("x")), 1);

    // ls
    {
        mock.expectCall("getAll()");
        mock.provideReturnValue(3);
        mock.provideReturnValue(HostTool::Info("aa", "blah a", "ka", false));
        mock.provideReturnValue(HostTool::Info("bb", "blah b", "kb", true));
        mock.provideReturnValue(HostTool::Info("cc", "blah c", "kc", false));

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("HOSTLS")));
        TS_ASSERT(p.get() != 0);

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 3U);
        TS_ASSERT_EQUALS(a[0]("id").toString(), "aa");
        TS_ASSERT_EQUALS(a[0]("description").toString(), "blah a");
        TS_ASSERT_EQUALS(a[0]("kind").toString(), "ka");
        TS_ASSERT_EQUALS(a[0]("default").toInteger(), 0);

        TS_ASSERT_EQUALS(a[1]("id").toString(), "bb");
        TS_ASSERT_EQUALS(a[1]("default").toInteger(), 1);
    }

    // cp
    mock.expectCall("copy(f,t)");
    testee.callVoid(Segment().pushBackString("HOSTCP").pushBackString("f").pushBackString("t"));

    // default
    mock.expectCall("setDefault(dh)");
    testee.callVoid(Segment().pushBackString("HOSTDEFAULT").pushBackString("dh"));

    // rating
    mock.expectCall("getDifficulty(dt)");
    mock.provideReturnValue(18);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("HOSTRATING").pushBackString("dt").pushBackString("GET")), 18);

    mock.expectCall("clearDifficulty(et)");
    testee.callVoid(Segment().pushBackString("HOSTRATING").pushBackString("et").pushBackString("NONE"));

    mock.expectCall("setDifficulty(ft,9,false)");
    mock.provideReturnValue(107);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("HOSTRATING").pushBackString("ft").pushBackString("SET").pushBackInteger(9).pushBackString("SHOW")), 107);
    mock.expectCall("setDifficulty(ft,19,true)");
    mock.provideReturnValue(98);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("HOSTRATING").pushBackString("ft").pushBackString("SET").pushBackInteger(19).pushBackString("USE")), 98);
    mock.expectCall("setDifficulty(ft,-1,false)");
    mock.provideReturnValue(42);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("HOSTRATING").pushBackString("ft").pushBackString("AUTO").pushBackString("SHOW")), 42);
    mock.expectCall("setDifficulty(ft,-1,true)");
    mock.provideReturnValue(77);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("HOSTRATING").pushBackString("ft").pushBackString("AUTO").pushBackString("USE")), 77);

    // Variants
    mock.expectCall("setDefault(dh)");
    testee.callVoid(Segment().pushBackString("hostdefault").pushBackString("dh"));
    mock.expectCall("setDifficulty(ft,-1,true)");
    mock.provideReturnValue(-99);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("hostRating").pushBackString("ft").pushBackString("Auto").pushBackString("use")), -99);

    mock.checkFinish();
}

/** Test erroneous calls. */
void
TestServerInterfaceHostToolServer::testErrors()
{
    HostToolMock mock("testErrors");
    server::interface::HostToolServer testee(mock, HostTool::Host);

    // Parameter count
    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HOSTDEFAULT")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HOSTDEFAULT").pushBackString("a").pushBackString("b")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HOSTRATING").pushBackString("q")), std::exception);

    // Bad options
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HOSTRATING").pushBackString("q").pushBackString("CLEAR")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HOSTRATING").pushBackString("q").pushBackString("AUTO").pushBackString("x")), std::exception);

    // Bad commands
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("MASTERDEFAULT")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("TOOLDEFAULT")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("SHIPLISTDEFAULT")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("DEFAULT")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HOST")), std::exception);
}

/** Test roundtrip with HostToolClient. */
void
TestServerInterfaceHostToolServer::testRoundtrip()
{
    HostToolMock mock("testRoundtrip");
    server::interface::HostToolServer level1(mock, HostTool::Host);
    server::interface::HostToolClient level2(level1, HostTool::Host);
    server::interface::HostToolServer level3(level2, HostTool::Host);
    server::interface::HostToolClient level4(level3, HostTool::Host);

    // add
    mock.expectCall("add(xi,xp,xf,xk)");
    level4.add("xi", "xp", "xf", "xk");

    // set
    mock.expectCall("set(si,sk,sv)");
    level4.set("si", "sk", "sv");

    // get
    mock.expectCall("get(gi,gk)");
    mock.provideReturnValue<String_t>("gr");
    TS_ASSERT_EQUALS(level4.get("gi", "gk"), "gr");

    // remove
    mock.expectCall("remove(dd)");
    mock.provideReturnValue(true);
    TS_ASSERT_EQUALS(level4.remove("dd"), true);

    // ls
    {
        mock.expectCall("getAll()");
        mock.provideReturnValue(2);
        mock.provideReturnValue(HostTool::Info("first", "first info", "first kind", true));
        mock.provideReturnValue(HostTool::Info("second", "second info", "second kind", false));

        std::vector<HostTool::Info> result;
        level4.getAll(result);

        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0].id, "first");
        TS_ASSERT_EQUALS(result[0].description, "first info");
        TS_ASSERT_EQUALS(result[0].kind, "first kind");
        TS_ASSERT_EQUALS(result[0].isDefault, true);
        TS_ASSERT_EQUALS(result[1].id, "second");
        TS_ASSERT_EQUALS(result[1].description, "second info");
        TS_ASSERT_EQUALS(result[1].kind, "second kind");
        TS_ASSERT_EQUALS(result[1].isDefault, false);
    }

    // cp
    mock.expectCall("copy(in,out)");
    level4.copy("in", "out");

    // default
    mock.expectCall("setDefault(zz)");
    level4.setDefault("zz");

    // getDifficulty
    mock.expectCall("getDifficulty(rq)");
    mock.provideReturnValue<int>(55);
    TS_ASSERT_EQUALS(level4.getDifficulty("rq"), 55);

    // clearDifficulty
    mock.expectCall("clearDifficulty(rc)");
    level4.clearDifficulty("rc");

    // setDifficulty
    mock.expectCall("setDifficulty(rs,99,false)");
    mock.provideReturnValue(105);
    TS_ASSERT_EQUALS(level4.setDifficulty("rs", 99, false), 105);
    mock.expectCall("setDifficulty(rs,-1,true)");
    mock.provideReturnValue(88);
    TS_ASSERT_EQUALS(level4.setDifficulty("rs", afl::base::Nothing, true), 88);

    mock.checkFinish();
}

/** Test different areas. */
void
TestServerInterfaceHostToolServer::testArea()
{
    HostToolMock mock("testArea");

    // Host
    mock.expectCall("copy(a,b)");
    server::interface::HostToolServer(mock, HostTool::Host).callVoid(Segment().pushBackString("HOSTCP").pushBackString("a").pushBackString("b"));

    // Shiplist
    mock.expectCall("copy(a,b)");
    server::interface::HostToolServer(mock, HostTool::ShipList).callVoid(Segment().pushBackString("SHIPLISTCP").pushBackString("a").pushBackString("b"));

    // Host
    mock.expectCall("copy(a,b)");
    server::interface::HostToolServer(mock, HostTool::Master).callVoid(Segment().pushBackString("MASTERCP").pushBackString("a").pushBackString("b"));

    // Host
    mock.expectCall("copy(a,b)");
    server::interface::HostToolServer(mock, HostTool::Tool).callVoid(Segment().pushBackString("TOOLCP").pushBackString("a").pushBackString("b"));

    mock.checkFinish();
}

/** Test area mismatch with client. */
void
TestServerInterfaceHostToolServer::testAreaMismatch()
{
    {
        // Mismatch at a Server->Client transition is not detected because we're just chaining C++ calls here.
        HostToolMock mock("testAreaMismatch");
        server::interface::HostToolServer level1(mock, HostTool::Host);
        server::interface::HostToolClient level2(level1, HostTool::Host);
        server::interface::HostToolServer level3(level2, HostTool::Master);
        server::interface::HostToolClient level4(level3, HostTool::Master);

        mock.expectCall("getDifficulty(x)");
        mock.provideReturnValue<int>(17);
        TS_ASSERT_EQUALS(level4.getDifficulty("x"), 17);
    }
    {
        // Mismatch at a Client->Server transition is detected due to command name mismatch.
        HostToolMock mock("testAreaMismatch 2");
        server::interface::HostToolServer level1(mock, HostTool::Host);
        server::interface::HostToolClient level2(level1, HostTool::Master);
        server::interface::HostToolServer level3(level2, HostTool::Master);
        server::interface::HostToolClient level4(level3, HostTool::Master);

        TS_ASSERT_THROWS(level4.getDifficulty("x"), std::exception);
    }
}
