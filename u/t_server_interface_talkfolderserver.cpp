/**
  *  \file u/t_server_interface_talkfolderserver.cpp
  *  \brief Test for server::interface::TalkFolderServer
  */

#include "server/interface/talkfolderserver.hpp"

#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/talkfolderclient.hpp"
#include "server/types.hpp"

using afl::string::Format;
using afl::data::Segment;
using afl::data::Access;

namespace {
    class TalkFolderMock : public server::interface::TalkFolder, public afl::test::CallReceiver {
     public:
        TalkFolderMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual void getFolders(afl::data::IntegerList_t& result)
            {
                checkCall("getFolders()");
                result.push_back(1);
                result.push_back(2);
                result.push_back(101);
            }
        virtual Info getInfo(int32_t ufid)
            {
                checkCall(Format("getInfo(%d)", ufid));
                return consumeReturnValue<Info>();
            }
        virtual void getInfo(afl::base::Memory<const int32_t> ufids, afl::container::PtrVector<Info>& results)
            {
                String_t cmd = "getInfos(";
                while (const int32_t* p = ufids.eat()) {
                    cmd += Format("%d", *p);
                    if (!ufids.empty()) {
                        cmd += ",";
                    }
                    results.pushBackNew(consumeReturnValue<Info*>());
                }
                cmd += ")";
                checkCall(cmd);
            }
        virtual int32_t create(String_t name, afl::base::Memory<const String_t> args)
            {
                String_t cmd = "create(" + name;
                while (const String_t* p = args.eat()) {
                    cmd += ",";
                    cmd += *p;
                }
                cmd += ")";
                checkCall(cmd);
                return consumeReturnValue<int32_t>();
            }
        virtual bool remove(int32_t ufid)
            {
                checkCall(Format("remove(%d)", ufid));
                return consumeReturnValue<bool>();
            }
        virtual void configure(int32_t ufid, afl::base::Memory<const String_t> args)
            {
                String_t cmd = Format("configure(%d", ufid);
                while (const String_t* p = args.eat()) {
                    cmd += ",";
                    cmd += *p;
                }
                cmd += ")";
                checkCall(cmd);
            }
        virtual afl::data::Value* getPMs(int32_t ufid, const ListParameters& params)
            {
                checkCall(Format("getPMs(%d,%s)", ufid, formatListParameters(params)));
                return consumeReturnValue<afl::data::Value*>();
            }

        // FIXME: copied..
        static String_t formatListParameters(const ListParameters& params)
            {
                String_t result;
                switch (params.mode) {
                 case ListParameters::WantAll:
                    result = "all";
                    break;
                 case ListParameters::WantRange:
                    result = Format("range(%d,%d)", params.start, params.count);
                    break;
                 case ListParameters::WantSize:
                    result = "size";
                    break;
                 case ListParameters::WantMemberCheck:
                    result = Format("member(%d)", params.item);
                    break;
                }
                if (const String_t* p = params.sortKey.get()) {
                    result += Format(",sort(%s)", *p);
                }
                return result;
            }
    };
}

