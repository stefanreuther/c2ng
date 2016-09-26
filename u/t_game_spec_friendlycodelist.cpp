/**
  *  \file u/t_game_spec_friendlycodelist.cpp
  *  \brief Test for game::spec::FriendlyCodeList
  */

#include "game/spec/friendlycodelist.hpp"

#include "t_game_spec.hpp"
#include "afl/io/constmemorystream.hpp"

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
    TS_ASSERT(!testee.isNumeric("--1", host));
    TS_ASSERT(!testee.isNumeric("1", host));
    TS_ASSERT(!testee.isNumeric(" 1", host));
    TS_ASSERT(!testee.isNumeric("1 ", host));
    TS_ASSERT(!testee.isNumeric(" 1 ", host));

    host.set(host.PHost, MKVERSION(4,0,0));
    TS_ASSERT(testee.isNumeric("-11", host));
    TS_ASSERT_EQUALS(testee.getNumericValue("-11", host), -11);
    TS_ASSERT(!testee.isNumeric("--1", host));
    TS_ASSERT(!testee.isNumeric("1", host));
    TS_ASSERT(!testee.isNumeric(" 1", host));
    TS_ASSERT(!testee.isNumeric("1 ", host));
    TS_ASSERT(!testee.isNumeric(" 1 ", host));

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

    host.set(host.PHost, MKVERSION(3,4,9));
    TS_ASSERT(!testee.isNumeric(" 1 ", host));

    host.set(host.PHost, MKVERSION(3,4,11));
    TS_ASSERT(testee.isNumeric(" 1 ", host));
}

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
    TS_ASSERT(testee.isAllowedRandomCode("elo", host));   // allowed, extra-fc is a PHost thing and thus case-sensitive
    TS_ASSERT(testee.isAllowedRandomCode("Zot", host));   // allowed, extra-fc is a PHost thing and thus case-sensitive
    TS_ASSERT(testee.isAllowedRandomCode("zoT", host));
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
}
