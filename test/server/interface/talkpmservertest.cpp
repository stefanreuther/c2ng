/**
  *  \file test/server/interface/talkpmservertest.cpp
  *  \brief Test for server::interface::TalkPMServer
  */

#include "server/interface/talkpmserver.hpp"

#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/talkpmclient.hpp"
#include <memory>
#include <stdexcept>

using afl::data::Access;
using afl::data::Segment;
using afl::data::Value;
using afl::string::Format;
using server::interface::TalkPM;

namespace {
    class TalkPMMock : public TalkPM, public afl::test::CallReceiver {
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
AFL_TEST("server.interface.TalkPMServer:commands", a)
{
    TalkPMMock mock(a);
    server::interface::TalkPMServer testee(mock);

    // create
    mock.expectCall("create(to,subj,text,-1)");
    mock.provideReturnValue<int32_t>(99);
    a.checkEqual("01. pmnew", testee.callInt(Segment().pushBackString("PMNEW").pushBackString("to").pushBackString("subj").pushBackString("text")), 99);

    mock.expectCall("create(to2,Re: subj,text2,99)");
    mock.provideReturnValue<int32_t>(105);
    a.checkEqual("11. pmnew", testee.callInt(Segment().pushBackString("PMNEW").pushBackString("to2").pushBackString("Re: subj").pushBackString("text2").pushBackString("PARENT").pushBackInteger(99)), 105);

    // getInfo
    {
        TalkPM::Info in;
        in.author = "aa";
        in.receivers = "rr";
        in.time = 778899;
        in.subject = "ss";
        in.flags = 5;
        in.parent = 111;
        in.parentFolder = 3;
        in.parentFolderName = "par";
        in.parentSubject = "pp";
        in.suggestedFolder = 7;
        in.suggestedFolderName = "sug";
        mock.expectCall("getInfo(106,97)");
        mock.provideReturnValue(in);

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("PMSTAT").pushBackInteger(106).pushBackInteger(97)));
        a.checkNonNull("21. pmstat", p.get());

        Access ap(p);
        a.checkEqual("31. author",              ap("author").toString(), "aa");
        a.checkEqual("32. to",                  ap("to").toString(), "rr");
        a.checkEqual("33. time",                ap("time").toInteger(), 778899);
        a.checkEqual("34. subject",             ap("subject").toString(), "ss");
        a.checkEqual("35. flags",               ap("flags").toInteger(), 5);
        a.checkEqual("36. parent",              ap("parent").toInteger(), 111);
        a.checkEqual("37. parentfolder",        ap("parentFolder").toInteger(), 3);
        a.checkEqual("38. parentfoldername",    ap("parentFolderName").toString(), "par");
        a.checkEqual("39. parentsubject",       ap("parentSubject").toString(), "pp");
        a.checkEqual("40. suggestedfolder",     ap("suggestedFolder").toInteger(), 7);
        a.checkEqual("41. suggestedfoldername", ap("suggestedFolderName").toString(), "sug");
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
        a.checkNonNull("51", p.get());

        Access ap(p);
        a.checkEqual("61. getArraySize",        ap.getArraySize(), 2U);
        a.checkNull("62. result",               ap[0].getValue());
        a.checkNonNull("63. result",            ap[1].getValue());
        a.checkEqual("64. author",              ap[1]("author").toString(), "AA");
        a.checkEqual("65. to",                  ap[1]("to").toString(), "RR");
        a.checkEqual("66. time",                ap[1]("time").toInteger(), 667788);
        a.checkEqual("67. subject",             ap[1]("subject").toString(), "SS");
        a.checkEqual("68. flags",               ap[1]("flags").toInteger(), 6);
        a.checkEqual("69. parent",              ap[1]("parent").toInteger(), 0);       // transmitted as 0 if not present
        a.checkNull ("70. parentfolder",        ap[1]("parentFolder").getValue());
        a.checkNull ("71. parentfoldername",    ap[1]("parentFolderName").getValue());
        a.checkNull ("72. parentsubject",       ap[1]("parentSubject").getValue());
        a.checkNull ("73. suggestedfolder",     ap[1]("suggestedFolder").getValue());
        a.checkNull ("74. suggestedfoldername", ap[1]("suggestedFolderName").getValue());
    }

