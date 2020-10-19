/**
  *  \file u/t_game_spec_friendlycodelist.cpp
  *  \brief Test for game::spec::FriendlyCodeList
  */

#include "game/spec/friendlycodelist.hpp"

#include "t_game_spec.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/sys/log.hpp"
#include "game/map/planet.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test isNumeric(). */
void
TestGameSpecFriendlyCodeList::testNumeric()
{
    // ex GameFcodeTestSuite::testFCode
    game::spec::FriendlyCodeList testee;

    game::HostVersion host;
    TS_ASSERT( testee.isNumeric("000", host));
    TS_ASSERT(!testee.isNumeric("00x", host));
    TS_ASSERT(!testee.isNumeric("x00", host));
    TS_ASSERT(!testee.isNumeric("+00", host));
    TS_ASSERT( testee.isNumeric("999", host));

    host.set(host.Host, MKVERSION(3,22,40));
    TS_ASSERT(!testee.isNumeric("-11", host));
    TS_ASSERT_EQUALS(testee.getNumericValue("-11", host), 1000);
    TS_ASSERT(!testee.isNumeric("--1", host));
    TS_ASSERT(!testee.isNumeric("1", host));
    TS_ASSERT(!testee.isNumeric(" 1", host));
    TS_ASSERT(!testee.isNumeric("1 ", host));
    TS_ASSERT(!testee.isNumeric(" 1 ", host));
    TS_ASSERT(!testee.isNumeric("-  ", host));
    TS_ASSERT(!testee.isNumeric("  -", host));
    TS_ASSERT(!testee.isNumeric("   ", host));

    host.set(host.PHost, MKVERSION(4,0,0));
    TS_ASSERT(testee.isNumeric("-11", host));
    TS_ASSERT_EQUALS(testee.getNumericValue("-11", host), -11);
    TS_ASSERT(!testee.isNumeric("--1", host));
    TS_ASSERT(!testee.isNumeric("1", host));
    TS_ASSERT(!testee.isNumeric(" 1", host));
    TS_ASSERT(!testee.isNumeric("1 ", host));
    TS_ASSERT(!testee.isNumeric(" 1 ", host));
    TS_ASSERT(!testee.isNumeric("-  ", host));
    TS_ASSERT(!testee.isNumeric("  -", host));
    TS_ASSERT(!testee.isNumeric("   ", host));

    host.set(host.PHost, MKVERSION(4,0,8));
    TS_ASSERT(testee.isNumeric("-11", host));
    TS_ASSERT_EQUALS(testee.getNumericValue("-11", host), -11);
    TS_ASSERT(!testee.isNumeric("--1", host));
    TS_ASSERT(testee.isNumeric("1", host));
    TS_ASSERT_EQUALS(testee.getNumericValue("1", host), 1);
    TS_ASSERT(testee.isNumeric(" 1", host));
    TS_ASSERT_EQUALS(testee.getNumericValue(" 1", host), 1);
    TS_ASSERT(testee.isNumeric("1 ", host));
    TS_ASSERT_EQUALS(testee.getNumericValue("1 ", host), 1);
    TS_ASSERT(testee.isNumeric(" 1 ", host));
    TS_ASSERT_EQUALS(testee.getNumericValue(" 1 ", host), 1);
    TS_ASSERT(!testee.isNumeric("-  ", host));
    TS_ASSERT(!testee.isNumeric("  -", host));
    TS_ASSERT(!testee.isNumeric("   ", host));

    host.set(host.PHost, MKVERSION(3,4,9));
    TS_ASSERT(!testee.isNumeric(" 1 ", host));

    host.set(host.PHost, MKVERSION(3,4,11));
    TS_ASSERT(testee.isNumeric(" 1 ", host));

    TS_ASSERT(testee.isNumeric("-11", testee.Pessimistic));
    TS_ASSERT_EQUALS(testee.getNumericValue("-11", testee.Pessimistic), -11);
    TS_ASSERT(!testee.isNumeric("--1", testee.Pessimistic));
    TS_ASSERT( testee.isNumeric("1",   testee.Pessimistic));
    TS_ASSERT( testee.isNumeric(" 1",  testee.Pessimistic));
    TS_ASSERT( testee.isNumeric("1 ",  testee.Pessimistic));
    TS_ASSERT( testee.isNumeric(" 1 ", testee.Pessimistic));
    TS_ASSERT(!testee.isNumeric("-  ", testee.Pessimistic));
    TS_ASSERT(!testee.isNumeric("  -", testee.Pessimistic));
    TS_ASSERT(!testee.isNumeric("   ", testee.Pessimistic));
}

