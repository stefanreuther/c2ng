/**
  *  \file test/interpreter/assemblertest.cpp
  *  \brief Test for interpreter::Assembler
  */

#include "interpreter/assembler.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/basevalue.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/vmio/filesavecontext.hpp"
#include "interpreter/values.hpp"

using afl::data::BooleanValue;
using afl::data::FloatValue;
using afl::data::IntegerValue;
using afl::data::StringValue;
using interpreter::BCORef_t;
using interpreter::BaseValue;
using interpreter::Opcode;
using interpreter::StructureType;
using interpreter::SubroutineValue;

namespace {
    struct Environment {
        afl::io::ConstMemoryStream mem;
        afl::io::TextFile in;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        afl::charset::Utf8Charset cs;
        interpreter::vmio::FileSaveContext fsc;
        interpreter::Assembler testee;

        Environment(const char* input)
            : mem(afl::string::toBytes(input)), in(mem), tx(), log(), cs(), fsc(cs),
              testee(in)
            { }
    };

    BCORef_t compile(const char* input)
    {
        Environment env(input);
        env.testee.compile();
        env.testee.finish(env.log, env.tx);
        return env.testee.saveTo(env.fsc);
    }
}

/*
 *  General
 */

/* Basic baseline test */
AFL_TEST("interpreter.Assembler:basic", a)
{
    interpreter::BCORef_t result = compile("sub test\npushint 42\nendsub\n");

    a.checkEqual("result name",       result->getSubroutineName(), "TEST");
    a.checkEqual("result code size",  result->code().size(), 1U);
    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sInteger);
    a.checkEqual("result code arg",   result->code()[0].arg, 42U);
}

/* Setting attributes */
AFL_TEST("interpreter.Assembler:basic:attributes", a)
{
    interpreter::BCORef_t result = compile("sub test\n"
                                           ".min_args 4\n"
                                           ".max_args 9\n"
                                           ".flags 3\n"
                                           ".name Other\n"
                                           ".num_labels 99\n"
                                           ".file \"x.z\"\n"
                                           "endsub\n");

    a.checkEqual("result name",       result->getSubroutineName(), "OTHER");
    a.checkEqual("result isVarargs",  result->isVarargs(), true);                // from .flags
    a.checkEqual("result min args",   result->getMinArgs(), 4U);
    a.checkEqual("result max args",   result->getMaxArgs(), 9U);
    a.checkEqual("result num labels", result->getNumLabels(), 99U);
}

/* Setting attributes: varargs */
AFL_TEST("interpreter.Assembler:basic:varargs", a)
{
    interpreter::BCORef_t result = compile("sub test\n"
                                           ".varargs\n"
                                           "endsub\n");

    a.checkEqual("result isVarargs",  result->isVarargs(), true);
}

/* Setting attributes: empty name */
AFL_TEST("interpreter.Assembler:basic:empty-name", a)
{
    interpreter::BCORef_t result = compile("sub test\n"
                                           ".name -\n"
                                           "endsub\n");

    a.checkEqual("result name",       result->getSubroutineName(), "");
}

