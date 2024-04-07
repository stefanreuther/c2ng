/**
  *  \file test/interpreter/vmio/processsavecontexttest.cpp
  *  \brief Test for interpreter::vmio::ProcessSaveContext
  */

#include "interpreter/vmio/processsavecontext.hpp"

#include <stdexcept>
#include "afl/data/hash.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/process.hpp"
#include "interpreter/structuretypedata.hpp"
#include "interpreter/structurevaluedata.hpp"
#include "interpreter/world.hpp"

/** Simple test. */
AFL_TEST("interpreter.vmio.ProcessSaveContext", a)
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
            { return false; }
    };
    TestParent parent;

    // Processes
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    interpreter::World world(log, tx, fs);
    interpreter::Process pa(world, "a", 42);
    interpreter::Process pb(world, "b", 43);

    // Test
    interpreter::vmio::ProcessSaveContext testee(parent, pa);
    a.check("01. isCurrentProcess a",    testee.isCurrentProcess(&pa));
    a.check("02. isCurrentProcess b",    !testee.isCurrentProcess(&pb));
    a.check("03. isCurrentProcess null", !testee.isCurrentProcess(0));

    // Test method passing (for coverage only)
    {
        interpreter::BytecodeObject bco;
        a.checkEqual("11. addBCO", testee.addBCO(bco), 10U);
    }
    {
        a.checkEqual("12. addHash", testee.addHash(*afl::data::Hash::create()), 20U);
    }
    {
        interpreter::ArrayData data;
        a.checkEqual("13. addArray", testee.addArray(data), 30U);
    }
    {
        interpreter::StructureTypeData type;
        a.checkEqual("14. addStructureType", testee.addStructureType(type), 40U);
    }
    {
        interpreter::StructureValueData value(*new interpreter::StructureTypeData());
        a.checkEqual("15. addStructureValue", testee.addStructureValue(value), 50U);
    }
}
