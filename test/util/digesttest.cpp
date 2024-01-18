/**
  *  \file test/util/digesttest.cpp
  *  \brief Test for util::Digest
  */

#include "util/digest.hpp"
#include "afl/test/testrunner.hpp"

/** Test Digest::add().
    Since these are too hard to compute manually, I generated the test digests using PHost's implementation. */
AFL_TEST("util.Digest:basics", a)
{
    using afl::base::ConstBytes_t;

    // ex GameChecksumTestSuite::testDigest
    util::Digest testee;

    // trivial cases
    a.checkEqual("01", testee.add(afl::base::Nothing, 0), 0U);
    a.checkEqual("02", testee.add(afl::base::Nothing, 42), 42U);

    // less trivial cases. For simplicity,
    static const uint8_t someTest[]     = {'S','O','M','E','T','E','S','T'};
    static const uint8_t someUmlauts[]  = {0xE4,0xF6,0xFC,0};
    static const uint8_t someUmlauts2[] = {0,0xE4,0xF6,0xFC};

    a.checkEqual("11", testee.add(ConstBytes_t(someTest),                   0), 0x5934F883U);
    a.checkEqual("12", testee.add(ConstBytes_t(someUmlauts).subrange(0, 3), 0), 0x2A39D50FU);
    a.checkEqual("13", testee.add(ConstBytes_t(someUmlauts),                0), 0x7AE64E40U);  // trailing null actualy modifies digest
    a.checkEqual("14", testee.add(ConstBytes_t(someUmlauts2),               0), 0x0B47A972U);  // leading null as well

    // distributive law (inner call is start of data)
    a.checkEqual("21", testee.add(ConstBytes_t(someTest).subrange(8), testee.add(ConstBytes_t(someTest).subrange(0, 8), 0)),0x5934F883U);
    a.checkEqual("22", testee.add(ConstBytes_t(someTest).subrange(7), testee.add(ConstBytes_t(someTest).subrange(0, 7), 0)),0x5934F883U);
    a.checkEqual("23", testee.add(ConstBytes_t(someTest).subrange(6), testee.add(ConstBytes_t(someTest).subrange(0, 6), 0)),0x5934F883U);
    a.checkEqual("24", testee.add(ConstBytes_t(someTest).subrange(5), testee.add(ConstBytes_t(someTest).subrange(0, 5), 0)),0x5934F883U);
    a.checkEqual("25", testee.add(ConstBytes_t(someTest).subrange(4), testee.add(ConstBytes_t(someTest).subrange(0, 4), 0)),0x5934F883U);
    a.checkEqual("26", testee.add(ConstBytes_t(someTest).subrange(3), testee.add(ConstBytes_t(someTest).subrange(0, 3), 0)),0x5934F883U);
    a.checkEqual("27", testee.add(ConstBytes_t(someTest).subrange(2), testee.add(ConstBytes_t(someTest).subrange(0, 2), 0)),0x5934F883U);
    a.checkEqual("28", testee.add(ConstBytes_t(someTest).subrange(1), testee.add(ConstBytes_t(someTest).subrange(0, 1), 0)),0x5934F883U);
    a.checkEqual("29", testee.add(ConstBytes_t(someTest).subrange(0), testee.add(ConstBytes_t(someTest).subrange(0, 0), 0)),0x5934F883U);
}

/** Test Digest::getDefaultInstance(). */
AFL_TEST("util.Digest:getDefaultInstance", a)
{
    static const uint8_t someTest[]     = {'S','O','M','E','T','E','S','T'};

    a.checkEqual("01. created", util::Digest().add(someTest, 0), 0x5934F883U);
    a.checkEqual("02. default", util::Digest::getDefaultInstance().add(someTest, 0), 0x5934F883U);
}

/** Test that Digest fulfils the dynamic type afl::checksums::Checksum. */
AFL_TEST("util.Digest:dynamic", a)
{
    static const uint8_t someTest[]     = {'S','O','M','E','T','E','S','T'};

    afl::checksums::Checksum& cs = util::Digest::getDefaultInstance();
    a.checkEqual("01. bits", cs.bits(), 32U);
    a.checkEqual("02. add", cs.add(someTest, 0), 0x5934F883U);
}
