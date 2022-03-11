/**
  *  \file u/t_interpreter_expr_builtinfunction.cpp
  *  \brief Test for interpreter::expr::BuiltinFunction
  *
  *  Untested:
  *      CC$Trace -- This is a debug feature that has been tried during debugging :-)
  *      Count -- Needs an indexable context mock
  *      Find -- Needs an indexable context mock
  */

#include "interpreter/expr/builtinfunction.hpp"

#include "t_interpreter_expr.hpp"
#include "interpreter/test/expressionverifier.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/values.hpp"
#include "interpreter/indexablevalue.hpp"

using interpreter::test::ExpressionVerifier;

/** Test trig functions: Sin, Cos, Tan, ATan.
    Instructions: ucos, usin, utan, batan */
void
TestInterpreterExprBuiltinFunction::testTrig()
{
    // ex IntBuiltinTestSuite::testTrig
    ExpressionVerifier h("testTrig");
    // Sin
    h.verifyFloat("sin(0)", 0);
    h.verifyFloat("sin(90)", 1);
    h.verifyFloat("sin(180)", 0);
    h.verifyFloat("sin(270)", -1);
    h.verifyFloat("sin(0.0)", 0);
    h.verifyFloat("sin(90.0)", 1);
    h.verifyFloat("sin(180.0)", 0);
    h.verifyFloat("sin(270.0)", -1);
    h.verifyNull("sin(z(0))");
    h.verifyExecutionError("sin('a')");
    h.verifyExecutionError("sin('')");
    h.verifyInteger("if(sin(90),3,2)",3);      // used as condition
    h.verifyInteger("if(sin(0);1,3,2)",3);     // used as effect
    h.verifyCompileError("sin(0):='x'");                // used as assigment target

    // Cos
    h.verifyFloat("cos(0)", 1);
    h.verifyFloat("cos(90)", 0);
    h.verifyFloat("cos(180)", -1);
    h.verifyFloat("cos(270)", 0);
    h.verifyFloat("cos(0.0)", 1);
    h.verifyFloat("cos(90.0)", 0);
    h.verifyFloat("cos(180.0)", -1);
    h.verifyFloat("cos(270.0)", 0);
    h.verifyNull("cos(z(0))");
    h.verifyExecutionError("cos('a')");
    h.verifyExecutionError("cos('')");
    h.verifyInteger("if(cos(0),3,2)",3);       // used as condition
    h.verifyInteger("if(cos(90);1,3,2)",3);    // used as effect
    h.verifyCompileError("cos(0):='x'");                // used as assigment target

    // Tan
    h.verifyFloat("tan(0)", 0);
    h.verifyFloat("tan(45)", 1);
    h.verifyExecutionError("tan(90)");
    h.verifyFloat("tan(135)", -1);
    h.verifyFloat("tan(180)", 0);
    h.verifyExecutionError("tan(270)");
    h.verifyFloat("tan(0.0)", 0);
    h.verifyFloat("tan(45.0)", 1);
    h.verifyExecutionError("tan(90.0)");
    h.verifyFloat("tan(135.0)", -1);
    h.verifyFloat("tan(180.0)", 0);
    h.verifyExecutionError("tan(270.0)");
    h.verifyNull("tan(z(0))");
    h.verifyExecutionError("tan('a')");
    h.verifyExecutionError("tan('')");
    h.verifyInteger("if(tan(45),3,2)",3);      // used as condition
    h.verifyInteger("if(tan(0);1,3,2)",3);     // used as effect
    h.verifyCompileError("tan(0):='x'");                // used as assigment target

    // Atan
    h.verifyFloat("atan(1)", 45);
    h.verifyFloat("atan(-1)", 315);
    h.verifyFloat("atan(1.0)", 45);
    h.verifyFloat("atan(-1.0)", 315);

    h.verifyFloat("atan(1,1)", 45);
    h.verifyFloat("atan(-1,1)", 315);
    h.verifyFloat("atan(1,-1)", 135);
    h.verifyFloat("atan(-1,-1)", 225);

    h.verifyFloat("atan(1.0,1.0)", 45);
    h.verifyFloat("atan(-1.0,1.0)", 315);
    h.verifyFloat("atan(1.0,-1.0)", 135);
    h.verifyFloat("atan(-1.0,-1.0)", 225);

    h.verifyFloat("atan(0)", 0);
    h.verifyFloat("atan(1, 0)", 90);
    h.verifyFloat("atan(0, 1)", 0);
    h.verifyFloat("atan(-1, 0.0)", 270);
    h.verifyFloat("atan(0, -1.0)", 180);

    h.verifyFloat("atan(sin(25), cos(25))", 25);

    h.verifyNull("atan(1, z(0))");
    h.verifyNull("atan(z(0))");
    h.verifyNull("atan(z(0), 1)");

    h.verifyExecutionError("atan('a')");
    h.verifyExecutionError("atan(1,'a')");
    h.verifyExecutionError("atan('a',1)");

    h.verifyInteger("if(atan(1,0),3,2)",3);    // used as condition
    h.verifyInteger("if(atan(0,1);1,3,2)",3);  // used as effect
    h.verifyCompileError("atan(0,1):='x'");             // used as assigment target
}

/** Test Abs function.
    Instruction: uabs */
void
TestInterpreterExprBuiltinFunction::testAbs()
{
    // ex IntBuiltinTestSuite::testAbs
    ExpressionVerifier h("testAbs");
    h.verifyInteger("abs(1)", 1);
    h.verifyInteger("abs(-1)", 1);
    h.verifyInteger("abs(true)", 1);
    h.verifyInteger("abs(0)", 0);
    h.verifyInteger("abs(-1111111111)", 1111111111);
    h.verifyInteger("abs(+1111111111)", 1111111111);

    h.verifyFloat("abs(1.0)", 1.0);
    h.verifyFloat("abs(-1.0)", 1.0);
    h.verifyFloat("abs(-9999.0)", 9999.0);
    h.verifyFloat("abs(pi)", 3.14159265);
    h.verifyFloat("abs(-pi)", 3.14159265);
    h.verifyFloat("abs(7777777.0)", 7777777.0);

    h.verifyNull("abs(z(0))");

    h.verifyExecutionError("abs('a')");
    h.verifyExecutionError("abs('')");

    h.verifyInteger("if(abs(1),9,0)", 9);    // used as condition
    h.verifyInteger("if(abs(0);1,9,0)", 9);  // used as effect
    h.verifyCompileError("abs(0):=2");                // used as assigment target
}

/** Test Asc function.
    Instruction: uasc */
void
TestInterpreterExprBuiltinFunction::testAsc()
{
    // ex IntBuiltinTestSuite::testAsc
    ExpressionVerifier h("testAsc");
    h.verifyInteger("asc('a')", 97);
    h.verifyInteger("asc('abcdef')", 97);
    h.verifyInteger("asc(0)", 48);
    h.verifyInteger("asc(2.5)", 50);
    h.verifyNull("asc('')");
    h.verifyNull("asc(z(0))");

    h.verifyInteger("if(asc('a'),9,0)", 9);    // used as condition
    h.verifyInteger("if(asc('x');0,0,9)", 9);  // used as effect
    h.verifyCompileError("asc('0'):=2");                // used as assigment target
}

/** Test bit operations.
    Instructions: bbitand, bbitor, bbitxor, ubitnot */
