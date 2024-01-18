/**
  *  \file test/game/spec/friendlycodelisttest.cpp
  *  \brief Test for game::spec::FriendlyCodeList
  */

#include "game/spec/friendlycodelist.hpp"

#include "afl/base/growablememory.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/loglistener.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/planet.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/registrationkey.hpp"

/** Test isNumeric(). */
AFL_TEST("game.spec.FriendlyCodeList:isNumeric", a)
{
    // ex GameFcodeTestSuite::testFCode
    game::spec::FriendlyCodeList testee;

    game::HostVersion host;
    a.check("01",  testee.isNumeric("000", host));
    a.check("02", !testee.isNumeric("00x", host));
    a.check("03", !testee.isNumeric("x00", host));
    a.check("04", !testee.isNumeric("+00", host));
    a.check("05",  testee.isNumeric("999", host));

    host.set(host.Host, MKVERSION(3,22,40));
    a.check("11", !testee.isNumeric("-11", host));
    a.checkEqual("12", testee.getNumericValue("-11", host), 1000);
    a.check("13", !testee.isNumeric("--1", host));
    a.check("14", !testee.isNumeric("1", host));
    a.check("15", !testee.isNumeric(" 1", host));
    a.check("16", !testee.isNumeric("1 ", host));
    a.check("17", !testee.isNumeric(" 1 ", host));
    a.check("18", !testee.isNumeric("-  ", host));
    a.check("19", !testee.isNumeric("  -", host));
    a.check("20", !testee.isNumeric("   ", host));

    host.set(host.PHost, MKVERSION(4,0,0));
    a.check("21", testee.isNumeric("-11", host));
    a.checkEqual("22", testee.getNumericValue("-11", host), -11);
    a.check("23", !testee.isNumeric("--1", host));
    a.check("24", !testee.isNumeric("1", host));
    a.check("25", !testee.isNumeric(" 1", host));
    a.check("26", !testee.isNumeric("1 ", host));
    a.check("27", !testee.isNumeric(" 1 ", host));
    a.check("28", !testee.isNumeric("-  ", host));
    a.check("29", !testee.isNumeric("  -", host));
    a.check("30", !testee.isNumeric("   ", host));

    host.set(host.PHost, MKVERSION(4,0,8));
    a.check("31", testee.isNumeric("-11", host));
    a.checkEqual("32", testee.getNumericValue("-11", host), -11);
    a.check("33", !testee.isNumeric("--1", host));
    a.check("34", testee.isNumeric("1", host));
    a.checkEqual("35", testee.getNumericValue("1", host), 1);
    a.check("36", testee.isNumeric(" 1", host));
    a.checkEqual("37", testee.getNumericValue(" 1", host), 1);
    a.check("38", testee.isNumeric("1 ", host));
    a.checkEqual("39", testee.getNumericValue("1 ", host), 1);
    a.check("40", testee.isNumeric(" 1 ", host));
    a.checkEqual("41", testee.getNumericValue(" 1 ", host), 1);
    a.check("42", !testee.isNumeric("-  ", host));
    a.check("43", !testee.isNumeric("  -", host));
    a.check("44", !testee.isNumeric("   ", host));

    host.set(host.PHost, MKVERSION(3,4,9));
    a.check("51", !testee.isNumeric(" 1 ", host));

    host.set(host.PHost, MKVERSION(3,4,11));
    a.check("61", testee.isNumeric(" 1 ", host));

    a.check("71", testee.isNumeric("-11", testee.Pessimistic));
    a.checkEqual("72", testee.getNumericValue("-11", testee.Pessimistic), -11);
    a.check("73", !testee.isNumeric("--1", testee.Pessimistic));
    a.check("74",  testee.isNumeric("1",   testee.Pessimistic));
    a.check("75",  testee.isNumeric(" 1",  testee.Pessimistic));
    a.check("76",  testee.isNumeric("1 ",  testee.Pessimistic));
    a.check("77",  testee.isNumeric(" 1 ", testee.Pessimistic));
    a.check("78", !testee.isNumeric("-  ", testee.Pessimistic));
    a.check("79", !testee.isNumeric("  -", testee.Pessimistic));
    a.check("80", !testee.isNumeric("   ", testee.Pessimistic));
}