/** Test isAllowedRandomCode(). */
void
TestGameSpecFriendlyCodeList::testRandom()
{
    // ex GameFcodeTestSuite::testRandom
    game::spec::FriendlyCodeList testee;

    afl::io::ConstMemoryStream in(afl::string::toBytes("E zot"));
    testee.loadExtraCodes(in);
    testee.addCode(game::spec::FriendlyCode("mkt", "sc,make torps"));

    game::HostVersion host;
    host.set(host.PHost, MKVERSION(4, 0, 0));

    // Now check:
    TS_ASSERT(testee.isAllowedRandomCode("abc", host));
    TS_ASSERT(testee.isAllowedRandomCode("01a", host));
    TS_ASSERT(testee.isAllowedRandomCode("a01", host));
    TS_ASSERT(testee.isAllowedRandomCode("0 1", host));   // allowed, but will not be generated
    TS_ASSERT(testee.isAllowedRandomCode("zxy", host));
    TS_ASSERT(testee.isAllowedRandomCode("0-1", host));   // allowed, but will not be generated
    TS_ASSERT(!testee.isAllowedRandomCode("elo", host));  // no longer allowed, extra-fc now case-insensitive
    TS_ASSERT(!testee.isAllowedRandomCode("Zot", host));  // no longer allowed, extra-fc now case-insensitive
    TS_ASSERT(!testee.isAllowedRandomCode("zoT", host));
    TS_ASSERT(testee.isAllowedRandomCode("zo ", host));

    TS_ASSERT(!testee.isAllowedRandomCode("mkt", host));  // fails: predefined code
    TS_ASSERT(!testee.isAllowedRandomCode("Mkt", host));  // fails: variant of predefined
    TS_ASSERT(!testee.isAllowedRandomCode("mKt", host));  // fails: variant of predefined
    TS_ASSERT(!testee.isAllowedRandomCode("mkT", host));  // fails: variant of predefined
    TS_ASSERT(!testee.isAllowedRandomCode("MKT", host));  // fails: variant of predefined

    TS_ASSERT(!testee.isAllowedRandomCode("aab", host));  // fails: duplicate character
    TS_ASSERT(!testee.isAllowedRandomCode("aba", host));  // fails: duplicate character
    TS_ASSERT(!testee.isAllowedRandomCode("baa", host));  // fails: duplicate character

    TS_ASSERT(!testee.isAllowedRandomCode("mf1", host));  // fails: universal minefield code
    TS_ASSERT(!testee.isAllowedRandomCode("mff", host));  // fails: universal minefield code
    TS_ASSERT(!testee.isAllowedRandomCode("MFx", host));  // fails: universal minefield code, case-insensitive in THost!
    TS_ASSERT(!testee.isAllowedRandomCode("Mfx", host));  // fails: universal minefield code, case-insensitive in THost!
    TS_ASSERT(!testee.isAllowedRandomCode("mFx", host));  // fails: universal minefield code, case-insensitive in THost!

    TS_ASSERT(!testee.isAllowedRandomCode("xyz", host));  // fails: starts with 'X' (bird men rule)
    TS_ASSERT(!testee.isAllowedRandomCode("Xyz", host));  // fails: starts with 'X' (bird men rule)

    TS_ASSERT(!testee.isAllowedRandomCode("000", host));  // fails: numeric
    TS_ASSERT(!testee.isAllowedRandomCode("012", host));  // fails: numeric
    TS_ASSERT(!testee.isAllowedRandomCode("-19", host));  // fails: numeric, and host allows it

    TS_ASSERT(!testee.isAllowedRandomCode("Elo", host));  // fails: prefix blocked by extra FC
    TS_ASSERT(!testee.isAllowedRandomCode("Eex", host));  // fails: prefix blocked by extra FC
    TS_ASSERT(!testee.isAllowedRandomCode("zot", host));  // fails: blocked by extra FC

    TS_ASSERT(!testee.isAllowedRandomCode("?xy", host));  // fails: '?' not allowed
    TS_ASSERT(!testee.isAllowedRandomCode("z?y", host));  // fails: '?' not allowed
    TS_ASSERT(!testee.isAllowedRandomCode("zx?", host));  // fails: '?' not allowed

    TS_ASSERT(!testee.isAllowedRandomCode("#xy", host));  // fails: '#' not allowed
    TS_ASSERT(!testee.isAllowedRandomCode("z#y", host));  // fails: '#' not allowed
    TS_ASSERT(!testee.isAllowedRandomCode("zx#", host));  // fails: '#' not allowed

    TS_ASSERT(!testee.isAllowedRandomCode("###", host));
    TS_ASSERT(!testee.isAllowedRandomCode("?""?""?", host));

    TS_ASSERT(!testee.isAllowedRandomCode("", host));     // fails: length mismatch
    TS_ASSERT(!testee.isAllowedRandomCode("a", host));    // fails: length mismatch
    TS_ASSERT(!testee.isAllowedRandomCode("ab", host));   // fails: length mismatch
    TS_ASSERT(!testee.isAllowedRandomCode("abcd", host)); // fails: length mismatch

    TS_ASSERT(testee.isAllowedRandomCode("-19", game::HostVersion(game::HostVersion::Host, MKVERSION(3,2,0))));
    TS_ASSERT(!testee.isAllowedRandomCode("-19", game::spec::FriendlyCodeList::Pessimistic));
}