/** Basic test. */
void
TestServerInterfaceTalkFolderServer::testIt()
{
    using server::interface::TalkFolder;

    TalkFolderMock mock("testIt");
    server::interface::TalkFolderServer testee(mock);

    // getFolders
    {
        mock.expectCall("getFolders()");
        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("FOLDERLS")));
        TS_ASSERT(p.get() != 0);
        TS_ASSERT_EQUALS(Access(p).getArraySize(), 3U);
        TS_ASSERT_EQUALS(Access(p)[0].toInteger(), 1);
        TS_ASSERT_EQUALS(Access(p)[1].toInteger(), 2);
        TS_ASSERT_EQUALS(Access(p)[2].toInteger(), 101);
    }

    // getInfo
    {
        TalkFolder::Info in;
        in.name = "The Name";
        in.description = "The Description";
        in.numMessages = 23;
        in.isFixedFolder = true;
        in.hasUnreadMessages = false;
        mock.expectCall("getInfo(23)");
        mock.provideReturnValue(in);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("FOLDERSTAT").pushBackInteger(23)));
        TS_ASSERT(p.get() != 0);
        TS_ASSERT_EQUALS(Access(p)("name").toString(), "The Name");
        TS_ASSERT_EQUALS(Access(p)("description").toString(), "The Description");
        TS_ASSERT_EQUALS(Access(p)("messages").toInteger(), 23);
        TS_ASSERT_EQUALS(Access(p)("fixed").toInteger(), 1);
        TS_ASSERT_EQUALS(Access(p)("unread").toInteger(), 0);
    }

    // getInfos
    {
        TalkFolder::Info in;
        in.name = "The Name";
        in.description = "The Description";
        in.numMessages = 24;
        in.isFixedFolder = true;
        in.hasUnreadMessages = false;
        mock.expectCall("getInfos(23,103)");
        mock.provideReturnValue<TalkFolder::Info*>(0);
        mock.provideReturnValue<TalkFolder::Info*>(new TalkFolder::Info(in));

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("FOLDERMSTAT").pushBackInteger(23).pushBackInteger(103)));
        TS_ASSERT(p.get() != 0);
        TS_ASSERT(Access(p)[0].getValue() == 0);
        TS_ASSERT(Access(p)[1].getValue() != 0);
        TS_ASSERT_EQUALS(Access(p)[1]("name").toString(), "The Name");
        TS_ASSERT_EQUALS(Access(p)[1]("description").toString(), "The Description");
        TS_ASSERT_EQUALS(Access(p)[1]("messages").toInteger(), 24);
        TS_ASSERT_EQUALS(Access(p)[1]("fixed").toInteger(), 1);
        TS_ASSERT_EQUALS(Access(p)[1]("unread").toInteger(), 0);
    }

    // create
    mock.expectCall("create(N1,description,D1)");
    mock.provideReturnValue<int32_t>(107);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FOLDERNEW").pushBackString("N1").pushBackString("description").pushBackString("D1")), 107);

    mock.expectCall("create(N2)");
    mock.provideReturnValue<int32_t>(108);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FOLDERNEW").pushBackString("N2")), 108);

    // remove
    mock.expectCall("remove(107)");
    mock.provideReturnValue<bool>(true);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FOLDERRM").pushBackInteger(107)), 1);

    mock.expectCall("remove(107)");
    mock.provideReturnValue<bool>(false);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FOLDERRM").pushBackInteger(107)), 0);

    // configure
    mock.expectCall("configure(105,description,D1)");
    testee.callVoid(Segment().pushBackString("FOLDERSET").pushBackInteger(105).pushBackString("description").pushBackString("D1"));

    mock.expectCall("configure(105)");
    testee.callVoid(Segment().pushBackString("FOLDERSET").pushBackInteger(105));

    // getPMs
    mock.expectCall("getPMs(104,all)");
    mock.provideReturnValue<afl::data::Value*>(0);
    testee.callVoid(Segment().pushBackString("FOLDERLSPM").pushBackInteger(104));

    mock.expectCall("getPMs(104,member(3))");
    mock.provideReturnValue<afl::data::Value*>(0);
    testee.callVoid(Segment().pushBackString("FOLDERLSPM").pushBackInteger(104).pushBackString("CONTAINS").pushBackInteger(3));

    {
        mock.expectCall("getPMs(104,range(40,10),sort(NAME))");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(987));
        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("FOLDERLSPM").pushBackInteger(104).pushBackString("SORT").pushBackString("name").pushBackString("LIMIT").pushBackInteger(40).pushBackInteger(10)));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 987);
    }

    // Variants
    mock.expectCall("getFolders()");
    testee.callVoid(Segment().pushBackString("folderls"));

    mock.expectCall("getPMs(104,member(3))");
    mock.provideReturnValue<afl::data::Value*>(0);
    testee.callVoid(Segment().pushBackString("folderlspm").pushBackInteger(104).pushBackString("contains").pushBackInteger(3));

    mock.expectCall("remove(107)");
    mock.provideReturnValue<bool>(true);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FOLDERRM").pushBackString("107")), 1);

    mock.checkFinish();
}

/** Test error cases. */
void
TestServerInterfaceTalkFolderServer::testErrors()
{
    TalkFolderMock mock("testErrors");
    server::interface::TalkFolderServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("hu")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("FOLDERRM")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("FOLDERLS").pushBackInteger(3)), std::exception);

    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_EQUALS(testee.handleCommand("huhu", args, p), false);

    mock.checkFinish();
}