/** Test isAllowedRandomCode(). */
AFL_TEST("game.spec.FriendlyCodeList:isAllowedRandomCode", a)
{
    // ex GameFcodeTestSuite::testRandom
    game::spec::FriendlyCodeList testee;
    afl::string::NullTranslator tx;

    afl::io::ConstMemoryStream in(afl::string::toBytes("E zot"));
    testee.loadExtraCodes(in, tx);
    testee.addCode(game::spec::FriendlyCode("mkt", "sc,make torps", tx));

    game::HostVersion host;
    host.set(host.PHost, MKVERSION(4, 0, 0));

    // Now check:
    a.check("01", testee.isAllowedRandomCode("abc", host));
    a.check("02", testee.isAllowedRandomCode("01a", host));
    a.check("03", testee.isAllowedRandomCode("a01", host));
    a.check("04", testee.isAllowedRandomCode("0 1", host));   // allowed, but will not be generated
    a.check("05", testee.isAllowedRandomCode("zxy", host));
    a.check("06", testee.isAllowedRandomCode("0-1", host));   // allowed, but will not be generated
    a.check("07", !testee.isAllowedRandomCode("elo", host));  // no longer allowed, extra-fc now case-insensitive
    a.check("08", !testee.isAllowedRandomCode("Zot", host));  // no longer allowed, extra-fc now case-insensitive
    a.check("09", !testee.isAllowedRandomCode("zoT", host));
    a.check("10", testee.isAllowedRandomCode("zo ", host));

    a.check("11", !testee.isAllowedRandomCode("mkt", host));  // fails: predefined code
    a.check("12", !testee.isAllowedRandomCode("Mkt", host));  // fails: variant of predefined
    a.check("13", !testee.isAllowedRandomCode("mKt", host));  // fails: variant of predefined
    a.check("14", !testee.isAllowedRandomCode("mkT", host));  // fails: variant of predefined
    a.check("15", !testee.isAllowedRandomCode("MKT", host));  // fails: variant of predefined

    a.check("21", !testee.isAllowedRandomCode("aab", host));  // fails: duplicate character
    a.check("22", !testee.isAllowedRandomCode("aba", host));  // fails: duplicate character
    a.check("23", !testee.isAllowedRandomCode("baa", host));  // fails: duplicate character

    a.check("31", !testee.isAllowedRandomCode("mf1", host));  // fails: universal minefield code
    a.check("32", !testee.isAllowedRandomCode("mff", host));  // fails: universal minefield code
    a.check("33", !testee.isAllowedRandomCode("MFx", host));  // fails: universal minefield code, case-insensitive in THost!
    a.check("34", !testee.isAllowedRandomCode("Mfx", host));  // fails: universal minefield code, case-insensitive in THost!
    a.check("35", !testee.isAllowedRandomCode("mFx", host));  // fails: universal minefield code, case-insensitive in THost!

    a.check("41", !testee.isAllowedRandomCode("xyz", host));  // fails: starts with 'X' (bird men rule)
    a.check("42", !testee.isAllowedRandomCode("Xyz", host));  // fails: starts with 'X' (bird men rule)

    a.check("51", !testee.isAllowedRandomCode("000", host));  // fails: numeric
    a.check("52", !testee.isAllowedRandomCode("012", host));  // fails: numeric
    a.check("53", !testee.isAllowedRandomCode("-19", host));  // fails: numeric, and host allows it

    a.check("61", !testee.isAllowedRandomCode("Elo", host));  // fails: prefix blocked by extra FC
    a.check("62", !testee.isAllowedRandomCode("Eex", host));  // fails: prefix blocked by extra FC
    a.check("63", !testee.isAllowedRandomCode("zot", host));  // fails: blocked by extra FC

    a.check("71", !testee.isAllowedRandomCode("?xy", host));  // fails: '?' not allowed
    a.check("72", !testee.isAllowedRandomCode("z?y", host));  // fails: '?' not allowed
    a.check("73", !testee.isAllowedRandomCode("zx?", host));  // fails: '?' not allowed

    a.check("81", !testee.isAllowedRandomCode("#xy", host));  // fails: '#' not allowed
    a.check("82", !testee.isAllowedRandomCode("z#y", host));  // fails: '#' not allowed
    a.check("83", !testee.isAllowedRandomCode("zx#", host));  // fails: '#' not allowed

    a.check("91", !testee.isAllowedRandomCode("###", host));
    a.check("92", !testee.isAllowedRandomCode("?""?""?", host));

    a.check("101", !testee.isAllowedRandomCode("", host));     // fails: length mismatch
    a.check("102", !testee.isAllowedRandomCode("a", host));    // fails: length mismatch
    a.check("103", !testee.isAllowedRandomCode("ab", host));   // fails: length mismatch
    a.check("104", !testee.isAllowedRandomCode("abcd", host)); // fails: length mismatch

    a.check("111", testee.isAllowedRandomCode("-19", game::HostVersion(game::HostVersion::Host, MKVERSION(3,2,0))));
    a.check("112", !testee.isAllowedRandomCode("-19", game::spec::FriendlyCodeList::Pessimistic));
}

