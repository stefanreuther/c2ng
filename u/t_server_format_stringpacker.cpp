/**
  *  \file u/t_server_format_stringpacker.cpp
  *  \brief Test for server::format::StringPacker
  */

#include "server/format/stringpacker.hpp"

#include "t_server_format.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/data/access.hpp"
#include "afl/data/stringvalue.hpp"

/** Test string handling with UTF-8. */
void
TestServerFormatStringPacker::testUtf8()
{
    server::format::StringPacker testee;
    afl::charset::Utf8Charset cs;

    {
        TS_ASSERT_EQUALS(testee.pack(0, cs), "");
    }
    {
        afl::data::StringValue sv("hi");
        TS_ASSERT_EQUALS(testee.pack(&sv, cs), "hi");
    }
    {
        afl::data::StringValue sv("\xE2\x96\xBA");
        TS_ASSERT_EQUALS(testee.pack(&sv, cs), "\xE2\x96\xBA");
    }
    {
        std::auto_ptr<afl::data::Value> p(testee.unpack("", cs));
        TS_ASSERT_EQUALS(afl::data::Access(p).toString(), "");
    }
    {
        std::auto_ptr<afl::data::Value> p(testee.unpack("\xC2\xAE", cs));
        TS_ASSERT_EQUALS(afl::data::Access(p).toString(), "\xC2\xAE");
    }
}

/** Test string handling with a codepage. */
void
TestServerFormatStringPacker::testCodepage()
{
    server::format::StringPacker testee;
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);

    {
        TS_ASSERT_EQUALS(testee.pack(0, cs), "");
    }
    {
        afl::data::StringValue sv("hi");
        TS_ASSERT_EQUALS(testee.pack(&sv, cs), "hi");
    }
    {
        // "greater-equal", U+2265, 0xF2 in codepage 437
        afl::data::StringValue sv("\xE2\x89\xA5");
        TS_ASSERT_EQUALS(testee.pack(&sv, cs), "\xF2");
    }
    {
        std::auto_ptr<afl::data::Value> p(testee.unpack("", cs));
        TS_ASSERT_EQUALS(afl::data::Access(p).toString(), "");
    }
    {
        std::auto_ptr<afl::data::Value> p(testee.unpack("x\xF2y", cs));
        TS_ASSERT_EQUALS(afl::data::Access(p).toString(), "x\xE2\x89\xA5y");
    }
}

