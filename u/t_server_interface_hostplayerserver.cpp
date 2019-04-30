/**
  *  \file u/t_server_interface_hostplayerserver.cpp
  *  \brief Test for server::interface::HostPlayerServer
  */

#include "server/interface/hostplayerserver.hpp"

#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
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

void
TestServerInterfaceHostPlayerServer::testIt()
{
    HostPlayerMock mock("testIt");
    server::interface::HostPlayerServer testee(mock);

    // join
    mock.expectCall("join(5,3,u)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("PLAYERJOIN").pushBackInteger(5).pushBackInteger(3).pushBackString("u")));

    // substitute
    mock.expectCall("substitute(97,12,q)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("PLAYERSUBST").pushBackInteger(97).pushBackInteger(12).pushBackString("q")));

    // resign
    mock.expectCall("resign(7,1,r)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("PLAYERRESIGN").pushBackInteger(7).pushBackInteger(1).pushBackString("r")));

    // add
    mock.expectCall("add(92,zz)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("PLAYERADD").pushBackInteger(92).pushBackString("zz")));

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
        Access a(p);

        // Validate
        // - We're transferring the result as a key/value array, not a native hash.
        //   See comment in t_server_interface_hostplayerclient.cpp for details [20170807].
        TS_ASSERT_EQUALS(a.getArraySize(), 4U);

        // - Anyhow, verifying the result using the hash accessor happens to be pretty convenient
        //   here, because it negates possible ordering issues (there are no guarantees about
        //   the sort order of the result).
        TS_ASSERT_EQUALS(a("8")("long").toString(), "long a");
        TS_ASSERT_EQUALS(a("8")("short").toString(), "short a");
        TS_ASSERT_EQUALS(a("8")("adj").toString(), "adj a");
        TS_ASSERT_EQUALS(a("8")("users").getArraySize(), 1U);
        TS_ASSERT_EQUALS(a("8")("users")[0].toString(), "ua1");
        TS_ASSERT_EQUALS(a("8")("editable").toInteger(), 1);
        TS_ASSERT_EQUALS(a("8")("joinable").toInteger(), 0);

        TS_ASSERT_EQUALS(a("11")("long").toString(), "long b");
        TS_ASSERT_EQUALS(a("11")("short").toString(), "short b");
        TS_ASSERT_EQUALS(a("11")("adj").toString(), "adj b");
        TS_ASSERT_EQUALS(a("11")("users").getArraySize(), 2U);
        TS_ASSERT_EQUALS(a("11")("users")[0].toString(), "ub1");
        TS_ASSERT_EQUALS(a("11")("users")[1].toString(), "ub2");
        TS_ASSERT_EQUALS(a("11")("editable").toInteger(), 0);
        TS_ASSERT_EQUALS(a("11")("joinable").toInteger(), 1);
    }
    {
        // Prepare call
        mock.expectCall("list(23,1)");
        mock.provideReturnValue(0);

        // Call
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("PLAYERLS").pushBackInteger(23).pushBackString("ALL")));
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 0U);
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
        Access a(p);

        TS_ASSERT_EQUALS(a("long").toString(), "info long");
        TS_ASSERT_EQUALS(a("short").toString(), "info short");
        TS_ASSERT_EQUALS(a("adj").toString(), "info adj");
        TS_ASSERT_EQUALS(a("users").getArraySize(), 3U);
        TS_ASSERT_EQUALS(a("users")[0].toString(), "a");
        TS_ASSERT_EQUALS(a("users")[1].toString(), "b");
        TS_ASSERT_EQUALS(a("users")[2].toString(), "c");
        TS_ASSERT_EQUALS(a("editable").toInteger(), 2);
        TS_ASSERT_EQUALS(a("joinable").toInteger(), 0);
    }

    // setDirectory
    mock.expectCall("setDirectory(12,u,dd)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("PLAYERSETDIR").pushBackInteger(12).pushBackString("u").pushBackString("dd")));

    // getDirectory
    mock.expectCall("getDirectory(14,aeiuo)");
    mock.provideReturnValue(String_t("u/d/a"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PLAYERGETDIR").pushBackInteger(14).pushBackString("aeiuo")), "u/d/a");

    // checkFile
    mock.expectCall("checkFile(9,oo,xyplan.dat,-)");
    mock.provideReturnValue(HostPlayer::Refuse);
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PLAYERCHECKFILE").pushBackInteger(9).pushBackString("oo").pushBackString("xyplan.dat")), "refuse");

    mock.expectCall("checkFile(9,oo,xyplan.dat,e/f/g)");
    mock.provideReturnValue(HostPlayer::Stale);
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PLAYERCHECKFILE").pushBackInteger(9).pushBackString("oo").pushBackString("xyplan.dat").pushBackString("DIR").pushBackString("e/f/g")), "stale");

    // get
    mock.expectCall("get(10,uq,fun)");
    mock.provideReturnValue<String_t>("answer");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PLAYERGET").pushBackInteger(10).pushBackString("uq").pushBackString("fun")), "answer");

    // set
    mock.expectCall("set(10,uq,k,v)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("PLAYERSET").pushBackInteger(10).pushBackString("uq").pushBackString("k").pushBackString("v")));

    // Variants
    mock.expectCall("join(5,3,u)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("playerjoin").pushBackInteger(5).pushBackInteger(3).pushBackString("u")));

    mock.expectCall("list(23,1)");
    mock.provideReturnValue(0);
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("PLAYERLS").pushBackInteger(23).pushBackString("all")));

    mock.expectCall("list(23,1)");
    mock.provideReturnValue(0);
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("PLAYERLS").pushBackInteger(23).pushBackString("all").pushBackString("all")));

    mock.expectCall("checkFile(9,oo,x,e)");
    mock.provideReturnValue(HostPlayer::Allow);
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("playercheckfile").pushBackInteger(9).pushBackString("oo").pushBackString("x").pushBackString("dir").pushBackString("e")), "allow");

    mock.checkFinish();
}

