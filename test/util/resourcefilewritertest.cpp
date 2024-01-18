/**
  *  \file test/util/resourcefilewritertest.cpp
  *  \brief Test for util::ResourceFileWriter
  */

#include "util/resourcefilewriter.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

using afl::base::Ref;
using afl::io::InternalStream;
using afl::io::Stream;
using afl::string::NullTranslator;

/** Test creation of empty file. */
AFL_TEST("util.ResourceFileWriter:empty", a)
{
    Ref<InternalStream> file(*new InternalStream());
    NullTranslator tx;
    util::ResourceFileWriter testee(file, tx);
    testee.finishFile();

    // Expected file size is 8 bytes (just a header)
    a.checkEqual("01. getSize", file->getSize(), 8U);
    a.checkEqual("02. content", *file->getContent().at(0), 'R');
    a.checkEqual("03. content", *file->getContent().at(1), 'Z');
    a.checkEqual("04. content", *file->getContent().at(2), 8);
    a.checkEqual("05. content", *file->getContent().at(3), 0);
    a.checkEqual("06. content", *file->getContent().at(4), 0);
    a.checkEqual("07. content", *file->getContent().at(5), 0);
    a.checkEqual("08. content", *file->getContent().at(6), 0);
    a.checkEqual("09. content", *file->getContent().at(7), 0);
}

/** Test normal operation. */
AFL_TEST("util.ResourceFileWriter:normal", a)
{
    Ref<InternalStream> file(*new InternalStream());
    NullTranslator tx;
    util::ResourceFileWriter testee(file, tx);

    // Create some entries
    testee.createMember(100)->fullWrite(afl::string::toBytes("a"));
    testee.createMember(101)->fullWrite(afl::string::toBytes("bc"));
    testee.finishFile();

    // Expected file size is 8 bytes (header) + 3 bytes (payload) + 20 bytes (directory)
    a.checkEqual("01. getSize", file->getSize(), 31U);

    static const uint8_t DATA[] = {
        'R','Z',11,0,0,0,2,0,
        'a',
        'b','c',
        100,0,8,0,0,0,1,0,0,0,
        101,0,9,0,0,0,2,0,0,0,
    };
    a.checkEqualContent<uint8_t>("11. content", file->getContent(), DATA);
}

/** Test details of write operation. */
AFL_TEST("util.ResourceFileWriter:write", a)
{
    Ref<InternalStream> file(*new InternalStream());
    NullTranslator tx;
    util::ResourceFileWriter testee(file, tx);

    // Output stream is seekable and readable
    // (size is not retrievable - restriction of LimitedStream)
    Ref<Stream> s = testee.createMember(77);
    a.checkEqual("01. getPos", s->getPos(), 0U);

    s->fullWrite(afl::string::toBytes("xyz"));
    a.checkEqual("11. getPos", s->getPos(), 3U);

    s->setPos(0);
    a.checkEqual("21. getPos", s->getPos(), 0U);

    uint8_t data[1];
    a.checkEqual("31. read", s->read(data), 1U);
    a.checkEqual("32. read", data[0], 'x');

    // Member is retrievable
    a.check("41. hasMember", testee.hasMember(77));

    // Finish and verify
    testee.finishFile();
    a.checkEqual("51. getSize", file->getSize(), 21U);

    static const uint8_t DATA[] = {
        'R','Z',11,0,0,0,1,0,
        'x','y','z',
        77,0,8,0,0,0,3,0,0,0,
    };
    a.checkEqualContent<uint8_t>("61. getContent", file->getContent(), DATA);
}

/** Test createHardlink(). */
AFL_TEST("util.ResourceFileWriter:createHardlink", a)
{
    Ref<InternalStream> file(*new InternalStream());
    NullTranslator tx;
    util::ResourceFileWriter testee(file, tx);

    // Cannot create a hardlink in empty status
    a.check("01. failure", !testee.createHardlink(100, 101));

    // Create a member
    testee.createMember(100)->fullWrite(afl::string::toBytes("a"));

    // Can now create a hardlink
    a.check("11. success", testee.createHardlink(100, 102));

    // Verify
    testee.finishFile();
    a.checkEqual("21. getSize", file->getSize(), 29U);

    static const uint8_t DATA[] = {
        'R','Z',9,0,0,0,2,0,
        'a',
        100,0,8,0,0,0,1,0,0,0,
        102,0,8,0,0,0,1,0,0,0,
    };
    a.checkEqualContent<uint8_t>("31. getContent", file->getContent(), DATA);
}

/** Test directory overflow. */
AFL_TEST("util.ResourceFileWriter:directory-overflow", a)
{
    Ref<InternalStream> file(*new InternalStream());
    NullTranslator tx;
    util::ResourceFileWriter testee(file, tx);

    // Limit is far below 20000. Must throw an exception eventually.
    bool ok = false;
    try {
        for (uint16_t i = 1; i < 20000; ++i) {
            testee.createMember(i)->fullWrite(afl::string::toBytes("a"));
        }
    }
    catch (afl::except::FileProblemException& e) {
        ok = true;
    }
    a.check("01", ok);
}