/** Test container behaviour. */
AFL_TEST("game.spec.FriendlyCodeList:container", a)
{
    game::spec::FriendlyCodeList testee;
    afl::string::NullTranslator tx;

    // Verify initial state
    a.checkEqual("01. size", testee.size(), 0U);
    a.check("02. begin", testee.begin() == testee.end());
    a.checkNull("03. at", testee.at(0));

    // Add some elements
    testee.addCode(game::spec::FriendlyCode("pfc", "p,xxx", tx));
    testee.addCode(game::spec::FriendlyCode("bfc", "b,xxx", tx));
    testee.addCode(game::spec::FriendlyCode("sfc", "s,xxx", tx));
    testee.addCode(game::spec::FriendlyCode("ffc", "p+1,xxx", tx));

    // Verify properties
    a.checkEqual("11. size", testee.size(), 4U);
    a.checkEqual("12. begin", (*testee.begin())->getCode(), "pfc");

    a.checkNonNull("21. at", testee.at(0));
    a.checkNonNull("22. at", testee.at(3));
    a.checkNull("23. at", testee.at(4));
    a.checkEqual("24. at", testee.at(0)->getCode(), "pfc");
    a.checkEqual("25. at", testee.at(1)->getCode(), "bfc");
    a.checkEqual("26. at", testee.at(2)->getCode(), "sfc");
    a.checkEqual("27. at", testee.at(3)->getCode(), "ffc");

    a.checkEqual("31. findIndexByName", testee.findIndexByName("sfc").orElse(9999), 2U);
    a.check("32. findIndexByName", !testee.findIndexByName("SFC").isValid());
    a.check("33. findIndexByName", !testee.findIndexByName("mkt").isValid());

    a.check("41. findCodeByName", testee.findCodeByName("sfc") == testee.begin() + 2);
    a.check("42. findCodeByName", testee.findCodeByName("mkt") == testee.end());

    // Sort
    testee.sort();
    a.checkEqual("51. size", testee.size(), 4U);
    a.checkEqual("52. at", testee.at(0)->getCode(), "bfc");
    a.checkEqual("53. at", testee.at(1)->getCode(), "ffc");
    a.checkEqual("54. at", testee.at(2)->getCode(), "pfc");
    a.checkEqual("55. at", testee.at(3)->getCode(), "sfc");
    a.checkEqual("56. begin", (*testee.begin())->getCode(), "bfc");

    a.checkEqual("61. findIndexByName", testee.findIndexByName("sfc").orElse(9999), 3U);

    // Create a sub-list
    game::map::Planet p(9);
    p.setOwner(1);
    p.setPlayability(p.ReadOnly);

    const game::config::HostConfiguration hostConfig;
    game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);
    game::spec::FriendlyCodeList sublist(testee, game::spec::FriendlyCode::Filter::fromPlanet(p, hostConfig), key);
    a.checkEqual("71. size", sublist.size(), 2U);
    a.checkEqual("72. at", sublist.at(0)->getCode(), "ffc");
    a.checkEqual("73. at", sublist.at(1)->getCode(), "pfc");

    // Clear original list. Sublist remains.
    testee.clear();
    a.checkEqual("81. size", testee.size(), 0U);
    a.checkEqual("82. size", sublist.size(), 2U);
    a.checkEqual("83. at", sublist.at(0)->getCode(), "ffc");
}

