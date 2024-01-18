/**
  *  \file test/server/interface/hosttoolservertest.cpp
  *  \brief Test for server::interface::HostToolServer
  */

#include "server/interface/hosttoolserver.hpp"

#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/hosttoolclient.hpp"
#include <stdexcept>

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
AFL_TEST("server.interface.HostToolServer:commands", a)
{
    HostToolMock mock(a);
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
    a.checkEqual("01. hostget", testee.callString(Segment().pushBackString("HOSTGET").pushBackString("qi").pushBackString("qk")), "qr");

    // rm
    mock.expectCall("remove(x)");
    mock.provideReturnValue(true);
    a.checkEqual("11. hostrm", testee.callInt(Segment().pushBackString("HOSTRM").pushBackString("x")), 1);

    // ls
    {
        mock.expectCall("getAll()");
        mock.provideReturnValue(3);
        mock.provideReturnValue(HostTool::Info("aa", "blah a", "ka", false));
        mock.provideReturnValue(HostTool::Info("bb", "blah b", "kb", true));
        mock.provideReturnValue(HostTool::Info("cc", "blah c", "kc", false));

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("HOSTLS")));
        a.checkNonNull("21. hostls", p.get());

        afl::data::Access ap(p);
        a.checkEqual("31. getArraySize", ap.getArraySize(), 3U);
        a.checkEqual("32. id",          ap[0]("id").toString(), "aa");
        a.checkEqual("33. description", ap[0]("description").toString(), "blah a");
        a.checkEqual("34. kind",        ap[0]("kind").toString(), "ka");
        a.checkEqual("35. default",     ap[0]("default").toInteger(), 0);

        a.checkEqual("41. id",          ap[1]("id").toString(), "bb");
        a.checkEqual("42. default",     ap[1]("default").toInteger(), 1);
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
    a.checkEqual("51. hostrating", testee.callInt(Segment().pushBackString("HOSTRATING").pushBackString("dt").pushBackString("GET")), 18);

    mock.expectCall("clearDifficulty(et)");
    testee.callVoid(Segment().pushBackString("HOSTRATING").pushBackString("et").pushBackString("NONE"));

    mock.expectCall("setDifficulty(ft,9,false)");
    mock.provideReturnValue(107);
    a.checkEqual("61. hostrating", testee.callInt(Segment().pushBackString("HOSTRATING").pushBackString("ft").pushBackString("SET").pushBackInteger(9).pushBackString("SHOW")), 107);
    mock.expectCall("setDifficulty(ft,19,true)");
    mock.provideReturnValue(98);
    a.checkEqual("62. hostrating", testee.callInt(Segment().pushBackString("HOSTRATING").pushBackString("ft").pushBackString("SET").pushBackInteger(19).pushBackString("USE")), 98);
    mock.expectCall("setDifficulty(ft,-1,false)");
    mock.provideReturnValue(42);
    a.checkEqual("63. hostrating", testee.callInt(Segment().pushBackString("HOSTRATING").pushBackString("ft").pushBackString("AUTO").pushBackString("SHOW")), 42);
    mock.expectCall("setDifficulty(ft,-1,true)");
    mock.provideReturnValue(77);
    a.checkEqual("64. hostrating", testee.callInt(Segment().pushBackString("HOSTRATING").pushBackString("ft").pushBackString("AUTO").pushBackString("USE")), 77);

    // Variants
    mock.expectCall("setDefault(dh)");
    testee.callVoid(Segment().pushBackString("hostdefault").pushBackString("dh"));
    mock.expectCall("setDifficulty(ft,-1,true)");
    mock.provideReturnValue(-99);
    a.checkEqual("71. hostrating", testee.callInt(Segment().pushBackString("hostRating").pushBackString("ft").pushBackString("Auto").pushBackString("use")), -99);

    mock.checkFinish();
}

