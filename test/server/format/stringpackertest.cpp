/**
  *  \file test/server/format/stringpackertest.cpp
  *  \brief Test for server::format::StringPacker
  */

#include "server/format/stringpacker.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/data/access.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/test/testrunner.hpp"

/** Test string handling with UTF-8. */
AFL_TEST("server.format.StringPacker:utf-8", a)
{
    server::format::StringPacker testee;
    afl::charset::Utf8Charset cs;

    {
        a.checkEqual("01", testee.pack(0, cs), "");
    }
    {
        afl::data::StringValue sv("hi");
        a.checkEqual("02", testee.pack(&sv, cs), "hi");
    }
    {
        afl::data::StringValue sv("\xE2\x96\xBA");
        a.checkEqual("03", testee.pack(&sv, cs), "\xE2\x96\xBA");
    }
    {
        std::auto_ptr<afl::data::Value> p(testee.unpack("", cs));
        a.checkEqual("04", afl::data::Access(p).toString(), "");
    }
    {
        std::auto_ptr<afl::data::Value> p(testee.unpack("\xC2\xAE", cs));
        a.checkEqual("05", afl::data::Access(p).toString(), "\xC2\xAE");
    }
}

/** Test string handling with a codepage. */
AFL_TEST("server.format.StringPacker:codepage", a)
{
    server::format::StringPacker testee;
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);

    {
        a.checkEqual("01", testee.pack(0, cs), "");
    }
    {
        afl::data::StringValue sv("hi");
        a.checkEqual("02", testee.pack(&sv, cs), "hi");
    }
    {
        // "greater-equal", U+2265, 0xF2 in codepage 437
        afl::data::StringValue sv("\xE2\x89\xA5");
        a.checkEqual("03", testee.pack(&sv, cs), "\xF2");
    }
    {
        std::auto_ptr<afl::data::Value> p(testee.unpack("", cs));
        a.checkEqual("04", afl::data::Access(p).toString(), "");
    }
    {
        std::auto_ptr<afl::data::Value> p(testee.unpack("x\xF2y", cs));
        a.checkEqual("05", afl::data::Access(p).toString(), "x\xE2\x89\xA5y");
    }
}
