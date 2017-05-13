/**
  *  \file u/t_interpreter_vmio_nullsavecontext.cpp
  *  \brief Test for interpreter::vmio::NullSaveContext
  */

#include "interpreter/vmio/nullsavecontext.hpp"

#include "t_interpreter_vmio.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/world.hpp"

/** Simple tests.
    All methods must throw. */
void
TestInterpreterVmioNullSaveContext::testIt()
{
    interpreter::vmio::NullSaveContext testee;

    {
        interpreter::BytecodeObject bco;
        TS_ASSERT_THROWS(testee.addBCO(bco), interpreter::Error);
    }
    {
        afl::data::Hash::Ref_t hash = afl::data::Hash::create();
        TS_ASSERT_THROWS(testee.addHash(*hash), interpreter::Error);
    }
    {
        interpreter::ArrayData array;
        TS_ASSERT_THROWS(testee.addArray(array), interpreter::Error);
    }
    {
        interpreter::StructureTypeData type;
        TS_ASSERT_THROWS(testee.addStructureType(type), interpreter::Error);
    }
    {
        interpreter::StructureValueData value(*new interpreter::StructureTypeData());
        TS_ASSERT_THROWS(testee.addStructureValue(value), interpreter::Error);
    }
    {
        TS_ASSERT_THROWS(testee.isCurrentProcess(0), interpreter::Error);
    }
    {
        afl::sys::Log log;
        afl::io::NullFileSystem fs;
        interpreter::World w(log, fs);
        interpreter::Process proc(w, "TestInterpreterVmioNullSaveContext", 1234);
        TS_ASSERT_THROWS(testee.isCurrentProcess(&proc), interpreter::Error);
    }
}
