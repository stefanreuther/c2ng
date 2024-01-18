/**
  *  \file test/server/interface/hostplayerservertest.cpp
  *  \brief Test for server::interface::HostPlayerServer
  */

#include "server/interface/hostplayerserver.hpp"

#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/hostplayerclient.hpp"

using server::interface::HostPlayer;
using afl::string::Format;
using afl::data::Segment;
using afl::data::Access;
using server::Value_t;

namespace {
    class HostPlayerMock : public HostPlayer, public afl::test::CallReceiver {
     public:
        HostPlayerMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual void join(int32_t gameId, int32_t slot, String_t userId)
            { checkCall(Format("join(%d,%d,%s)", gameId, slot, userId)); }
        virtual void substitute(int32_t gameId, int32_t slot, String_t userId)
            { checkCall(Format("substitute(%d,%d,%s)", gameId, slot, userId)); }
        virtual void resign(int32_t gameId, int32_t slot, String_t userId)
            { checkCall(Format("resign(%d,%d,%s)", gameId, slot, userId)); }
        virtual void add(int32_t gameId, String_t userId)
            { checkCall(Format("add(%d,%s)", gameId, userId)); }
        virtual void list(int32_t gameId, bool all, std::map<int,Info>& result)
            {
                checkCall(Format("list(%d,%d)", gameId, int(all)));
                while (int n = consumeReturnValue<int>()) {
                    result[n] = consumeReturnValue<Info>();
                }
            }
        virtual Info getInfo(int32_t gameId, int32_t slot)
            {
                checkCall(Format("getInfo(%d,%d)", gameId, slot));
                return consumeReturnValue<Info>();
            }
        virtual void setDirectory(int32_t gameId, String_t userId, String_t dirName)
            { checkCall(Format("setDirectory(%d,%s,%s)", gameId, userId, dirName)); }
        virtual String_t getDirectory(int32_t gameId, String_t userId)
            {
                checkCall(Format("getDirectory(%d,%s)", gameId, userId));
                return consumeReturnValue<String_t>();
            }
        virtual FileStatus checkFile(int32_t gameId, String_t userId, String_t fileName, afl::base::Optional<String_t> dirName)
            {
                checkCall(Format("checkFile(%d,%s,%s,%s)", gameId, userId, fileName, dirName.orElse("-")));
                return consumeReturnValue<FileStatus>();
            }
        virtual void set(int32_t gameId, String_t userId, String_t key, String_t value)
            { checkCall(Format("set(%d,%s,%s,%s)", gameId, userId, key, value)); }
        virtual String_t get(int32_t gameId, String_t userId, String_t key)
            {
                checkCall(Format("get(%d,%s,%s)", gameId, userId, key));
                return consumeReturnValue<String_t>();
            }
    };
}