/** Test special friendly code detection. */
AFL_TEST("game.spec.FriendlyCodeList:isSpecial", a)
{
    game::spec::FriendlyCodeList testee;
    afl::string::NullTranslator tx;

    // Provide normal
    testee.addCode(game::spec::FriendlyCode("pfc", "p,xxx", tx));
    testee.addCode(game::spec::FriendlyCode("bfc", "b,xxx", tx));
    testee.addCode(game::spec::FriendlyCode("ufc", "u,xxx", tx));

    // Load extras
    afl::io::ConstMemoryStream ms(afl::string::toBytes("ab\n"
                                                       "z\n"
                                                       "pppp\n"
                                                       "e f"));
    testee.loadExtraCodes(ms, tx);

    // Verify
    a.check("01",  testee.isSpecial("ab",   true));
    a.check("02",  testee.isSpecial("abc",  true));
    a.check("03",  testee.isSpecial("z",    true));
    a.check("04", !testee.isSpecial("ZZ",   false));
    a.check("05",  testee.isSpecial("ZZ",   true));
    a.check("06", !testee.isSpecial("ppp",  true));   // no truncation to 3 characters!
    a.check("07",  testee.isSpecial("pppp", true));
    a.check("08",  testee.isSpecial("e",    true));
    a.check("09",  testee.isSpecial("e11",  true));
    a.check("10",  testee.isSpecial("fff",  true));

    // Check special
    a.check("11",  testee.isSpecial("pfc", false));
    a.check("12",  testee.isSpecial("bfc", false));
    a.check("13", !testee.isSpecial("ufc", false));
    a.check("14", !testee.isSpecial("PFC", false));
    a.check("15", !testee.isSpecial("BFC", false));
    a.check("16", !testee.isSpecial("UFC", false));
    a.check("17",  testee.isSpecial("PFC", true));
    a.check("18",  testee.isSpecial("BFC", true));
    a.check("19", !testee.isSpecial("UFC", true));

    // Clear
    testee.clear();
    a.check("21", !testee.isSpecial("ab",  true));
    a.check("22", !testee.isSpecial("abc", true));
    a.check("23", !testee.isSpecial("z",   true));
}

/** Test generateRandomCode(). */
AFL_TEST("game.spec.FriendlyCodeList:generateRandomCode", a)
{
    // Environment
    game::HostVersion host;
    util::RandomNumberGenerator rng(0);

    // Testee
    game::spec::FriendlyCodeList testee;

    // Test.
    // Checking whether the result satisfies the rules means reimplementing them,
    // but let's test that the result is sufficiently random.
    // (This test will also fail if the generator fails to advance the random seed.)
    String_t fa = testee.generateRandomCode(rng, host);
    String_t fb = testee.generateRandomCode(rng, host);
    String_t fc = testee.generateRandomCode(rng, host);

    a.checkDifferent("01", fa, fb);
    a.checkDifferent("02", fa, fc);
    a.checkDifferent("03", fb, fc);
}

/** Test isUniversalMinefieldFCode(). */
AFL_TEST("game.spec.FriendlyCodeList:isUniversalMinefieldFCode", a)
{
    // Environment
    game::HostVersion phost;
    phost.set(phost.PHost, MKVERSION(4, 0, 0));

    game::HostVersion thost;
    thost.set(thost.Host, MKVERSION(3, 2, 0));

    // Testee
    game::spec::FriendlyCodeList testee;

    // Test
    a.check("01", testee.isUniversalMinefieldFCode("mfx", false, phost));
    a.check("02", testee.isUniversalMinefieldFCode("mfx", false, thost));
    a.check("03", testee.isUniversalMinefieldFCode("mfx", false, testee.Pessimistic));
    a.check("04", !testee.isUniversalMinefieldFCode("abc", false, thost));

    a.check("11", !testee.isUniversalMinefieldFCode("MFX", false, phost));
    a.check("12", testee.isUniversalMinefieldFCode("MFX", false, thost));
    a.check("13", testee.isUniversalMinefieldFCode("MFX", false, testee.Pessimistic));
    a.check("14", !testee.isUniversalMinefieldFCode("ABC", false, thost));

    a.check("21", testee.isUniversalMinefieldFCode("MFX", true, phost));
    a.check("22", testee.isUniversalMinefieldFCode("MFX", true, thost));
    a.check("23", !testee.isUniversalMinefieldFCode("ABC", true, thost));

    a.check("31", !testee.isUniversalMinefieldFCode("ABC", false, testee.Pessimistic));
}