    // copy
    mock.expectCall("copy(105,107)");
    mock.provideReturnValue<int32_t>(0);
    a.checkEqual("81. pmcp", testee.callInt(Segment().pushBackString("PMCP").pushBackInteger(105).pushBackInteger(107)), 0);

    mock.expectCall("copy(105,107,9,8,10)");
    mock.provideReturnValue<int32_t>(2);
    a.checkEqual("91. pmcp", testee.callInt(Segment().pushBackString("PMCP").pushBackInteger(105).pushBackInteger(107).pushBackInteger(9).pushBackInteger(8).pushBackInteger(10)), 2);

    // move
    mock.expectCall("move(105,117)");
    mock.provideReturnValue<int32_t>(0);
    a.checkEqual("101. pmmv", testee.callInt(Segment().pushBackString("PMMV").pushBackInteger(105).pushBackInteger(117)), 0);

    mock.expectCall("move(105,117,9,8,10)");
    mock.provideReturnValue<int32_t>(2);
    a.checkEqual("111. pmmv", testee.callInt(Segment().pushBackString("PMMV").pushBackInteger(105).pushBackInteger(117).pushBackInteger(9).pushBackInteger(8).pushBackInteger(10)), 2);

    // remove
    mock.expectCall("remove(105)");
    mock.provideReturnValue<int32_t>(0);
    a.checkEqual("121. pmrm", testee.callInt(Segment().pushBackString("PMRM").pushBackInteger(105)), 0);

    mock.expectCall("remove(106,3,1,4,1,5)");
    mock.provideReturnValue<int32_t>(4);
    a.checkEqual("131. pmrm", testee.callInt(Segment().pushBackString("PMRM").pushBackInteger(106).pushBackInteger(3).pushBackInteger(1).pushBackInteger(4).pushBackInteger(1).pushBackInteger(5)), 4);

    // render
    mock.expectCall("render(1,95,no-url,no-format)");
    mock.provideReturnValue<String_t>("text");
    a.checkEqual("141. pmrender", testee.callString(Segment().pushBackString("PMRENDER").pushBackInteger(1).pushBackInteger(95)), "text");

    mock.expectCall("render(1,95,/u,mail)");
    mock.provideReturnValue<String_t>("> text");
    a.checkEqual("151. pmrender", testee.callString(Segment().pushBackString("PMRENDER").pushBackInteger(1).pushBackInteger(95).pushBackString("FORMAT").pushBackString("mail").pushBackString("BASEURL").pushBackString("/u")), "> text");

    // render
    mock.expectCall("render(1)");
    testee.callVoid(Segment().pushBackString("PMMRENDER").pushBackInteger(1));

    {
        mock.expectCall("render(1,95,96)");
        mock.provideReturnValue<String_t*>(0);
        mock.provideReturnValue<String_t*>(new String_t("txt"));

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("PMMRENDER").pushBackInteger(1).pushBackInteger(95).pushBackInteger(96)));
        Access ap(p);

        a.checkEqual("161. getArraySize", ap.getArraySize(), 2U);
        a.checkNull("162. result", ap[0].getValue());
        a.checkNonNull("163. result", ap[1].getValue());
        a.checkEqual("164. result", ap[1].toString(), "txt");
    }

    // changeFlags
    mock.expectCall("changeFlags(105,2,5)");
    mock.provideReturnValue<int32_t>(0);
    a.checkEqual("171. pmflag", testee.callInt(Segment().pushBackString("PMFLAG").pushBackInteger(105).pushBackInteger(2).pushBackInteger(5)), 0);

    mock.expectCall("changeFlags(105,2,5,33,34)");
    mock.provideReturnValue<int32_t>(2);
    a.checkEqual("181. pmflag", testee.callInt(Segment().pushBackString("PMFLAG").pushBackInteger(105).pushBackInteger(2).pushBackInteger(5).pushBackInteger(33).pushBackInteger(34)), 2);

    // Variations
    mock.expectCall("changeFlags(105,2,5)");
    mock.provideReturnValue<int32_t>(0);
    a.checkEqual("191. pmflag", testee.callInt(Segment().pushBackString("pmflag").pushBackInteger(105).pushBackInteger(2).pushBackInteger(5)), 0);

    mock.expectCall("render(1,95,/u,mail)");
    mock.provideReturnValue<String_t>("> text");
    a.checkEqual("201. pmrender", testee.callString(Segment().pushBackString("pmrender").pushBackInteger(1).pushBackInteger(95).pushBackString("format").pushBackString("mail").pushBackString("baseurl").pushBackString("/u")), "> text");

    mock.checkFinish();
}

