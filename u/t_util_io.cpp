/**
  *  \file u/t_util_io.cpp
  *  \brief Test for util/io.hpp
  */

#include "util/io.hpp"

#include "t_util.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/nullfilesystem.hpp"

/** Test storePascalString. */
void
TestUtilIo::testStorePascalString()
{
    afl::charset::Utf8Charset cs;

    // Border case: empty
    {
        afl::io::InternalSink sink;
        bool ok = util::storePascalString(sink, "", cs);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(sink.getContent().size(), 1U);
        TS_ASSERT_EQUALS(*sink.getContent().at(0), 0);
    }

    // Regular case
    {
        afl::io::InternalSink sink;
        bool ok = util::storePascalString(sink, "hello", cs);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(sink.getContent().size(), 6U);
        TS_ASSERT(sink.getContent().equalContent(afl::string::toBytes("\5hello")));
    }

    // Border case: 255 ch
    {
        afl::io::InternalSink sink;
        bool ok = util::storePascalString(sink, String_t(255, 'x'), cs);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(sink.getContent().size(), 256U);
        TS_ASSERT_EQUALS(*sink.getContent().at(0), 255);
        TS_ASSERT_EQUALS(*sink.getContent().at(1), 'x');
        TS_ASSERT_EQUALS(*sink.getContent().at(255), 'x');
    }

    // Border case: 256 ch
    {
        afl::io::InternalSink sink;
        bool ok = util::storePascalString(sink, String_t(256, 'x'), cs);
        TS_ASSERT(!ok);
        TS_ASSERT_EQUALS(sink.getContent().size(), 0U);
    }

    // Too much
    {
        afl::io::InternalSink sink;
        bool ok = util::storePascalString(sink, String_t(999, 'x'), cs);
        TS_ASSERT(!ok);
        TS_ASSERT_EQUALS(sink.getContent().size(), 0U);
    }

    // Limit applies to encoded size!
    {
        // Build a long string
        String_t str;
        for (int i = 0; i < 128; ++i) {
            afl::charset::Utf8().append(str, 0x2248);     // U+2248 = 0xF7 = 247 in cp437
        }
        TS_ASSERT_EQUALS(str.size(), 3*128U);

        // Pack in cp437
        afl::charset::CodepageCharset cpcs(afl::charset::g_codepage437);
        afl::io::InternalSink sink;
        bool ok = util::storePascalString(sink, str, cpcs);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(sink.getContent().size(), 129U);
        TS_ASSERT_EQUALS(*sink.getContent().at(0), 128);
        TS_ASSERT_EQUALS(*sink.getContent().at(1), 247);
        TS_ASSERT_EQUALS(*sink.getContent().at(128), 247);
    }
}