/** Test container behaviour. */
void
TestGameSpecFriendlyCodeList::testContainer()
{
    game::spec::FriendlyCodeList testee;

    // Verify initial state
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT_EQUALS(testee.begin(), testee.end());
    TS_ASSERT(testee.at(0) == 0);

    // Add some elements
    testee.addCode(game::spec::FriendlyCode("pfc", "p,xxx"));
    testee.addCode(game::spec::FriendlyCode("bfc", "b,xxx"));
    testee.addCode(game::spec::FriendlyCode("sfc", "s,xxx"));
    testee.addCode(game::spec::FriendlyCode("ffc", "p+1,xxx"));

    // Verify properties
    TS_ASSERT_EQUALS(testee.size(), 4U);
    TS_ASSERT_EQUALS((*testee.begin())->getCode(), "pfc");

    TS_ASSERT(testee.at(0) != 0);
    TS_ASSERT(testee.at(3) != 0);
    TS_ASSERT(testee.at(4) == 0);
    TS_ASSERT_EQUALS(testee.at(0)->getCode(), "pfc");
    TS_ASSERT_EQUALS(testee.at(1)->getCode(), "bfc");
    TS_ASSERT_EQUALS(testee.at(2)->getCode(), "sfc");
    TS_ASSERT_EQUALS(testee.at(3)->getCode(), "ffc");

    size_t index;
    TS_ASSERT(testee.getIndexByName("sfc", index));
    TS_ASSERT_EQUALS(index, 2U);
    TS_ASSERT(!testee.getIndexByName("SFC", index));
    TS_ASSERT(!testee.getIndexByName("mkt", index));

    TS_ASSERT_EQUALS(testee.getCodeByName("sfc"), testee.begin() + 2);
    TS_ASSERT_EQUALS(testee.getCodeByName("mkt"), testee.end());

    // Sort
    testee.sort();
    TS_ASSERT_EQUALS(testee.size(), 4U);
    TS_ASSERT_EQUALS(testee.at(0)->getCode(), "bfc");
    TS_ASSERT_EQUALS(testee.at(1)->getCode(), "ffc");
    TS_ASSERT_EQUALS(testee.at(2)->getCode(), "pfc");
    TS_ASSERT_EQUALS(testee.at(3)->getCode(), "sfc");
    TS_ASSERT_EQUALS((*testee.begin())->getCode(), "bfc");

    TS_ASSERT(testee.getIndexByName("sfc", index));
    TS_ASSERT_EQUALS(index, 3U);

    // Create a sub-list
    game::map::Planet p(9);
    p.setOwner(1);
    p.setPlayability(p.ReadOnly);

    const game::config::HostConfiguration hostConfig;
    const game::spec::ShipList shipList;
    const game::UnitScoreDefinitionList scoreDefinitions;
    game::spec::FriendlyCodeList sublist(testee, p, scoreDefinitions, shipList, hostConfig);
    TS_ASSERT_EQUALS(sublist.size(), 2U);
    TS_ASSERT_EQUALS(sublist.at(0)->getCode(), "ffc");
    TS_ASSERT_EQUALS(sublist.at(1)->getCode(), "pfc");

    // Clear original list. Sublist remains.
    testee.clear();
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT_EQUALS(sublist.size(), 2U);
    TS_ASSERT_EQUALS(sublist.at(0)->getCode(), "ffc");
}