/* Syntax errors */
AFL_TEST("interpreter.Assembler:basic:error", a)
{
    AFL_CHECK_THROWS(a("not an identifier"),   compile("sub test\n" "99"                 "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad instruction"),     compile("sub test\n" "whatever"           "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad gen 1"),           compile("sub test\n" "genint1."           "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad gen 2"),           compile("sub test\n" "genxy"              "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad gen 3"),           compile("sub test\n" "genlit"             "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad gen 4"),           compile("sub test\n" "genlit4.5"          "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("duplicate label"),     compile("sub test\n" "a: a: uinc"         "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("label overflow"),      compile("sub test\n" ".num_labels 999999" "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("garbage after name"),  compile("sub test\n" ".name x y z"        "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("garbage after num"),   compile("sub test\n" ".num_labels 5 x"    "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad number"),          compile("sub test\n" ".num_labels XXX"    "\nendsub\n"), interpreter::Error);

    AFL_CHECK_THROWS(a("bad declaration 1"),   compile("declare sub a+b\n" "sub a\nendsub\nsub b\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad declaration 2"),   compile("declare struct a+b\n" "struct a\nendstruct\nstruct b\nendstruct\n"), interpreter::Error);

    AFL_CHECK_THROWS(a("empty"),               compile(""),                        interpreter::Error);
    AFL_CHECK_THROWS(a("just struct"),         compile("struct x\nendstruct\n"),   interpreter::Error);
    AFL_CHECK_THROWS(a("bad directive"),       compile(".hello\n"),                interpreter::Error);
    AFL_CHECK_THROWS(a("undefined function"),  compile("declare function x\n"),    interpreter::Error);
    AFL_CHECK_THROWS(a("undefined sub"),       compile("declare sub x\n"),         interpreter::Error);
    AFL_CHECK_THROWS(a("undefined struct"),    compile("declare struct x\n"),      interpreter::Error);
    AFL_CHECK_THROWS(a("bad declaration"),     compile("declare spaceship x\n"),   interpreter::Error);
    AFL_CHECK_THROWS(a("bad struct"),          compile("struct x y\nendstruct\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad jumps directive"), compile(".jumps nowhere\n"),        interpreter::Error);
    AFL_CHECK_THROWS(a("empty jumps"),         compile(".jumps\n"),                interpreter::Error);

    AFL_CHECK_THROWS(a("sub not terminated"),  compile("sub a\n"),                 interpreter::Error);
}

/*
 *  Argument variants
 */

/* Argument type test: positive integer */
AFL_TEST("interpreter.Assembler:arg:positive", a)
{
    interpreter::BCORef_t result = compile("sub test\npushint +5\nendsub\n");

    a.checkEqual("result code size",  result->code().size(), 1U);
    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sInteger);
    a.checkEqual("result code arg",   result->code()[0].arg, 5U);
}

/* Argument type test: negative integer */
AFL_TEST("interpreter.Assembler:arg:negative", a)
{
    interpreter::BCORef_t result = compile("sub test\npushint -36\nendsub\n");

    a.checkEqual("result code size",  result->code().size(), 1U);
    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sInteger);
    a.checkEqual("result code arg",   result->code()[0].arg, 65500U);
}

/* Argument type test: symbol arg */
AFL_TEST("interpreter.Assembler:arg:symbol", a)
{
    interpreter::BCORef_t result = compile("sub test\npushvar GV\nendsub\n");

    a.checkEqual("result code size",  result->code().size(), 1U);
    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sNamedVariable);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);
    a.checkEqual("result code syms",  result->names().getNumNames(), 1U);
    a.checkEqual("result code name",  result->names().getNameByIndex(0), "GV");
}

/* Argument type test: local variable */
AFL_TEST("interpreter.Assembler:arg:local", a)
{
    interpreter::BCORef_t result = compile("sub test(a,b,c,d)\npushloc C\nendsub\n");

    a.checkEqual("result code size",  result->code().size(), 1U);
    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLocal);
    a.checkEqual("result code arg",   result->code()[0].arg, 2U);
    a.checkEqual("result code local", result->localVariables().getNumNames(), 4U);
    a.checkEqual("result code name",  result->localVariables().getNameByIndex(2), "C");
}

/* Argument type test: label (also exercises relocation) */
AFL_TEST("interpreter.Assembler:arg:label", a)
{
    interpreter::BCORef_t result = compile("sub test\n"
                                           "j two\n"
                                           "one: pushint 1\n"
                                           "two:\n"
                                           "j one\n"
                                           "endsub\n");

    a.checkEqual("result code size",    result->code().size(), 3U);
    a.checkEqual("result code 0 major", result->code()[0].major, Opcode::maJump);
    a.checkEqual("result code 0 minor", result->code()[0].minor, Opcode::jAlways);
    a.checkEqual("result code 0 arg",   result->code()[0].arg, 2U);
    a.checkEqual("result code 1 major", result->code()[1].major, Opcode::maPush);
    a.checkEqual("result code 1 minor", result->code()[1].minor, Opcode::sInteger);
    a.checkEqual("result code 1 arg",   result->code()[1].arg, 1);
    a.checkEqual("result code 2 major", result->code()[2].major, Opcode::maJump);
    a.checkEqual("result code 2 minor", result->code()[2].minor, Opcode::jAlways);
    a.checkEqual("result code 2 arg",   result->code()[2].arg, 1);
}