AFL_TEST("server.interface.HostPlayerServer:commands", a)
{
    HostPlayerMock mock(a);
    server::interface::HostPlayerServer testee(mock);

    // join
    mock.expectCall("join(5,3,u)");
    AFL_CHECK_SUCCEEDS(a("01. playerjoin"), testee.callVoid(Segment().pushBackString("PLAYERJOIN").pushBackInteger(5).pushBackInteger(3).pushBackString("u")));

    // substitute
    mock.expectCall("substitute(97,12,q)");
    AFL_CHECK_SUCCEEDS(a("11. playersubst"), testee.callVoid(Segment().pushBackString("PLAYERSUBST").pushBackInteger(97).pushBackInteger(12).pushBackString("q")));

    // resign
    mock.expectCall("resign(7,1,r)");
    AFL_CHECK_SUCCEEDS(a("21. playerresign"), testee.callVoid(Segment().pushBackString("PLAYERRESIGN").pushBackInteger(7).pushBackInteger(1).pushBackString("r")));

    // add
    mock.expectCall("add(92,zz)");
    AFL_CHECK_SUCCEEDS(a("31. playeradd"), testee.callVoid(Segment().pushBackString("PLAYERADD").pushBackInteger(92).pushBackString("zz")));

    // list
    {
        // Prepare two infos
        HostPlayer::Info ia;
        ia.longName = "long a";
        ia.shortName = "short a";
        ia.adjectiveName = "adj a";
        ia.userIds.push_back("ua1");
        ia.numEditable = 1;
        ia.joinable = false;

        HostPlayer::Info ib;
        ib.longName = "long b";
        ib.shortName = "short b";
        ib.adjectiveName = "adj b";
        ib.userIds.push_back("ub1");
        ib.userIds.push_back("ub2");
        ib.numEditable = 0;
        ib.joinable = true;

        // Prepare call
        mock.expectCall("list(23,0)");
        mock.provideReturnValue(8);
        mock.provideReturnValue(ia);
        mock.provideReturnValue(11);
        mock.provideReturnValue(ib);
        mock.provideReturnValue(0);

        // Call
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("PLAYERLS").pushBackInteger(23)));
        Access ap(p);

        // Validate
        // - We're transferring the result as a key/value array, not a native hash.
        //   See comment in t_server_interface_hostplayerclient.cpp for details [20170807].
        a.checkEqual("41. getArraySize", ap.getArraySize(), 4U);

        // - Anyhow, verifying the result using the hash accessor happens to be pretty convenient
        //   here, because it negates possible ordering issues (there are no guarantees about
        //   the sort order of the result).
        a.checkEqual("51. long",     ap("8")("long").toString(), "long a");
        a.checkEqual("52. short",    ap("8")("short").toString(), "short a");
        a.checkEqual("53. adj",      ap("8")("adj").toString(), "adj a");
        a.checkEqual("54. users",    ap("8")("users").getArraySize(), 1U);
        a.checkEqual("55. users",    ap("8")("users")[0].toString(), "ua1");
        a.checkEqual("56. editable", ap("8")("editable").toInteger(), 1);
        a.checkEqual("57. joinable", ap("8")("joinable").toInteger(), 0);

        a.checkEqual("61. long",     ap("11")("long").toString(), "long b");
        a.checkEqual("62. short",    ap("11")("short").toString(), "short b");
        a.checkEqual("63. adj",      ap("11")("adj").toString(), "adj b");
        a.checkEqual("64. users",    ap("11")("users").getArraySize(), 2U);
        a.checkEqual("65. users",    ap("11")("users")[0].toString(), "ub1");
        a.checkEqual("66. users",    ap("11")("users")[1].toString(), "ub2");
        a.checkEqual("67. editable", ap("11")("editable").toInteger(), 0);
        a.checkEqual("68. joinable", ap("11")("joinable").toInteger(), 1);
    }
    {
        // Prepare call
        mock.expectCall("list(23,1)");
        mock.provideReturnValue(0);

        // Call
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("PLAYERLS").pushBackInteger(23).pushBackString("ALL")));
        Access ap(p);
        a.checkEqual("71. getArraySize", ap.getArraySize(), 0U);
    }

    // getInfo
    {
        HostPlayer::Info ia;
        ia.longName = "info long";
        ia.shortName = "info short";
        ia.adjectiveName = "info adj";
        ia.userIds.push_back("a");
        ia.userIds.push_back("b");
        ia.userIds.push_back("c");
        ia.numEditable = 2;
        ia.joinable = false;

        mock.expectCall("getInfo(13,2)");
        mock.provideReturnValue(ia);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("PLAYERSTAT").pushBackInteger(13).pushBackInteger(2)));
        Access ap(p);

        a.checkEqual("81. long",     ap("long").toString(), "info long");
        a.checkEqual("82. short",    ap("short").toString(), "info short");
        a.checkEqual("83. adj",      ap("adj").toString(), "info adj");
        a.checkEqual("84. users",    ap("users").getArraySize(), 3U);
        a.checkEqual("85. users",    ap("users")[0].toString(), "a");
        a.checkEqual("86. users",    ap("users")[1].toString(), "b");
        a.checkEqual("87. users",    ap("users")[2].toString(), "c");
        a.checkEqual("88. editable", ap("editable").toInteger(), 2);
        a.checkEqual("89. joinable", ap("joinable").toInteger(), 0);
    }

    // setDirectory
    mock.expectCall("setDirectory(12,u,dd)");
    AFL_CHECK_SUCCEEDS(a("91. playersetdir"), testee.callVoid(Segment().pushBackString("PLAYERSETDIR").pushBackInteger(12).pushBackString("u").pushBackString("dd")));

    // getDirectory
    mock.expectCall("getDirectory(14,aeiuo)");
    mock.provideReturnValue(String_t("u/d/a"));
    a.checkEqual("101. playergetdir", testee.callString(Segment().pushBackString("PLAYERGETDIR").pushBackInteger(14).pushBackString("aeiuo")), "u/d/a");

    // checkFile
    mock.expectCall("checkFile(9,oo,xyplan.dat,-)");
    mock.provideReturnValue(HostPlayer::Refuse);
    a.checkEqual("111. playercheckfile", testee.callString(Segment().pushBackString("PLAYERCHECKFILE").pushBackInteger(9).pushBackString("oo").pushBackString("xyplan.dat")), "refuse");

    mock.expectCall("checkFile(9,oo,xyplan.dat,e/f/g)");
    mock.provideReturnValue(HostPlayer::Stale);
    a.checkEqual("121. playercheckfile", testee.callString(Segment().pushBackString("PLAYERCHECKFILE").pushBackInteger(9).pushBackString("oo").pushBackString("xyplan.dat").pushBackString("DIR").pushBackString("e/f/g")), "stale");

    // get
    mock.expectCall("get(10,uq,fun)");
    mock.provideReturnValue<String_t>("answer");
    a.checkEqual("131. playerget", testee.callString(Segment().pushBackString("PLAYERGET").pushBackInteger(10).pushBackString("uq").pushBackString("fun")), "answer");

    // set
    mock.expectCall("set(10,uq,k,v)");
    AFL_CHECK_SUCCEEDS(a("141. playerset"), testee.callVoid(Segment().pushBackString("PLAYERSET").pushBackInteger(10).pushBackString("uq").pushBackString("k").pushBackString("v")));

    // Variants
    mock.expectCall("join(5,3,u)");
    AFL_CHECK_SUCCEEDS(a("151. playerjoin"), testee.callVoid(Segment().pushBackString("playerjoin").pushBackInteger(5).pushBackInteger(3).pushBackString("u")));

    mock.expectCall("list(23,1)");
    mock.provideReturnValue(0);
    AFL_CHECK_SUCCEEDS(a("161. playerls"), testee.callVoid(Segment().pushBackString("PLAYERLS").pushBackInteger(23).pushBackString("all")));

    mock.expectCall("list(23,1)");
    mock.provideReturnValue(0);
    AFL_CHECK_SUCCEEDS(a("171. playerls"), testee.callVoid(Segment().pushBackString("PLAYERLS").pushBackInteger(23).pushBackString("all").pushBackString("all")));

    mock.expectCall("checkFile(9,oo,x,e)");
    mock.provideReturnValue(HostPlayer::Allow);
    a.checkEqual("181. playercheckfile", testee.callString(Segment().pushBackString("playercheckfile").pushBackInteger(9).pushBackString("oo").pushBackString("x").pushBackString("dir").pushBackString("e")), "allow");

    mock.checkFinish();
}