void
TestInterpreterExprBuiltinFunction::testBitOps()
{
    // ex IntBuiltinTestSuite::testBitOps
    ExpressionVerifier h("testBitOps");
    // BitAnd
    h.verifyInteger("bitand(1)", 1);
    h.verifyInteger("bitand(7,85)", 5);
    h.verifyInteger("bitand(4,4,4)", 4);
    h.verifyInteger("bitand(4,4,0)", 0);
    h.verifyInteger("bitand(true,true)", 1);
    h.verifyNull("bitand(z(0))");
    h.verifyNull("bitand(z(0),1)");
    h.verifyNull("bitand(1,z(0))");
    h.verifyNull("bitand(1,1,z(0))");
    h.verifyNull("bitand(z(0),1,1)");
    h.verifyExecutionError("bitand('a')");
    h.verifyExecutionError("bitand(1,'a')");
    h.verifyExecutionError("bitand('a',1)");
    h.verifyInteger("if(bitand(2,3,7),7,9)", 7);
    h.verifyInteger("if(bitand(1,2,4);1,7,9)", 7);
    h.verifyCompileError("bitand(1,2):=3");

    // BitOr
    h.verifyInteger("bitor(1)", 1);
    h.verifyInteger("bitor(7,85)", 87);
    h.verifyInteger("bitor(4,4,4)", 4);
    h.verifyInteger("bitor(4,4,0)", 4);
    h.verifyInteger("bitor(true,false)", 1);
    h.verifyNull("bitor(z(0))");
    h.verifyNull("bitor(z(0),1)");
    h.verifyNull("bitor(1,z(0))");
    h.verifyNull("bitor(1,1,z(0))");
    h.verifyNull("bitor(z(0),1,1)");
    h.verifyExecutionError("bitor('a')");
    h.verifyExecutionError("bitor(1,'a')");
    h.verifyExecutionError("bitor('a',1)");
    h.verifyInteger("if(bitor(2,3,5),7,9)", 7);
    h.verifyInteger("if(bitor(1,2,4);0,7,9)", 9);
    h.verifyCompileError("bitor(1,2):=3");

    // BitXor
    h.verifyInteger("bitxor(1)", 1);
    h.verifyInteger("bitxor(7,85)", 82);
    h.verifyInteger("bitxor(4,4,4)", 4);
    h.verifyInteger("bitxor(4,4,0)", 0);
    h.verifyInteger("bitxor(true,true)", 0);
    h.verifyNull("bitxor(z(0))");
    h.verifyNull("bitxor(z(0),1)");
    h.verifyNull("bitxor(1,z(0))");
    h.verifyNull("bitxor(1,1,z(0))");
    h.verifyNull("bitxor(z(0),1,1)");
    h.verifyExecutionError("bitxor('a')");
    h.verifyExecutionError("bitxor(1,'a')");
    h.verifyExecutionError("bitxor('a',1)");
    h.verifyInteger("if(bitxor(2,3,5),7,9)", 7);
    h.verifyInteger("if(bitxor(1,2,4);0,7,9)", 9);
    h.verifyCompileError("bitxor(1,2):=3");

    // BitNot
    h.verifyInteger("bitnot(1)", -2);
    h.verifyInteger("bitnot(0)", -1);
    h.verifyInteger("bitnot(true)", -2);
    h.verifyNull("bitnot(z(0))");
    h.verifyExecutionError("bitnot('a')");
    h.verifyInteger("if(bitnot(0),2,3)", 2);
    h.verifyInteger("if(bitnot(-1);1,3,5)", 3);
    h.verifyCompileError("bitnot(9):=4");
}

/** Test Min/Max operators.
    Instructions: bmin, bmax, and NC versions thereof */
void
TestInterpreterExprBuiltinFunction::testMinMax()
{
    // ex IntBuiltinTestSuite::testMinMax
    ExpressionVerifier h("testMinMax");
    // Integers
    h.verifyInteger("min(1)", 1);
    h.verifyInteger("min(3,1,4,1,5)", 1);
    h.verifyInteger("min(99,-22,22)", -22);
    h.verifyInteger("max(1)", 1);
    h.verifyInteger("max(3,1,4,1,5)", 5);
    h.verifyInteger("max(99,-22,22)", 99);
    h.verifyNull("min(1,z(0))");
    h.verifyNull("min(z(0),1)");
    h.verifyNull("min(z(0))");
    h.verifyNull("max(1,z(0))");
    h.verifyNull("max(z(0),1)");
    h.verifyNull("max(z(0))");
    h.verifyInteger("if(max(-1,0),99,22)", 22);
    h.verifyInteger("if(max(-1,0);1,99,22)", 99);
    h.verifyCompileError("max(1,2):=3");

    // Floats and Mixes
    // Note that min/max do not lift their argument to a common type before returning it!
    h.verifyFloat("min(1.0)", 1);
    h.verifyFloat("min(3.1, 4.1, 5.9, 2.6)", 2.6);
    h.verifyInteger("min(99.0,-22,22)", -22);
    h.verifyFloat("max(1.0)", 1);
    h.verifyFloat("max(3.1, 4.1, 5.9, 2.6)", 5.9);
    h.verifyFloat("max(99.0,-22,22)", 99);
    h.verifyNull("min(1.0,z(0))");
    h.verifyNull("min(z(0),1.0)");
    h.verifyNull("min(z(0))");
    h.verifyNull("max(1.0,z(0))");
    h.verifyNull("max(z(0),1.0)");
    h.verifyNull("max(z(0))");
    h.verifyInteger("max(false,9)", 9);
    h.verifyInteger("max(9,false)", 9);
    h.verifyBoolean("min(true,9)", true);
    h.verifyBoolean("min(9,true)", true);
    h.verifyInteger("if(min(-1,0),99,22)", 99);
    h.verifyInteger("if(min(-1,0);0,99,22)", 22);
    h.verifyCompileError("min(1,2):=3");

    // Same things with StrCase, to exercise NC versions
    h.verifyInteger("strcase(min(1))", 1);
    h.verifyInteger("strcase(min(3,1,4,1,5))", 1);
    h.verifyInteger("strcase(min(99,-22,22))", -22);
    h.verifyInteger("strcase(max(1))", 1);
    h.verifyInteger("strcase(max(3,1,4,1,5))", 5);
    h.verifyInteger("strcase(max(99,-22,22))", 99);

    h.verifyFloat("strcase(min(1.0))", 1);
    h.verifyFloat("strcase(min(3.1, 4.1, 5.9, 2.6))", 2.6);
    h.verifyInteger("strcase(min(99.0,-22,22))", -22);
    h.verifyFloat("strcase(max(1.0))", 1);
    h.verifyFloat("strcase(max(3.1, 4.1, 5.9, 2.6))", 5.9);
    h.verifyFloat("strcase(max(99.0,-22,22))", 99);

    h.verifyNull("strcase(min(1.0,z(0)))");
    h.verifyNull("strcase(min(z(0),1.0))");
    h.verifyNull("strcase(min(z(0)))");
    h.verifyNull("strcase(max(1.0,z(0)))");
    h.verifyNull("strcase(max(z(0),1.0))");
    h.verifyNull("strcase(max(z(0)))");
    h.verifyInteger("strcase(if(max(-1,0),99,22))", 22);
    h.verifyInteger("if(strcase(max(-1,0));1,99,22)", 99);
    h.verifyCompileError("strcase(max(1,2)):=3");

    h.verifyInteger("max(1,2);9", 9);
    h.verifyInteger("strcase(max(1,2));3", 3);

    // Strings
    h.verifyString("min('h','a','l','l','o')", "a");
    h.verifyString("max('h','a','l','l','o')", "o");
    h.verifyString("min('H','a','L','l','O')", "a");
    h.verifyString("max('H','a','L','l','O')", "O");

    h.verifyString("strcase(min('h','a','l','l','o'))", "a");
    h.verifyString("strcase(max('h','a','l','l','o'))", "o");
    h.verifyString("strcase(min('H','a','L','l','O'))", "H");
    h.verifyString("strcase(max('H','a','L','l','O'))", "l");

    h.verifyNull("min('a',z(0))");
    h.verifyNull("min(z(0),'a')");
    h.verifyNull("strcase(min('a',z(0)))");
    h.verifyNull("strcase(min(z(0),'a'))");
    h.verifyInteger("strcase(if(min(-1,0),99,22))", 99);
    h.verifyInteger("if(strcase(min(-1,0));0,99,22)", 22);
    h.verifyCompileError("strcase(min(1,2):=3)");

    // Mixed types
    h.verifyExecutionError("min('H', 3)");
    h.verifyExecutionError("max('H', 3)");
    h.verifyExecutionError("min('H', 3.5)");
    h.verifyExecutionError("max('H', 3.5)");
    h.verifyExecutionError("min(3, 'H')");
    h.verifyExecutionError("max(3, 'H')");
    h.verifyExecutionError("min(3.5, 'H')");
    h.verifyExecutionError("max(3.5, 'H')");
}