AFL_TEST("server.interface.TalkPMServer:errors", a)
{
    TalkPMMock mock(a);
    server::interface::TalkPMServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    AFL_CHECK_THROWS(a("01. no verb"),     testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. bad verb"),    testee.callVoid(Segment().pushBackString("huh")), std::exception);
    AFL_CHECK_THROWS(a("03. missing arg"), testee.callVoid(Segment().pushBackString("PMFLAG")), std::exception);
    AFL_CHECK_THROWS(a("04. missing arg"), testee.callVoid(Segment().pushBackString("PMRENDER")), std::exception);
    AFL_CHECK_THROWS(a("05. missing arg"), testee.callVoid(Segment().pushBackString("PMNEW").pushBackString("a")), std::exception);

    AFL_CHECK_THROWS(a("11. missing option"), testee.callVoid(Segment().pushBackString("PMNEW").pushBackString("to2").pushBackString("Re: subj").pushBackString("text2").pushBackString("PARENT")), std::exception);
    AFL_CHECK_THROWS(a("12. bad option"),     testee.callVoid(Segment().pushBackString("PMNEW").pushBackString("to2").pushBackString("Re: subj").pushBackString("text2").pushBackInteger(99)), std::exception);

    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    a.checkEqual("21. bad verb", testee.handleCommand("huhu", args, p), false);

    mock.checkFinish();
}

AFL_TEST("server.interface.TalkPMServer:roundtrip", a)
{
    TalkPMMock mock(a);
    server::interface::TalkPMServer level1(mock);
    server::interface::TalkPMClient level2(level1);
    server::interface::TalkPMServer level3(level2);
    server::interface::TalkPMClient level4(level3);

    // create
    mock.expectCall("create(to,subj,text,-1)");
    mock.provideReturnValue<int32_t>(99);
    a.checkEqual("01. create", level4.create("to", "subj", "text", afl::base::Nothing), 99);

    mock.expectCall("create(to2,Re: subj,text2,99)");
    mock.provideReturnValue<int32_t>(105);
    a.checkEqual("11. create", level4.create("to2", "Re: subj", "text2", 99), 105);

    // getInfo
    {
        TalkPM::Info in;
        in.author = "aa";
        in.receivers = "rr";
        in.time = 778899;
        in.subject = "ss";
        in.flags = 5;
        in.parent = 111;
        in.parentFolder = 3;
        in.parentFolderName = "par";
        in.parentSubject = "pp";
        in.suggestedFolder = 7;
        in.suggestedFolderName = "sug";
        mock.expectCall("getInfo(106,97)");
        mock.provideReturnValue(in);

        TalkPM::Info out = level4.getInfo(106, 97);
        a.checkEqual("21. author",              out.author, "aa");
        a.checkEqual("22. receivers",           out.receivers, "rr");
        a.checkEqual("23. time",                out.time, 778899);
        a.checkEqual("24. subject",             out.subject, "ss");
        a.checkEqual("25. flags",               out.flags, 5);
        a.checkEqual("26. parent",              out.parent.orElse(-1), 111);
        a.checkEqual("27. parentFolder",        out.parentFolder.orElse(-1), 3);
        a.checkEqual("28. parentFolderName",    out.parentFolderName.orElse(""), "par");
        a.checkEqual("29. parentSubject",       out.parentSubject.orElse(""), "pp");
        a.checkEqual("30. suggestedFolder",     out.suggestedFolder.orElse(-1), 7);
        a.checkEqual("31. suggestedFolderName", out.suggestedFolderName.orElse(""), "sug");
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

        a.checkEqual("41. size",      out.size(), 2U);
        a.checkNull("42. result",     out[0]);
        a.checkNonNull("43. result",  out[1]);
        a.checkEqual("44. author",    out[1]->author, "AA");
        a.checkEqual("45. receivers", out[1]->receivers, "RR");
        a.checkEqual("46. time",      out[1]->time, 667788);
        a.checkEqual("47. subject",   out[1]->subject, "SS");
        a.checkEqual("48. flags",     out[1]->flags, 6);
        a.check("49. parent",        !out[1]->parent.isValid());
    }

    // copy
    mock.expectCall("copy(105,107)");
    mock.provideReturnValue<int32_t>(0);
    a.checkEqual("51. copy", level4.copy(105, 107, afl::base::Nothing), 0);

    {
        static const int32_t pmids[] = { 9, 8, 10 };
        mock.expectCall("copy(105,107,9,8,10)");
        mock.provideReturnValue<int32_t>(2);
        a.checkEqual("61. copy", level4.copy(105, 107, pmids), 2);
    }

    // move
    mock.expectCall("move(105,117)");
    mock.provideReturnValue<int32_t>(0);
    a.checkEqual("71. move", level4.move(105, 117, afl::base::Nothing), 0);

    {
        static const int32_t pmids[] = { 9, 8, 10 };
        mock.expectCall("move(105,117,9,8,10)");
        mock.provideReturnValue<int32_t>(2);
        a.checkEqual("81. move", level4.move(105, 117, pmids), 2);
    }

    // remove
    mock.expectCall("remove(105)");
    mock.provideReturnValue<int32_t>(0);
    a.checkEqual("91. remove", level4.remove(105, afl::base::Nothing), 0);

    {
        static const int32_t pmids[] = { 3, 1, 4, 1, 5 };
        mock.expectCall("remove(106,3,1,4,1,5)");
        mock.provideReturnValue<int32_t>(4);
        a.checkEqual("101. remove", level4.remove(106, pmids), 4);
    }

    // render
    mock.expectCall("render(1,95,no-url,no-format)");
    mock.provideReturnValue<String_t>("text");
    a.checkEqual("111. render", level4.render(1, 95, TalkPM::Options()), "text");

    {
        TalkPM::Options opts;
        opts.baseUrl = "/u";
        opts.format = "mail";
        mock.expectCall("render(1,95,/u,mail)");
        mock.provideReturnValue<String_t>("> text");
        a.checkEqual("121. render", level4.render(1, 95, opts), "> text");
    }

    // render
    {
        mock.expectCall("render(1)");
        afl::container::PtrVector<String_t> result;
        level4.render(1, afl::base::Nothing, result);
        a.checkEqual("131. render", result.size(), 0U);
    }

    {
        mock.expectCall("render(1,95,96)");
        mock.provideReturnValue<String_t*>(0);
        mock.provideReturnValue<String_t*>(new String_t("txt"));

        static const int32_t pmids[] = { 95, 96 };
        afl::container::PtrVector<String_t> result;
        level4.render(1, pmids, result);

        a.checkEqual("141. size", result.size(), 2U);
        a.checkNull("142. result", result[0]);
        a.checkNonNull("143. result", result[1]);
        a.checkEqual("144. result", *result[1], "txt");
    }

    // changeFlags
    mock.expectCall("changeFlags(105,2,5)");
    mock.provideReturnValue<int32_t>(0);
    a.checkEqual("151. changeFlags", level4.changeFlags(105, 2, 5, afl::base::Nothing), 0);

    {
        mock.expectCall("changeFlags(105,2,5,33,34)");
        mock.provideReturnValue<int32_t>(2);
        static const int32_t pmids[] = { 33, 34 };
        a.checkEqual("161. changeFlags", level4.changeFlags(105, 2, 5, pmids), 2);
    }

    mock.checkFinish();
}
