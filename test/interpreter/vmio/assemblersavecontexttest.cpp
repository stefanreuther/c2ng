/**
  *  \file test/interpreter/vmio/assemblersavecontexttest.cpp
  *  \brief Test for interpreter::vmio::AssemblerSaveContext
  */

#include "interpreter/vmio/assemblersavecontext.hpp"

#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/internaltextwriter.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/filevalue.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/structuretypedata.hpp"
#include "interpreter/subroutinevalue.hpp"

namespace {
    template<typename T>
    const T* addr(const T& t)
    {
        return &t;
    }
}

/** Test addBCO(), base case. */
AFL_TEST("interpreter.vmio.AssemblerSaveContext:BytecodeObject", a)
{
    // Create a bytecode object
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    bco->addArgument("M", false);
    bco->addArgument("O", true);
    bco->addInstruction(interpreter::Opcode::maSpecial, interpreter::Opcode::miSpecialSuspend, 0);

    // Save it
    interpreter::vmio::AssemblerSaveContext testee;
    testee.addBCO(*bco);

    afl::io::InternalTextWriter out;
    testee.save(out);

    // Verify
    a.checkEqual("result", afl::string::fromMemory(out.getContent()),
                 "Sub BCO1 (M, Optional O)\n"
                 "  .name -\n"
                 "    ssuspend\n"
                 "EndSub\n"
                 "\n");
}

/** Test addBCO(), duplicate name handling. */
AFL_TEST("interpreter.vmio.AssemblerSaveContext:BytecodeObject:duplicate-name", a)
{
    // Create two bytecode objects with identical name
    interpreter::BCORef_t bco1 = interpreter::BytecodeObject::create(true);
    bco1->setSubroutineName("S");
    interpreter::BCORef_t bco2 = interpreter::BytecodeObject::create(true);
    bco2->setSubroutineName("S");

    // Save them
    interpreter::vmio::AssemblerSaveContext testee;
    testee.addBCO(*bco1);
    testee.addBCO(*bco2);

    afl::io::InternalTextWriter out;
    testee.save(out);

    // Verify
    a.checkEqual("result", afl::string::fromMemory(out.getContent()),
                 "Sub S\n"
                 "EndSub\n"
                 "\n"
                 "Sub BCO1\n"
                 "  .name S\n"
                 "EndSub\n"
                 "\n");
}

/** Test addBCO(), use of most options. */
AFL_TEST("interpreter.vmio.AssemblerSaveContext:BytecodeObject:options", a)
{
    // Create a bytecode object
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    bco->addArgument("M", false);
    bco->addArgument("O", true);
    bco->setIsVarargs(true);
    bco->setFileName("t.q");
    bco->addLineNumber(20);
    bco->addInstruction(interpreter::Opcode::maSpecial, interpreter::Opcode::miSpecialSuspend, 0);
    bco->addInstruction(interpreter::Opcode::maJump, interpreter::Opcode::jAlways, 0);
    bco->setSubroutineName("S");
    bco->addLocalVariable("A");
    bco->addLocalVariable("B");

    // Save it
    interpreter::vmio::AssemblerSaveContext testee;
    testee.addBCO(*bco);
    testee.setDebugInformation(true);

    afl::io::InternalTextWriter out;
    testee.save(out);

    // Verify
    a.checkEqual("result", afl::string::fromMemory(out.getContent()),
                 "Sub S (M, Optional O)\n"
                 "  .local A\n"
                 "  .local B\n"
                 "  .varargs\n"
                 "  .file t.q\n"
                 "  label0:\n"
                 "    .line 20\n"
                 "    ssuspend\n"
                 "    j               #0\n"
                 "EndSub\n"
                 "\n");
}