/** Test Chr/Chr$ function (two names for the same function).
    Instructions: uchr */
void
TestInterpreterExprBuiltinFunction::testChr()
{
    // ex IntBuiltinTestSuite::testChr
    ExpressionVerifier h("testChr");
    h.verifyString("chr(1)", "\001");
    h.verifyString("chr(97)", "a");
    h.verifyNull("chr(z(0))");
    h.verifyInteger("len(chr(0))", 1);
    h.verifyExecutionError("chr('a')");
    h.verifyString("chr(128)", "\xC2\x80");
    h.verifyString("chr(57665)", "\xEE\x85\x81");

    h.verifyString("chr$(1)", "\001");
    h.verifyString("chr$(97)", "a");
    h.verifyNull("chr$(z(0))");
    h.verifyInteger("len(chr$(0))", 1);
    h.verifyExecutionError("chr$('a')");
    h.verifyString("chr$(128)", "\xC2\x80");
    h.verifyString("chr$(57665)", "\xEE\x85\x81");

    h.verifyInteger("if(chr(99),3,4)", 3);
    h.verifyInteger("if(chr(77);0,3,4)", 4);
    h.verifyCompileError("chr(88):='x'");
}

/** Test type tests: IsEmpty, IsNum, IsString.
    Instructions: uisempty, uisnum, uisstr */
void
TestInterpreterExprBuiltinFunction::testTypeChecks()
{
    // ex IntBuiltinTestSuite::testTypeChecks
    ExpressionVerifier h("testTypeChecks");
    h.verifyBoolean("isempty(0)", false);
    h.verifyBoolean("isempty(1)", false);
    h.verifyBoolean("isempty(0.0)", false);
    h.verifyBoolean("isempty(1.0)", false);
    h.verifyBoolean("isempty(true)", false);
    h.verifyBoolean("isempty(false)", false);
    h.verifyBoolean("isempty('')", false);
    h.verifyBoolean("isempty('foo')", false);
    h.verifyBoolean("isempty(z(0))", true);
    h.verifyBoolean("isempty(zap(0))", true);
    h.verifyInteger("if(isempty(''),0,9)", 9);
    h.verifyInteger("if(isempty('');0,9,0)", 0);
    h.verifyCompileError("isempty(''):=1");

    h.verifyBoolean("isnum(0)", true);
    h.verifyBoolean("isnum(1)", true);
    h.verifyBoolean("isnum(0.0)", true);
    h.verifyBoolean("isnum(1.0)", true);
    h.verifyBoolean("isnum(true)", true);
    h.verifyBoolean("isnum(false)", true);
    h.verifyBoolean("isnum('')", false);
    h.verifyBoolean("isnum('foo')", false);
    h.verifyBoolean("isnum(z(0))", false);
    h.verifyBoolean("isnum(zap(0))", false);
    h.verifyInteger("if(isnum(7),9,0)", 9);
    h.verifyInteger("if(isnum(7);0,9,0)", 0);
    h.verifyCompileError("isnum(7):=1");

    h.verifyBoolean("isstring(0)", false);
    h.verifyBoolean("isstring(1)", false);
    h.verifyBoolean("isstring(0.0)", false);
    h.verifyBoolean("isstring(1.0)", false);
    h.verifyBoolean("isstring(true)", false);
    h.verifyBoolean("isstring(false)", false);
    h.verifyBoolean("isstring('')", true);
    h.verifyBoolean("isstring('foo')", true);
    h.verifyBoolean("isstring(z(0))", false);
    h.verifyBoolean("isstring(zap(0))", false);
    h.verifyInteger("if(isstring(''),9,0)", 9);
    h.verifyInteger("if(isstring('');0,9,0)", 0);
    h.verifyCompileError("isstring(''):=1");
}

/** Test Exp/Log.
    Instructions: uexp, ulog */
void
TestInterpreterExprBuiltinFunction::testExp()
{
    // ex IntBuiltinTestSuite::testExp
    ExpressionVerifier h("testExp");
    h.verifyFloat("exp(-1)", 1/2.7172);
    h.verifyFloat("exp(0)", 1);
    h.verifyFloat("exp(1)", 2.7172);
    h.verifyFloat("exp(2)", 2.7172*2.7172);

    h.verifyFloat("exp(-1.0)", 1/2.7172);
    h.verifyFloat("exp(0.0)", 1);
    h.verifyFloat("exp(1.0)", 2.7172);
    h.verifyFloat("exp(2.0)", 2.7172*2.7172);

    h.verifyFloat("log(2.7172*2.7172)", 2);
    h.verifyFloat("log(2.7172)", 1);
    h.verifyFloat("log(1)", 0);

    h.verifyFloat("exp(log(10000))", 10000);
    h.verifyFloat("log(exp(10))", 10);
    // This fails because exp(10000) overflows, yielding NaN. CxxTest then fails
    // to convert the offending value into a string. D'oh.
    // verifyFloat("log(exp(10000))", 10000);

    h.verifyExecutionError("log(0)");
    h.verifyExecutionError("log('a')");
    h.verifyExecutionError("exp('a')");
    h.verifyNull("log(z(0))");
    h.verifyNull("exp(z(0))");

    h.verifyInteger("if(log(99),9,0)", 9);
    h.verifyInteger("if(log(1);1,9,0)", 9);
    h.verifyCompileError("log(1):=2");

    h.verifyInteger("if(exp(0),9,0)", 9);
    h.verifyInteger("if(exp(1);0,0,9)", 9);
    h.verifyCompileError("exp(1):=2");
}

/** Test substring-finding routines: InStr, First, Rest.
    Instructions: bfindstr, bfirststr, breststr, and NC versions thereof. */
