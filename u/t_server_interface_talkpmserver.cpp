/**
  *  \file u/t_server_interface_talkpmserver.cpp
  *  \brief Test for server::interface::TalkPMServer
  */

#include "server/interface/talkpmserver.hpp"

#include <memory>
#include <stdexcept>
#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/talkpmclient.hpp"

using afl::string::Format;
using afl::data::Segment;
using afl::data::Value;
using afl::data::Access;

namespace {
    class TalkPMMock : public server::interface::TalkPM, public afl::test::CallReceiver {
     public:
        TalkPMMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual int32_t create(String_t receivers, String_t subject, String_t text, afl::base::Optional<int32_t> parent)
            {
                checkCall(Format("create(%s,%s,%s,%d)", receivers, subject, text, parent.orElse(-1)));
                return consumeReturnValue<int32_t>();
            }
        virtual Info getInfo(int32_t folder, int32_t pmid)
            {
                checkCall(Format("getInfo(%d,%d)", folder, pmid));
                return consumeReturnValue<Info>();
            }
        virtual void getInfo(int32_t folder, afl::base::Memory<const int32_t> pmids, afl::container::PtrVector<Info>& results)
            {
                String_t cmd = Format("getInfos(%d", folder);
                while (const int32_t* p = pmids.eat()) {
                    cmd += Format(",%d", *p);
                    results.pushBackNew(consumeReturnValue<Info*>());
                }
                cmd += ")";
                checkCall(cmd);
            }
        virtual int32_t copy(int32_t sourceFolder, int32_t destFolder, afl::base::Memory<const int32_t> pmids)
            {
                String_t cmd = Format("copy(%d,%d", sourceFolder, destFolder);
                while (const int32_t* p = pmids.eat()) {
                    cmd += Format(",%d", *p);
                }
                cmd += ")";
                checkCall(cmd);
                return consumeReturnValue<int32_t>();
            }
        virtual int32_t move(int32_t sourceFolder, int32_t destFolder, afl::base::Memory<const int32_t> pmids)
            {
                String_t cmd = Format("move(%d,%d", sourceFolder, destFolder);
                while (const int32_t* p = pmids.eat()) {
                    cmd += Format(",%d", *p);
                }
                cmd += ")";
                checkCall(cmd);
                return consumeReturnValue<int32_t>();
            }
        virtual int32_t remove(int32_t folder, afl::base::Memory<const int32_t> pmids)
            {
                String_t cmd = Format("remove(%d", folder);
                while (const int32_t* p = pmids.eat()) {
                    cmd += Format(",%d", *p);
                }
                cmd += ")";
                checkCall(cmd);
                return consumeReturnValue<int32_t>();
            }
        virtual String_t render(int32_t folder, int32_t pmid, const Options& options)
            {
                checkCall(Format("render(%d,%d,%s,%s)", folder, pmid, options.baseUrl.orElse("no-url"), options.format.orElse("no-format")));
                return consumeReturnValue<String_t>();
            }
        virtual void render(int32_t folder, afl::base::Memory<const int32_t> pmids, afl::container::PtrVector<String_t>& result)
            {
                String_t cmd = Format("render(%d", folder);
                while (const int32_t* p = pmids.eat()) {
                    cmd += Format(",%d", *p);
                    result.pushBackNew(consumeReturnValue<String_t*>());
                }
                cmd += ")";
                checkCall(cmd);
            }
        virtual int32_t changeFlags(int32_t folder, int32_t flagsToClear, int32_t flagsToSet, afl::base::Memory<const int32_t> pmids)
            {
                String_t cmd = Format("changeFlags(%d,%d,%d", folder, flagsToClear, flagsToSet);
                while (const int32_t* p = pmids.eat()) {
                    cmd += Format(",%d", *p);
                }
                cmd += ")";
                checkCall(cmd);
                return consumeReturnValue<int32_t>();
            }
    };
}