/** Test generateRandomCode() infinite loop avoidance. */
AFL_TEST("game.spec.FriendlyCodeList:generateRandomCode:loop", a)
{
    // Environment
    afl::string::NullTranslator tx;
    game::HostVersion host;
    util::RandomNumberGenerator rng(0);

    // Create a friendly code list that blocks all ASCII characters
    afl::base::GrowableMemory<uint8_t> mem;
    for (uint8_t ch = ' '; ch < 127; ++ch) {
        mem.append(ch);
        mem.append('\n');
    }
    afl::io::ConstMemoryStream ms(mem);
    game::spec::FriendlyCodeList testee;
    testee.loadExtraCodes(ms, tx);

    // generateRandomCode() must still finish
    a.checkEqual("01", testee.generateRandomCode(rng, host).size(), 3U);
}

/** Test generateRandomCode() infinite loop avoidance. */
AFL_TEST("game.spec.FriendlyCodeList:generateRandomCode:mostly-blocked", a)
{
    // Environment
    afl::string::NullTranslator tx;
    game::HostVersion host;
    util::RandomNumberGenerator rng(0);

    // Create a friendly code list that blocks all ASCII characters except for 3
    afl::base::GrowableMemory<uint8_t> mem;
    for (uint8_t ch = ' '; ch < 127; ++ch) {
        if (ch != '3') {
            mem.append(ch);
            mem.append('\n');
        }
    }
    afl::io::ConstMemoryStream ms(mem);
    game::spec::FriendlyCodeList testee;
    testee.loadExtraCodes(ms, tx);

    a.check("01",  testee.isSpecial("NXY", true));
    a.check("02", !testee.isSpecial("3XY", true));

    // generateRandomCode() must create a code starting with '3'
    String_t s = testee.generateRandomCode(rng, host);
    a.checkEqual("11. size", s.size(), 3U);
    a.checkEqual("12. s[0]", s[0], '3');
}

/** Test load(). */
AFL_TEST("game.spec.FriendlyCodeList:load", a)
{
    using game::spec::FriendlyCode;

    // Environment
    static const char* FILE =
        "; comment\n"           // comment
        "mkt,s,Make\n"          // ship code
        "\n"                    // blank line
        "  NUK  ,p,Nuke\n"      // planet code
        "a=b,c,d\n"             // assignment associated with planet code
        "???,u,Unspecial\n";    // unspecial
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));
    afl::sys::Log log;

    // Load
    game::spec::FriendlyCodeList testee;
    afl::string::NullTranslator tx;
    testee.load(ms, log, tx);

    // Verify
    a.checkEqual("01. size", testee.size(), 3U);
    a.checkEqual("02. getCode",  testee.at(0)->getCode(), "mkt");
    a.checkEqual("03. getFlags", testee.at(0)->getFlags(), FriendlyCode::FlagSet_t(FriendlyCode::ShipCode));
    a.checkEqual("04. getCode",  testee.at(1)->getCode(), "NUK");
    a.checkEqual("05. getFlags", testee.at(1)->getFlags(), FriendlyCode::FlagSet_t(FriendlyCode::PlanetCode));
    a.checkEqual("06. getCode",  testee.at(2)->getCode(), "???");
    a.checkEqual("07. getFlags", testee.at(2)->getFlags(), FriendlyCode::FlagSet_t(FriendlyCode::UnspecialCode));
}

/** Test sort order. */
AFL_TEST("game.spec.FriendlyCodeList:sort", a)
{
    using game::spec::FriendlyCode;
    afl::string::NullTranslator tx;

    // Alphanumeric goes before non-alphanumeric, capital before lower-case.
    game::spec::FriendlyCodeList testee;
    testee.addCode(FriendlyCode("!bc", ",x", tx));
    testee.addCode(FriendlyCode("abc", ",x", tx));
    testee.addCode(FriendlyCode("0bc", ",x", tx));
    testee.addCode(FriendlyCode("Abc", ",x", tx));
    testee.addCode(FriendlyCode("ABC", ",x", tx));
    testee.addCode(FriendlyCode("?bc", ",x", tx));

    // Sort
    testee.sort();

    // Verify
    a.checkEqual("01. size", testee.size(), 6U);
    a.checkEqual("02. getCode", testee.at(0)->getCode(), "0bc");
    a.checkEqual("03. getCode", testee.at(1)->getCode(), "ABC");
    a.checkEqual("04. getCode", testee.at(2)->getCode(), "Abc");
    a.checkEqual("05. getCode", testee.at(3)->getCode(), "abc");
    a.checkEqual("06. getCode", testee.at(4)->getCode(), "!bc");
    a.checkEqual("07. getCode", testee.at(5)->getCode(), "?bc");
}