void
TestInterpreterExprBuiltinFunction::testStrFind()
{
    // ex IntBuiltinTestSuite::testStrFind
    ExpressionVerifier h("testStrFind");
    // InStr
    h.verifyInteger("instr('foobar', 'o')", 2);
    h.verifyInteger("instr('foobar', 'O')", 2);
    h.verifyInteger("instr('foobar', 'oO')", 2);
    h.verifyInteger("instr('quuxUUM', 'UU')", 2);
    h.verifyInteger("instr('foo', 'bar')", 0);
    h.verifyInteger("instr('foobar', 'foo')", 1);
    h.verifyInteger("instr('foo', 'foobar')", 0);
    h.verifyInteger("instr('foobar', 'b')", 4);
    h.verifyInteger("if(instr('foobar','b'),8,2)", 8);
    h.verifyInteger("if(instr('foobar','x');1,8,2)", 8);
    h.verifyCompileError("instr('foobar','b'):=0");

    h.verifyInteger("strcase(instr('foobar', 'o'))", 2);
    h.verifyInteger("strcase(instr('foobar', 'O'))", 0);
    h.verifyInteger("strcase(instr('foobar', 'oO'))", 0);
    h.verifyInteger("strcase(instr('quuxUUM', 'UU'))", 5);
    h.verifyInteger("strcase(instr('foo', 'bar'))", 0);
    h.verifyInteger("strcase(instr('foobar', 'foo'))", 1);
    h.verifyInteger("strcase(instr('foo', 'foobar'))", 0);
    h.verifyInteger("strcase(instr('foobar', 'b'))", 4);
    h.verifyInteger("if(strcase(instr('foobar','b')),8,2)", 8);
    h.verifyInteger("if(strcase(instr('foobar','x'));1,8,2)", 8);
    h.verifyCompileError("strcase(instr('foobar','b'):=0)");

    // First - different order of parameters!
    h.verifyString("first('o', 'foobar')", "f");
    h.verifyString("first('O', 'foobar')", "f");
    h.verifyString("first('oO', 'foobar')", "f");
    h.verifyString("first('UU', 'quuxUUM')", "q");
    h.verifyString("first('bar', 'foo')", "foo");
    h.verifyString("first('foo', 'foobar')", "");
    h.verifyString("first('foobar', 'foo')", "foo");
    h.verifyString("first('b', 'foobar')", "foo");
    h.verifyInteger("if(first('b','foobar'),8,2)", 8);
    h.verifyInteger("if(first('x','foobar');0,8,2)", 2);
    h.verifyCompileError("first('b','foobar'):=0");

    h.verifyString("strcase(first('o', 'foobar'))", "f");
    h.verifyString("strcase(first('O', 'foobar'))", "foobar");
    h.verifyString("strcase(first('oO', 'foobar'))", "foobar");
    h.verifyString("strcase(first('UU', 'quuxUUM'))", "quux");
    h.verifyString("strcase(first('bar', 'foo'))", "foo");
    h.verifyString("strcase(first('foo', 'foobar'))", "");
    h.verifyString("strcase(first('foobar', 'foo'))", "foo");
    h.verifyString("strcase(first('b', 'foobar'))", "foo");
    h.verifyInteger("if(strcase(first('b','foobar')),8,2)", 8);
    h.verifyInteger("if(strcase(first('x','foobar'));0,8,2)", 2);
    h.verifyCompileError("strcase(first('b','foobar')):=0");

    // Rest
    h.verifyString("rest('o', 'foobar')", "obar");
    h.verifyString("rest('O', 'foobar')", "obar");
    h.verifyString("rest('oO', 'foobar')", "bar");
    h.verifyString("rest('UU', 'quuxUUM')", "xUUM");
    h.verifyNull("rest('bar', 'foo')");
    h.verifyString("rest('foo', 'foobar')", "bar");
    h.verifyNull("rest('foobar', 'foo')");
    h.verifyString("rest('b', 'foobar')", "ar");

    h.verifyString("strcase(rest('o', 'foobar'))", "obar");
    h.verifyNull("strcase(rest('O', 'foobar'))");
    h.verifyNull("strcase(rest('oO', 'foobar'))");
    h.verifyString("strcase(rest('UU', 'quuxUUM'))", "M");
    h.verifyNull("strcase(rest('bar', 'foo'))");
    h.verifyString("strcase(rest('foo', 'foobar'))", "bar");
    h.verifyNull("strcase(rest('foobar', 'foo'))");
    h.verifyString("strcase(rest('b', 'foobar'))", "ar");
    h.verifyInteger("if(rest('b','foobar'),8,2)", 8);
    h.verifyInteger("if(rest('x','foobar');1,8,2)", 8);
    h.verifyCompileError("rest('b','foobar'):=0");

    // Null
    h.verifyNull("instr(z(0),'a')");
    h.verifyNull("instr('a',z(0))");
    h.verifyNull("instr(z(0),z(0))");
    h.verifyNull("first(z(0),'a')");
    h.verifyNull("first('a',z(0))");
    h.verifyNull("first(z(0),z(0))");
    h.verifyNull("rest(z(0),'a')");
    h.verifyNull("rest('a',z(0))");
    h.verifyNull("rest(z(0),z(0))");
    h.verifyInteger("if(strcase(rest('b','foobar')),8,2)", 8);
    h.verifyInteger("if(strcase(rest('x','foobar'));1,8,2)", 8);
    h.verifyCompileError("strcase(rest('b','foobar')):=0");

    // Type errors
    // FIXME: should these really be type errors, or should we implicitly stringify?
    h.verifyExecutionError("instr('a', 1)");
    h.verifyExecutionError("instr(1, 'a')");
    h.verifyExecutionError("instr(1, 2)");
    h.verifyExecutionError("first('a', 1)");
    h.verifyExecutionError("first(1, 'a')");
    h.verifyExecutionError("first(1, 2)");
    h.verifyExecutionError("rest('a', 1)");
    h.verifyExecutionError("rest(1, 'a')");
    h.verifyExecutionError("rest(1, 2)");
}

/** Test substring operations: Mid, Left, Right.
    Instructions: blcut, brcut, bendcut */
