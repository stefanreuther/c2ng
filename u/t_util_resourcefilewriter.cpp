/**
  *  \file u/t_util_resourcefilewriter.cpp
  *  \brief Test for util::ResourceFileWriter
  */

#include "util/resourcefilewriter.hpp"

#include "t_util.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/except/fileproblemexception.hpp"

using afl::base::Ref;
using afl::io::InternalStream;
using afl::io::Stream;
using afl::string::NullTranslator;

/** Test creation of empty file. */
void
TestUtilResourceFileWriter::testEmpty()
{
    Ref<InternalStream> file(*new InternalStream());
    NullTranslator tx;
    util::ResourceFileWriter testee(file, tx);
    testee.finishFile();

    // Expected file size is 8 bytes (just a header)
    TS_ASSERT_EQUALS(file->getSize(), 8U);
    TS_ASSERT_EQUALS(*file->getContent().at(0), 'R');
    TS_ASSERT_EQUALS(*file->getContent().at(1), 'Z');
    TS_ASSERT_EQUALS(*file->getContent().at(2), 8);
    TS_ASSERT_EQUALS(*file->getContent().at(3), 0);
    TS_ASSERT_EQUALS(*file->getContent().at(4), 0);
    TS_ASSERT_EQUALS(*file->getContent().at(5), 0);
    TS_ASSERT_EQUALS(*file->getContent().at(6), 0);
    TS_ASSERT_EQUALS(*file->getContent().at(7), 0);
}

/** Test normal operation. */
void
TestUtilResourceFileWriter::testNormal()
{
    Ref<InternalStream> file(*new InternalStream());
    NullTranslator tx;
    util::ResourceFileWriter testee(file, tx);

    // Create some entries
    testee.createMember(100)->fullWrite(afl::string::toBytes("a"));
    testee.createMember(101)->fullWrite(afl::string::toBytes("bc"));
    testee.finishFile();

    // Expected file size is 8 bytes (header) + 3 bytes (payload) + 20 bytes (directory)
    TS_ASSERT_EQUALS(file->getSize(), 31U);

    static const uint8_t DATA[] = {
        'R','Z',11,0,0,0,2,0,
        'a',
        'b','c',
        100,0,8,0,0,0,1,0,0,0,
        101,0,9,0,0,0,2,0,0,0,
    };
    TS_ASSERT_SAME_DATA(file->getContent().at(0), DATA, 31U);
}

/** Test details of write operation. */
void
TestUtilResourceFileWriter::testWriteDetails()
{
    Ref<InternalStream> file(*new InternalStream());
    NullTranslator tx;
    util::ResourceFileWriter testee(file, tx);

    // Output stream is seekable and readable
    // (size is not retrievable - restriction of LimitedStream)
    Ref<Stream> s = testee.createMember(77);
    TS_ASSERT_EQUALS(s->getPos(), 0U);

    s->fullWrite(afl::string::toBytes("xyz"));
    TS_ASSERT_EQUALS(s->getPos(), 3U);

    s->setPos(0);
    TS_ASSERT_EQUALS(s->getPos(), 0U);

    uint8_t data[1];
    TS_ASSERT_EQUALS(s->read(data), 1U);
    TS_ASSERT_EQUALS(data[0], 'x');

    // Member is retrievable
    TS_ASSERT(testee.hasMember(77));

    // Finish and verify
    testee.finishFile();
    TS_ASSERT_EQUALS(file->getSize(), 21U);

    static const uint8_t DATA[] = {
        'R','Z',11,0,0,0,1,0,
        'x','y','z',
        77,0,8,0,0,0,3,0,0,0,
    };
    TS_ASSERT_SAME_DATA(file->getContent().at(0), DATA, 21U);
}

/** Test createHardlink(). */
void
TestUtilResourceFileWriter::testHardlink()
{
    Ref<InternalStream> file(*new InternalStream());
    NullTranslator tx;
    util::ResourceFileWriter testee(file, tx);

    // Cannot create a hardlink in empty status
    TS_ASSERT(!testee.createHardlink(100, 101));

    // Create a member
    testee.createMember(100)->fullWrite(afl::string::toBytes("a"));

    // Can now create a hardlink
    TS_ASSERT(testee.createHardlink(100, 102));

    // Verify
    testee.finishFile();
    TS_ASSERT_EQUALS(file->getSize(), 29U);

    static const uint8_t DATA[] = {
        'R','Z',9,0,0,0,2,0,
        'a',
        100,0,8,0,0,0,1,0,0,0,
        102,0,8,0,0,0,1,0,0,0,
    };
    TS_ASSERT_SAME_DATA(file->getContent().at(0), DATA, 29U);
}

/** Test directory overflow. */
void
TestUtilResourceFileWriter::testDirectoryOverflow()
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
    TS_ASSERT(ok);
}