/** Test syntax errors in load(). */

// Badly formatted line
AFL_TEST("game.spec.FriendlyCodeList:load:syntax-error", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("foo\n"));
    afl::test::LogListener log;
    game::spec::FriendlyCodeList list;
    afl::string::NullTranslator tx;
    list.load(ms, log, tx);
    a.checkEqual("01. getNumMessages", log.getNumMessages(), 1U);
    a.checkEqual("02. size", list.size(), 0U);
}

AFL_TEST("game.spec.FriendlyCodeList:load:code-too-long", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("longcode,,foo\n"));
    afl::test::LogListener log;
    game::spec::FriendlyCodeList list;
    afl::string::NullTranslator tx;
    list.load(ms, log, tx);
    a.checkEqual("01. getNumMessages", log.getNumMessages(), 1U);
    a.checkEqual("02. size", list.size(), 1U);
    a.checkEqual("03. getCode", list.at(0)->getCode(), "lon");
}

/** Test Pessimistic. */
AFL_TEST("game.spec.FriendlyCodeList:isAllowedRandomCode:pessimistic", a)
{
    using game::HostVersion;
    game::spec::FriendlyCodeList testee;

    a.check("01",  testee.isAllowedRandomCode(" 12", HostVersion(HostVersion::Host,  MKVERSION(3, 0, 0))));
    a.check("02", !testee.isAllowedRandomCode(" 12", HostVersion(HostVersion::PHost, MKVERSION(4, 1, 0))));
    a.check("03", !testee.isAllowedRandomCode(" 12", testee.Pessimistic));

    a.check("11",  testee.isAllowedRandomCode("-12", HostVersion(HostVersion::Host,  MKVERSION(3, 0, 0))));
    a.check("12", !testee.isAllowedRandomCode("-12", HostVersion(HostVersion::PHost, MKVERSION(4, 1, 0))));
    a.check("13", !testee.isAllowedRandomCode("-12", testee.Pessimistic));

    a.check("21", !testee.isAllowedRandomCode("Mff", HostVersion(HostVersion::Host,  MKVERSION(3, 0, 0))));
    a.check("22", !testee.isAllowedRandomCode("Mff", HostVersion(HostVersion::PHost, MKVERSION(4, 1, 0)))); // not a special friendly code, but isAllowedRandomCode() always is pessimistic
    a.check("23", !testee.isAllowedRandomCode("Mff", testee.Pessimistic));
}

/** Test pack(). */
AFL_TEST("game.spec.FriendlyCodeList:pack", a)
{
    // Friendly code list
    afl::string::NullTranslator tx;
    game::spec::FriendlyCodeList testee;
    testee.addCode(game::spec::FriendlyCode("pfc", "p,whatever", tx));
    testee.addCode(game::spec::FriendlyCode("gs3", "s,give to %3", tx));
    testee.addCode(game::spec::FriendlyCode("gs4", "s,give to %4", tx));
    afl::io::ConstMemoryStream ms(afl::string::toBytes("ab"));
    testee.loadExtraCodes(ms, tx);

    // Player list
    game::PlayerList pl;
    game::Player* p3 = pl.create(3);
    a.check("01", p3);
    p3->setName(game::Player::ShortName, "Threes");
    p3->setName(game::Player::AdjectiveName, "threeish");

    // Pack
    game::spec::FriendlyCodeList::Infos_t info;
    testee.pack(info, pl, tx);

    // Verify
    a.checkEqual("11. size", info.size(), 3U);
    a.checkEqual("12. code",        info[0].code, "pfc");
    a.checkEqual("13. description", info[0].description, "whatever");
    a.checkEqual("14. code",        info[1].code, "gs3");
    a.checkEqual("15. description", info[1].description, "give to Threes");
    a.checkEqual("16. code",        info[2].code, "gs4");
    a.checkEqual("17. description", info[2].description, "give to 4");

    // Original list has four elements
    a.checkEqual("21. size", testee.size(), 4U);
    a.checkEqual("22. at", testee.at(0)->getCode(), "pfc");
    a.checkEqual("23. at", testee.at(1)->getCode(), "gs3");
    a.checkEqual("24. at", testee.at(2)->getCode(), "gs4");
    a.checkEqual("25. at", testee.at(3)->getCode(), "ab");
}