void
TestInterpreterExprBuiltinFunction::testSubstr()
{
    // ex IntBuiltinTestSuite::testSubstr
    ExpressionVerifier h("testSubstr");
    // Mid, 2-arg
    h.verifyString("mid('foobar',-2)", "foobar");
    h.verifyString("mid('foobar',-1)", "foobar");
    h.verifyString("mid('foobar',0)", "foobar");
    h.verifyString("mid('foobar',1)", "foobar");
    h.verifyString("mid('foobar',2)", "oobar");
    h.verifyString("mid('foobar',3)", "obar");
    h.verifyString("mid('foobar',4)", "bar");
    h.verifyString("mid('foobar',5)", "ar");
    h.verifyString("mid('foobar',6)", "r");
    h.verifyString("mid('foobar',7)", "");
    h.verifyString("mid('foobar',8)", "");

    h.verifyNull("mid(z(0),1)");
    h.verifyNull("mid(z(0),z(0))");
    h.verifyNull("mid('foo',z(0))");

    h.verifyInteger("if(mid('foobar',3),8,2)",8);
    h.verifyInteger("if(mid('foobar',30);1,8,2)",8);
    h.verifyCompileError("mid('foobar',2):='a'");                 // FIXME: This may get legalized someday

    // Mid, 3-arg
    h.verifyString("mid('foobar',-2,3)", "foo");
    h.verifyString("mid('foobar',-1,3)", "foo");
    h.verifyString("mid('foobar',0,3)", "foo");
    h.verifyString("mid('foobar',1,3)", "foo");
    h.verifyString("mid('foobar',2,3)", "oob");
    h.verifyString("mid('foobar',3,3)", "oba");
    h.verifyString("mid('foobar',4,3)", "bar");
    h.verifyString("mid('foobar',5,3)", "ar");
    h.verifyString("mid('foobar',6,3)", "r");
    h.verifyString("mid('foobar',7,3)", "");
    h.verifyString("mid('foobar',8,3)", "");

    h.verifyString("mid('foobar',2,0)", "");

    h.verifyNull("mid(z(0),1,1)");
    h.verifyNull("mid(z(0),z(0),1)");
    h.verifyNull("mid('foo',z(0),1)");
    h.verifyNull("mid(z(0),1,z(0))");
    h.verifyNull("mid(z(0),z(0),z(0))");
    h.verifyNull("mid('foo',z(0),z(0))");
    h.verifyNull("mid('foo',1,z(0))");

    h.verifyInteger("if(mid('foobar',3,1),8,2)",8);
    h.verifyInteger("if(mid('foobar',30,1);1,8,2)",8);
    h.verifyCompileError("mid('foobar',2,1):='a'");                 // FIXME: This may get legalized someday

    // Left
    h.verifyString("left('foobar',-3)", "");
    h.verifyString("left('foobar',-2)", "");
    h.verifyString("left('foobar',-1)", "");
    h.verifyString("left('foobar',0)", "");
    h.verifyString("left('foobar',1)", "f");
    h.verifyString("left('foobar',2)", "fo");
    h.verifyString("left('foobar',3)", "foo");
    h.verifyString("left('foobar',4)", "foob");
    h.verifyString("left('foobar',5)", "fooba");
    h.verifyString("left('foobar',6)", "foobar");
    h.verifyString("left('foobar',7)", "foobar");
    h.verifyString("left('foobar',8)", "foobar");

    h.verifyNull("left(z(0),1)");
    h.verifyNull("left(z(0),z(0))");
    h.verifyNull("left('foo',z(0))");

    h.verifyInteger("if(left('foobar',3),8,2)",8);
    h.verifyInteger("if(left('foobar',30);0,8,2)",2);
    h.verifyCompileError("left('foobar',2):='a'");                 // FIXME: This may get legalized someday

    // Right
    h.verifyString("right('foobar',-3)", "");
    h.verifyString("right('foobar',-2)", "");
    h.verifyString("right('foobar',-1)", "");
    h.verifyString("right('foobar',0)", "");
    h.verifyString("right('foobar',1)", "r");
    h.verifyString("right('foobar',2)", "ar");
    h.verifyString("right('foobar',3)", "bar");
    h.verifyString("right('foobar',4)", "obar");
    h.verifyString("right('foobar',5)", "oobar");
    h.verifyString("right('foobar',6)", "foobar");
    h.verifyString("right('foobar',7)", "foobar");
    h.verifyString("right('foobar',8)", "foobar");

    h.verifyNull("right(z(0),1)");
    h.verifyNull("right(z(0),z(0))");
    h.verifyNull("right('foo',z(0))");

    h.verifyInteger("if(right('foobar',3),8,2)",8);
    h.verifyInteger("if(right('foobar',30);0,8,2)",2);
    h.verifyCompileError("right('foobar',2):='a'");                 // FIXME: This may get legalized someday

    // Type errors
    h.verifyExecutionError("mid(10,1,1)");
    h.verifyExecutionError("mid('foo','1','1')");
    h.verifyExecutionError("mid('foo','1')");

    h.verifyExecutionError("left(10, 1)");
    h.verifyExecutionError("left('foo', '1')");

    h.verifyExecutionError("right(10, 1)");
    h.verifyExecutionError("right('foo', '1')");
}

/** Test trim functions: Trim, LTrim, RTrim.
    Instructions: ulrtrim, ultrim, urtrim */
void
TestInterpreterExprBuiltinFunction::testTrim()
{
    // ex IntBuiltinTestSuite::testTrim
    ExpressionVerifier h("testTrim");
    // Trim
    h.verifyString("trim('foo')", "foo");
    h.verifyString("trim('  foo')", "foo");
    h.verifyString("trim('foo  ')", "foo");
    h.verifyString("trim('  foo  ')", "foo");
    h.verifyString("trim('    ')", "");
    h.verifyNull("trim(z(0))");
    h.verifyExecutionError("trim(1)");
    h.verifyExecutionError("trim(1.0)");

    h.verifyInteger("if(trim(' x'),7,6)",7);
    h.verifyInteger("if(trim(' ');1,7,6)",7);
    h.verifyCompileError("trim(''):=9");

    // LTrim
    h.verifyString("ltrim('foo')", "foo");
    h.verifyString("ltrim('  foo')", "foo");
    h.verifyString("ltrim('foo  ')", "foo  ");
    h.verifyString("ltrim('  foo  ')", "foo  ");
    h.verifyString("ltrim('    ')", "");
    h.verifyNull("ltrim(z(0))");
    h.verifyExecutionError("ltrim(1)");
    h.verifyExecutionError("ltrim(1.0)");

    h.verifyInteger("if(ltrim(' x'),7,6)",7);
    h.verifyInteger("if(ltrim(' ');1,7,6)",7);
    h.verifyCompileError("ltrim(''):=9");

    // RTrim
    h.verifyString("rtrim('foo')", "foo");
    h.verifyString("rtrim('  foo')", "  foo");
    h.verifyString("rtrim('foo  ')", "foo");
    h.verifyString("rtrim('  foo  ')", "  foo");
    h.verifyString("rtrim('    ')", "");
    h.verifyNull("rtrim(z(0))");
    h.verifyExecutionError("rtrim(1)");
    h.verifyExecutionError("rtrim(1.0)");

    h.verifyInteger("if(rtrim(' x'),7,6)",7);
    h.verifyInteger("if(rtrim(' ');1,7,6)",7);
    h.verifyCompileError("rtrim(''):=9");
}

/** Test square root (Sqr/Sqrt).
    Instructions: usqrt */