void
TestServerInterfaceHostPlayerServer::testErrors()
{
    HostPlayerMock mock("testErrors");
    server::interface::HostPlayerServer testee(mock);

    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("X")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PLAYERJOIN")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PLAYERADD").pushBackInteger(1).pushBackString("a").pushBackString("x")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PLAYERLS").pushBackInteger(23).pushBackString("what")), std::exception);
}

void
TestServerInterfaceHostPlayerServer::testRoundtrip()
{
    HostPlayerMock mock("testRoundtrip");
    server::interface::HostPlayerServer level1(mock);
    server::interface::HostPlayerClient level2(level1);
    server::interface::HostPlayerServer level3(level2);
    server::interface::HostPlayerClient level4(level3);

    // join
    mock.expectCall("join(5,3,u)");
    TS_ASSERT_THROWS_NOTHING(level4.join(5, 3, "u"));

     // substitute
    mock.expectCall("substitute(97,12,q)");
    TS_ASSERT_THROWS_NOTHING(level4.substitute(97, 12, "q"));

    // resign
    mock.expectCall("resign(7,1,r)");
    TS_ASSERT_THROWS_NOTHING(level4.resign(7, 1, "r"));

    // add
    mock.expectCall("add(92,zz)");
    TS_ASSERT_THROWS_NOTHING(level4.add(92, "zz"));

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
        TS_ASSERT_THROWS_NOTHING(level4.list(23, false, result));

        // Validate
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[8].longName, "long a");
        TS_ASSERT_EQUALS(result[8].shortName, "short a");
        TS_ASSERT_EQUALS(result[8].adjectiveName, "adj a");
        TS_ASSERT_EQUALS(result[8].userIds.size(), 1U);
        TS_ASSERT_EQUALS(result[8].userIds[0], "ua1");
        TS_ASSERT_EQUALS(result[8].numEditable, 1);
        TS_ASSERT_EQUALS(result[8].joinable, 0);
        TS_ASSERT_EQUALS(result[11].longName, "long b");
        TS_ASSERT_EQUALS(result[11].shortName, "short b");
        TS_ASSERT_EQUALS(result[11].adjectiveName, "adj b");
        TS_ASSERT_EQUALS(result[11].userIds.size(), 2U);
        TS_ASSERT_EQUALS(result[11].userIds[0], "ub1");
        TS_ASSERT_EQUALS(result[11].userIds[1], "ub2");
        TS_ASSERT_EQUALS(result[11].numEditable, 0);
        TS_ASSERT_EQUALS(result[11].joinable, 1);
    }
    {
        // Prepare call
        mock.expectCall("list(23,1)");
        mock.provideReturnValue(0);

        std::map<int, HostPlayer::Info> result;
        TS_ASSERT_THROWS_NOTHING(level4.list(23, true, result));
        TS_ASSERT(result.empty());
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

        TS_ASSERT_EQUALS(out.longName, "info long");
        TS_ASSERT_EQUALS(out.shortName, "info short");
        TS_ASSERT_EQUALS(out.adjectiveName, "info adj");
        TS_ASSERT_EQUALS(out.userIds.size(), 3U);
        TS_ASSERT_EQUALS(out.userIds[0], "a");
        TS_ASSERT_EQUALS(out.userIds[1], "b");
        TS_ASSERT_EQUALS(out.userIds[2], "c");
        TS_ASSERT_EQUALS(out.numEditable, 2);
        TS_ASSERT_EQUALS(out.joinable, 0);
    }

    // setDirectory
    mock.expectCall("setDirectory(12,u,dd)");
    TS_ASSERT_THROWS_NOTHING(level4.setDirectory(12, "u", "dd"));

    // getDirectory
    mock.expectCall("getDirectory(14,aeiuo)");
    mock.provideReturnValue(String_t("u/d/a"));
    TS_ASSERT_EQUALS(level4.getDirectory(14, "aeiuo"), "u/d/a");

    // checkFile
    mock.expectCall("checkFile(9,oo,xyplan.dat,-)");
    mock.provideReturnValue(HostPlayer::Refuse);
    TS_ASSERT_EQUALS(level4.checkFile(9, "oo", "xyplan.dat", afl::base::Nothing), HostPlayer::Refuse);

    mock.expectCall("checkFile(9,oo,xyplan.dat,e/f/g)");
    mock.provideReturnValue(HostPlayer::Stale);
    TS_ASSERT_EQUALS(level4.checkFile(9, "oo", "xyplan.dat", String_t("e/f/g")), HostPlayer::Stale);

    // set
    mock.expectCall("set(10,u,kk,vv)");
    TS_ASSERT_THROWS_NOTHING(level4.set(10, "u", "kk", "vv"));

    // get
    mock.expectCall("get(11,uu,kkk)");
    mock.provideReturnValue<String_t>("vvv");
    TS_ASSERT_EQUALS(level4.get(11, "uu", "kkk"), "vvv");

    mock.checkFinish();
}