/** Test loadExtraCodes, load when duplicates are present. */
AFL_TEST("game.spec.FriendlyCodeList:loadExtraCodes:dup", a)
{
    afl::string::NullTranslator tx;
    game::spec::FriendlyCodeList testee;

    // fcodes.cc
    {
        // load() will sort the list, so give it a sorted list in the first place to avoid surprises.
        static const char FILE[] =
            "ATT,p,Attack\n"
            "HYP,s,Hyper\n"
            "mkt,s,Make\n";
        afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));
        afl::sys::Log log;
        testee.load(ms, log, tx);
    }

    // xtrafcode.txt
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("A mkt NUK j"));
        testee.loadExtraCodes(ms, tx);
    }

    // Verify. Content must be mkt/ATT/HYP/A/NUK/j
    a.checkEqual("01. size", testee.size(), 6U);
    a.checkEqual("02. at", testee.at(0)->getCode(), "ATT");
    a.checkEqual("03. at", testee.at(1)->getCode(), "HYP");
    a.checkEqual("04. at", testee.at(2)->getCode(), "mkt");
    a.checkEqual("05. at", testee.at(3)->getCode(), "A");
    a.checkEqual("06. at", testee.at(4)->getCode(), "NUK");
    a.checkEqual("07. at", testee.at(5)->getCode(), "j");

    // Verify specialness
    a.checkEqual("11. isSpecial", testee.isSpecial("ATT", false), true);
    a.checkEqual("12. isSpecial", testee.isSpecial("AXE", false), true);    // due to 'A'
}