/** Test special friendly code detection. */
void
TestGameSpecFriendlyCodeList::testSpecial()
{
    game::spec::FriendlyCodeList testee;

    // Provide normal
    testee.addCode(game::spec::FriendlyCode("pfc", "p,xxx"));
    testee.addCode(game::spec::FriendlyCode("bfc", "b,xxx"));
    testee.addCode(game::spec::FriendlyCode("ufc", "u,xxx"));

    // Load extras
    afl::io::ConstMemoryStream ms(afl::string::toBytes("ab\n"
                                                       "z\n"
                                                       "pppp\n"
                                                       "e f"));
    testee.loadExtraCodes(ms);

    // Verify
    TS_ASSERT( testee.isSpecial("ab",   true));
    TS_ASSERT( testee.isSpecial("abc",  true));
    TS_ASSERT( testee.isSpecial("z",    true));
    TS_ASSERT(!testee.isSpecial("ZZ",   false));
    TS_ASSERT( testee.isSpecial("ZZ",   true));
    TS_ASSERT(!testee.isSpecial("ppp",  true));   // no truncation to 3 characters!
    TS_ASSERT( testee.isSpecial("pppp", true));
    TS_ASSERT( testee.isSpecial("e",    true));
    TS_ASSERT( testee.isSpecial("e11",  true));
    TS_ASSERT( testee.isSpecial("fff",  true));

    // Check special
    TS_ASSERT( testee.isSpecial("pfc", false));
    TS_ASSERT( testee.isSpecial("bfc", false));
    TS_ASSERT(!testee.isSpecial("ufc", false));
    TS_ASSERT(!testee.isSpecial("PFC", false));
    TS_ASSERT(!testee.isSpecial("BFC", false));
    TS_ASSERT(!testee.isSpecial("UFC", false));
    TS_ASSERT( testee.isSpecial("PFC", true));
    TS_ASSERT( testee.isSpecial("BFC", true));
    TS_ASSERT(!testee.isSpecial("UFC", true));

    // Clear
    testee.clear();
    TS_ASSERT(!testee.isSpecial("ab",  true));
    TS_ASSERT(!testee.isSpecial("abc", true));
    TS_ASSERT(!testee.isSpecial("z",   true));
}

/** Test generateRandomCode(). */
void
TestGameSpecFriendlyCodeList::testGenerateRandom()
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
    String_t a = testee.generateRandomCode(rng, host);
    String_t b = testee.generateRandomCode(rng, host);
    String_t c = testee.generateRandomCode(rng, host);

    TS_ASSERT_DIFFERS(a, b);
    TS_ASSERT_DIFFERS(a, c);
    TS_ASSERT_DIFFERS(b, c);
}

/** Test isUniversalMinefieldFCode(). */
void
TestGameSpecFriendlyCodeList::testUniversalMF()
{
    // Environment
    game::HostVersion phost;
    phost.set(phost.PHost, MKVERSION(4, 0, 0));

    game::HostVersion thost;
    thost.set(thost.Host, MKVERSION(3, 2, 0));

    // Testee
    game::spec::FriendlyCodeList testee;

    // Test
    TS_ASSERT(testee.isUniversalMinefieldFCode("mfx", false, phost));
    TS_ASSERT(testee.isUniversalMinefieldFCode("mfx", false, thost));
    TS_ASSERT(testee.isUniversalMinefieldFCode("mfx", false, testee.Pessimistic));
    TS_ASSERT(!testee.isUniversalMinefieldFCode("abc", false, thost));

    TS_ASSERT(!testee.isUniversalMinefieldFCode("MFX", false, phost));
    TS_ASSERT(testee.isUniversalMinefieldFCode("MFX", false, thost));
    TS_ASSERT(testee.isUniversalMinefieldFCode("MFX", false, testee.Pessimistic));
    TS_ASSERT(!testee.isUniversalMinefieldFCode("ABC", false, thost));

    TS_ASSERT(testee.isUniversalMinefieldFCode("MFX", true, phost));
    TS_ASSERT(testee.isUniversalMinefieldFCode("MFX", true, thost));
    TS_ASSERT(!testee.isUniversalMinefieldFCode("ABC", true, thost));

    TS_ASSERT(!testee.isUniversalMinefieldFCode("ABC", false, testee.Pessimistic));
}