/* Argument type test: literal */
AFL_TEST("interpreter.Assembler:arg:literal", a)
{
    interpreter::BCORef_t result = compile("sub test\npushlit 'foo'\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);

    StringValue* sv = dynamic_cast<StringValue*>(result->literals()[0]);
    a.checkNonNull("result code is string", sv);
    a.checkEqual("result code string value", sv->getValue(), "foo");
}

/* Argument type test: override */
AFL_TEST("interpreter.Assembler:arg:override", a)
{
    interpreter::BCORef_t result = compile("sub test\npushlit #4242\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code arg",   result->code()[0].arg, 4242U);
}

/* Argument errors */
AFL_TEST("interpreter.Assembler:arg:error", a)
{
    AFL_CHECK_THROWS(a("bad override"),          compile("sub test\n" "pushlit #xyz"   "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad local 1"),           compile("sub test\n" "pushloc 9"      "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad local 2"),           compile("sub test\n" "pushloc foo"    "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad literal 1"),         compile("sub test\n" "pushlit *"      "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad literal 2"),         compile("sub test\n" "pushlit -x"     "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("garbage at end"),        compile("sub test\n" "pushint 1,2"    "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("undefined label"),       compile("sub test\n" "j where"        "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("undefined symbol"),      compile("sub test\n" "pushlit what"   "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("unterminated string 1"), compile("sub test\n" "pushlit 'foo"   "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("unterminated string 2"), compile("sub test\n" "pushlit \"foo"  "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("unterminated string 3"), compile("sub test\n" "pushlit \"\\\"" "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad serialisation 1"),   compile("sub test\n" "pushlit (1)"    "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad serialisation 2"),   compile("sub test\n" "pushlit ('X')"  "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("insn takes no arg"),     compile("sub test\n" "uinc 9"         "\nendsub\n"), interpreter::Error);
}

/*
 *  Literal variants
 */

/* Boolean literal: true (not generated by script compiler, uses 'pushbool 1' instead) */
AFL_TEST("interpreter.Assembler:literal:true", a)
{
    interpreter::BCORef_t result = compile("sub test\npushlit true\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);

    BooleanValue* bv = dynamic_cast<BooleanValue*>(result->literals()[0]);
    a.checkNonNull("result code is boolean", bv);
    a.checkEqual("result code boolean value", bv->getValue(), true);
}

/* Boolean literal: false (not generated by script compiler, uses 'pushbool 0' instead) */
AFL_TEST("interpreter.Assembler:literal:false", a)
{
    interpreter::BCORef_t result = compile("sub test\npushlit false\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);

    BooleanValue* bv = dynamic_cast<BooleanValue*>(result->literals()[0]);
    a.checkNonNull("result code is boolean", bv);
    a.checkEqual("result code boolean value", bv->getValue(), false);
}

/* Null literal (not generated by script compiler, uses 'pushbool -1' instead) */
AFL_TEST("interpreter.Assembler:literal:null", a)
{
    interpreter::BCORef_t result = compile("sub test\npushlit null\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);

    a.checkNull("result code is boolean", result->literals()[0]);
}

/* Integer literal (generated by compiler for literals outside pushint range) */
AFL_TEST("interpreter.Assembler:literal:int", a)
{
    interpreter::BCORef_t result = compile("sub test\npushlit 32\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);

    IntegerValue* iv = dynamic_cast<IntegerValue*>(result->literals()[0]);
    a.checkNonNull("result code is integer", iv);
    a.checkEqual("result code integer value", iv->getValue(), 32);
}

/* Positive integer literal */
AFL_TEST("interpreter.Assembler:literal:positive", a)
{
    interpreter::BCORef_t result = compile("sub test\npushlit +55\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);

    IntegerValue* iv = dynamic_cast<IntegerValue*>(result->literals()[0]);
    a.checkNonNull("result code is integer", iv);
    a.checkEqual("result code integer value", iv->getValue(), 55);
}

/* Negative integer literal */
AFL_TEST("interpreter.Assembler:literal:negative", a)
{
    interpreter::BCORef_t result = compile("sub test\npushlit -35\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);

    IntegerValue* iv = dynamic_cast<IntegerValue*>(result->literals()[0]);
    a.checkNonNull("result code is integer", iv);
    a.checkEqual("result code integer value", iv->getValue(), -35);
}

/* Float literal */
AFL_TEST("interpreter.Assembler:literal:float", a)
{
    interpreter::BCORef_t result = compile("sub test\npushlit -2.5\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);

    FloatValue* fv = dynamic_cast<FloatValue*>(result->literals()[0]);
    a.checkNonNull("result code is float", fv);
    a.checkEqual("result code float value", fv->getValue(), -2.5);
}

/* Subroutine literal */
AFL_TEST("interpreter.Assembler:literal:sub", a)
{
    interpreter::BCORef_t result = compile("sub other\n"
                                           "endsub\n"
                                           "sub test\n"
                                           "pushlit other\n"
                                           "endsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);

    SubroutineValue* sv = dynamic_cast<SubroutineValue*>(result->literals()[0]);
    a.checkNonNull("result code is sub", sv);
    a.checkEqual("result code sub value", sv->getBytecodeObject()->getSubroutineName(), "OTHER");
}

/* Struct literal */
AFL_TEST("interpreter.Assembler:literal:struct", a)
{
    interpreter::BCORef_t result = compile("struct data\n"
                                           "endstruct\n"
                                           "sub test\n"
                                           "pushlit data\n"
                                           "endsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);

    a.checkNonNull("result code is struct", dynamic_cast<StructureType*>(result->literals()[0]));
}

/* String with special content */
AFL_TEST("interpreter.Assembler:literal:string-escape", a)
{
    interpreter::BCORef_t result = compile("sub test\npushlit \"a\\nb\\tc\\\"d\"\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);

    StringValue* sv = dynamic_cast<StringValue*>(result->literals()[0]);
    a.checkNonNull("result code is string", sv);
    a.checkEqual("result code string value", sv->getValue(), "a\nb\tc\"d");
}

/* Serialized literal */
AFL_TEST("interpreter.Assembler:literal:serial", a)
{
    interpreter::BCORef_t result = compile("sub test\n"
                                           "pushlit (4,5)\n"
                                           "endsub\n");

    BaseValue* bv = dynamic_cast<BaseValue*>(result->literals()[0]);
    a.checkNonNull("literal exists", bv);
    a.checkEqual("literal textual form", bv->toString(false), "(4,5)");

    interpreter::test::ValueVerifier verif(*bv, a);
    verif.verifyBasics();
    verif.verifySerializable(0x400, 5, afl::base::Nothing);
}

/* Literal pooling */
AFL_TEST("interpreter.Assembler:literal-pool", a)
{
    interpreter::BCORef_t result = compile("sub test\n"
                                           "pushlit 'xy'\n"
                                           "pushlit 'xy'\n"
                                           "pushlit 'xy'\n"
                                           "pushlit 'xy'\n"
                                           "endsub\n");

    a.checkEqual("must have one literal", result->literals().size(), 1U);
}

/* Literal pooling, forcing new entry */
AFL_TEST("interpreter.Assembler:literal-pool:new", a)
{
    interpreter::BCORef_t result = compile("sub test\n"
                                           "pushlit 'xy'\n"     // first
                                           "pushlit !'xy'\n"    // force new
                                           "pushlit new'xy'\n"  // force new
                                           "pushlit 'xy'\n"     // re-use
                                           "endsub\n");

    a.checkEqual("must have four literals", result->literals().size(), 3U);
}

/*
 *  Instructions
 *
 *  Test specimen of most categories to check initialisation of instruction table
 */

AFL_TEST("interpreter.Assembler:insn:unary", a)
{
    interpreter::BCORef_t result = compile("sub test\nusqrt\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maUnary);
    a.checkEqual("result code minor", result->code()[0].minor, interpreter::unSqrt);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);
}

AFL_TEST("interpreter.Assembler:insn:binary", a)
{
    interpreter::BCORef_t result = compile("sub test\nbidiv\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maBinary);
    a.checkEqual("result code minor", result->code()[0].minor, interpreter::biIntegerDivide);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);
}

AFL_TEST("interpreter.Assembler:insn:ternary", a)
{
    interpreter::BCORef_t result = compile("sub test\ntkeyadd\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maTernary);
    a.checkEqual("result code minor", result->code()[0].minor, interpreter::teKeyAdd);
    a.checkEqual("result code arg",   result->code()[0].arg, 0U);
}

AFL_TEST("interpreter.Assembler:insn:push", a)
{
    interpreter::BCORef_t result = compile("sub test\npushvar X\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sNamedVariable);
}

AFL_TEST("interpreter.Assembler:insn:pop", a)
{
    interpreter::BCORef_t result = compile("sub test\npopvar X\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPop);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sNamedVariable);
}

AFL_TEST("interpreter.Assembler:insn:store", a)
{
    interpreter::BCORef_t result = compile("sub test\nstorevar X\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maStore);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sNamedVariable);
}

AFL_TEST("interpreter.Assembler:insn:stack", a)
{
    interpreter::BCORef_t result = compile("sub test\ndup 4\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maStack);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::miStackDup);
    a.checkEqual("result code arg",   result->code()[0].arg, 4U);
}

AFL_TEST("interpreter.Assembler:insn:special", a)
{
    interpreter::BCORef_t result = compile("sub test\nsprint\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maSpecial);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::miSpecialPrint);
}

AFL_TEST("interpreter.Assembler:insn:dim", a)
{
    interpreter::BCORef_t result = compile("sub test\ndimloc A\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maDim);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLocal);
}

AFL_TEST("interpreter.Assembler:insn:memref", a)
{
    interpreter::BCORef_t result = compile("sub test\nloadmem A\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maMemref);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::miIMLoad);
}

AFL_TEST("interpreter.Assembler:insn:indirect", a)
{
    interpreter::BCORef_t result = compile("sub test\npopind 2\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maIndirect);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::miIMPop);
}

AFL_TEST("interpreter.Assembler:insn:addhook", a)
{
    interpreter::BCORef_t result = compile("sub test\nsaddhook\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maSpecial);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::miSpecialAddHook);
    a.checkEqual("result code arg",   result->code()[0].arg, 0);
}

AFL_TEST("interpreter.Assembler:insn:addhook:with-arg", a)
{
    interpreter::BCORef_t result = compile("sub test\nsaddhook 42\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maSpecial);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::miSpecialAddHook);
    a.checkEqual("result code arg",   result->code()[0].arg, 42);
}

AFL_TEST("interpreter.Assembler:insn:genint", a)
{
    interpreter::BCORef_t result = compile("sub test\ngenint10.11 23\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, 10);
    a.checkEqual("result code minor", result->code()[0].minor, 11);
    a.checkEqual("result code arg",   result->code()[0].arg, 23);
}

AFL_TEST("interpreter.Assembler:insn:gensym", a)
{
    interpreter::BCORef_t result = compile("sub test\ngensym55.66 XYZ\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, 55);
    a.checkEqual("result code minor", result->code()[0].minor, 66);
    a.checkEqual("result code arg",   result->code()[0].arg, 0);
    a.checkEqual("result symbol",     result->names().getNameByIndex(0), "XYZ");
}

AFL_TEST("interpreter.Assembler:insn:genloc", a)
{
    interpreter::BCORef_t result = compile("sub test(a,b,c)\ngenloc10.11 C\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, 10);
    a.checkEqual("result code minor", result->code()[0].minor, 11);
    a.checkEqual("result code arg",   result->code()[0].arg, 2);
}

AFL_TEST("interpreter.Assembler:insn:genlit", a)
{
    interpreter::BCORef_t result = compile("sub test\ngenlit9.8 'XY'\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, 9);
    a.checkEqual("result code minor", result->code()[0].minor, 8);
    a.checkEqual("result code arg",   result->code()[0].arg, 0);
    a.checkEqual("result literal",    interpreter::toString(result->literals()[0], true), "\"XY\"");
}

AFL_TEST("interpreter.Assembler:insn:genlabel", a)
{
    interpreter::BCORef_t result = compile("sub test\nhere: genlabel44.33 here\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, 44);
    a.checkEqual("result code minor", result->code()[0].minor, 33);
    a.checkEqual("result code arg",   result->code()[0].arg, 0);
}

AFL_TEST("interpreter.Assembler:insn:gen", a)
{
    interpreter::BCORef_t result = compile("sub test\ngen9.7 5\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, 9);
    a.checkEqual("result code minor", result->code()[0].minor, 7);
    a.checkEqual("result code arg",   result->code()[0].arg, 5);
}

AFL_TEST("interpreter.Assembler:insn:gen:without-arg", a)
{
    interpreter::BCORef_t result = compile("sub test\ngen9.7\nendsub\n");

    a.checkEqual("result code major", result->code()[0].major, 9);
    a.checkEqual("result code minor", result->code()[0].minor, 7);
    a.checkEqual("result code arg",   result->code()[0].arg, 0);
}


/*
 *  Structures
 *
 *  Always define a sub to avoid hitting the "no code" case
 */

/* Successful structure definition */
AFL_TEST("interpreter.Assembler:struct", a)
{
    interpreter::BCORef_t result = compile("struct X\n"
                                           ".field a\n"
                                           ".field b, c\n"
                                           "endstruct\n"
                                           "sub test\n"
                                           "pushlit X\n"
                                           "endsub\n");

    StructureType* st = dynamic_cast<StructureType*>(result->literals()[0]);
    a.checkNonNull("result struct", st);
    a.checkEqual("struct num fields", st->getType()->names().getNumNames(), 3U);
    a.checkEqual("struct field 1", st->getType()->names().getNameByIndex(0), "A");
    a.checkEqual("struct field 2", st->getType()->names().getNameByIndex(1), "B");
    a.checkEqual("struct field 3", st->getType()->names().getNameByIndex(2), "C");
}

/* Erroneous structure definitions */
AFL_TEST("interpreter.Assembler:struct:error", a)
{
    AFL_CHECK_THROWS(a("struct not terminated"), compile("sub a\nendsub\n" "struct b\n"),                 interpreter::Error);
    AFL_CHECK_THROWS(a("bad struct content"),    compile("sub a\nendsub\n" "struct b\nfoo\nendstruct\n"), interpreter::Error);
}

/*
 *  Declarations
 */

/* Subroutine header with optional args */
AFL_TEST("interpreter.Assembler:declare:optional-args", a)
{
    interpreter::BCORef_t result = compile("sub test(a, b, optional c, d)\nendsub\n");

    a.checkEqual("result min args", result->getMinArgs(), 2U);
    a.checkEqual("result max args", result->getMaxArgs(), 4U);
    a.checkEqual("result varargs",  result->isVarargs(), false);
    a.checkEqual("result is proc",  result->isProcedure(), true);
}

/* Subroutine header with varargs */
AFL_TEST("interpreter.Assembler:declare:varargs", a)
{
    interpreter::BCORef_t result = compile("sub test(a, b, x())\nendsub\n");

    a.checkEqual("result min args", result->getMinArgs(), 2U);
    a.checkEqual("result max args", result->getMaxArgs(), 2U);
    a.checkEqual("result varargs",  result->isVarargs(), true);
    a.checkEqual("result is proc",  result->isProcedure(), true);
}

/* Function header */
AFL_TEST("interpreter.Assembler:declare:function", a)
{
    interpreter::BCORef_t result = compile("function test(a, b, c)\nendfunction\n");

    a.checkEqual("result min args", result->getMinArgs(), 3U);
    a.checkEqual("result max args", result->getMaxArgs(), 3U);
    a.checkEqual("result varargs",  result->isVarargs(), false);
    a.checkEqual("result is proc",  result->isProcedure(), false);
}

/* Circular reference */
AFL_TEST("interpreter.Assembler:declare:loop", a)
{
    interpreter::BCORef_t result = compile("declare sub a\n"
                                           "sub b\n"
                                           "pushlit a\n"
                                           "endsub\n"
                                           "sub a\n"
                                           "pushlit b\n"
                                           "endsub");

    a.checkEqual("result name", result->getSubroutineName(), "A");

    SubroutineValue* sv1 = dynamic_cast<SubroutineValue*>(result->literals()[0]);
    a.checkNonNull("result code", sv1);
    interpreter::BCORef_t other = sv1->getBytecodeObject();

    a.checkEqual("other name", other->getSubroutineName(), "B");

    SubroutineValue* sv2 = dynamic_cast<SubroutineValue*>(other->literals()[0]);
    a.checkNonNull("other code", sv2);
    interpreter::BCORef_t back = sv2->getBytecodeObject();

    a.checkEqual("loop back", &*result, &*back);

    // Note that this will create a cyclic reference that the interpreter cannot clean up.
    // Therefore, the regular compiler will not allow creating this.
    // To make our test valgrind-clean, clean up manually.
    result->literals().clear();
    other->literals().clear();
}

/* Declaring multiple subs */
AFL_TEST("interpreter.Assembler:declare:multiple:sub", a)
{
    interpreter::BCORef_t result = compile("declare sub a, b\nsub a\nendsub\nsub b\nendsub\n");

    a.checkEqual("result name", result->getSubroutineName(), "B");
}

/* Declaring multiple structures */
AFL_TEST("interpreter.Assembler:declare:multiple:struct", a)
{
    interpreter::BCORef_t result = compile("declare struct a, b\nstruct a\nendstruct\nstruct b\nendstruct\nsub x\npushlit a\npushlit b\nendsub");

    a.checkEqual("result name", result->getSubroutineName(), "X");
    a.checkNonNull("struct lit 1", dynamic_cast<StructureType*>(result->literals()[0]));
    a.checkNonNull("struct lit 2", dynamic_cast<StructureType*>(result->literals()[1]));
}

/* Some varargs error cases */
AFL_TEST("interpreter.Assembler:declare:varargs:error", a)
{
    AFL_CHECK_THROWS(a("bad varargs 1"), compile("sub test(a, b, x("        "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad varargs 2"), compile("sub test(a, b, x(x)"      "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad varargs 3"), compile("sub test(a, b, x()"       "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad varargs 4"), compile("sub test(a, b, x(), y)"   "\nendsub\n"), interpreter::Error);
    AFL_CHECK_THROWS(a("bad varargs 5"), compile("sub test(a, b, x()) z"    "\nendsub\n"), interpreter::Error);
}

/*
 *  Directives
 */

/* Defining local variables */
AFL_TEST("interpreter.Assembler:directive:local", a)
{
    interpreter::BCORef_t result = compile("sub a(b,c)\n"
                                           ".local c\n"
                                           ".local d\n"
                                           "pushloc c\n"
                                           "endsub");

    a.checkEqual("result code major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code minor", result->code()[0].minor, Opcode::sLocal);
    a.checkEqual("result code arg",   result->code()[0].arg, 2);
    a.checkEqual("result locals",     result->localVariables().getNumNames(), 4U);
}

/* Defining literals */
AFL_TEST("interpreter.Assembler:directive:lit", a)
{
    interpreter::BCORef_t result = compile("sub a\n"
                                           ".lit 99\n"
                                           ".lit 'xy'\n"
                                           "endsub");

    a.checkEqual("result literal 1", interpreter::toString(result->literals()[0], true), "99");
    a.checkEqual("result literal 2", interpreter::toString(result->literals()[1], true), "\"xy\"");
}

/* Defining symbols */
AFL_TEST("interpreter.Assembler:directive:sym", a)
{
    interpreter::BCORef_t result = compile("sub a\n"
                                           ".sym q\n"
                                           ".sym 'p'\n"
                                           ".sym -\n"
                                           ".sym zz\n"
                                           "endsub");

    a.checkEqual("result names",  result->names().getNumNames(), 4U);
    a.checkEqual("result name 1", result->names().getNameByIndex(0), "Q");
    a.checkEqual("result name 2", result->names().getNameByIndex(1), "p");
    a.checkEqual("result name 3", result->names().getNameByIndex(2), "");
    a.checkEqual("result name 4", result->names().getNameByIndex(3), "ZZ");
}

/* Line numbers */
AFL_TEST("interpreter.Assembler:directive:line", a)
{
    interpreter::BCORef_t result = compile("sub a\n"
                                           ".line 5\n"
                                           "pushint 4\n"
                                           ".line 7\n"
                                           "pushint 9\n"
                                           "pushint 10\n"
                                           ".line 9,5\n"
                                           "endsub");

    a.checkEqual("result lines",  result->lineNumbers().size(), 6U);
    a.checkEqual("result line 1", result->lineNumbers()[0], 0U);
    a.checkEqual("result line 2", result->lineNumbers()[1], 5U);
    a.checkEqual("result line 3", result->lineNumbers()[2], 1U);
    a.checkEqual("result line 4", result->lineNumbers()[3], 7U);
    a.checkEqual("result line 5", result->lineNumbers()[4], 5U);
    a.checkEqual("result line 6", result->lineNumbers()[5], 9U);
}

/* Defsubs */
AFL_TEST("interpreter.Assembler:directive:defsubs", a)
{
    interpreter::BCORef_t result = compile("sub a\n"
                                           "endsub\n"
                                           "sub b\n"
                                           "endsub\n"
                                           "sub main\n"
                                           ".defsubs\n"
                                           "endsub");

    a.checkEqual("result code size",    result->getNumInstructions(), 4U);
    a.checkEqual("result code 0 major", result->code()[0].major, Opcode::maPush);
    a.checkEqual("result code 0 minor", result->code()[0].minor, Opcode::sLiteral);
    a.checkEqual("result code 0 arg",   result->code()[0].arg, 0);
    a.checkEqual("result code 1 major", result->code()[1].major, Opcode::maSpecial);
    a.checkEqual("result code 1 minor", result->code()[1].minor, Opcode::miSpecialDefSub);
    a.checkEqual("result code 1 arg",   result->code()[1].arg, 0);
    a.checkEqual("result code 2 major", result->code()[2].major, Opcode::maPush);
    a.checkEqual("result code 2 minor", result->code()[2].minor, Opcode::sLiteral);
    a.checkEqual("result code 2 arg",   result->code()[2].arg, 1);
    a.checkEqual("result code 3 major", result->code()[3].major, Opcode::maSpecial);
    a.checkEqual("result code 3 minor", result->code()[3].minor, Opcode::miSpecialDefSub);
    a.checkEqual("result code 3 arg",   result->code()[3].arg, 1);
    a.checkEqual("result symbol 1",     result->names().getNameByIndex(0), "A");
    a.checkEqual("result symbol 2",     result->names().getNameByIndex(1), "B");

    SubroutineValue* sa = dynamic_cast<SubroutineValue*>(result->literals()[0]);
    SubroutineValue* sb = dynamic_cast<SubroutineValue*>(result->literals()[1]);
    a.checkNonNull("result literal 1 is sub", sa);
    a.checkEqual("result literal 1 name", sa->getBytecodeObject()->getSubroutineName(), "A");
    a.checkNonNull("result literal 2 is sub", sb);
    a.checkEqual("result literal 2 name", sb->getBytecodeObject()->getSubroutineName(), "B");
}

/*
 *  Symbolic jumps
 */

AFL_TEST("interpreter.Assembler:symbolic", a)
{
    interpreter::BCORef_t result = compile(".jumps sym\n"
                                           "sub a\n"
                                           "jt lab\n"
                                           "pushint 1\n"
                                           "lab:\n"
                                           "jf lab2\n"
                                           "pushint 2\n"
                                           "lab2:\n"
                                           "endsub\n");

    a.checkEqual("result code size",    result->getNumInstructions(), 6U);
    a.checkEqual("result code 0 major", result->code()[0].major, Opcode::maJump);
    a.checkEqual("result code 0 minor", result->code()[0].minor, Opcode::jIfTrue | Opcode::jSymbolic);
    a.checkEqual("result code 0 arg",   result->code()[0].arg, 0);
    a.checkEqual("result code 2 major", result->code()[2].major, Opcode::maJump);
    a.checkEqual("result code 2 minor", result->code()[2].minor, Opcode::jSymbolic);
    a.checkEqual("result code 2 arg",   result->code()[2].arg, 0);
    a.checkEqual("result code 3 major", result->code()[3].major, Opcode::maJump);
    a.checkEqual("result code 3 minor", result->code()[3].minor, Opcode::jIfFalse | Opcode::jSymbolic);
    a.checkEqual("result code 3 arg",   result->code()[3].arg, 1);
    a.checkEqual("result code 5 major", result->code()[5].major, Opcode::maJump);
    a.checkEqual("result code 5 minor", result->code()[5].minor, Opcode::jSymbolic);
    a.checkEqual("result code 5 arg",   result->code()[5].arg, 1);
}

AFL_TEST("interpreter.Assembler:absolute", a)
{
    interpreter::BCORef_t result = compile(".jumps abs\n"
                                           "sub a\n"
                                           "jt lab\n"
                                           "pushint 1\n"
                                           "lab:\n"
                                           "jf lab2\n"
                                           "pushint 2\n"
                                           "lab2:\n"
                                           "endsub\n");

    a.checkEqual("result code size",    result->getNumInstructions(), 4U);
    a.checkEqual("result code 0 major", result->code()[0].major, Opcode::maJump);
    a.checkEqual("result code 0 minor", result->code()[0].minor, Opcode::jIfTrue);
    a.checkEqual("result code 0 arg",   result->code()[0].arg, 2);
    a.checkEqual("result code 2 major", result->code()[2].major, Opcode::maJump);
    a.checkEqual("result code 2 minor", result->code()[2].minor, Opcode::jIfFalse);
    a.checkEqual("result code 2 arg",   result->code()[2].arg, 4);
}