/** Test storePascalStringTruncate. */
void
TestUtilIo::testStorePascalStringTruncate()
{
    afl::charset::Utf8Charset cs;

    // Border case: empty
    {
        afl::io::InternalSink sink;
        bool ok = util::storePascalStringTruncate(sink, "", cs);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(sink.getContent().size(), 1U);
        TS_ASSERT_EQUALS(*sink.getContent().at(0), 0);
    }

    // Regular case
    {
        afl::io::InternalSink sink;
        bool ok = util::storePascalStringTruncate(sink, "hello", cs);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(sink.getContent().size(), 6U);
        TS_ASSERT(sink.getContent().equalContent(afl::string::toBytes("\5hello")));
    }

    // Border case: 255 ch
    {
        afl::io::InternalSink sink;
        bool ok = util::storePascalStringTruncate(sink, String_t(255, 'x'), cs);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(sink.getContent().size(), 256U);
        TS_ASSERT_EQUALS(*sink.getContent().at(0), 255);
        TS_ASSERT_EQUALS(*sink.getContent().at(1), 'x');
        TS_ASSERT_EQUALS(*sink.getContent().at(255), 'x');
    }

    // Border case: 256 ch
    {
        afl::io::InternalSink sink;
        bool ok = util::storePascalStringTruncate(sink, String_t(256, 'x'), cs);
        TS_ASSERT(!ok);
        TS_ASSERT_EQUALS(sink.getContent().size(), 256U);
        TS_ASSERT_EQUALS(*sink.getContent().at(0), 255);
        TS_ASSERT_EQUALS(*sink.getContent().at(1), 'x');
        TS_ASSERT_EQUALS(*sink.getContent().at(255), 'x');
    }

    // Too much
    {
        afl::io::InternalSink sink;
        bool ok = util::storePascalStringTruncate(sink, String_t(999, 'x'), cs);
        TS_ASSERT(!ok);
        TS_ASSERT_EQUALS(sink.getContent().size(), 256U);
        TS_ASSERT_EQUALS(*sink.getContent().at(0), 255);
        TS_ASSERT_EQUALS(*sink.getContent().at(1), 'x');
        TS_ASSERT_EQUALS(*sink.getContent().at(255), 'x');
    }

    // Limit applies to encoded size!
    {
        // Build a long string
        String_t str;
        for (int i = 0; i < 128; ++i) {
            afl::charset::Utf8().append(str, 0x2248);     // U+2248 = 0xF7 = 247 in cp437
        }
        TS_ASSERT_EQUALS(str.size(), 3*128U);

        // Pack in cp437
        afl::charset::CodepageCharset cpcs(afl::charset::g_codepage437);
        afl::io::InternalSink sink;
        bool ok = util::storePascalStringTruncate(sink, str, cpcs);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(sink.getContent().size(), 129U);
        TS_ASSERT_EQUALS(*sink.getContent().at(0), 128);
        TS_ASSERT_EQUALS(*sink.getContent().at(1), 247);
        TS_ASSERT_EQUALS(*sink.getContent().at(128), 247);
    }
}

/** Test loadPascalString. */
void
TestUtilIo::testLoadPascalString()
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);

    // Trivial case
    {
        static const uint8_t data[] = {0};
        afl::io::ConstMemoryStream ms(data);
        TS_ASSERT_EQUALS(util::loadPascalString(ms, cs), "");
    }

    // Normal case
    {
        static const uint8_t data[] = {7, 'h','i',' ',0x81 /* U+00FC */, 0xDB /* U+2588 */,'x','y'};
        afl::io::ConstMemoryStream ms(data);
        TS_ASSERT_EQUALS(util::loadPascalString(ms, cs), "hi \xC3\xBC\xE2\x96\x88xy");
    }

    // Error case: truncated at length byte
    {
        afl::io::ConstMemoryStream ms(afl::base::Nothing);
        TS_ASSERT_THROWS(util::loadPascalString(ms, cs), afl::except::FileProblemException);
    }

    // Error case: truncated at payload byte
    {
        static const uint8_t data[] = {3,'y','y'};
        afl::io::ConstMemoryStream ms(data);
        TS_ASSERT_THROWS(util::loadPascalString(ms, cs), afl::except::FileProblemException);
    }
}

/** Test appendFileNameExtension. */
void
TestUtilIo::testAppendExt()
{
    // NullFileSystem uses PosixFileNames.
    afl::io::NullFileSystem fs;

    TS_ASSERT_EQUALS(util::appendFileNameExtension(fs, "readme", "txt", false), "readme.txt");
    TS_ASSERT_EQUALS(util::appendFileNameExtension(fs, "readme", "txt", true),  "readme.txt");

    TS_ASSERT_EQUALS(util::appendFileNameExtension(fs, "readme.doc", "txt", false), "readme.doc");
    TS_ASSERT_EQUALS(util::appendFileNameExtension(fs, "readme.doc", "txt", true),  "readme.txt");

    TS_ASSERT_EQUALS(util::appendFileNameExtension(fs, ".emacs", "txt", false), ".emacs.txt");
    TS_ASSERT_EQUALS(util::appendFileNameExtension(fs, ".emacs", "txt", true),  ".emacs.txt");

    TS_ASSERT_EQUALS(util::appendFileNameExtension(fs, "/a/b/c", "txt", false), "/a/b/c.txt");
    TS_ASSERT_EQUALS(util::appendFileNameExtension(fs, "/a/b/c", "txt", true),  "/a/b/c.txt");
}

