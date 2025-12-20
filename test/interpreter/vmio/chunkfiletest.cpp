/**
  *  \file test/interpreter/vmio/chunkfiletest.cpp
  *  \brief Test for interpreter::vmio::ChunkFile
  */

#include "interpreter/vmio/chunkfile.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/except/fileproblemexception.hpp"

/** Check using ChunkFile classes to copy an object file. */
AFL_TEST("interpreter.vmio.ChunkFile:copy", a)
{
    // Test file, contains two bytecode objects (subroutine and main)
    static const uint8_t INPUT[] = {
        // ObjectFileHeader                                                                 ObjectHeader
        0x43, 0x43, 0x6f, 0x62, 0x6a, 0x1a, 0x64, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
        0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06,
        0x05, 0x00, 0x00, 0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x0d,
        //                                                                                  ObjectHeader
        0x0b, 0x48, 0x49, 0x74, 0x2e, 0x71, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x07, 0x00,
        0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00,
        0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x07,
        0x02, 0x00, 0x00, 0x00, 0x02, 0x48, 0x49, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x09, 0x0b, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x02, 0x07, 0x00, 0x0e, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x04, 0x05, 0x74, 0x2e, 0x71, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00,
        0x00, 0x00, 0x04, 0x00, 0x00, 0x00
    };

    afl::base::Ref<afl::io::Stream> in(*new afl::io::ConstMemoryStream(INPUT));
    afl::string::NullTranslator tx;

    // Read header
    uint32_t entryId = interpreter::vmio::ChunkFile::loadObjectFileHeader(in, tx);
    a.checkEqual("entryId", entryId, 1U);

    // Copy to new file
    afl::io::InternalStream out;
    interpreter::vmio::ChunkFile::writeObjectFileHeader(out, entryId);

    // Read/copy objects
    int numObjects = 0;
    uint32_t type = 0, id = 0;
    interpreter::vmio::ChunkFile::Loader loader(in, tx);
    interpreter::vmio::ChunkFile::Writer writer(out);
    while (loader.readObject(type, id)) {
        // Verify object
        ++numObjects;
        uint32_t numProp = loader.getNumProperties();
        a.checkEqual("must have 8 properties", numProp, 8U);
        a.checkEqual("must be a bytecode object", type, interpreter::vmio::structures::otyp_Bytecode);

        // Verify property metadata for one object
        if (id == 2) {
            a.checkEqual("property 0 count", loader.getPropertyCount(0), 0U);
            a.checkEqual("property 0 size",  loader.getPropertySize(0),  0U);

            a.checkEqual("property 1 count", loader.getPropertyCount(1), 0U);
            a.checkEqual("property 1 size",  loader.getPropertySize(1),  8U);

            a.checkEqual("property 2 count", loader.getPropertyCount(2), 1U);
            a.checkEqual("property 2 size",  loader.getPropertySize(2), 11U);

            a.checkEqual("property 8 count", loader.getPropertyCount(8), 1U);
            a.checkEqual("property 8 size",  loader.getPropertySize(8),  8U);

            // Nonexistant
            a.checkEqual("property 9 count", loader.getPropertyCount(9), 0U);
            a.checkEqual("property 9 size",  loader.getPropertySize(9),  0U);
        }

        // Copy properties
        uint32_t expectedPropId = 0;
        uint32_t propId = 0, propCount = 0;
        writer.start(type, id, numProp);
        while (afl::io::Stream* propStream = loader.readProperty(propId, propCount)) {
            ++expectedPropId;
            a.checkEqual("property Id must have expected value", propId, expectedPropId);
            writer.startProperty(propCount);
            out.copyFrom(*propStream);
            writer.endProperty();
        }
        writer.end();
    }

    // Verify
    a.checkEqual("must have copied two objects", numObjects, 2);
    a.checkEqualContent("output file must be identical", afl::base::ConstBytes_t(INPUT), out.getContent());
}

/** loadObjectFileHeader(), error case: bad magic */
AFL_TEST("interpreter.vmio.ChunkFile:loadObjectFileHeader:error:bad-magic", a)
{
    static const uint8_t INPUT[] = {
        //          vvvv  vvvv  Bad magic
        0x43, 0x43, 0x99, 0x99, 0x6a, 0x1a, 0x64, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
    };

    afl::base::Ref<afl::io::Stream> in(*new afl::io::ConstMemoryStream(INPUT));
    afl::string::NullTranslator tx;
    AFL_CHECK_THROWS(a, interpreter::vmio::ChunkFile::loadObjectFileHeader(in, tx), afl::except::FileProblemException);
}

/** loadObjectFileHeader(), error case: bad version */
AFL_TEST("interpreter.vmio.ChunkFile:loadObjectFileHeader:error:bad-version", a)
{
    static const uint8_t INPUT[] = {
        //                                  vvvv  Bad version
        0x43, 0x43, 0x6f, 0x62, 0x6a, 0x1a, 0x63, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
    };

    afl::base::Ref<afl::io::Stream> in(*new afl::io::ConstMemoryStream(INPUT));
    afl::string::NullTranslator tx;
    AFL_CHECK_THROWS(a, interpreter::vmio::ChunkFile::loadObjectFileHeader(in, tx), afl::except::FileProblemException);
}

/** Loader, error case: truncated header. */
AFL_TEST("interpreter.vmio.ChunkFile:Loader:error:truncated-header", a)
{
    // Header, should have 16 bytes
    static const uint8_t INPUT[] = { 0x00, 0x00, 0x00, 0x00 };

    afl::base::Ref<afl::io::Stream> in(*new afl::io::ConstMemoryStream(INPUT));
    afl::string::NullTranslator tx;
    interpreter::vmio::ChunkFile::Loader loader(in, tx);
    uint32_t type = 0, id = 0;
    AFL_CHECK_THROWS(a, loader.readObject(type, id), afl::except::FileProblemException);
}

/** Loader, error case: bad size (not enough to cover property headers). */
AFL_TEST("interpreter.vmio.ChunkFile:Loader:error:bad-size", a)
{
    // Header, should have 16 bytes
    static const uint8_t INPUT[] = {
        //                                              vvvv size (too small)   vvvv numProp
        0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    afl::base::Ref<afl::io::Stream> in(*new afl::io::ConstMemoryStream(INPUT));
    afl::string::NullTranslator tx;
    interpreter::vmio::ChunkFile::Loader loader(in, tx);
    uint32_t type = 0, id = 0;
    AFL_CHECK_THROWS(a, loader.readObject(type, id), afl::except::FileProblemException);
}