/** Simple test. */
void
TestServerInterfaceTalkPMServer::testIt()
{
    using server::interface::TalkPM;

    TalkPMMock mock("testIt");
    server::interface::TalkPMServer testee(mock);

    // create
    mock.expectCall("create(to,subj,text,-1)");
    mock.provideReturnValue<int32_t>(99);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PMNEW").pushBackString("to").pushBackString("subj").pushBackString("text")), 99);

    mock.expectCall("create(to2,Re: subj,text2,99)");
    mock.provideReturnValue<int32_t>(105);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PMNEW").pushBackString("to2").pushBackString("Re: subj").pushBackString("text2").pushBackString("PARENT").pushBackInteger(99)), 105);

    // getInfo
    {
        TalkPM::Info in;
        in.author = "aa";
        in.receivers = "rr";
        in.time = 778899;
        in.subject = "ss";
        in.flags = 5;
        in.parent = 111;
        mock.expectCall("getInfo(106,97)");
        mock.provideReturnValue(in);

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("PMSTAT").pushBackInteger(106).pushBackInteger(97)));
        TS_ASSERT(p.get() != 0);

        Access a(p);
        TS_ASSERT_EQUALS(a("author").toString(), "aa");
        TS_ASSERT_EQUALS(a("to").toString(), "rr");
        TS_ASSERT_EQUALS(a("time").toInteger(), 778899);
        TS_ASSERT_EQUALS(a("subject").toString(), "ss");
        TS_ASSERT_EQUALS(a("flags").toInteger(), 5);
        TS_ASSERT_EQUALS(a("parent").toInteger(), 111);
    }

    // getInfos
    {
        TalkPM::Info in;
        in.author = "AA";
        in.receivers = "RR";
        in.time = 667788;
        in.subject = "SS";
        in.flags = 6;
        mock.expectCall("getInfos(106,96,97)");
        mock.provideReturnValue<TalkPM::Info*>(0);
        mock.provideReturnValue<TalkPM::Info*>(new TalkPM::Info(in));

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("PMMSTAT").pushBackInteger(106).pushBackInteger(96).pushBackInteger(97)));
        TS_ASSERT(p.get() != 0);

        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT(a[0].getValue() == 0);
        TS_ASSERT(a[1].getValue() != 0);
        TS_ASSERT_EQUALS(a[1]("author").toString(), "AA");
        TS_ASSERT_EQUALS(a[1]("to").toString(), "RR");
        TS_ASSERT_EQUALS(a[1]("time").toInteger(), 667788);
        TS_ASSERT_EQUALS(a[1]("subject").toString(), "SS");
        TS_ASSERT_EQUALS(a[1]("flags").toInteger(), 6);
        TS_ASSERT_EQUALS(a[1]("parent").toInteger(), 0);       // transmitted as 0 if not present
    }

    // copy
    mock.expectCall("copy(105,107)");
    mock.provideReturnValue<int32_t>(0);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PMCP").pushBackInteger(105).pushBackInteger(107)), 0);

    mock.expectCall("copy(105,107,9,8,10)");
    mock.provideReturnValue<int32_t>(2);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PMCP").pushBackInteger(105).pushBackInteger(107).pushBackInteger(9).pushBackInteger(8).pushBackInteger(10)), 2);

    // move
    mock.expectCall("move(105,117)");
    mock.provideReturnValue<int32_t>(0);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PMMV").pushBackInteger(105).pushBackInteger(117)), 0);

    mock.expectCall("move(105,117,9,8,10)");
    mock.provideReturnValue<int32_t>(2);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PMMV").pushBackInteger(105).pushBackInteger(117).pushBackInteger(9).pushBackInteger(8).pushBackInteger(10)), 2);

    // remove
    mock.expectCall("remove(105)");
    mock.provideReturnValue<int32_t>(0);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PMRM").pushBackInteger(105)), 0);

    mock.expectCall("remove(106,3,1,4,1,5)");
    mock.provideReturnValue<int32_t>(4);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PMRM").pushBackInteger(106).pushBackInteger(3).pushBackInteger(1).pushBackInteger(4).pushBackInteger(1).pushBackInteger(5)), 4);

    // render
    mock.expectCall("render(1,95,no-url,no-format)");
    mock.provideReturnValue<String_t>("text");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PMRENDER").pushBackInteger(1).pushBackInteger(95)), "text");

    mock.expectCall("render(1,95,/u,mail)");
    mock.provideReturnValue<String_t>("> text");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PMRENDER").pushBackInteger(1).pushBackInteger(95).pushBackString("FORMAT").pushBackString("mail").pushBackString("BASEURL").pushBackString("/u")), "> text");

    // render
    mock.expectCall("render(1)");
    testee.callVoid(Segment().pushBackString("PMMRENDER").pushBackInteger(1));

    {
        mock.expectCall("render(1,95,96)");
        mock.provideReturnValue<String_t*>(0);
        mock.provideReturnValue<String_t*>(new String_t("txt"));

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("PMMRENDER").pushBackInteger(1).pushBackInteger(95).pushBackInteger(96)));
        Access a(p);

        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT(a[0].getValue() == 0);
        TS_ASSERT(a[1].getValue() != 0);
        TS_ASSERT_EQUALS(a[1].toString(), "txt");
    }

    // changeFlags
    mock.expectCall("changeFlags(105,2,5)");
    mock.provideReturnValue<int32_t>(0);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PMFLAG").pushBackInteger(105).pushBackInteger(2).pushBackInteger(5)), 0);

    mock.expectCall("changeFlags(105,2,5,33,34)");
    mock.provideReturnValue<int32_t>(2);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PMFLAG").pushBackInteger(105).pushBackInteger(2).pushBackInteger(5).pushBackInteger(33).pushBackInteger(34)), 2);

    // Variations
    mock.expectCall("changeFlags(105,2,5)");
    mock.provideReturnValue<int32_t>(0);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("pmflag").pushBackInteger(105).pushBackInteger(2).pushBackInteger(5)), 0);

    mock.expectCall("render(1,95,/u,mail)");
    mock.provideReturnValue<String_t>("> text");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("pmrender").pushBackInteger(1).pushBackInteger(95).pushBackString("format").pushBackString("mail").pushBackString("baseurl").pushBackString("/u")), "> text");

    mock.checkFinish();
}