AFL_TEST("server.interface.HostPlayerServer:errors", a)
{
    HostPlayerMock mock(a);
    server::interface::HostPlayerServer testee(mock);

    Segment empty;
    AFL_CHECK_THROWS(a("01. empty"),       testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. bad verb"),    testee.callVoid(Segment().pushBackString("")), std::exception);
    AFL_CHECK_THROWS(a("03. bad verb"),    testee.callVoid(Segment().pushBackString("X")), std::exception);
    AFL_CHECK_THROWS(a("04. missing arg"), testee.callVoid(Segment().pushBackString("PLAYERJOIN")), std::exception);
    AFL_CHECK_THROWS(a("05. bad type"),    testee.callVoid(Segment().pushBackString("PLAYERADD").pushBackInteger(1).pushBackString("a").pushBackString("x")), std::exception);
    AFL_CHECK_THROWS(a("06. bad option"),  testee.callVoid(Segment().pushBackString("PLAYERLS").pushBackInteger(23).pushBackString("what")), std::exception);
}

AFL_TEST("server.interface.HostPlayerServer:roundtrip", a)
{
    HostPlayerMock mock(a);
    server::interface::HostPlayerServer level1(mock);
    server::interface::HostPlayerClient level2(level1);
    server::interface::HostPlayerServer level3(level2);
    server::interface::HostPlayerClient level4(level3);

    // join
    mock.expectCall("join(5,3,u)");
    AFL_CHECK_SUCCEEDS(a("01. join"), level4.join(5, 3, "u"));

     // substitute
    mock.expectCall("substitute(97,12,q)");
    AFL_CHECK_SUCCEEDS(a("11. substitute"), level4.substitute(97, 12, "q"));

    // resign
    mock.expectCall("resign(7,1,r)");
    AFL_CHECK_SUCCEEDS(a("21. resign"), level4.resign(7, 1, "r"));

    // add
    mock.expectCall("add(92,zz)");
    AFL_CHECK_SUCCEEDS(a("31. add"), level4.add(92, "zz"));

    // list
    {
        // Prepare two infos
        HostPlayer::Info ia;
        ia.longName = "long a";
        ia.shortName = "short a";
        ia.adjectiveName = "adj a";
        ia.userIds.push_back("ua1");
        ia.numEditable = 1;
        ia.joinable = false;

        HostPlayer::Info ib;
        ib.longName = "long b";
        ib.shortName = "short b";
        ib.adjectiveName = "adj b";
        ib.userIds.push_back("ub1");
        ib.userIds.push_back("ub2");
        ib.numEditable = 0;
        ib.joinable = true;

        // Prepare call
        mock.expectCall("list(23,0)");
        mock.provideReturnValue(8);
        mock.provideReturnValue(ia);
        mock.provideReturnValue(11);
        mock.provideReturnValue(ib);
        mock.provideReturnValue(0);

        // Call
        std::map<int, HostPlayer::Info> result;
        AFL_CHECK_SUCCEEDS(a("41. list"), level4.list(23, false, result));

        // Validate
        a.checkEqual("51. size",          result.size(), 2U);
        a.checkEqual("52. longName",      result[8].longName, "long a");
        a.checkEqual("53. shortName",     result[8].shortName, "short a");
        a.checkEqual("54. adjectiveName", result[8].adjectiveName, "adj a");
        a.checkEqual("55. userIds",       result[8].userIds.size(), 1U);
        a.checkEqual("56. userIds",       result[8].userIds[0], "ua1");
        a.checkEqual("57. numEditable",   result[8].numEditable, 1);
        a.checkEqual("58. joinable",      result[8].joinable, 0);
        a.checkEqual("59. longName",      result[11].longName, "long b");
        a.checkEqual("60. shortName",     result[11].shortName, "short b");
        a.checkEqual("61. adjectiveName", result[11].adjectiveName, "adj b");
        a.checkEqual("62. userIds",       result[11].userIds.size(), 2U);
        a.checkEqual("63. userIds",       result[11].userIds[0], "ub1");
        a.checkEqual("64. userIds",       result[11].userIds[1], "ub2");
        a.checkEqual("65. numEditable",   result[11].numEditable, 0);
        a.checkEqual("66. joinable",      result[11].joinable, 1);
    }
    {
        // Prepare call
        mock.expectCall("list(23,1)");
        mock.provideReturnValue(0);

        std::map<int, HostPlayer::Info> result;
        AFL_CHECK_SUCCEEDS(a("71. list"), level4.list(23, true, result));
        a.check("72. empty", result.empty());
    }

    // getInfo
    {
        HostPlayer::Info in;
        in.longName = "info long";
        in.shortName = "info short";
        in.adjectiveName = "info adj";
        in.userIds.push_back("a");
        in.userIds.push_back("b");
        in.userIds.push_back("c");
        in.numEditable = 2;
        in.joinable = false;

        mock.expectCall("getInfo(13,2)");
        mock.provideReturnValue(in);

        HostPlayer::Info out = level4.getInfo(13, 2);

        a.checkEqual("81. longName",      out.longName, "info long");
        a.checkEqual("82. shortName",     out.shortName, "info short");
        a.checkEqual("83. adjectiveName", out.adjectiveName, "info adj");
        a.checkEqual("84. userIds",       out.userIds.size(), 3U);
        a.checkEqual("85. userIds",       out.userIds[0], "a");
        a.checkEqual("86. userIds",       out.userIds[1], "b");
        a.checkEqual("87. userIds",       out.userIds[2], "c");
        a.checkEqual("88. numEditable",   out.numEditable, 2);
        a.checkEqual("89. joinable",      out.joinable, 0);
    }

    // setDirectory
    mock.expectCall("setDirectory(12,u,dd)");
    AFL_CHECK_SUCCEEDS(a("91. setDirectory"), level4.setDirectory(12, "u", "dd"));

    // getDirectory
    mock.expectCall("getDirectory(14,aeiuo)");
    mock.provideReturnValue(String_t("u/d/a"));
    a.checkEqual("101. getDirectory", level4.getDirectory(14, "aeiuo"), "u/d/a");

    // checkFile
    mock.expectCall("checkFile(9,oo,xyplan.dat,-)");
    mock.provideReturnValue(HostPlayer::Refuse);
    a.checkEqual("111. checkFile", level4.checkFile(9, "oo", "xyplan.dat", afl::base::Nothing), HostPlayer::Refuse);

    mock.expectCall("checkFile(9,oo,xyplan.dat,e/f/g)");
    mock.provideReturnValue(HostPlayer::Stale);
    a.checkEqual("121. checkFile", level4.checkFile(9, "oo", "xyplan.dat", String_t("e/f/g")), HostPlayer::Stale);

    // set
    mock.expectCall("set(10,u,kk,vv)");
    AFL_CHECK_SUCCEEDS(a("131. set"), level4.set(10, "u", "kk", "vv"));

    // get
    mock.expectCall("get(11,uu,kkk)");
    mock.provideReturnValue<String_t>("vvv");
    a.checkEqual("141. get", level4.get(11, "uu", "kkk"), "vvv");

    mock.checkFinish();
}
