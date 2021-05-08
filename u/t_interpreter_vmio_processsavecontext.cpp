/**
  *  \file u/t_interpreter_vmio_processsavecontext.cpp
  *  \brief Test for interpreter::vmio::ProcessSaveContext
  */

#include "interpreter/vmio/processsavecontext.hpp"

#include "t_interpreter_vmio.hpp"
#include "afl/data/hash.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/process.hpp"
#include "interpreter/structuretypedata.hpp"
#include "interpreter/structurevaluedata.hpp"
#include "interpreter/world.hpp"

/** Simple test. */
void
TestInterpreterVmioProcessSaveContext::testIt()
{
    // Parent
    class TestParent : public interpreter::SaveContext {
     public:
        virtual uint32_t addBCO(const interpreter::BytecodeObject& /*bco*/)
            { return 10; }
        virtual uint32_t addHash(const afl::data::Hash& /*hash*/)
            { return 20; }
        virtual uint32_t addArray(const interpreter::ArrayData& /*array*/)
            { return 30; }
        virtual uint32_t addStructureType(const interpreter::StructureTypeData& /*type*/)
            { return 40; }
        virtual uint32_t addStructureValue(const interpreter::StructureValueData& /*value*/)
            { return 50; }
        virtual bool isCurrentProcess(const interpreter::Process* /*p*/)
            { TS_FAIL("unexpected"); return false; }
    };
    TestParent parent;

    // Processes
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    interpreter::World world(log, tx, fs);
    interpreter::Process a(world, "a", 42);
    interpreter::Process b(world, "b", 43);

    // Test
    interpreter::vmio::ProcessSaveContext testee(parent, a);
    TS_ASSERT(testee.isCurrentProcess(&a));
    TS_ASSERT(!testee.isCurrentProcess(&b));
    TS_ASSERT(!testee.isCurrentProcess(0));

    // Test method passing (for coverage only)
    {
        interpreter::BytecodeObject bco;
        TS_ASSERT_EQUALS(testee.addBCO(bco), 10U);
    }
    {
        TS_ASSERT_EQUALS(testee.addHash(*afl::data::Hash::create()), 20U);
    }
    {
        interpreter::ArrayData data;
        TS_ASSERT_EQUALS(testee.addArray(data), 30U);
    }
    {
        interpreter::StructureTypeData type;
        TS_ASSERT_EQUALS(testee.addStructureType(type), 40U);
    }
    {
        interpreter::StructureValueData value(*new interpreter::StructureTypeData());
        TS_ASSERT_EQUALS(testee.addStructureValue(value), 50U);
    }
}

