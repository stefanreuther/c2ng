/**
  *  \file u/t_interpreter_vmio_filesavecontext.cpp
  *  \brief Test for interpreter::vmio::FileSaveContext
  */

#include "interpreter/vmio/filesavecontext.hpp"

#include "t_interpreter_vmio.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/hashvalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/structuretypedata.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/structurevaluedata.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

/** A simple test.
    Save a process that exercises (most) features.

    Note that this test requires more than the FileSaveContext interface actually requires by testing the binary format.
    FileSaveContext does not guarantee a particular Id assignment scheme; object Ids (header and embedded) thus could change.
    FileSaveContext does not guarantee a particular ordering other than minimizing forward references;
    the order of unrelated objects like array and hash object could change. */
void
TestInterpreterVmioFileSaveContext::testIt()
{
    // Create a BCO; push some literals
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    {
        afl::data::StringValue sv("h\xc3\xb6");  // c3b6 = U+00F6 = small o with diarrhoe
        bco->addPushLiteral(&sv);
    }
    {
        interpreter::HashValue hv(afl::data::Hash::create());
        hv.getData()->setNew("a", interpreter::makeIntegerValue(66));
        bco->addPushLiteral(&hv);
    }
    {
        afl::base::Ref<interpreter::ArrayData> ad(*new interpreter::ArrayData());
        TS_ASSERT(ad->addDimension(10));
        ad->content.setNew(1, interpreter::makeIntegerValue(77));
        interpreter::ArrayValue av(ad);
        bco->addPushLiteral(&av);
    }
    {
        interpreter::StructureTypeData::Ref_t st(*new interpreter::StructureTypeData());
        interpreter::StructureValueData::Ref_t svd(*new interpreter::StructureValueData(st));
        svd->data.setNew(st->names().add("X"), interpreter::makeIntegerValue(88));
        interpreter::StructureValue sv(svd);
        bco->addPushLiteral(&sv);
    }

    // Create a process
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    interpreter::World world(log, tx, fs);
    interpreter::Process p(world, "p", 99);
    p.pushFrame(bco, false);

    // Test
    // - Save as UTF-8
    {
        afl::charset::Utf8Charset cs;
        interpreter::vmio::FileSaveContext t(cs);
        t.addProcess(p);

        // Must be 6 objects: process, BCO, hash, array, structure type, structure value
        TS_ASSERT_EQUALS(t.getNumPreparedObjects(), 6U);

        afl::io::InternalStream s;
        t.save(s);

        // Verify content
        TS_ASSERT_EQUALS(s.getContent().size(), 478U);

        static const uint8_t EXPECTED[478] = {
            // Hash #2, 0x20 bytes, 3 properties
            0x05, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x61, 0x00, 0x02, 0x42, 0x00, 0x00, 0x00,

            // Array #3, 0x28 bytes, 3 properties
            0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x02, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x02, 0x4D, 0x00, 0x00, 0x00,

            // Structure type #5, 0x12 bytes, 2 properties
            0x07, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
            0x01, 0x58,

            // Structure value #4, 0x22 bytes, 3 properties
            0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x02, 0x58, 0x00,
            0x00, 0x00,

            // BCO #1, 0x7b bytes, 9 properties
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x7B, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x02, 0x00, 0x00, 0x00, 0x00, 0x08, 0x03, 0x00,
            0x00, 0x00, 0x00, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x68, 0xC3, 0xB6, 0x00, 0x00, 0x05, 0x00, 0x01,
            0x00, 0x05, 0x00, 0x02, 0x00, 0x05, 0x00, 0x03, 0x00, 0x05, 0x00,

            // Process #0, 0x87 bytes, 7 properties
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x70, 0x03, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x89, 0x00, 0x00, 0x00, 0x00
        };
        TS_ASSERT_SAME_DATA(s.getContent().unsafeData(), EXPECTED, sizeof(EXPECTED));
    }

    // - Save as latin 1. This must produce a slightly different object.
    {
        afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
        interpreter::vmio::FileSaveContext t(cs);
        t.addProcess(p);

        // Must be 6 objects: process, BCO, hash, array, structure type, structure value
        TS_ASSERT_EQUALS(t.getNumPreparedObjects(), 6U);

        afl::io::InternalStream s;
        t.save(s);

        // Verify content
        TS_ASSERT_EQUALS(s.getContent().size(), 477U);

        static const uint8_t EXPECTED[477] = {
            // Hash #2, 0x20 bytes, 3 properties
            0x05, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x61, 0x00, 0x02, 0x42, 0x00, 0x00, 0x00,

            // Array #3, 0x28 bytes, 3 properties
            0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x02, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x02, 0x4D, 0x00, 0x00, 0x00,

            // Structure type #5, 0x12 bytes, 2 properties
            0x07, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
            0x01, 0x58,

            // Structure value #4, 0x22 bytes, 3 properties
            0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x02, 0x58, 0x00,
            0x00, 0x00,

            // BCO #1, 0x7a bytes, 9 properties
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x7A, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x06, 0x02, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x02, 0x00, 0x00, 0x00, 0x00, 0x08, 0x03, 0x00,
            0x00, 0x00, 0x00, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x68,     0xF6,   0x00, 0x00, 0x05, 0x00, 0x01,
            0x00, 0x05, 0x00, 0x02, 0x00, 0x05, 0x00, 0x03, 0x00, 0x05, 0x00,

            // Process #0, 0x87 bytes, 7 properties
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x70, 0x03, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x89, 0x00, 0x00, 0x00, 0x00
        };
        TS_ASSERT_SAME_DATA(s.getContent().unsafeData(), EXPECTED, sizeof(EXPECTED));
    }
}