void
TestServerInterfaceTalkPMServer::testErrors()
{
    using server::interface::TalkPM;

    TalkPMMock mock("testErrors");
    server::interface::TalkPMServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("huh")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PMFLAG")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PMRENDER")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PMNEW").pushBackString("a")), std::exception);

    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PMNEW").pushBackString("to2").pushBackString("Re: subj").pushBackString("text2").pushBackString("PARENT")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PMNEW").pushBackString("to2").pushBackString("Re: subj").pushBackString("text2").pushBackInteger(99)), std::exception);

    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_EQUALS(testee.handleCommand("huhu", args, p), false);

    mock.checkFinish();
}

void
TestServerInterfaceTalkPMServer::testRoundtrip()
{
    using server::interface::TalkPM;

    TalkPMMock mock("testRoundtrip");
    server::interface::TalkPMServer level1(mock);
    server::interface::TalkPMClient level2(level1);
    server::interface::TalkPMServer level3(level2);
    server::interface::TalkPMClient level4(level3);

    // create
    mock.expectCall("create(to,subj,text,-1)");
    mock.provideReturnValue<int32_t>(99);
    TS_ASSERT_EQUALS(level4.create("to", "subj", "text", afl::base::Nothing), 99);

    mock.expectCall("create(to2,Re: subj,text2,99)");
    mock.provideReturnValue<int32_t>(105);
    TS_ASSERT_EQUALS(level4.create("to2", "Re: subj", "text2", 99), 105);

    // getInfo
    {
        TalkPM::Info in;
        in.author = "aa";
        in.receivers = "rr";
        in.time = 778899;
        in.subject = "ss";
        in.flags = 5;
        in.parent = 111;
        mock.expectCall("getInfo(106,97)");
        mock.provideReturnValue(in);

        TalkPM::Info out = level4.getInfo(106, 97);
        TS_ASSERT_EQUALS(out.author, "aa");
        TS_ASSERT_EQUALS(out.receivers, "rr");
        TS_ASSERT_EQUALS(out.time, 778899);
        TS_ASSERT_EQUALS(out.subject, "ss");
        TS_ASSERT_EQUALS(out.flags, 5);
        TS_ASSERT_EQUALS(out.parent.orElse(-1), 111);
    }

    // getInfos
    {
        TalkPM::Info in;
        in.author = "AA";
        in.receivers = "RR";
        in.time = 667788;
        in.subject = "SS";
        in.flags = 6;
        mock.expectCall("getInfos(106,96,97)");
        mock.provideReturnValue<TalkPM::Info*>(0);
        mock.provideReturnValue<TalkPM::Info*>(new TalkPM::Info(in));

        static const int32_t pmids[] = { 96, 97 };
        afl::container::PtrVector<TalkPM::Info> out;
        level4.getInfo(106, pmids, out);

        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT(out[0] == 0);
        TS_ASSERT(out[1] != 0);
        TS_ASSERT_EQUALS(out[1]->author, "AA");
        TS_ASSERT_EQUALS(out[1]->receivers, "RR");
        TS_ASSERT_EQUALS(out[1]->time, 667788);
        TS_ASSERT_EQUALS(out[1]->subject, "SS");
        TS_ASSERT_EQUALS(out[1]->flags, 6);
        TS_ASSERT(!out[1]->parent.isValid());
    }

    // copy
    mock.expectCall("copy(105,107)");
    mock.provideReturnValue<int32_t>(0);
    TS_ASSERT_EQUALS(level4.copy(105, 107, afl::base::Nothing), 0);

    {
        static const int32_t pmids[] = { 9, 8, 10 };
        mock.expectCall("copy(105,107,9,8,10)");
        mock.provideReturnValue<int32_t>(2);
        TS_ASSERT_EQUALS(level4.copy(105, 107, pmids), 2);
    }

    // move
    mock.expectCall("move(105,117)");
    mock.provideReturnValue<int32_t>(0);
    TS_ASSERT_EQUALS(level4.move(105, 117, afl::base::Nothing), 0);

    {
        static const int32_t pmids[] = { 9, 8, 10 };
        mock.expectCall("move(105,117,9,8,10)");
        mock.provideReturnValue<int32_t>(2);
        TS_ASSERT_EQUALS(level4.move(105, 117, pmids), 2);
    }

    // remove
    mock.expectCall("remove(105)");
    mock.provideReturnValue<int32_t>(0);
    TS_ASSERT_EQUALS(level4.remove(105, afl::base::Nothing), 0);

    {
        static const int32_t pmids[] = { 3, 1, 4, 1, 5 };
        mock.expectCall("remove(106,3,1,4,1,5)");
        mock.provideReturnValue<int32_t>(4);
        TS_ASSERT_EQUALS(level4.remove(106, pmids), 4);
    }

    // render
    mock.expectCall("render(1,95,no-url,no-format)");
    mock.provideReturnValue<String_t>("text");
    TS_ASSERT_EQUALS(level4.render(1, 95, TalkPM::Options()), "text");

    {
        TalkPM::Options opts;
        opts.baseUrl = "/u";
        opts.format = "mail";
        mock.expectCall("render(1,95,/u,mail)");
        mock.provideReturnValue<String_t>("> text");
        TS_ASSERT_EQUALS(level4.render(1, 95, opts), "> text");
    }

    // render
    {
        mock.expectCall("render(1)");
        afl::container::PtrVector<String_t> result;
        level4.render(1, afl::base::Nothing, result);
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    {
        mock.expectCall("render(1,95,96)");
        mock.provideReturnValue<String_t*>(0);
        mock.provideReturnValue<String_t*>(new String_t("txt"));

        static const int32_t pmids[] = { 95, 96 };
        afl::container::PtrVector<String_t> result;
        level4.render(1, pmids, result);

        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT(result[0] == 0);
        TS_ASSERT(result[1] != 0);
        TS_ASSERT_EQUALS(*result[1], "txt");
    }

    // changeFlags
    mock.expectCall("changeFlags(105,2,5)");
    mock.provideReturnValue<int32_t>(0);
    TS_ASSERT_EQUALS(level4.changeFlags(105, 2, 5, afl::base::Nothing), 0);

    {
        mock.expectCall("changeFlags(105,2,5,33,34)");
        mock.provideReturnValue<int32_t>(2);
        static const int32_t pmids[] = { 33, 34 };
        TS_ASSERT_EQUALS(level4.changeFlags(105, 2, 5, pmids), 2);
    }

    mock.checkFinish();
}