/** Test isAcceptedFriendlyCode(). */
AFL_TEST("game.spec.FriendlyCodeList:isAcceptedFriendlyCode", a)
{
    // Environment
    // - keys
    const game::test::RegistrationKey regKey(game::RegistrationKey::Registered, 10);
    const game::test::RegistrationKey unregKey(game::RegistrationKey::Unregistered, 6);

    // - filters
    // -- for that, a planet
    game::map::Planet p(77);
    p.setOwner(3);
    p.setPlayability(game::map::Object::Playable);

    // -- for that, a configuration
    game::config::HostConfiguration hostConfig;

    // -- the filters
    game::spec::FriendlyCode::Filter emptyFilter;
    game::spec::FriendlyCode::Filter planetFilter(game::spec::FriendlyCode::Filter::fromPlanet(p, hostConfig));

    // - FriendlyCodeList
    // -- for that, a translator
    afl::string::NullTranslator tx;

    // -- the list
    game::spec::FriendlyCodeList testee;
    testee.addCode(game::spec::FriendlyCode("sfc", "s,whatever", tx));
    testee.addCode(game::spec::FriendlyCode("gp3", "p+3,give to %3", tx));
    testee.addCode(game::spec::FriendlyCode("gp4", "p+4,give to %4", tx));
    testee.addCode(game::spec::FriendlyCode("mf1", "pX,", tx));

    // Test cases
    // - unknown code > result tracks DefaultAcceptance flag
    a.check("01",  testee.isAcceptedFriendlyCode("unk", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("02", !testee.isAcceptedFriendlyCode("unk", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("03",  testee.isAcceptedFriendlyCode("unk", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultRegistered));
    a.check("04",  testee.isAcceptedFriendlyCode("unk", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("05", !testee.isAcceptedFriendlyCode("unk", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("06", !testee.isAcceptedFriendlyCode("unk", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultRegistered));

    // - ship code > fails due to emptyFilter, planetFilter (not a planet)
    a.check("11", !testee.isAcceptedFriendlyCode("sfc", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("12", !testee.isAcceptedFriendlyCode("sfc", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("13", !testee.isAcceptedFriendlyCode("sfc", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultRegistered));
    a.check("14", !testee.isAcceptedFriendlyCode("sfc", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("15", !testee.isAcceptedFriendlyCode("sfc", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("16", !testee.isAcceptedFriendlyCode("sfc", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultRegistered));

    a.check("21", !testee.isAcceptedFriendlyCode("sfc", planetFilter, regKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("22", !testee.isAcceptedFriendlyCode("sfc", planetFilter, regKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("23", !testee.isAcceptedFriendlyCode("sfc", planetFilter, regKey, game::spec::FriendlyCodeList::DefaultRegistered));
    a.check("24", !testee.isAcceptedFriendlyCode("sfc", planetFilter, unregKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("25", !testee.isAcceptedFriendlyCode("sfc", planetFilter, unregKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("26", !testee.isAcceptedFriendlyCode("sfc", planetFilter, unregKey, game::spec::FriendlyCodeList::DefaultRegistered));

    // - planet code > fails on emptyFilter, accepted on planetFilter
    a.check("31", !testee.isAcceptedFriendlyCode("gp3", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("32", !testee.isAcceptedFriendlyCode("gp3", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("33", !testee.isAcceptedFriendlyCode("gp3", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultRegistered));
    a.check("34", !testee.isAcceptedFriendlyCode("gp3", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("35", !testee.isAcceptedFriendlyCode("gp3", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("36", !testee.isAcceptedFriendlyCode("gp3", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultRegistered));

    a.check("41",  testee.isAcceptedFriendlyCode("gp3", planetFilter, regKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("42",  testee.isAcceptedFriendlyCode("gp3", planetFilter, regKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("43",  testee.isAcceptedFriendlyCode("gp3", planetFilter, regKey, game::spec::FriendlyCodeList::DefaultRegistered));
    a.check("44",  testee.isAcceptedFriendlyCode("gp3", planetFilter, unregKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("45",  testee.isAcceptedFriendlyCode("gp3", planetFilter, unregKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("46",  testee.isAcceptedFriendlyCode("gp3", planetFilter, unregKey, game::spec::FriendlyCodeList::DefaultRegistered));

    // - planet code > fails due to emptyFilter, planetFilter (wrong player)
    a.check("51", !testee.isAcceptedFriendlyCode("gp4", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("52", !testee.isAcceptedFriendlyCode("gp4", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("53", !testee.isAcceptedFriendlyCode("gp4", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultRegistered));
    a.check("54", !testee.isAcceptedFriendlyCode("gp4", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("55", !testee.isAcceptedFriendlyCode("gp4", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("56", !testee.isAcceptedFriendlyCode("gp4", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultRegistered));

    a.check("61", !testee.isAcceptedFriendlyCode("gp4", planetFilter, regKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("62", !testee.isAcceptedFriendlyCode("gp4", planetFilter, regKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("63", !testee.isAcceptedFriendlyCode("gp4", planetFilter, regKey, game::spec::FriendlyCodeList::DefaultRegistered));
    a.check("64", !testee.isAcceptedFriendlyCode("gp4", planetFilter, unregKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("65", !testee.isAcceptedFriendlyCode("gp4", planetFilter, unregKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("66", !testee.isAcceptedFriendlyCode("gp4", planetFilter, unregKey, game::spec::FriendlyCodeList::DefaultRegistered));

    // - prefix code > fails, prefix never accepted
    a.check("71", !testee.isAcceptedFriendlyCode("mf1", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("72", !testee.isAcceptedFriendlyCode("mf1", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("73", !testee.isAcceptedFriendlyCode("mf1", emptyFilter, regKey, game::spec::FriendlyCodeList::DefaultRegistered));
    a.check("74", !testee.isAcceptedFriendlyCode("mf1", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("75", !testee.isAcceptedFriendlyCode("mf1", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("76", !testee.isAcceptedFriendlyCode("mf1", emptyFilter, unregKey, game::spec::FriendlyCodeList::DefaultRegistered));

    a.check("81", !testee.isAcceptedFriendlyCode("mf1", planetFilter, regKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("82", !testee.isAcceptedFriendlyCode("mf1", planetFilter, regKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("83", !testee.isAcceptedFriendlyCode("mf1", planetFilter, regKey, game::spec::FriendlyCodeList::DefaultRegistered));
    a.check("84", !testee.isAcceptedFriendlyCode("mf1", planetFilter, unregKey, game::spec::FriendlyCodeList::DefaultAvailable));
    a.check("85", !testee.isAcceptedFriendlyCode("mf1", planetFilter, unregKey, game::spec::FriendlyCodeList::DefaultUnavailable));
    a.check("86", !testee.isAcceptedFriendlyCode("mf1", planetFilter, unregKey, game::spec::FriendlyCodeList::DefaultRegistered));
}
