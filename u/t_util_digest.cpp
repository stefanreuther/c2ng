/**
  *  \file u/t_util_digest.cpp
  *  \brief Test for util::Digest
  */

#include "util/digest.hpp"

#include "t_util.hpp"

/** Test Digest::add().
    Since these are too hard to compute manually, I generated the test digests using PHost's implementation. */
void
TestUtilDigest::testIt()
{
    using afl::base::ConstBytes_t;

    // ex GameChecksumTestSuite::testDigest
    util::Digest testee;

    // trivial cases
    TS_ASSERT_EQUALS(testee.add(afl::base::Nothing, 0), 0U);
    TS_ASSERT_EQUALS(testee.add(afl::base::Nothing, 42), 42U);

    // less trivial cases. For simplicity,
    static const uint8_t someTest[]     = {'S','O','M','E','T','E','S','T'};
    static const uint8_t someUmlauts[]  = {0xE4,0xF6,0xFC,0};
    static const uint8_t someUmlauts2[] = {0,0xE4,0xF6,0xFC};

    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someTest),                   0), 0x5934F883U);
    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someUmlauts).subrange(0, 3), 0), 0x2A39D50FU);
    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someUmlauts),                0), 0x7AE64E40U);  // trailing null actualy modifies digest
    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someUmlauts2),               0), 0x0B47A972U);  // leading null as well

    // distributive law (inner call is start of data)
    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someTest).subrange(8), testee.add(ConstBytes_t(someTest).subrange(0, 8), 0)),0x5934F883U);
    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someTest).subrange(7), testee.add(ConstBytes_t(someTest).subrange(0, 7), 0)),0x5934F883U);
    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someTest).subrange(6), testee.add(ConstBytes_t(someTest).subrange(0, 6), 0)),0x5934F883U);
    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someTest).subrange(5), testee.add(ConstBytes_t(someTest).subrange(0, 5), 0)),0x5934F883U);
    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someTest).subrange(4), testee.add(ConstBytes_t(someTest).subrange(0, 4), 0)),0x5934F883U);
    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someTest).subrange(3), testee.add(ConstBytes_t(someTest).subrange(0, 3), 0)),0x5934F883U);
    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someTest).subrange(2), testee.add(ConstBytes_t(someTest).subrange(0, 2), 0)),0x5934F883U);
    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someTest).subrange(1), testee.add(ConstBytes_t(someTest).subrange(0, 1), 0)),0x5934F883U);
    TS_ASSERT_EQUALS(testee.add(ConstBytes_t(someTest).subrange(0), testee.add(ConstBytes_t(someTest).subrange(0, 0), 0)),0x5934F883U);
}

/** Test Digest::getDefaultInstance(). */
void
TestUtilDigest::testStaticInstance()
{
    static const uint8_t someTest[]     = {'S','O','M','E','T','E','S','T'};

    TS_ASSERT_EQUALS(util::Digest().add(someTest, 0), 0x5934F883U);
    TS_ASSERT_EQUALS(util::Digest::getDefaultInstance().add(someTest, 0), 0x5934F883U);
}

/** Test that Digest fulfils the dynamic type afl::checksums::Checksum. */
void
TestUtilDigest::testDynamicType()
{
    static const uint8_t someTest[]     = {'S','O','M','E','T','E','S','T'};

    afl::checksums::Checksum& cs = util::Digest::getDefaultInstance();
    TS_ASSERT_EQUALS(cs.bits(), 32U);
    TS_ASSERT_EQUALS(cs.add(someTest, 0), 0x5934F883U);
}

