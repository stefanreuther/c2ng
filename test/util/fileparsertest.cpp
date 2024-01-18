/**
  *  \file test/util/fileparsertest.cpp
  *  \brief Test for util::FileParser
  */

#include "util/fileparser.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/format.hpp"
#include "afl/test/assert.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include <vector>

namespace {
    using afl::string::Format;
    class TesterMock : public util::FileParser, public afl::test::CallReceiver {
     public:
        TesterMock(const afl::test::Assert& a)
            : FileParser("#"),
              CallReceiver(a)
            { }
        virtual void handleLine(const String_t& fileName, int lineNr, String_t line)
            { checkCall(Format("handleLine(%s,%d,%s)", fileName, lineNr, line)); }
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line)
            { checkCall(Format("handleIgnoredLine(%s,%d,%s)", fileName, lineNr, line)); }
    };
}

/** Interface test. */
AFL_TEST_NOARG("util.FileParser:interface")
{
    class Tester : public util::FileParser {
     public:
        Tester()
            : FileParser(";")
            { }
        virtual void handleLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
            { }
        virtual void handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
            { }
    };
    Tester t;
}

/** Simple functionality test. */
AFL_TEST("util.FileParser:basics", a)
{
    TesterMock t(a);
    t.expectCall("handleLine(<memory>,1,first)");
    t.expectCall("handleIgnoredLine(<memory>,2,   #comment)");
    t.expectCall("handleIgnoredLine(<memory>,3,# another comment)");
    t.expectCall("handleLine(<memory>,4,not # comment)");
    t.expectCall("handleIgnoredLine(<memory>,5,)");
    t.expectCall("handleLine(<memory>,6,final)");

    afl::io::ConstMemoryStream ms(afl::string::toBytes("first\n"
                                                       "   #comment\n"
                                                       "# another comment\n"
                                                       "not # comment\n"
                                                       "\n"
                                                       "final"));
    a.checkEqual("getName", ms.getName(), "<memory>");  // not contractual, but embedded in above expectations
    t.parseFile(ms);
    t.checkFinish();
}

/** Functionality test using charset. */
AFL_TEST("util.FileParser:setCharsetNew", a)
{
    TesterMock t(a);
    t.expectCall("handleLine(<memory>,1,x\xc3\x97y)");
    t.setCharsetNew(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));

    afl::io::ConstMemoryStream ms(afl::string::toBytes("x\xd7y\n"));
    t.parseFile(ms);
    t.checkFinish();
}

/** Test trimComments(). */
AFL_TEST("util.FileParser:trimComments", a)
{
    TesterMock t(a);

    {
        String_t s = "a#b#c";
        t.trimComments(s);
        a.checkEqual("01", s, "a");
    }

    {
        String_t s = "foo # bar";
        t.trimComments(s);
        a.checkEqual("11", s, "foo");
    }

    {
        String_t s = "    \t   # hi!";
        t.trimComments(s);
        a.checkEqual("21", s, "");
    }
}

/*
 *  Test parseOptionalFile().
 */

AFL_TEST("util.FileParser:parseOptionalFile:found", a)
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    dir->addStream("a", *new afl::io::ConstMemoryStream(afl::string::toBytes("first\n")));
    TesterMock t(a);
    t.expectCall("handleLine(<memory>,1,first)");

    bool ok = t.parseOptionalFile(*dir, "a");
    a.check("ok", ok);
    t.checkFinish();
}

AFL_TEST("util.FileParser:parseOptionalFile:not-found", a)
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    TesterMock t(a);
    bool ok = t.parseOptionalFile(*dir, "b");
    a.check("ok", !ok);
    t.checkFinish();
}
