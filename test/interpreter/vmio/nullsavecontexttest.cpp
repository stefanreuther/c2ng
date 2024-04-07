/**
  *  \file test/interpreter/vmio/nullsavecontexttest.cpp
  *  \brief Test for interpreter::vmio::NullSaveContext
  */

#include "interpreter/vmio/nullsavecontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/world.hpp"

/** Simple tests.
    All methods must throw. */
AFL_TEST("interpreter.vmio.NullSaveContext:addBCO", a)
{
    interpreter::vmio::NullSaveContext testee;
    interpreter::BytecodeObject bco;
    AFL_CHECK_THROWS(a, testee.addBCO(bco), interpreter::Error);
}

AFL_TEST("interpreter.vmio.NullSaveContext:addHash", a)
{
    interpreter::vmio::NullSaveContext testee;
    afl::data::Hash::Ref_t hash = afl::data::Hash::create();
    AFL_CHECK_THROWS(a, testee.addHash(*hash), interpreter::Error);
}

AFL_TEST("interpreter.vmio.NullSaveContext:addArray", a)
{
    interpreter::vmio::NullSaveContext testee;
    interpreter::ArrayData array;
    AFL_CHECK_THROWS(a, testee.addArray(array), interpreter::Error);
}

AFL_TEST("interpreter.vmio.NullSaveContext:addStructureType", a)
{
    interpreter::vmio::NullSaveContext testee;
    interpreter::StructureTypeData type;
    AFL_CHECK_THROWS(a, testee.addStructureType(type), interpreter::Error);
}

AFL_TEST("interpreter.vmio.NullSaveContext:addStructureValue", a)
{
    interpreter::vmio::NullSaveContext testee;
    interpreter::StructureValueData value(*new interpreter::StructureTypeData());
    AFL_CHECK_THROWS(a, testee.addStructureValue(value), interpreter::Error);
}

AFL_TEST("interpreter.vmio.NullSaveContext:isCurrentProcess:null", a)
{
    interpreter::vmio::NullSaveContext testee;
    a.checkEqual("", testee.isCurrentProcess(0), false);
}

AFL_TEST("interpreter.vmio.NullSaveContext:isCurrentProcess:non-null", a)
{
    interpreter::vmio::NullSaveContext testee;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World w(log, tx, fs);
    interpreter::Process proc(w, a.getLocation(), 1234);
    a.checkEqual("", testee.isCurrentProcess(&proc), false);
}
