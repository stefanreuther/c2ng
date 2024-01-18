/**
  *  \file test/util/resourcefilereadertest.cpp
  *  \brief Test for util::ResourceFileReader
  */

#include "util/resourcefilereader.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include <memory>

/** Test normal reading. */
AFL_TEST("util.ResourceFileReader:basics", a)
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
    a.checkEqual("01. getNumMembers", testee.getNumMembers(), 2U);
    a.checkEqual("02. getMemberIdByIndex", testee.getMemberIdByIndex(0), 100U);
    a.checkEqual("03. getMemberIdByIndex", testee.getMemberIdByIndex(1), 101U);
    a.checkEqual("04. getMemberIdByIndex", testee.getMemberIdByIndex(2), 0U);          // out-of-range access
    a.checkEqual("05. findPrimaryIdByIndex", testee.findPrimaryIdByIndex(0), 100U);
    a.checkEqual("06. findPrimaryIdByIndex", testee.findPrimaryIdByIndex(1), 101U);

    // Read a file
    {
        afl::base::Ptr<afl::io::Stream> s = testee.openMember(101);
        a.checkNonNull("11. openMember", s.get());
        uint8_t result[20];
        size_t got = s->read(result);
        a.checkEqual("12. read", got, 11U);
        a.checkEqualContent<uint8_t>("13. content", afl::base::ConstBytes_t(result).trim(11), afl::string::toBytes("more text\015\012"));
    }

    // Read a file by index
    {
        afl::base::Ptr<afl::io::Stream> s = testee.openMemberByIndex(0);
        a.checkNonNull("21. openMemberByIndex", s.get());
        uint8_t result[20];
        size_t got = s->read(result);
        a.checkEqual("22. read", got, 14U);
        a.checkEqualContent<uint8_t>("23. content", afl::base::ConstBytes_t(result).trim(14), afl::string::toBytes("hello, world\015\012"));
    }

    // Nonexistant member
    {
        a.checkNull("31. openMember", testee.openMember(102).get());
    }
    {
        a.checkNull("32. openMemberByIndex", testee.openMemberByIndex(2).get());
    }
}

/** Test hardlink alias resolution. */
AFL_TEST("util.ResourceFileReader:findPrimaryIdByIndex", a)
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
    a.checkEqual("01. getNumMembers", testee.getNumMembers(), 3U);
    a.checkEqual("02. getMemberIdByIndex", testee.getMemberIdByIndex(0), 100U);
    a.checkEqual("03. getMemberIdByIndex", testee.getMemberIdByIndex(1), 101U);
    a.checkEqual("04. getMemberIdByIndex", testee.getMemberIdByIndex(2), 200U);
    a.checkEqual("05. getMemberIdByIndex", testee.getMemberIdByIndex(3), 0U);          // out-of-range access
    a.checkEqual("06. findPrimaryIdByIndex", testee.findPrimaryIdByIndex(0), 100U);
    a.checkEqual("07. findPrimaryIdByIndex", testee.findPrimaryIdByIndex(1), 101U);
    a.checkEqual("08. findPrimaryIdByIndex", testee.findPrimaryIdByIndex(2), 100U);
    a.checkEqual("09. findPrimaryIdByIndex", testee.findPrimaryIdByIndex(3), 0U);
}

/*
 *  Test errors.
 */

// Too short
AFL_TEST("util.ResourceFileReader:error:too-short", a)
{
    afl::string::NullTranslator tx;
    std::auto_ptr<util::ResourceFileReader> rdr;
    static const uint8_t FILE[] = {
        0x52, 0x5a, 0x21
    };
    AFL_CHECK_THROWS(a, rdr.reset(new util::ResourceFileReader(*new afl::io::ConstMemoryStream(FILE), tx)), afl::except::FileProblemException);
}

// Index truncated
AFL_TEST("util.ResourceFileReader:error:truncated-index", a)
{
    afl::string::NullTranslator tx;
    std::auto_ptr<util::ResourceFileReader> rdr;
    static const uint8_t FILE[] = {
        0x52, 0x5a, 0x21, 0x00, 0x00, 0x00, 0x03, 0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x77,
        0x6f, 0x72, 0x6c, 0x64, 0x0d, 0x0a, 0x6d, 0x6f, 0x72, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x0d,
        0x0a, 0x64, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x65, 0x00, 0x16, 0x00, 0x00,
    };
    AFL_CHECK_THROWS(a, rdr.reset(new util::ResourceFileReader(*new afl::io::ConstMemoryStream(FILE), tx)), afl::except::FileProblemException);
}

// Bad magic
AFL_TEST("util.ResourceFileReader:error:bad-magic", a)
{
    afl::string::NullTranslator tx;
    std::auto_ptr<util::ResourceFileReader> rdr;
    static const uint8_t FILE[] = {
        0x52, 0x5c, 0x21, 0x00, 0x00, 0x00, 0x02, 0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x77,
        0x6f, 0x72, 0x6c, 0x64, 0x0d, 0x0a, 0x6d, 0x6f, 0x72, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x0d,
        0x0a, 0x64, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x65, 0x00, 0x16, 0x00, 0x00,
        0x00, 0x0b, 0x00, 0x00, 0x00
    };
    AFL_CHECK_THROWS(a, rdr.reset(new util::ResourceFileReader(*new afl::io::ConstMemoryStream(FILE), tx)), afl::except::FileProblemException);
}


/** Test parallel read access. */
AFL_TEST("util.ResourceFileReader:parallel-read", a)
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
    a.checkNonNull("01. openMember", f1.get());
    a.checkNonNull("02. openMember", f2.get());

    uint8_t result[1];
    a.checkEqual("11", f1->read(result), 1U);
    a.checkEqual("12", result[0], 'm');

    a.checkEqual("21", f2->read(result), 1U);
    a.checkEqual("22", result[0], 'h');

    a.checkEqual("31", f1->read(result), 1U);
    a.checkEqual("32", result[0], 'o');

    a.checkEqual("41", f2->read(result), 1U);
    a.checkEqual("42", result[0], 'e');
}