void
TestInterpreterExprBuiltinFunction::testSqrt()
{
    // ex IntBuiltinTestSuite::testSqrt
    ExpressionVerifier h("testSqrt");
    // Sqrt
    h.verifyFloat("sqrt(0)", 0);
    h.verifyFloat("sqrt(1)", 1);
    h.verifyFloat("sqrt(2)", 1.4142);
    h.verifyFloat("sqrt(3)", 1.73205);
    h.verifyFloat("sqrt(4)", 2);

    h.verifyFloat("sqrt(0.0)", 0);
    h.verifyFloat("sqrt(1.0)", 1);
    h.verifyFloat("sqrt(2.0)", 1.4142);
    h.verifyFloat("sqrt(3.0)", 1.73205);
    h.verifyFloat("sqrt(4.0)", 2);

    h.verifyFloat("sqrt(1.23456^2)", 1.23456);
    h.verifyFloat("sqrt(12345)^2", 12345);

    h.verifyNull("sqrt(z(0))");
    h.verifyNull("sqrt(z(0)^2)");

    h.verifyExecutionError("sqrt(-1)");
    h.verifyExecutionError("sqrt('a')");

    h.verifyInteger("if(sqrt(9),4,3)", 4);
    h.verifyInteger("if(sqrt(9);0,2,1)", 1);
    h.verifyCompileError("sqrt(7):=9");

    // Same things again with Sqr
    h.verifyFloat("sqr(0)", 0);
    h.verifyFloat("sqr(1)", 1);
    h.verifyFloat("sqr(2)", 1.4142);
    h.verifyFloat("sqr(3)", 1.73205);
    h.verifyFloat("sqr(4)", 2);

    h.verifyFloat("sqr(0.0)", 0);
    h.verifyFloat("sqr(1.0)", 1);
    h.verifyFloat("sqr(2.0)", 1.4142);
    h.verifyFloat("sqr(3.0)", 1.73205);
    h.verifyFloat("sqr(4.0)", 2);

    h.verifyFloat("sqr(1.23456^2)", 1.23456);
    h.verifyFloat("sqr(12345)^2", 12345);

    h.verifyNull("sqr(z(0))");
    h.verifyNull("sqr(z(0)^2)");

    h.verifyExecutionError("sqr(-1)");
    h.verifyExecutionError("sqr('a')");
}

/** Test rounding functions: Int, Round.
    Instructions: utrunc, uround */
void
TestInterpreterExprBuiltinFunction::testRound()
{
    // ex IntBuiltinTestSuite::testRound
    ExpressionVerifier h("testRound");
    // Integers
    h.verifyInteger("int(-1)", -1);
    h.verifyInteger("int(0)", 0);
    h.verifyInteger("int(+1)", 1);
    h.verifyInteger("int(true)", 1);
    h.verifyInteger("round(-1)", -1);
    h.verifyInteger("round(0)", 0);
    h.verifyInteger("round(+1)", 1);
    h.verifyInteger("round(true)", 1);

    h.verifyInteger("if(round(9),4,3)", 4);
    h.verifyInteger("if(round(9);0,2,1)", 1);
    h.verifyCompileError("round(7):=9");

    // Floats, Int
    h.verifyInteger("int(1.9)", 1);
    h.verifyInteger("int(1.5)", 1);
    h.verifyInteger("int(1.1)", 1);
    h.verifyInteger("int(0.4)", 0);
    h.verifyInteger("int(0.0)", 0);
    h.verifyInteger("int(-0.4)", 0);
    h.verifyInteger("int(-1.1)", -1);
    h.verifyInteger("int(-1.5)", -1);
    h.verifyInteger("int(-1.9)", -1);
    h.verifyInteger("int(2147483647)", 2147483647);
    h.verifyNull("int(z(0))");
    h.verifyExecutionError("int('a')");
    h.verifyExecutionError("int(2147483648)");

    h.verifyInteger("if(int(9),4,3)", 4);
    h.verifyInteger("if(int(9);0,2,1)", 1);
    h.verifyCompileError("int(7):=9");

    // Floats, Round
    h.verifyInteger("round(1.9)", 2);
    h.verifyInteger("round(1.5)", 2);
    h.verifyInteger("round(1.1)", 1);
    h.verifyInteger("round(0.4)", 0);
    h.verifyInteger("round(0.0)", 0);
    h.verifyInteger("round(-0.4)", 0);
    h.verifyInteger("round(-1.1)", -1);
    h.verifyInteger("round(-1.5)", -2);
    h.verifyInteger("round(-1.9)", -2);
    h.verifyInteger("round(2147483647)", 2147483647);
    h.verifyNull("round(z(0))");
    h.verifyExecutionError("round('a')");
    h.verifyExecutionError("round(2147483648)");
}

/** Test If function.
    Several other tests can be found in IntParseExprTestSuite::testAnd etc. */
void
TestInterpreterExprBuiltinFunction::testIf()
{
    // ex IntBuiltinTestSuite::testIf
    ExpressionVerifier h("testIf");
    h.verifyInteger("if(1,2,3)", 2);
    h.verifyInteger("if(0,2,3)", 3);
    h.verifyNull("if(0,2)");
    h.verifyInteger("if(1,2)", 2);

    h.verifyInteger("if(1,2,'a')",2);
    h.verifyString("if(0,2,'a')", "a");

    h.verifyInteger("if(if(1,2,0),3,4)", 3);
    h.verifyInteger("if(if(1,2,0);0,3,4)", 4);
    h.verifyCompileError("if(1,2,3):=9");
}

/** Test Str function.
    Instructions: ustr, bstr */
void
TestInterpreterExprBuiltinFunction::testStr()
{
    // ex IntBuiltinTestSuite::testStr
    ExpressionVerifier h("testStr");
    // Unary
    h.verifyString("str(1)", "1");
    h.verifyString("str(123456789)", "123456789");
    h.verifyString("str(1.0)", "1");
    h.verifyString("str(1.01)", "1.01");
    // FIXME: the following holds for PCC1, but not for PCC2:
    // verifyString("str(1.001)", "1");
    h.verifyString("str('a')", "a");
    h.verifyString("str(true)", "YES");
    h.verifyString("str(false)", "NO");
    h.verifyNull("str(z(0))");

    h.verifyInteger("if(str(9),4,3)", 4);
    h.verifyInteger("if(str(9);0,2,1)", 1);
    h.verifyCompileError("str(7):=9");

    // Binary
    h.verifyString("str(1,3)", "1.000");
    h.verifyString("str(123456789,3)", "123456789.000");
    h.verifyString("str(1.0,3)", "1.000");
    h.verifyString("str(1.01,3)", "1.010");
    h.verifyString("str(1.001,3)", "1.001");
    h.verifyExecutionError("str('a',3)");
    h.verifyString("str(true,3)", "YES");
    h.verifyString("str(false,3)", "NO");
    h.verifyNull("str(z(0),3)");
    h.verifyNull("str(1,z(0))");
    h.verifyExecutionError("str(1, 'a')");

    h.verifyInteger("if(str(9,2),4,3)", 4);
    h.verifyInteger("if(str(9,2);0,2,1)", 1);
    h.verifyCompileError("str(7,2):=9");
}

/** Test Val function.
    Instruction: uval */
void
TestInterpreterExprBuiltinFunction::testVal()
{
    // ex IntBuiltinTestSuite::testVal
    ExpressionVerifier h("testVal");
    h.verifyInteger("val('1')", 1);
    h.verifyInteger("val('99')", 99);
    h.verifyInteger("val('-99')", -99);
    h.verifyInteger("val('   1 ')", 1);
    h.verifyFloat("val('1.0')", 1);
    h.verifyFloat("val('99.0')", 99);
    h.verifyFloat("val('-99.0')", -99);
    h.verifyFloat("val('.5')", 0.5);
    h.verifyFloat("val('1.')", 1.0);
    h.verifyFloat("val('   1.0 ')", 1);

    h.verifyNull("val('')");
    h.verifyNull("val('a')");

    h.verifyExecutionError("val(1)");
    h.verifyExecutionError("val(true)");

    h.verifyInteger("if(val('9'),4,3)", 4);
    h.verifyInteger("if(val('9');0,2,1)", 1);
    h.verifyCompileError("val('7'):=9");
}

