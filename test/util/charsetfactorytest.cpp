/**
  *  \file test/util/charsetfactorytest.cpp
  *  \brief Test for util::CharsetFactory
  */

#include "util/charsetfactory.hpp"

#include "afl/charset/charset.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Test that iteration works correctly.
    All indexes obtained by iteration must produce meaningful results. */
AFL_TEST("util.CharsetFactory:iteration", a)
{
    util::CharsetFactory testee;
    afl::string::NullTranslator tx;
    for (util::CharsetFactory::Index_t i = 0; i < testee.getNumCharsets(); ++i) {
        // Verify self-description
        a.check("01. getCharsetKey",         !testee.getCharsetKey(i).empty());
        a.check("02. getCharsetName",        !testee.getCharsetName(i, tx).empty());
        a.check("03. getCharsetDescription", !testee.getCharsetDescription(i, tx).empty());

        // Reverse-lookup
        util::CharsetFactory::Index_t other = 0;
        a.check("11. findIndexByKey", testee.findIndexByKey(testee.getCharsetKey(i)).get(other));
        a.checkEqual("12. found", other, i);

        // Creation
        std::auto_ptr<afl::charset::Charset> aa(testee.createCharset(testee.getCharsetKey(i)));
        std::auto_ptr<afl::charset::Charset> bb(testee.createCharsetByIndex(i));
        a.checkNonNull("21. createCharset", aa.get());
        a.checkNonNull("22. createCharset", bb.get());
        a.checkEqual("23. decode", aa->decode(afl::string::toBytes("\x82\xC2")), bb->decode(afl::string::toBytes("\x82\xC2")));
    }
}

/** Test resolving some names.
    Verifies that variants of names resolve correctly. */
AFL_TEST("util.CharsetFactory:findIndexByKey", a)
{
    util::CharsetFactory testee;
    util::CharsetFactory::Index_t result;

    // UTF-8, variants
    result = 99;
    a.check("01", testee.findIndexByKey("utf-8").get(result));
    a.checkEqual("02", result, util::CharsetFactory::UNICODE_INDEX);

    result = 99;
    a.check("11", testee.findIndexByKey("UTF-8").get(result));
    a.checkEqual("12", result, util::CharsetFactory::UNICODE_INDEX);

    result = 99;
    a.check("21", testee.findIndexByKey("utf8").get(result));
    a.checkEqual("22", result, util::CharsetFactory::UNICODE_INDEX);

    // Latin-1, variants
    result = 99;
    a.check("31", testee.findIndexByKey("latin1").get(result));
    a.checkEqual("32", result, util::CharsetFactory::LATIN1_INDEX);

    result = 99;
    a.check("41", testee.findIndexByKey("ISO-8859-1").get(result));
    a.checkEqual("42", result, util::CharsetFactory::LATIN1_INDEX);
}

/** Test some codes.
    Verifies that characters generated using a character set by name are translated correctly. */
AFL_TEST("util.CharsetFactory:decode", a)
{
    util::CharsetFactory testee;
    std::auto_ptr<afl::charset::Charset> aa(testee.createCharset("ansi"));
    std::auto_ptr<afl::charset::Charset> bb(testee.createCharset("pcc1"));
    std::auto_ptr<afl::charset::Charset> cc(testee.createCharset("cp437"));
    std::auto_ptr<afl::charset::Charset> dd(testee.createCharset("koi8r"));
    a.checkNonNull("01. createCharset", aa.get());
    a.checkNonNull("02. createCharset", bb.get());
    a.checkNonNull("03. createCharset", cc.get());
    a.checkNonNull("04. createCharset", dd.get());

    static const uint8_t CH[] = {
        0xA9           // ANSI: 00A9, PCC: 00AE, 437: 2310, KOI8R: 2515
    };

    a.checkEqual("11. decode", aa->decode(CH), "\xC2\xA9");
    a.checkEqual("12. decode", bb->decode(CH), "\xC2\xAE");
    a.checkEqual("13. decode", cc->decode(CH), "\xE2\x8C\x90");
    a.checkEqual("14. decode", dd->decode(CH), "\xE2\x94\x95");
}

/** Test error behaviour. */
AFL_TEST("util.CharsetFactory:errors", a)
{
    util::CharsetFactory testee;
    afl::string::NullTranslator tx;
    a.checkNull("01. createCharset",          testee.createCharset(""));
    a.checkNull("02. createCharset",          testee.createCharset("hi mom"));
    a.checkNull("03. createCharsetByIndex",   testee.createCharsetByIndex(99999));
    a.checkEqual("04. getCharsetKey",         testee.getCharsetKey(99999), "");
    a.checkEqual("05. getCharsetName",        testee.getCharsetName(99999, tx), "");
    a.checkEqual("06. getCharsetDescription", testee.getCharsetDescription(99999, tx), "");
}