/** Test addBCO(), circular references. */
AFL_TEST("interpreter.vmio.AssemblerSaveContext:BytecodeObject:circular-link", a)
{
    // Create two bytecode objects with identical name
    interpreter::BCORef_t bco1 = interpreter::BytecodeObject::create(true);
    bco1->setSubroutineName("ONE");
    interpreter::BCORef_t bco2 = interpreter::BytecodeObject::create(true);
    bco2->setSubroutineName("TWO");

    bco2->addPushLiteral(addr(interpreter::SubroutineValue(bco1)));
    bco1->addPushLiteral(addr(interpreter::SubroutineValue(bco2)));

    // Save them
    interpreter::vmio::AssemblerSaveContext testee;
    testee.addBCO(*bco1);

    afl::io::InternalTextWriter out;
    testee.save(out);

    // Verify
    a.checkEqual("result", afl::string::fromMemory(out.getContent()),
                 "Declare Sub ONE\n"
                 "Sub TWO\n"
                 "    pushlit         ONE\n"
                 "EndSub\n"
                 "\n"
                 "Sub ONE\n"
                 "    pushlit         TWO\n"
                 "EndSub\n"
                 "\n");
}

/** Test addStructureType(). */
AFL_TEST("interpreter.vmio.AssemblerSaveContext:StructureTypeData", a)
{
    // Create a structure type
    interpreter::StructureTypeData::Ref_t sd = *new interpreter::StructureTypeData();
    sd->names().add("FIRST");
    sd->names().add("SECOND");

    // Save it
    interpreter::vmio::AssemblerSaveContext testee;
    testee.addStructureType(*sd);

    afl::io::InternalTextWriter out;
    testee.save(out);

    // Verify
    a.checkEqual("result", afl::string::fromMemory(out.getContent()),
                 "Struct TYPE1\n"
                 "    .field FIRST\n"
                 "    .field SECOND\n"
                 "EndStruct\n"
                 "\n");
}

/** Test literal handling in disassembly. */
AFL_TEST("interpreter.vmio.AssemblerSaveContext:BytecodeObject:literals", a)
{
    // Create a bytecode object
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    bco->addPushLiteral(addr(afl::data::IntegerValue(999999)));
    bco->addPushLiteral(addr(afl::data::FloatValue(3.5)));
    bco->addPushLiteral(addr(afl::data::StringValue("x")));
    bco->addPushLiteral(addr(afl::data::BooleanValue(true)));
    bco->addPushLiteral(addr(interpreter::SubroutineValue(interpreter::BytecodeObject::create(false))));    // BCO1
    bco->addPushLiteral(addr(interpreter::StructureType(*new interpreter::StructureTypeData())));           // TYPE2
    bco->addPushLiteral(addr(interpreter::FileValue(17)));                                                  // serialized as (tag,data) pair

    // Save it
    interpreter::vmio::AssemblerSaveContext testee;
    testee.addBCO(*bco);

    afl::io::InternalTextWriter out;
    testee.save(out);

    // Verify
    a.checkEqual("result", afl::string::fromMemory(out.getContent()),
                 "Function BCO1\n"
                 "  .name -\n"
                 "EndFunction\n"
                 "\n"
                 "Struct TYPE2\n"
                 "EndStruct\n"
                 "\n"
                 "Sub BCO3\n"
                 "  .name -\n"
                 "    pushlit         999999\n"
                 "    pushlit         3.5\n"
                 "    pushlit         \"x\"\n"
                 "    pushbool        1\n"
                 "    pushlit         BCO1\n"
                 "    pushlit         TYPE2\n"
                 "    pushlit         (10,17)\n"
                 "EndSub\n"
                 "\n"
                 "");
}

/** Test parameter handling in disassembly. */
AFL_TEST("interpreter.vmio.AssemblerSaveContext:BytecodeObject:parameters", a)
{
    // Create a bytecode object
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sNamedVariable, bco->addName("N"));
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sLocal, bco->addLocalVariable("Y"));
    bco->addInstruction(interpreter::Opcode::maBinary, interpreter::biAdd, 0);
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 42);

    // Save it
    interpreter::vmio::AssemblerSaveContext testee;
    testee.addBCO(*bco);

    afl::io::InternalTextWriter out;
    testee.save(out);

    // Verify
    a.checkEqual("result", afl::string::fromMemory(out.getContent()),
                 "Function BCO1\n"
                 "  .name -\n"
                 "  .local Y\n"
                 "    pushvar         N                   % name #0\n"
                 "    pushloc         Y                   % local #0\n"
                 "    badd\n"
                 "    pushint         42\n"
                 "EndFunction\n"
                 "\n");
}