/** Test Z/Zap function.
    Instruction: uzap */
void
TestInterpreterExprBuiltinFunction::testZap()
{
    // ex IntBuiltinTestSuite::testZap
    ExpressionVerifier h("testZap");
    // Null
    h.verifyNull("z(0)");
    h.verifyNull("z('')");
    h.verifyNull("z(0.0)");
    h.verifyNull("z(0.00000000000001)");
    h.verifyNull("z(false)");

    // Non-Null
    h.verifyInteger("z(1)", 1);
    h.verifyInteger("z(999)", 999);
    h.verifyString("z('a')", "a");
    h.verifyFloat("z(0.1)", 0.1);
    h.verifyBoolean("z(true)", true);

    // Null, using Zap
    h.verifyNull("zap(0)");
    h.verifyNull("zap('')");
    h.verifyNull("zap(0.0)");
    h.verifyNull("zap(0.00000000000001)");
    h.verifyNull("zap(false)");

    // Non-Null, using Zap
    h.verifyInteger("zap(1)", 1);
    h.verifyInteger("zap(999)", 999);
    h.verifyString("zap('a')", "a");
    h.verifyFloat("zap(0.1)", 0.1);
    h.verifyBoolean("zap(true)", true);

    // Variants
    h.verifyInteger("if(z(9),4,3)", 4);
    h.verifyInteger("if(z(9);0,2,1)", 1);
    h.verifyCompileError("z(7):=9");
}

/** Test Len function.
    Instruction: ulen */
void
TestInterpreterExprBuiltinFunction::testLen()
{
    // ex IntBuiltinTestSuite::testLen
    ExpressionVerifier h("testLen");
    h.verifyInteger("len('')", 0);
    h.verifyInteger("len('a')", 1);
    h.verifyInteger("len('foobar')", 6);
    h.verifyNull("len(z(0))");

    // FIXME: those yield 1 and 2, respectively, in PCC1:
    h.verifyExecutionError("len(2)");
    h.verifyExecutionError("len(12)");

    // Variants
    h.verifyInteger("if(len('x'),4,3)", 4);
    h.verifyInteger("if(len('');1,2,1)", 2);
    h.verifyCompileError("len(''):=9");
}

/** Test String/String$ function.
    Instruction: bstrmult */
void
TestInterpreterExprBuiltinFunction::testStrMult()
{
    // ex IntBuiltinTestSuite::testStrMult
    ExpressionVerifier h("testStrMult");
    // String
    h.verifyString("string(10)", "          ");
    h.verifyString("string(0)", "");
    h.verifyString("string(-1)", "");

    h.verifyString("string(10, '')", "");
    h.verifyString("string(10, 'a')", "aaaaaaaaaa");
    h.verifyString("string(5, 'ab')", "ababababab");

    h.verifyNull("string(z(0), 10)");
    h.verifyNull("string(10, z(0))");
    h.verifyNull("string(z(0), z(0))");
    h.verifyNull("string('a', z(0))");
    h.verifyNull("string(z(0))");

    h.verifyExecutionError("string('a', 10)");
    h.verifyExecutionError("string('a', 'b')");
    h.verifyExecutionError("string(1,2)");

    h.verifyInteger("if(string(10,'a'),9,8)", 9);
    h.verifyInteger("if(string(10,'a');0,9,8)", 8);
    h.verifyCompileError("string(10,'a'):='y'");

    h.verifyInteger("if(string(10),9,8)", 9);
    h.verifyInteger("if(string(10);0,9,8)", 8);
    h.verifyCompileError("string(10):='y'");

    // Same thing using String$
    h.verifyString("string$(10)", "          ");
    h.verifyString("string$(0)", "");
    h.verifyString("string$(-1)", "");

    h.verifyString("string$(10, '')", "");
    h.verifyString("string$(10, 'a')", "aaaaaaaaaa");
    h.verifyString("string$(5, 'ab')", "ababababab");

    h.verifyNull("string$(z(0), 10)");
    h.verifyNull("string$(10, z(0))");
    h.verifyNull("string$(z(0), z(0))");
    h.verifyNull("string$('a', z(0))");
    h.verifyNull("string$(z(0))");

    h.verifyExecutionError("string$('a', 10)");
    h.verifyExecutionError("string$('a', 'b')");
    h.verifyExecutionError("string$(1,2)");
}

/** Test StrCase function.
    Instructions affected by StrCase have already been tested elsewhere,
    so this tests that StrCase doesn't affect too much. */
void
TestInterpreterExprBuiltinFunction::testStrCase()
{
    // IntBuiltinTestSuite::testStrCase
    ExpressionVerifier h("testStrCase");
    h.verifyInteger("strcase(1+1)", 2);

    h.verifyInteger("strcase(instr('foo','O')) + instr('foo','O')", 2);
    h.verifyInteger("instr('foo','O') + strcase(instr('foo','O'))", 2);
    h.verifyInteger("strcase(instr('foo','O')) + strcase(instr('foo','O'))", 0);
    h.verifyInteger("instr('foo','O') + instr('foo','O')", 4);
    h.verifyInteger("strcase(instr('foo','O') + instr('foo','O'))", 0);
    h.verifyInteger("instr(strcase('foo'),'O')", 2);
    h.verifyInteger("instr('foo',strcase('O'))", 2);

    h.verifyInteger("if(strcase(1 or 2),3,4)", 3);

    h.verifyInteger("if(strcase(instr('foo','O') or instr('foo','O')),3,4)", 4);
    h.verifyInteger("if(strcase(instr('foo','O') or instr('foo','o')),3,4)", 3);
    h.verifyInteger("if(strcase(instr('foo','o') or instr('foo','O')),3,4)", 3);
    h.verifyInteger("strcase(if(instr('foo','O') or instr('foo','O'),3,4))", 4);

    h.verifyInteger("if('a' = 'A', 3, 4)", 3);
    h.verifyInteger("if('a' <> 'A', 3, 4)", 4);

    h.verifyInteger("if(strcase(instr('foo','O'));1,9,2)", 9);
    h.verifyCompileError("strcase('x'):=9");
}

/** Test atom functions: Atom, AtomStr.
    Instructions: uatom, uatomstr */
void
TestInterpreterExprBuiltinFunction::testAtom()
{
    // ex IntBuiltinTestSuite::testAtom
    ExpressionVerifier h("testAtom");
    h.verifyInteger("atom('')", 0);
    h.verifyString("atomstr(0)", "");

    h.verifyNull("atom(z(0))");
    h.verifyNull("atomstr(z(0))");

    h.verifyString("atomstr(atom(1))", "1");
    h.verifyString("atomstr(atom('haha'))", "haha");

    h.verifyInteger("if(atom('x'),3,5)", 3);
    h.verifyInteger("if(atom('');1,3,5)", 3);
    h.verifyCompileError("atom('y'):=3");

    h.verifyInteger("if(atomstr(atom('x')),3,5)", 3);
    h.verifyInteger("if(atomstr(0);1,3,5)", 3);
    h.verifyCompileError("atomstr(77):=3");
}

/** Test Eval function.
    Instructions: sevalx */
