/**
  *  \file u/t_util_resourcefilereader.cpp
  *  \brief Test for util::ResourceFileReader
  */

#include <memory>
#include "util/resourcefilereader.hpp"

#include "t_util.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test normal reading. */
void
TestUtilResourceFileReader::testNormal()
{
    /* Test case generated using PCC1 rc2.exe
       100 .text
       hello, world
       .endtext

       101 .text
       more text
       .endtext */
    static const uint8_t FILE[] = {
        0x52, 0x5a, 0x21, 0x00, 0x00, 0x00, 0x02, 0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x77,
        0x6f, 0x72, 0x6c, 0x64, 0x0d, 0x0a, 0x6d, 0x6f, 0x72, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x0d,
        0x0a, 0x64, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x65, 0x00, 0x16, 0x00, 0x00,
        0x00, 0x0b, 0x00, 0x00, 0x00
    };
    afl::string::NullTranslator tx;
    util::ResourceFileReader testee(*new afl::io::ConstMemoryStream(FILE), tx);

    // Introspection
    TS_ASSERT_EQUALS(testee.getNumMembers(), 2U);
    TS_ASSERT_EQUALS(testee.getMemberIdByIndex(0), 100U);
    TS_ASSERT_EQUALS(testee.getMemberIdByIndex(1), 101U);
    TS_ASSERT_EQUALS(testee.getMemberIdByIndex(2), 0U);          // out-of-range access
    TS_ASSERT_EQUALS(testee.findPrimaryIdByIndex(0), 100U);
    TS_ASSERT_EQUALS(testee.findPrimaryIdByIndex(1), 101U);

    // Read a file
    {
        afl::base::Ptr<afl::io::Stream> s = testee.openMember(101);
        TS_ASSERT(s.get() != 0);
        uint8_t result[20];
        size_t got = s->read(result);
        TS_ASSERT_EQUALS(got, 11U);
        TS_ASSERT_SAME_DATA(result, "more text\015\012", 11);
    }

    // Read a file by index
    {
        afl::base::Ptr<afl::io::Stream> s = testee.openMemberByIndex(0);
        TS_ASSERT(s.get() != 0);
        uint8_t result[20];
        size_t got = s->read(result);
        TS_ASSERT_EQUALS(got, 14U);
        TS_ASSERT_SAME_DATA(result, "hello, world\015\012", 14);
    }

    // Nonexistant member
    {
        TS_ASSERT(testee.openMember(102).get() == 0);
    }
    {
        TS_ASSERT(testee.openMemberByIndex(2).get() == 0);
    }
}

/** Test hardlink alias resolution. */
void
TestUtilResourceFileReader::testAlias()
{
    /* Test case generated using PCC1 rc2.exe.
       Same as above, plus '200 eq 100' (hardlink) */
    static const uint8_t FILE[] = {
        0x52, 0x5a, 0x21, 0x00, 0x00, 0x00, 0x03, 0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x77,
        0x6f, 0x72, 0x6c, 0x64, 0x0d, 0x0a, 0x6d, 0x6f, 0x72, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x0d,
        0x0a, 0x64, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x65, 0x00, 0x16, 0x00, 0x00,
        0x00, 0x0b, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00
    };
    afl::string::NullTranslator tx;
    util::ResourceFileReader testee(*new afl::io::ConstMemoryStream(FILE), tx);

    // Introspection
    TS_ASSERT_EQUALS(testee.getNumMembers(), 3U);
    TS_ASSERT_EQUALS(testee.getMemberIdByIndex(0), 100U);
    TS_ASSERT_EQUALS(testee.getMemberIdByIndex(1), 101U);
    TS_ASSERT_EQUALS(testee.getMemberIdByIndex(2), 200U);
    TS_ASSERT_EQUALS(testee.getMemberIdByIndex(3), 0U);          // out-of-range access
    TS_ASSERT_EQUALS(testee.findPrimaryIdByIndex(0), 100U);
    TS_ASSERT_EQUALS(testee.findPrimaryIdByIndex(1), 101U);
    TS_ASSERT_EQUALS(testee.findPrimaryIdByIndex(2), 100U);
    TS_ASSERT_EQUALS(testee.findPrimaryIdByIndex(3), 0U);
}

/** Test errors. */
void
TestUtilResourceFileReader::testError()
{
    afl::string::NullTranslator tx;
    std::auto_ptr<util::ResourceFileReader> rdr;

    // Too short
    {
        static const uint8_t FILE[] = {
            0x52, 0x5a, 0x21
        };
        TS_ASSERT_THROWS(rdr.reset(new util::ResourceFileReader(*new afl::io::ConstMemoryStream(FILE), tx)), afl::except::FileProblemException);
    }

    // Index truncated
    {
        static const uint8_t FILE[] = {
            0x52, 0x5a, 0x21, 0x00, 0x00, 0x00, 0x03, 0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x77,
            0x6f, 0x72, 0x6c, 0x64, 0x0d, 0x0a, 0x6d, 0x6f, 0x72, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x0d,
            0x0a, 0x64, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x65, 0x00, 0x16, 0x00, 0x00,
        };
        TS_ASSERT_THROWS(rdr.reset(new util::ResourceFileReader(*new afl::io::ConstMemoryStream(FILE), tx)), afl::except::FileProblemException);
    }

    // Bad magic
    {
        static const uint8_t FILE[] = {
            0x52, 0x5c, 0x21, 0x00, 0x00, 0x00, 0x02, 0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x77,
            0x6f, 0x72, 0x6c, 0x64, 0x0d, 0x0a, 0x6d, 0x6f, 0x72, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x0d,
            0x0a, 0x64, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x65, 0x00, 0x16, 0x00, 0x00,
            0x00, 0x0b, 0x00, 0x00, 0x00
        };
        TS_ASSERT_THROWS(rdr.reset(new util::ResourceFileReader(*new afl::io::ConstMemoryStream(FILE), tx)), afl::except::FileProblemException);
    }
}

/** Test parallel read access. */
void
TestUtilResourceFileReader::testParallelRead()
{
    /* Test case from testNormal() */
    static const uint8_t FILE[] = {
        0x52, 0x5a, 0x21, 0x00, 0x00, 0x00, 0x02, 0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x77,
        0x6f, 0x72, 0x6c, 0x64, 0x0d, 0x0a, 0x6d, 0x6f, 0x72, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x0d,
        0x0a, 0x64, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x65, 0x00, 0x16, 0x00, 0x00,
        0x00, 0x0b, 0x00, 0x00, 0x00
    };
    afl::string::NullTranslator tx;
    util::ResourceFileReader testee(*new afl::io::ConstMemoryStream(FILE), tx);

    // Open two streams
    afl::base::Ptr<afl::io::Stream> f1 = testee.openMember(101);     // reads 'more text'
    afl::base::Ptr<afl::io::Stream> f2 = testee.openMember(100);     // reads 'hello, world'
    TS_ASSERT(f1.get() != 0);
    TS_ASSERT(f2.get() != 0);

    uint8_t result[1];
    TS_ASSERT_EQUALS(f1->read(result), 1U);
    TS_ASSERT_EQUALS(result[0], 'm');

    TS_ASSERT_EQUALS(f2->read(result), 1U);
    TS_ASSERT_EQUALS(result[0], 'h');

    TS_ASSERT_EQUALS(f1->read(result), 1U);
    TS_ASSERT_EQUALS(result[0], 'o');

    TS_ASSERT_EQUALS(f2->read(result), 1U);
    TS_ASSERT_EQUALS(result[0], 'e');
}