/** Test a recursive structure. */
void
TestInterpreterVmioFileSaveContext::testCycle()
{
    // Create a BCO; push some literals
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);

    // - three arrays
    afl::base::Ref<interpreter::ArrayData> aa(*new interpreter::ArrayData());
    TS_ASSERT(aa->addDimension(10));
    afl::base::Ref<interpreter::ArrayData> ab(*new interpreter::ArrayData());
    TS_ASSERT(ab->addDimension(10));
    afl::base::Ref<interpreter::ArrayData> ac(*new interpreter::ArrayData());
    TS_ASSERT(ac->addDimension(10));

    // - data in the arrays
    aa->content.setNew(1, interpreter::makeIntegerValue(1));
    ab->content.setNew(1, interpreter::makeIntegerValue(2));
    ac->content.setNew(1, interpreter::makeIntegerValue(3));

    // - references: a -> b <-> c
    aa->content.setNew(2, new interpreter::ArrayValue(ab));
    ab->content.setNew(2, new interpreter::ArrayValue(ac));
    ac->content.setNew(2, new interpreter::ArrayValue(ab));

    {
        interpreter::ArrayValue av(aa);
        bco->addPushLiteral(&av);
    }

    // Create a process
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    interpreter::World world(log, tx, fs);
    interpreter::Process p(world, "p", 99);
    p.pushFrame(bco, false);

    // Test
    {
        afl::charset::Utf8Charset cs;
        interpreter::vmio::FileSaveContext t(cs);
        t.addProcess(p);

        // Must be 5 objects: process, BCO, 3 arrays, structure type, structure value
        TS_ASSERT_EQUALS(t.getNumPreparedObjects(), 5U);

        afl::io::InternalStream s;
        t.save(s);

        // Verify content
        TS_ASSERT_EQUALS(s.getContent().size(), 443U);

        static const uint8_t EXPECTED[443] = {
            // Array #4, 0x2E bytes = ac
            0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x03, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x08, 0x03, 0x00, 0x00, 0x00,

            // Array #3, 0x2E bytes = ab
            0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x03, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x08, 0x04, 0x00, 0x00, 0x00,

            // Array #2, 0x2E bytes = aa
            0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x03, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
            //                                                                      ^^^^^^ (0) = empty ^^^
            0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x08, 0x03, 0x00, 0x00, 0x00,
            //          ^^^^^^^^^^ (1) = int(1) ^^^^^^^^^^  ^^^^^^ (2) = ref to array #3 ^^^^^

            // BCO #1, 0x5A bytes
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x5A, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x08, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00,
            // ^^^^^^^ ref to array #2 ^^^^^^^^

            // Process #0, 0x87 bytes
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x70, 0x03, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x89, 0x00, 0x00, 0x00, 0x00
        };
        TS_ASSERT_SAME_DATA(s.getContent().unsafeData(), EXPECTED, sizeof(EXPECTED));
    }

    // Break the cycle to allow clean up!
    aa->content.setNew(2, 0);
    ab->content.setNew(2, 0);
    ac->content.setNew(2, 0);
}