void
TestInterpreterExprBuiltinFunction::testEval()
{
    // ex IntBuiltinTestSuite::testEval
    ExpressionVerifier h("testEval");
    h.verifyInteger("eval(1)", 1);
    h.verifyInteger("eval('1')", 1);
    h.verifyInteger("eval('1+1')", 2);
    h.verifyInteger("eval('1;2;3')", 3);
    h.verifyNull("eval(z(0))");
    h.verifyNull("eval('z(0)')");
    h.verifyString("eval('\"foo\"')", "foo");

    h.verifyInteger("if(eval(1),2,3)", 2);
    h.verifyInteger("if(eval(0);1,2,3)", 2);
    h.verifyCompileError("eval(1):=2");

    // Two-argument forms
    // FIXME: cannot test the actual eval(expr,context) form yet
    h.verifyNull("eval('1',z(0))");
    h.verifyExecutionError("eval('1',1)");
}

/** Test miscellaneous. */
void
TestInterpreterExprBuiltinFunction::testMisc()
{
    ExpressionVerifier h("testMisc");

    // This does not execute because we don't have a user-defined function (but it compiles)
    h.verifyExecutionError("udf(9)");

    // This does not compile
    h.verifyCompileError("ByName('a')");
    h.verifyCompileError("ByName('a'):=2");
    h.verifyCompileError("If(ByName('a'),1,2)");
    h.verifyCompileError("If(ByName('a');1,1,2)");

    // Special error handling branch
    h.verifyParseError("a:=1;");
}

/** Test iteration functions (find/count). */
void
TestInterpreterExprBuiltinFunction::testIteration()
{
    /* Mock for an array element: returns a sequence VAL=1..10, with ID=10..100 */
    class ElementMock : public interpreter::Context, private interpreter::Context::ReadOnlyAccessor {
     public:
        ElementMock()
            : m_value(1)
            { }
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                if (name.match("VAL")) {
                    result = 1;
                    return this;
                } else if (name.match("ID")) {
                    result = 2;
                    return this;
                } else {
                    return 0;
                }
            }

        virtual bool next()
            {
                if (m_value < 10) {
                    ++m_value;
                    return true;
                } else {
                    return false;
                }
            }

        virtual Context* clone() const
            { return new ElementMock(*this); }
        virtual game::map::Object* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/)
            { }
        virtual afl::data::Value* get(PropertyIndex_t index)
            { return interpreter::makeIntegerValue(index == 1 ? m_value : m_value*10); }
        virtual String_t toString(bool /*readable*/) const
            { return "#<ElementMock>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { }
     private:
        int m_value;
    };

    /* Mock for an array */
    class ArrayMock : public interpreter::IndexableValue {
     public:
        virtual afl::data::Value* get(interpreter::Arguments& /*args*/)
            { throw interpreter::Error("not invokable"); }
        virtual void set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
            { throw interpreter::Error("not assignable"); }
        virtual int32_t getDimension(int32_t /*which*/) const
            { return 0; }
        virtual interpreter::Context* makeFirstContext()
            { return new ElementMock(); }
        virtual ArrayMock* clone() const
            { return new ArrayMock(); }
        virtual String_t toString(bool /*readable*/) const
            { return "#<ArrayMock>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { }

    };

    /* Provide a ArrayMock as CONT. */
    class ContainerMock : public interpreter::SingleContext, private interpreter::Context::ReadOnlyAccessor {
     public:
        ContainerMock()
            { }
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                if (name.match("CONT")) {
                    result = 1;
                    return this;
                } else {
                    return 0;
                }
            }
        virtual Context* clone() const
            { return new ContainerMock(); }
        virtual game::map::Object* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/)
            { }
        virtual afl::data::Value* get(PropertyIndex_t /*index*/)
            { return new ArrayMock(); }
        virtual String_t toString(bool /*readable*/) const
            { return "#<ContainerMock>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { }
    };

    ExpressionVerifier h("testIteration");
    h.setNewExtraContext(new ContainerMock());

    // Count
    // - 1 argument: count everything
    h.verifyInteger("Count(Cont)", 10);

    // - 2 arguments: match condition
    h.verifyInteger("Count(Cont, Val>3)", 7);
    h.verifyInteger("Count(Cont, Val<=3)", 3);
    h.verifyInteger("Count(Cont, Val>30)", 0);

    // - errors
    h.verifyExecutionError("Count(1)");
    h.verifyParseError("Count()");
    h.verifyParseError("Count(Cont, 1, 2)");

    // - varying compilation context
    h.verifyCompileError("Count(Cont, Val>3) := 2");
    h.verifyInteger("Count(Cont, Val>3); 9", 9);
    h.verifyInteger("If(Count(Cont, Val>3), 33, 44)", 33);

    // Find
    // - requires 3 arguments
    h.verifyInteger("Find(Cont, Val=4, Id)", 40);
    h.verifyInteger("Find(Cont, True, Id)", 10);
    h.verifyNull("Find(Cont, Val=99, Id)");
    h.verifyNull("Find(Cont, False, Id)");

    // - Errors
    h.verifyParseError("Find(Cont, Val=4)");  // might someday become legal?
    h.verifyParseError("Find(Cont, Val=4, X, Y)");
    h.verifyParseError("Find()");

    // - Invalid name in the 'return' position is not fatal if we don't find anything
    h.verifyNull("Find(Cont, False, Whatever)");
    h.verifyExecutionError("Find(Cont, True, Whatever)");

    // - varying compilation context
    h.verifyCompileError("Find(Cont, Val=4, Id) := 2");
    h.verifyInteger("Find(Cont, Val=4, Id); 77", 77);
    h.verifyInteger("Find(Cont, Val=99, Id); 77", 77);
    h.verifyInteger("If(Find(Cont, Val=4, Id), 55, 66)", 55);
    h.verifyInteger("If(Find(Cont, Val=99, Id), 55, 66)", 66);
    h.verifyInteger("If(Find(Cont, Val=4, 7), 55, 66)", 55);       // constant in 'return' position is handled specially
    h.verifyInteger("If(Find(Cont, Val=99, 7), 55, 66)", 66);
}

/** Test Key() function. */
void
TestInterpreterExprBuiltinFunction::testKey()
{
    // Prepare a keymap
    ExpressionVerifier h("testKey");
    std::auto_ptr<util::Keymap> kk(new util::Keymap("KK"));
    kk->addKey('x', 44, 55);
    h.setNewExtraKeymap(kk.release());

    // Success cases
    // - bound
    h.verifyInteger("Key(KK, 'x')", 44);
    h.verifyInteger("Key(ByName(String(2, 'k')), 'x')", 44);

    // - not bound
    h.verifyNull("Key(KK, 'y')");
    h.verifyNull("Key(ByName(String(2, 'k')), 'y')");

    // - null keymap
    h.verifyNull("Key(ByName(Z(0)), 'x')");

    // - null key
    h.verifyNull("Key(KK, Z(0))");

    // - codegen variations
    h.verifyInteger("If(Key(KK, 'x'), 111, 222)", 111);
    h.verifyInteger("If(Key(KK, 'y'), 111, 222)", 222);
    h.verifyInteger("Key(KK, 'x'); 111", 111);

    // Errors
    // - wrong keymap
    h.verifyCompileError("Key(Z(0), 'x')");
    h.verifyCompileError("Key(9, 'x')");

    // - wrong arity
    h.verifyParseError("Key(KK)");
    h.verifyParseError("Key(KK, 'x', 'y')");

    // - invalid key
    h.verifyExecutionError("Key(KK, 'whatwhatwhat')");
}