/** Test generateRandomCode() infinite loop avoidance. */
void
TestGameSpecFriendlyCodeList::testGenerateRandomLoop()
{
    // Environment
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
    testee.loadExtraCodes(ms);

    // generateRandomCode() must still finish
    TS_ASSERT_EQUALS(testee.generateRandomCode(rng, host).size(), 3U);
}

/** Test generateRandomCode() infinite loop avoidance. */
void
TestGameSpecFriendlyCodeList::testGenerateRandomBlock()
{
    // Environment
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
    testee.loadExtraCodes(ms);

    TS_ASSERT( testee.isSpecial("NXY", true));
    TS_ASSERT(!testee.isSpecial("3XY", true));

    // generateRandomCode() must create a code starting with '3'
    String_t s = testee.generateRandomCode(rng, host);
    TS_ASSERT_EQUALS(s.size(), 3U);
    TS_ASSERT_EQUALS(s[0], '3');
}

/** Test load(). */
void
TestGameSpecFriendlyCodeList::testLoad()
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
    TS_ASSERT_EQUALS(testee.size(), 3U);
    TS_ASSERT_EQUALS(testee.at(0)->getCode(), "mkt");
    TS_ASSERT_EQUALS(testee.at(0)->getFlags(), FriendlyCode::FlagSet_t(FriendlyCode::ShipCode));
    TS_ASSERT_EQUALS(testee.at(1)->getCode(), "NUK");
    TS_ASSERT_EQUALS(testee.at(1)->getFlags(), FriendlyCode::FlagSet_t(FriendlyCode::PlanetCode));
    TS_ASSERT_EQUALS(testee.at(2)->getCode(), "???");
    TS_ASSERT_EQUALS(testee.at(2)->getFlags(), FriendlyCode::FlagSet_t(FriendlyCode::UnspecialCode));
}

/** Test sort order. */
void
TestGameSpecFriendlyCodeList::testSort()
{
    using game::spec::FriendlyCode;

    // Alphanumeric goes before non-alphanumeric, capital before lower-case.
    game::spec::FriendlyCodeList testee;
    testee.addCode(FriendlyCode("!bc", ",x"));
    testee.addCode(FriendlyCode("abc", ",x"));
    testee.addCode(FriendlyCode("0bc", ",x"));
    testee.addCode(FriendlyCode("Abc", ",x"));
    testee.addCode(FriendlyCode("ABC", ",x"));
    testee.addCode(FriendlyCode("?bc", ",x"));

    // Sort
    testee.sort();

    // Verify
    TS_ASSERT_EQUALS(testee.size(), 6U);
    TS_ASSERT_EQUALS(testee.at(0)->getCode(), "0bc");
    TS_ASSERT_EQUALS(testee.at(1)->getCode(), "ABC");
    TS_ASSERT_EQUALS(testee.at(2)->getCode(), "Abc");
    TS_ASSERT_EQUALS(testee.at(3)->getCode(), "abc");
    TS_ASSERT_EQUALS(testee.at(4)->getCode(), "!bc");
    TS_ASSERT_EQUALS(testee.at(5)->getCode(), "?bc");
}

/** Test syntax errors in load(). */
void
TestGameSpecFriendlyCodeList::testSyntaxErrors()
{
    class CountingLogger : public afl::sys::LogListener {
     public:
        CountingLogger()
            : m_numMessages(0)
            { }
        virtual void handleMessage(const Message& /*msg*/)
            { ++m_numMessages; }
        size_t getNumMessages() const
            { return m_numMessages; }
     private:
        size_t m_numMessages;
    };

    // Badly formatted line
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("foo\n"));
        CountingLogger log;
        game::spec::FriendlyCodeList list;
        afl::string::NullTranslator tx;
        list.load(ms, log, tx);
        TS_ASSERT_EQUALS(log.getNumMessages(), 1U);
        TS_ASSERT_EQUALS(list.size(), 0U);
    }
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("longcode,,foo\n"));
        CountingLogger log;
        game::spec::FriendlyCodeList list;
        afl::string::NullTranslator tx;
        list.load(ms, log, tx);
        TS_ASSERT_EQUALS(log.getNumMessages(), 1U);
        TS_ASSERT_EQUALS(list.size(), 1U);
        TS_ASSERT_EQUALS(list.at(0)->getCode(), "lon");
    }
}