/** Test roundtrip behaviour. */
void
TestServerInterfaceTalkFolderServer::testRoundtrip()
{
    using server::interface::TalkFolder;

    TalkFolderMock mock("testRoundtrip");
    server::interface::TalkFolderServer level1(mock);
    server::interface::TalkFolderClient level2(level1);
    server::interface::TalkFolderServer level3(level2);
    server::interface::TalkFolderClient level4(level3);

    // getFolders
    {
        mock.expectCall("getFolders()");
        afl::data::IntegerList_t result;
        level4.getFolders(result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 1);
        TS_ASSERT_EQUALS(result[1], 2);
        TS_ASSERT_EQUALS(result[2], 101);
    }

    // getInfo
    {
        TalkFolder::Info in;
        in.name = "The Name";
        in.description = "The Description";
        in.numMessages = 23;
        in.isFixedFolder = true;
        in.hasUnreadMessages = false;
        mock.expectCall("getInfo(23)");
        mock.provideReturnValue(in);

        TalkFolder::Info out = level4.getInfo(23);
        TS_ASSERT_EQUALS(out.name, "The Name");
        TS_ASSERT_EQUALS(out.description, "The Description");
        TS_ASSERT_EQUALS(out.numMessages, 23);
        TS_ASSERT_EQUALS(out.isFixedFolder, true);
        TS_ASSERT_EQUALS(out.hasUnreadMessages, false);
    }

    // getInfos
    {
        TalkFolder::Info in;
        in.name = "The Name";
        in.description = "The Description";
        in.numMessages = 24;
        in.isFixedFolder = true;
        in.hasUnreadMessages = false;
        mock.expectCall("getInfos(23,103)");
        mock.provideReturnValue<TalkFolder::Info*>(0);
        mock.provideReturnValue<TalkFolder::Info*>(new TalkFolder::Info(in));

        afl::container::PtrVector<TalkFolder::Info> out;
        static const int32_t ufids[] = { 23, 103 };
        level4.getInfo(ufids, out);

        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT(out[0] == 0);
        TS_ASSERT(out[1] != 0);
        TS_ASSERT_EQUALS(out[1]->name, "The Name");
    }

    // create
    {
        mock.expectCall("create(N1,description,D1)");
        mock.provideReturnValue<int32_t>(107);
        const String_t config[] = { "description", "D1" };
        TS_ASSERT_EQUALS(level4.create("N1", config), 107);
    }

    mock.expectCall("create(N2)");
    mock.provideReturnValue<int32_t>(108);
    TS_ASSERT_EQUALS(level4.create("N2", afl::base::Nothing), 108);

    // remove
    mock.expectCall("remove(107)");
    mock.provideReturnValue<bool>(true);
    TS_ASSERT(level4.remove(107));

    mock.expectCall("remove(107)");
    mock.provideReturnValue<bool>(false);
    TS_ASSERT(!level4.remove(107));

    // configure
    {
        mock.expectCall("configure(105,description,D1)");
        const String_t config[] = { "description", "D1" };
        level4.configure(105, config);
    }

    mock.expectCall("configure(105)");
    level4.configure(105, afl::base::Nothing);

    // getPMs
    {
        mock.expectCall("getPMs(104,all)");
        mock.provideReturnValue<afl::data::Value*>(0);
        std::auto_ptr<afl::data::Value> p(level4.getPMs(104, TalkFolder::ListParameters()));
        TS_ASSERT(p.get() == 0);
    }

    {
        mock.expectCall("getPMs(104,member(3))");
        mock.provideReturnValue<afl::data::Value*>(0);
        TalkFolder::ListParameters ps;
        ps.mode = ps.WantMemberCheck;
        ps.item = 3;
        std::auto_ptr<afl::data::Value> p(level4.getPMs(104, ps));
        TS_ASSERT(p.get() == 0);
    }

    {
        mock.expectCall("getPMs(104,range(40,10),sort(NAME))");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(987));
        TalkFolder::ListParameters ps;
        ps.mode = ps.WantRange;
        ps.start = 40;
        ps.count = 10;
        ps.sortKey = "NAME";
        std::auto_ptr<afl::data::Value> p(level4.getPMs(104, ps));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 987);
    }

    mock.checkFinish();
}