/** Test erroneous calls. */
AFL_TEST("server.interface.HostToolServer:errors", a)
{
    HostToolMock mock(a);
    server::interface::HostToolServer testee(mock, HostTool::Host);

    // Parameter count
    Segment empty;
    AFL_CHECK_THROWS(a("01. empty"), testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. missing arg"), testee.callVoid(Segment().pushBackString("HOSTDEFAULT")), std::exception);
    AFL_CHECK_THROWS(a("03. missing arg"), testee.callVoid(Segment().pushBackString("HOSTDEFAULT").pushBackString("a").pushBackString("b")), std::exception);
    AFL_CHECK_THROWS(a("04. missing arg"), testee.callVoid(Segment().pushBackString("HOSTRATING").pushBackString("q")), std::exception);

    // Bad options
    AFL_CHECK_THROWS(a("11. bad option"), testee.callVoid(Segment().pushBackString("HOSTRATING").pushBackString("q").pushBackString("CLEAR")), std::exception);
    AFL_CHECK_THROWS(a("12. bad option"), testee.callVoid(Segment().pushBackString("HOSTRATING").pushBackString("q").pushBackString("AUTO").pushBackString("x")), std::exception);

    // Bad commands
    AFL_CHECK_THROWS(a("21. bad verb"), testee.callVoid(Segment().pushBackString("")), std::exception);
    AFL_CHECK_THROWS(a("22. bad verb"), testee.callVoid(Segment().pushBackString("MASTERDEFAULT")), std::exception);
    AFL_CHECK_THROWS(a("23. bad verb"), testee.callVoid(Segment().pushBackString("TOOLDEFAULT")), std::exception);
    AFL_CHECK_THROWS(a("24. bad verb"), testee.callVoid(Segment().pushBackString("SHIPLISTDEFAULT")), std::exception);
    AFL_CHECK_THROWS(a("25. bad verb"), testee.callVoid(Segment().pushBackString("DEFAULT")), std::exception);
    AFL_CHECK_THROWS(a("26. bad verb"), testee.callVoid(Segment().pushBackString("HOST")), std::exception);
}

/** Test roundtrip with HostToolClient. */
AFL_TEST("server.interface.HostToolServer:roundtrip", a)
{
    HostToolMock mock(a);
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
    a.checkEqual("01. get", level4.get("gi", "gk"), "gr");

    // remove
    mock.expectCall("remove(dd)");
    mock.provideReturnValue(true);
    a.checkEqual("11. remove", level4.remove("dd"), true);

    // ls
    {
        mock.expectCall("getAll()");
        mock.provideReturnValue(2);
        mock.provideReturnValue(HostTool::Info("first", "first info", "first kind", true));
        mock.provideReturnValue(HostTool::Info("second", "second info", "second kind", false));

        std::vector<HostTool::Info> result;
        level4.getAll(result);

        a.checkEqual("21. size",        result.size(), 2U);
        a.checkEqual("22. id",          result[0].id, "first");
        a.checkEqual("23. description", result[0].description, "first info");
        a.checkEqual("24. kind",        result[0].kind, "first kind");
        a.checkEqual("25. isDefault",   result[0].isDefault, true);
        a.checkEqual("26. id",          result[1].id, "second");
        a.checkEqual("27. description", result[1].description, "second info");
        a.checkEqual("28. kind",        result[1].kind, "second kind");
        a.checkEqual("29. isDefault",   result[1].isDefault, false);
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
    a.checkEqual("31. getDifficulty", level4.getDifficulty("rq"), 55);

    // clearDifficulty
    mock.expectCall("clearDifficulty(rc)");
    level4.clearDifficulty("rc");

    // setDifficulty
    mock.expectCall("setDifficulty(rs,99,false)");
    mock.provideReturnValue(105);
    a.checkEqual("41. setDifficulty", level4.setDifficulty("rs", 99, false), 105);
    mock.expectCall("setDifficulty(rs,-1,true)");
    mock.provideReturnValue(88);
    a.checkEqual("42. setDifficulty", level4.setDifficulty("rs", afl::base::Nothing, true), 88);

    mock.checkFinish();
}

/** Test different areas. */
AFL_TEST("server.interface.HostToolServer:area", a)
{
    HostToolMock mock(a);

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

/*
 *  Test area mismatch with client.
 */

// Mismatch at a Server->Client transition is not detected because we're just chaining C++ calls here.
AFL_TEST("server.interface.HostToolServer:area-mismatch:server-to-client", a)
{
    HostToolMock mock(a);
    server::interface::HostToolServer level1(mock, HostTool::Host);
    server::interface::HostToolClient level2(level1, HostTool::Host);
    server::interface::HostToolServer level3(level2, HostTool::Master);
    server::interface::HostToolClient level4(level3, HostTool::Master);

    mock.expectCall("getDifficulty(x)");
    mock.provideReturnValue<int>(17);
    a.checkEqual("", level4.getDifficulty("x"), 17);
}

// Mismatch at a Client->Server transition is detected due to command name mismatch.
AFL_TEST("server.interface.HostToolServer:area-mismatch:client-to-server", a)
{
    HostToolMock mock(a);
    server::interface::HostToolServer level1(mock, HostTool::Host);
    server::interface::HostToolClient level2(level1, HostTool::Master);
    server::interface::HostToolServer level3(level2, HostTool::Master);
    server::interface::HostToolClient level4(level3, HostTool::Master);

    AFL_CHECK_THROWS(a, level4.getDifficulty("x"), std::exception);
}