/** Test Pessimistic. */
void
TestGameSpecFriendlyCodeList::testPessimistic()
{
    using game::HostVersion;
    game::spec::FriendlyCodeList testee;

    TS_ASSERT( testee.isAllowedRandomCode(" 12", HostVersion(HostVersion::Host,  MKVERSION(3, 0, 0))));
    TS_ASSERT(!testee.isAllowedRandomCode(" 12", HostVersion(HostVersion::PHost, MKVERSION(4, 1, 0))));
    TS_ASSERT(!testee.isAllowedRandomCode(" 12", testee.Pessimistic));

    TS_ASSERT( testee.isAllowedRandomCode("-12", HostVersion(HostVersion::Host,  MKVERSION(3, 0, 0))));
    TS_ASSERT(!testee.isAllowedRandomCode("-12", HostVersion(HostVersion::PHost, MKVERSION(4, 1, 0))));
    TS_ASSERT(!testee.isAllowedRandomCode("-12", testee.Pessimistic));

    TS_ASSERT(!testee.isAllowedRandomCode("Mff", HostVersion(HostVersion::Host,  MKVERSION(3, 0, 0))));
    TS_ASSERT(!testee.isAllowedRandomCode("Mff", HostVersion(HostVersion::PHost, MKVERSION(4, 1, 0)))); // not a special friendly code, but isAllowedRandomCode() always is pessimistic
    TS_ASSERT(!testee.isAllowedRandomCode("Mff", testee.Pessimistic));
}

/** Test pack(). */
void
TestGameSpecFriendlyCodeList::testPack()
{
    // Friendly code list
    game::spec::FriendlyCodeList testee;
    testee.addCode(game::spec::FriendlyCode("pfc", "p,whatever"));
    testee.addCode(game::spec::FriendlyCode("gs3", "s,give to %3"));
    testee.addCode(game::spec::FriendlyCode("gs4", "s,give to %4"));
    afl::io::ConstMemoryStream ms(afl::string::toBytes("ab"));
    testee.loadExtraCodes(ms);

    // Player list
    game::PlayerList pl;
    game::Player* p3 = pl.create(3);
    TS_ASSERT(p3);
    p3->setName(game::Player::ShortName, "Threes");
    p3->setName(game::Player::AdjectiveName, "threeish");

    // Pack
    game::spec::FriendlyCodeList::Infos_t info;
    testee.pack(info, pl);

    // Verify
    TS_ASSERT_EQUALS(info.size(), 3U);
    TS_ASSERT_EQUALS(info[0].code, "pfc");
    TS_ASSERT_EQUALS(info[0].description, "whatever");
    TS_ASSERT_EQUALS(info[1].code, "gs3");
    TS_ASSERT_EQUALS(info[1].description, "give to Threes");
    TS_ASSERT_EQUALS(info[2].code, "gs4");
    TS_ASSERT_EQUALS(info[2].description, "give to 4");

    // Original list has four elements
    TS_ASSERT_EQUALS(testee.size(), 4U);
    TS_ASSERT_EQUALS(testee.at(0)->getCode(), "pfc");
    TS_ASSERT_EQUALS(testee.at(1)->getCode(), "gs3");
    TS_ASSERT_EQUALS(testee.at(2)->getCode(), "gs4");
    TS_ASSERT_EQUALS(testee.at(3)->getCode(), "ab");
}

/** Test loadExtraCodes, load when duplicates are present. */
void
TestGameSpecFriendlyCodeList::testLoadExtraDup()
{
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
        afl::string::NullTranslator tx;
        testee.load(ms, log, tx);
    }

    // xtrafcode.txt
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("A mkt NUK j"));
        testee.loadExtraCodes(ms);
    }

    // Verify. Content must be mkt/ATT/HYP/A/NUK/j
    TS_ASSERT_EQUALS(testee.size(), 6U);
    TS_ASSERT_EQUALS(testee.at(0)->getCode(), "ATT");
    TS_ASSERT_EQUALS(testee.at(1)->getCode(), "HYP");
    TS_ASSERT_EQUALS(testee.at(2)->getCode(), "mkt");
    TS_ASSERT_EQUALS(testee.at(3)->getCode(), "A");
    TS_ASSERT_EQUALS(testee.at(4)->getCode(), "NUK");
    TS_ASSERT_EQUALS(testee.at(5)->getCode(), "j");

    // Verify specialness
    TS_ASSERT_EQUALS(testee.isSpecial("ATT", false), true);
    TS_ASSERT_EQUALS(testee.isSpecial("AXE", false), true);    // due to 'A'
}

