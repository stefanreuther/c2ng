/**
  *  \file u/t_util_charsetfactory.cpp
  *  \brief Test for util::CharsetFactory
  */

#include "util/charsetfactory.hpp"

#include "t_util.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/charset/charset.hpp"

/** Test that iteration works correctly.
    All indexes obtained by iteration must produce meaningful results. */
void
TestUtilCharsetFactory::testIteration()
{
    util::CharsetFactory testee;
    afl::string::NullTranslator tx;
    for (util::CharsetFactory::Index_t i = 0; i < testee.getNumCharsets(); ++i) {
        // Verify self-description
        TS_ASSERT(!testee.getCharsetKey(i).empty());
        TS_ASSERT(!testee.getCharsetName(i, tx).empty());
        TS_ASSERT(!testee.getCharsetDescription(i, tx).empty());

        // Reverse-lookup
        util::CharsetFactory::Index_t other = 0;
        TS_ASSERT(testee.findIndexByKey(testee.getCharsetKey(i)).get(other));
        TS_ASSERT_EQUALS(other, i);

        // Creation
        std::auto_ptr<afl::charset::Charset> a(testee.createCharset(testee.getCharsetKey(i)));
        std::auto_ptr<afl::charset::Charset> b(testee.createCharsetByIndex(i));
        TS_ASSERT(a.get() != 0);
        TS_ASSERT(b.get() != 0);
        TS_ASSERT_EQUALS(a->decode(afl::string::toBytes("\x82\xC2")), b->decode(afl::string::toBytes("\x82\xC2")));
    }
}

/** Test resolving some names.
    Verifies that variants of names resolve correctly. */
void
TestUtilCharsetFactory::testNames()
{
    util::CharsetFactory testee;
    util::CharsetFactory::Index_t result;

    // UTF-8, variants
    result = 99;
    TS_ASSERT(testee.findIndexByKey("utf-8").get(result));
    TS_ASSERT_EQUALS(result, util::CharsetFactory::UNICODE_INDEX);

    result = 99;
    TS_ASSERT(testee.findIndexByKey("UTF-8").get(result));
    TS_ASSERT_EQUALS(result, util::CharsetFactory::UNICODE_INDEX);

    result = 99;
    TS_ASSERT(testee.findIndexByKey("utf8").get(result));
    TS_ASSERT_EQUALS(result, util::CharsetFactory::UNICODE_INDEX);

    // Latin-1, variants
    result = 99;
    TS_ASSERT(testee.findIndexByKey("latin1").get(result));
    TS_ASSERT_EQUALS(result, util::CharsetFactory::LATIN1_INDEX);

    result = 99;
    TS_ASSERT(testee.findIndexByKey("ISO-8859-1").get(result));
    TS_ASSERT_EQUALS(result, util::CharsetFactory::LATIN1_INDEX);
}

/** Test some codes.
    Verifies that characters generated using a character set by name are translated correctly. */
void
TestUtilCharsetFactory::testCodes()
{
    util::CharsetFactory testee;
    std::auto_ptr<afl::charset::Charset> a(testee.createCharset("ansi"));
    std::auto_ptr<afl::charset::Charset> b(testee.createCharset("pcc1"));
    std::auto_ptr<afl::charset::Charset> c(testee.createCharset("cp437"));
    std::auto_ptr<afl::charset::Charset> d(testee.createCharset("koi8r"));
    TS_ASSERT(a.get() != 0);
    TS_ASSERT(b.get() != 0);
    TS_ASSERT(c.get() != 0);
    TS_ASSERT(d.get() != 0);

    static const uint8_t CH[] = {
        0xA9           // ANSI: 00A9, PCC: 00AE, 437: 2310, KOI8R: 2515
    };

    TS_ASSERT_EQUALS(a->decode(CH), "\xC2\xA9");
    TS_ASSERT_EQUALS(b->decode(CH), "\xC2\xAE");
    TS_ASSERT_EQUALS(c->decode(CH), "\xE2\x8C\x90");
    TS_ASSERT_EQUALS(d->decode(CH), "\xE2\x94\x95");
}

/** Test error behaviour. */
void
TestUtilCharsetFactory::testErrors()
{
    util::CharsetFactory testee;
    afl::string::NullTranslator tx;
    TS_ASSERT(testee.createCharset("") == 0);
    TS_ASSERT(testee.createCharset("hi mom") == 0);
    TS_ASSERT(testee.createCharsetByIndex(99999) == 0);
    TS_ASSERT_EQUALS(testee.getCharsetKey(99999), "");
    TS_ASSERT_EQUALS(testee.getCharsetName(99999, tx), "");
    TS_ASSERT_EQUALS(testee.getCharsetDescription(99999, tx), "");
}

