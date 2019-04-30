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
#include "t_interpreter.hpp"

/** Test trig functions: Sin, Cos, Tan, ATan.
    Instructions: ucos, usin, utan, batan */
void
TestInterpreterExprBuiltinFunction::testTrig()
{
    // ex IntBuiltinTestSuite::testTrig
    ExpressionTestHelper h;
    // Sin
    h.checkFloatExpression("sin(0)", 0);
    h.checkFloatExpression("sin(90)", 1);
    h.checkFloatExpression("sin(180)", 0);
    h.checkFloatExpression("sin(270)", -1);
    h.checkFloatExpression("sin(0.0)", 0);
    h.checkFloatExpression("sin(90.0)", 1);
    h.checkFloatExpression("sin(180.0)", 0);
    h.checkFloatExpression("sin(270.0)", -1);
    h.checkNullExpression("sin(z(0))");
    h.checkFailureExpression("sin('a')");
    h.checkFailureExpression("sin('')");
    h.checkIntegerExpression("if(sin(90),3,2)",3);      // used as condition
    h.checkIntegerExpression("if(sin(0);1,3,2)",3);     // used as effect
    h.checkBadExpression("sin(0):='x'");                // used as assigment target

    // Cos
    h.checkFloatExpression("cos(0)", 1);
    h.checkFloatExpression("cos(90)", 0);
    h.checkFloatExpression("cos(180)", -1);
    h.checkFloatExpression("cos(270)", 0);
    h.checkFloatExpression("cos(0.0)", 1);
    h.checkFloatExpression("cos(90.0)", 0);
    h.checkFloatExpression("cos(180.0)", -1);
    h.checkFloatExpression("cos(270.0)", 0);
    h.checkNullExpression("cos(z(0))");
    h.checkFailureExpression("cos('a')");
    h.checkFailureExpression("cos('')");
    h.checkIntegerExpression("if(cos(0),3,2)",3);       // used as condition
    h.checkIntegerExpression("if(cos(90);1,3,2)",3);    // used as effect
    h.checkBadExpression("cos(0):='x'");                // used as assigment target

    // Tan
    h.checkFloatExpression("tan(0)", 0);
    h.checkFloatExpression("tan(45)", 1);
    h.checkFailureExpression("tan(90)");
    h.checkFloatExpression("tan(135)", -1);
    h.checkFloatExpression("tan(180)", 0);
    h.checkFailureExpression("tan(270)");
    h.checkFloatExpression("tan(0.0)", 0);
    h.checkFloatExpression("tan(45.0)", 1);
    h.checkFailureExpression("tan(90.0)");
    h.checkFloatExpression("tan(135.0)", -1);
    h.checkFloatExpression("tan(180.0)", 0);
    h.checkFailureExpression("tan(270.0)");
    h.checkNullExpression("tan(z(0))");
    h.checkFailureExpression("tan('a')");
    h.checkFailureExpression("tan('')");
    h.checkIntegerExpression("if(tan(45),3,2)",3);      // used as condition
    h.checkIntegerExpression("if(tan(0);1,3,2)",3);     // used as effect
    h.checkBadExpression("tan(0):='x'");                // used as assigment target

    // Atan
    h.checkFloatExpression("atan(1)", 45);
    h.checkFloatExpression("atan(-1)", 315);
    h.checkFloatExpression("atan(1.0)", 45);
    h.checkFloatExpression("atan(-1.0)", 315);

    h.checkFloatExpression("atan(1,1)", 45);
    h.checkFloatExpression("atan(-1,1)", 315);
    h.checkFloatExpression("atan(1,-1)", 135);
    h.checkFloatExpression("atan(-1,-1)", 225);

    h.checkFloatExpression("atan(1.0,1.0)", 45);
    h.checkFloatExpression("atan(-1.0,1.0)", 315);
    h.checkFloatExpression("atan(1.0,-1.0)", 135);
    h.checkFloatExpression("atan(-1.0,-1.0)", 225);

    h.checkFloatExpression("atan(0)", 0);
    h.checkFloatExpression("atan(1, 0)", 90);
    h.checkFloatExpression("atan(0, 1)", 0);
    h.checkFloatExpression("atan(-1, 0.0)", 270);
    h.checkFloatExpression("atan(0, -1.0)", 180);

    h.checkFloatExpression("atan(sin(25), cos(25))", 25);

    h.checkNullExpression("atan(1, z(0))");
    h.checkNullExpression("atan(z(0))");
    h.checkNullExpression("atan(z(0), 1)");

    h.checkFailureExpression("atan('a')");
    h.checkFailureExpression("atan(1,'a')");
    h.checkFailureExpression("atan('a',1)");

    h.checkIntegerExpression("if(atan(1,0),3,2)",3);    // used as condition
    h.checkIntegerExpression("if(atan(0,1);1,3,2)",3);  // used as effect
    h.checkBadExpression("atan(0,1):='x'");             // used as assigment target
}

/** Test Abs function.
    Instruction: uabs */
void
TestInterpreterExprBuiltinFunction::testAbs()
{
    // ex IntBuiltinTestSuite::testAbs
    ExpressionTestHelper h;
    h.checkIntegerExpression("abs(1)", 1);
    h.checkIntegerExpression("abs(-1)", 1);
    h.checkIntegerExpression("abs(true)", 1);
    h.checkIntegerExpression("abs(0)", 0);
    h.checkIntegerExpression("abs(-1111111111)", 1111111111);
    h.checkIntegerExpression("abs(+1111111111)", 1111111111);

    h.checkFloatExpression("abs(1.0)", 1.0);
    h.checkFloatExpression("abs(-1.0)", 1.0);
    h.checkFloatExpression("abs(-9999.0)", 9999.0);
    h.checkFloatExpression("abs(pi)", 3.14159265);
    h.checkFloatExpression("abs(-pi)", 3.14159265);
    h.checkFloatExpression("abs(7777777.0)", 7777777.0);

    h.checkNullExpression("abs(z(0))");

    h.checkFailureExpression("abs('a')");
    h.checkFailureExpression("abs('')");

    h.checkIntegerExpression("if(abs(1),9,0)", 9);    // used as condition
    h.checkIntegerExpression("if(abs(0);1,9,0)", 9);  // used as effect
    h.checkBadExpression("abs(0):=2");                // used as assigment target
}

/** Test Asc function.
    Instruction: uasc */
void
TestInterpreterExprBuiltinFunction::testAsc()
{
    // ex IntBuiltinTestSuite::testAsc
    ExpressionTestHelper h;
    h.checkIntegerExpression("asc('a')", 97);
    h.checkIntegerExpression("asc('abcdef')", 97);
    h.checkIntegerExpression("asc(0)", 48);
    h.checkIntegerExpression("asc(2.5)", 50);
    h.checkNullExpression("asc('')");
    h.checkNullExpression("asc(z(0))");

    h.checkIntegerExpression("if(asc('a'),9,0)", 9);    // used as condition
    h.checkIntegerExpression("if(asc('x');0,0,9)", 9);  // used as effect
    h.checkBadExpression("asc('0'):=2");                // used as assigment target
}

/** Test bit operations.
    Instructions: bbitand, bbitor, bbitxor, ubitnot */
void
TestInterpreterExprBuiltinFunction::testBitOps()
{
    // ex IntBuiltinTestSuite::testBitOps
    ExpressionTestHelper h;
    // BitAnd
    h.checkIntegerExpression("bitand(1)", 1);
    h.checkIntegerExpression("bitand(7,85)", 5);
    h.checkIntegerExpression("bitand(4,4,4)", 4);
    h.checkIntegerExpression("bitand(4,4,0)", 0);
    h.checkIntegerExpression("bitand(true,true)", 1);
    h.checkNullExpression("bitand(z(0))");
    h.checkNullExpression("bitand(z(0),1)");
    h.checkNullExpression("bitand(1,z(0))");
    h.checkNullExpression("bitand(1,1,z(0))");
    h.checkNullExpression("bitand(z(0),1,1)");
    h.checkFailureExpression("bitand('a')");
    h.checkFailureExpression("bitand(1,'a')");
    h.checkFailureExpression("bitand('a',1)");
    h.checkIntegerExpression("if(bitand(2,3,7),7,9)", 7);
    h.checkIntegerExpression("if(bitand(1,2,4);1,7,9)", 7);
    h.checkBadExpression("bitand(1,2):=3");

    // BitOr
    h.checkIntegerExpression("bitor(1)", 1);
    h.checkIntegerExpression("bitor(7,85)", 87);
    h.checkIntegerExpression("bitor(4,4,4)", 4);
    h.checkIntegerExpression("bitor(4,4,0)", 4);
    h.checkIntegerExpression("bitor(true,false)", 1);
    h.checkNullExpression("bitor(z(0))");
    h.checkNullExpression("bitor(z(0),1)");
    h.checkNullExpression("bitor(1,z(0))");
    h.checkNullExpression("bitor(1,1,z(0))");
    h.checkNullExpression("bitor(z(0),1,1)");
    h.checkFailureExpression("bitor('a')");
    h.checkFailureExpression("bitor(1,'a')");
    h.checkFailureExpression("bitor('a',1)");
    h.checkIntegerExpression("if(bitor(2,3,5),7,9)", 7);
    h.checkIntegerExpression("if(bitor(1,2,4);0,7,9)", 9);
    h.checkBadExpression("bitor(1,2):=3");

    // BitXor
    h.checkIntegerExpression("bitxor(1)", 1);
    h.checkIntegerExpression("bitxor(7,85)", 82);
    h.checkIntegerExpression("bitxor(4,4,4)", 4);
    h.checkIntegerExpression("bitxor(4,4,0)", 0);
    h.checkIntegerExpression("bitxor(true,true)", 0);
    h.checkNullExpression("bitxor(z(0))");
    h.checkNullExpression("bitxor(z(0),1)");
    h.checkNullExpression("bitxor(1,z(0))");
    h.checkNullExpression("bitxor(1,1,z(0))");
    h.checkNullExpression("bitxor(z(0),1,1)");
    h.checkFailureExpression("bitxor('a')");
    h.checkFailureExpression("bitxor(1,'a')");
    h.checkFailureExpression("bitxor('a',1)");
    h.checkIntegerExpression("if(bitxor(2,3,5),7,9)", 7);
    h.checkIntegerExpression("if(bitxor(1,2,4);0,7,9)", 9);
    h.checkBadExpression("bitxor(1,2):=3");

    // BitNot
    h.checkIntegerExpression("bitnot(1)", -2);
    h.checkIntegerExpression("bitnot(0)", -1);
    h.checkIntegerExpression("bitnot(true)", -2);
    h.checkNullExpression("bitnot(z(0))");
    h.checkFailureExpression("bitnot('a')");
    h.checkIntegerExpression("if(bitnot(0),2,3)", 2);
    h.checkIntegerExpression("if(bitnot(-1);1,3,5)", 3);
    h.checkBadExpression("bitnot(9):=4");
}

/** Test Min/Max operators.
    Instructions: bmin, bmax, and NC versions thereof */
void
TestInterpreterExprBuiltinFunction::testMinMax()
{
    // ex IntBuiltinTestSuite::testMinMax
    ExpressionTestHelper h;
    // Integers
    h.checkIntegerExpression("min(1)", 1);
    h.checkIntegerExpression("min(3,1,4,1,5)", 1);
    h.checkIntegerExpression("min(99,-22,22)", -22);
    h.checkIntegerExpression("max(1)", 1);
    h.checkIntegerExpression("max(3,1,4,1,5)", 5);
    h.checkIntegerExpression("max(99,-22,22)", 99);
    h.checkNullExpression("min(1,z(0))");
    h.checkNullExpression("min(z(0),1)");
    h.checkNullExpression("min(z(0))");
    h.checkNullExpression("max(1,z(0))");
    h.checkNullExpression("max(z(0),1)");
    h.checkNullExpression("max(z(0))");
    h.checkIntegerExpression("if(max(-1,0),99,22)", 22);
    h.checkIntegerExpression("if(max(-1,0);1,99,22)", 99);
    h.checkBadExpression("max(1,2):=3");

    // Floats and Mixes
    // Note that min/max do not lift their argument to a common type before returning it!
    h.checkFloatExpression("min(1.0)", 1);
    h.checkFloatExpression("min(3.1, 4.1, 5.9, 2.6)", 2.6);
    h.checkIntegerExpression("min(99.0,-22,22)", -22);
    h.checkFloatExpression("max(1.0)", 1);
    h.checkFloatExpression("max(3.1, 4.1, 5.9, 2.6)", 5.9);
    h.checkFloatExpression("max(99.0,-22,22)", 99);
    h.checkNullExpression("min(1.0,z(0))");
    h.checkNullExpression("min(z(0),1.0)");
    h.checkNullExpression("min(z(0))");
    h.checkNullExpression("max(1.0,z(0))");
    h.checkNullExpression("max(z(0),1.0)");
    h.checkNullExpression("max(z(0))");
    h.checkIntegerExpression("if(min(-1,0),99,22)", 99);
    h.checkIntegerExpression("if(min(-1,0);0,99,22)", 22);
    h.checkBadExpression("min(1,2):=3");

    // Same things with StrCase, to exercise NC versions
    h.checkIntegerExpression("strcase(min(1))", 1);
    h.checkIntegerExpression("strcase(min(3,1,4,1,5))", 1);
    h.checkIntegerExpression("strcase(min(99,-22,22))", -22);
    h.checkIntegerExpression("strcase(max(1))", 1);
    h.checkIntegerExpression("strcase(max(3,1,4,1,5))", 5);
    h.checkIntegerExpression("strcase(max(99,-22,22))", 99);

    h.checkFloatExpression("strcase(min(1.0))", 1);
    h.checkFloatExpression("strcase(min(3.1, 4.1, 5.9, 2.6))", 2.6);
    h.checkIntegerExpression("strcase(min(99.0,-22,22))", -22);
    h.checkFloatExpression("strcase(max(1.0))", 1);
    h.checkFloatExpression("strcase(max(3.1, 4.1, 5.9, 2.6))", 5.9);
    h.checkFloatExpression("strcase(max(99.0,-22,22))", 99);

    h.checkNullExpression("strcase(min(1.0,z(0)))");
    h.checkNullExpression("strcase(min(z(0),1.0))");
    h.checkNullExpression("strcase(min(z(0)))");
    h.checkNullExpression("strcase(max(1.0,z(0)))");
    h.checkNullExpression("strcase(max(z(0),1.0))");
    h.checkNullExpression("strcase(max(z(0)))");
    h.checkIntegerExpression("strcase(if(max(-1,0),99,22))", 22);
    h.checkIntegerExpression("if(strcase(max(-1,0));1,99,22)", 99);
    h.checkBadExpression("strcase(max(1,2)):=3");

    // Strings
    h.checkStringExpression("min('h','a','l','l','o')", "a");
    h.checkStringExpression("max('h','a','l','l','o')", "o");
    h.checkStringExpression("min('H','a','L','l','O')", "a");
    h.checkStringExpression("max('H','a','L','l','O')", "O");

    h.checkStringExpression("strcase(min('h','a','l','l','o'))", "a");
    h.checkStringExpression("strcase(max('h','a','l','l','o'))", "o");
    h.checkStringExpression("strcase(min('H','a','L','l','O'))", "H");
    h.checkStringExpression("strcase(max('H','a','L','l','O'))", "l");

    h.checkNullExpression("min('a',z(0))");
    h.checkNullExpression("min(z(0),'a')");
    h.checkNullExpression("strcase(min('a',z(0)))");
    h.checkNullExpression("strcase(min(z(0),'a'))");
    h.checkIntegerExpression("strcase(if(min(-1,0),99,22))", 99);
    h.checkIntegerExpression("if(strcase(min(-1,0));0,99,22)", 22);
    h.checkBadExpression("strcase(min(1,2):=3)");
}

/** Test Chr/Chr$ function (two names for the same function).
    Instructions: uchr */
void
TestInterpreterExprBuiltinFunction::testChr()
{
    // ex IntBuiltinTestSuite::testChr
    ExpressionTestHelper h;
    h.checkStringExpression("chr(1)", "\001");
    h.checkStringExpression("chr(97)", "a");
    h.checkNullExpression("chr(z(0))");
    h.checkIntegerExpression("len(chr(0))", 1);
    h.checkFailureExpression("chr('a')");
    h.checkStringExpression("chr(128)", "\xC2\x80");
    h.checkStringExpression("chr(57665)", "\xEE\x85\x81");

    h.checkStringExpression("chr$(1)", "\001");
    h.checkStringExpression("chr$(97)", "a");
    h.checkNullExpression("chr$(z(0))");
    h.checkIntegerExpression("len(chr$(0))", 1);
    h.checkFailureExpression("chr$('a')");
    h.checkStringExpression("chr$(128)", "\xC2\x80");
    h.checkStringExpression("chr$(57665)", "\xEE\x85\x81");

    h.checkIntegerExpression("if(chr(99),3,4)", 3);
    h.checkIntegerExpression("if(chr(77);0,3,4)", 4);
    h.checkBadExpression("chr(88):='x'");
}

/** Test type tests: IsEmpty, IsNum, IsString.
    Instructions: uisempty, uisnum, uisstr */
void
TestInterpreterExprBuiltinFunction::testTypeChecks()
{
    // ex IntBuiltinTestSuite::testTypeChecks
    ExpressionTestHelper h;
    h.checkBooleanExpression("isempty(0)", false);
    h.checkBooleanExpression("isempty(1)", false);
    h.checkBooleanExpression("isempty(0.0)", false);
    h.checkBooleanExpression("isempty(1.0)", false);
    h.checkBooleanExpression("isempty(true)", false);
    h.checkBooleanExpression("isempty(false)", false);
    h.checkBooleanExpression("isempty('')", false);
    h.checkBooleanExpression("isempty('foo')", false);
    h.checkBooleanExpression("isempty(z(0))", true);
    h.checkBooleanExpression("isempty(zap(0))", true);
    h.checkIntegerExpression("if(isempty(''),0,9)", 9);
    h.checkIntegerExpression("if(isempty('');0,9,0)", 0);
    h.checkBadExpression("isempty(''):=1");

    h.checkBooleanExpression("isnum(0)", true);
    h.checkBooleanExpression("isnum(1)", true);
    h.checkBooleanExpression("isnum(0.0)", true);
    h.checkBooleanExpression("isnum(1.0)", true);
    h.checkBooleanExpression("isnum(true)", true);
    h.checkBooleanExpression("isnum(false)", true);
    h.checkBooleanExpression("isnum('')", false);
    h.checkBooleanExpression("isnum('foo')", false);
    h.checkBooleanExpression("isnum(z(0))", false);
    h.checkBooleanExpression("isnum(zap(0))", false);
    h.checkIntegerExpression("if(isnum(7),9,0)", 9);
    h.checkIntegerExpression("if(isnum(7);0,9,0)", 0);
    h.checkBadExpression("isnum(7):=1");

    h.checkBooleanExpression("isstring(0)", false);
    h.checkBooleanExpression("isstring(1)", false);
    h.checkBooleanExpression("isstring(0.0)", false);
    h.checkBooleanExpression("isstring(1.0)", false);
    h.checkBooleanExpression("isstring(true)", false);
    h.checkBooleanExpression("isstring(false)", false);
    h.checkBooleanExpression("isstring('')", true);
    h.checkBooleanExpression("isstring('foo')", true);
    h.checkBooleanExpression("isstring(z(0))", false);
    h.checkBooleanExpression("isstring(zap(0))", false);
    h.checkIntegerExpression("if(isstring(''),9,0)", 9);
    h.checkIntegerExpression("if(isstring('');0,9,0)", 0);
    h.checkBadExpression("isstring(''):=1");
}

/** Test Exp/Log.
    Instructions: uexp, ulog */
void
TestInterpreterExprBuiltinFunction::testExp()
{
    // ex IntBuiltinTestSuite::testExp
    ExpressionTestHelper h;
    h.checkFloatExpression("exp(-1)", 1/2.7172);
    h.checkFloatExpression("exp(0)", 1);
    h.checkFloatExpression("exp(1)", 2.7172);
    h.checkFloatExpression("exp(2)", 2.7172*2.7172);

    h.checkFloatExpression("exp(-1.0)", 1/2.7172);
    h.checkFloatExpression("exp(0.0)", 1);
    h.checkFloatExpression("exp(1.0)", 2.7172);
    h.checkFloatExpression("exp(2.0)", 2.7172*2.7172);

    h.checkFloatExpression("log(2.7172*2.7172)", 2);
    h.checkFloatExpression("log(2.7172)", 1);
    h.checkFloatExpression("log(1)", 0);

    h.checkFloatExpression("exp(log(10000))", 10000);
    h.checkFloatExpression("log(exp(10))", 10);
    // This fails because exp(10000) overflows, yielding NaN. CxxTest then fails
    // to convert the offending value into a string. D'oh.
    // checkFloatExpression("log(exp(10000))", 10000);

    h.checkFailureExpression("log(0)");
    h.checkFailureExpression("log('a')");
    h.checkFailureExpression("exp('a')");
    h.checkNullExpression("log(z(0))");
    h.checkNullExpression("exp(z(0))");

    h.checkIntegerExpression("if(log(99),9,0)", 9);
    h.checkIntegerExpression("if(log(1);1,9,0)", 9);
    h.checkBadExpression("log(1):=2");

    h.checkIntegerExpression("if(exp(0),9,0)", 9);
    h.checkIntegerExpression("if(exp(1);0,0,9)", 9);
    h.checkBadExpression("exp(1):=2");
}

/** Test substring-finding routines: InStr, First, Rest.
    Instructions: bfindstr, bfirststr, breststr, and NC versions thereof. */
void
TestInterpreterExprBuiltinFunction::testStrFind()
{
    // ex IntBuiltinTestSuite::testStrFind
    ExpressionTestHelper h;
    // InStr
    h.checkIntegerExpression("instr('foobar', 'o')", 2);
    h.checkIntegerExpression("instr('foobar', 'O')", 2);
    h.checkIntegerExpression("instr('foobar', 'oO')", 2);
    h.checkIntegerExpression("instr('quuxUUM', 'UU')", 2);
    h.checkIntegerExpression("instr('foo', 'bar')", 0);
    h.checkIntegerExpression("instr('foobar', 'foo')", 1);
    h.checkIntegerExpression("instr('foo', 'foobar')", 0);
    h.checkIntegerExpression("instr('foobar', 'b')", 4);
    h.checkIntegerExpression("if(instr('foobar','b'),8,2)", 8);
    h.checkIntegerExpression("if(instr('foobar','x');1,8,2)", 8);
    h.checkBadExpression("instr('foobar','b'):=0");

    h.checkIntegerExpression("strcase(instr('foobar', 'o'))", 2);
    h.checkIntegerExpression("strcase(instr('foobar', 'O'))", 0);
    h.checkIntegerExpression("strcase(instr('foobar', 'oO'))", 0);
    h.checkIntegerExpression("strcase(instr('quuxUUM', 'UU'))", 5);
    h.checkIntegerExpression("strcase(instr('foo', 'bar'))", 0);
    h.checkIntegerExpression("strcase(instr('foobar', 'foo'))", 1);
    h.checkIntegerExpression("strcase(instr('foo', 'foobar'))", 0);
    h.checkIntegerExpression("strcase(instr('foobar', 'b'))", 4);
    h.checkIntegerExpression("if(strcase(instr('foobar','b')),8,2)", 8);
    h.checkIntegerExpression("if(strcase(instr('foobar','x'));1,8,2)", 8);
    h.checkBadExpression("strcase(instr('foobar','b'):=0)");

    // First - different order of parameters!
    h.checkStringExpression("first('o', 'foobar')", "f");
    h.checkStringExpression("first('O', 'foobar')", "f");
    h.checkStringExpression("first('oO', 'foobar')", "f");
    h.checkStringExpression("first('UU', 'quuxUUM')", "q");
    h.checkStringExpression("first('bar', 'foo')", "foo");
    h.checkStringExpression("first('foo', 'foobar')", "");
    h.checkStringExpression("first('foobar', 'foo')", "foo");
    h.checkStringExpression("first('b', 'foobar')", "foo");
    h.checkIntegerExpression("if(first('b','foobar'),8,2)", 8);
    h.checkIntegerExpression("if(first('x','foobar');0,8,2)", 2);
    h.checkBadExpression("first('b','foobar'):=0");

    h.checkStringExpression("strcase(first('o', 'foobar'))", "f");
    h.checkStringExpression("strcase(first('O', 'foobar'))", "foobar");
    h.checkStringExpression("strcase(first('oO', 'foobar'))", "foobar");
    h.checkStringExpression("strcase(first('UU', 'quuxUUM'))", "quux");
    h.checkStringExpression("strcase(first('bar', 'foo'))", "foo");
    h.checkStringExpression("strcase(first('foo', 'foobar'))", "");
    h.checkStringExpression("strcase(first('foobar', 'foo'))", "foo");
    h.checkStringExpression("strcase(first('b', 'foobar'))", "foo");
    h.checkIntegerExpression("if(strcase(first('b','foobar')),8,2)", 8);
    h.checkIntegerExpression("if(strcase(first('x','foobar'));0,8,2)", 2);
    h.checkBadExpression("strcase(first('b','foobar')):=0");

    // Rest
    h.checkStringExpression("rest('o', 'foobar')", "obar");
    h.checkStringExpression("rest('O', 'foobar')", "obar");
    h.checkStringExpression("rest('oO', 'foobar')", "bar");
    h.checkStringExpression("rest('UU', 'quuxUUM')", "xUUM");
    h.checkNullExpression("rest('bar', 'foo')");
    h.checkStringExpression("rest('foo', 'foobar')", "bar");
    h.checkNullExpression("rest('foobar', 'foo')");
    h.checkStringExpression("rest('b', 'foobar')", "ar");

    h.checkStringExpression("strcase(rest('o', 'foobar'))", "obar");
    h.checkNullExpression("strcase(rest('O', 'foobar'))");
    h.checkNullExpression("strcase(rest('oO', 'foobar'))");
    h.checkStringExpression("strcase(rest('UU', 'quuxUUM'))", "M");
    h.checkNullExpression("strcase(rest('bar', 'foo'))");
    h.checkStringExpression("strcase(rest('foo', 'foobar'))", "bar");
    h.checkNullExpression("strcase(rest('foobar', 'foo'))");
    h.checkStringExpression("strcase(rest('b', 'foobar'))", "ar");
    h.checkIntegerExpression("if(rest('b','foobar'),8,2)", 8);
    h.checkIntegerExpression("if(rest('x','foobar');1,8,2)", 8);
    h.checkBadExpression("rest('b','foobar'):=0");

    // Null
    h.checkNullExpression("instr(z(0),'a')");
    h.checkNullExpression("instr('a',z(0))");
    h.checkNullExpression("instr(z(0),z(0))");
    h.checkNullExpression("first(z(0),'a')");
    h.checkNullExpression("first('a',z(0))");
    h.checkNullExpression("first(z(0),z(0))");
    h.checkNullExpression("rest(z(0),'a')");
    h.checkNullExpression("rest('a',z(0))");
    h.checkNullExpression("rest(z(0),z(0))");
    h.checkIntegerExpression("if(strcase(rest('b','foobar')),8,2)", 8);
    h.checkIntegerExpression("if(strcase(rest('x','foobar'));1,8,2)", 8);
    h.checkBadExpression("strcase(rest('b','foobar')):=0");

    // Type errors
    // FIXME: should these really be type errors, or should we implicitly stringify?
    h.checkFailureExpression("instr('a', 1)");
    h.checkFailureExpression("instr(1, 'a')");
    h.checkFailureExpression("instr(1, 2)");
    h.checkFailureExpression("first('a', 1)");
    h.checkFailureExpression("first(1, 'a')");
    h.checkFailureExpression("first(1, 2)");
    h.checkFailureExpression("rest('a', 1)");
    h.checkFailureExpression("rest(1, 'a')");
    h.checkFailureExpression("rest(1, 2)");
}

/** Test substring operations: Mid, Left, Right.
    Instructions: blcut, brcut, bendcut */
void
TestInterpreterExprBuiltinFunction::testSubstr()
{
    // ex IntBuiltinTestSuite::testSubstr
    ExpressionTestHelper h;
    // Mid, 2-arg
    h.checkStringExpression("mid('foobar',-2)", "foobar");
    h.checkStringExpression("mid('foobar',-1)", "foobar");
    h.checkStringExpression("mid('foobar',0)", "foobar");
    h.checkStringExpression("mid('foobar',1)", "foobar");
    h.checkStringExpression("mid('foobar',2)", "oobar");
    h.checkStringExpression("mid('foobar',3)", "obar");
    h.checkStringExpression("mid('foobar',4)", "bar");
    h.checkStringExpression("mid('foobar',5)", "ar");
    h.checkStringExpression("mid('foobar',6)", "r");
    h.checkStringExpression("mid('foobar',7)", "");
    h.checkStringExpression("mid('foobar',8)", "");

    h.checkNullExpression("mid(z(0),1)");
    h.checkNullExpression("mid(z(0),z(0))");
    h.checkNullExpression("mid('foo',z(0))");

    h.checkIntegerExpression("if(mid('foobar',3),8,2)",8);
    h.checkIntegerExpression("if(mid('foobar',30);1,8,2)",8);
    h.checkBadExpression("mid('foobar',2):='a'");                 // FIXME: This may get legalized someday

    // Mid, 3-arg
    h.checkStringExpression("mid('foobar',-2,3)", "foo");
    h.checkStringExpression("mid('foobar',-1,3)", "foo");
    h.checkStringExpression("mid('foobar',0,3)", "foo");
    h.checkStringExpression("mid('foobar',1,3)", "foo");
    h.checkStringExpression("mid('foobar',2,3)", "oob");
    h.checkStringExpression("mid('foobar',3,3)", "oba");
    h.checkStringExpression("mid('foobar',4,3)", "bar");
    h.checkStringExpression("mid('foobar',5,3)", "ar");
    h.checkStringExpression("mid('foobar',6,3)", "r");
    h.checkStringExpression("mid('foobar',7,3)", "");
    h.checkStringExpression("mid('foobar',8,3)", "");

    h.checkStringExpression("mid('foobar',2,0)", "");

    h.checkNullExpression("mid(z(0),1,1)");
    h.checkNullExpression("mid(z(0),z(0),1)");
    h.checkNullExpression("mid('foo',z(0),1)");
    h.checkNullExpression("mid(z(0),1,z(0))");
    h.checkNullExpression("mid(z(0),z(0),z(0))");
    h.checkNullExpression("mid('foo',z(0),z(0))");
    h.checkNullExpression("mid('foo',1,z(0))");

    h.checkIntegerExpression("if(mid('foobar',3,1),8,2)",8);
    h.checkIntegerExpression("if(mid('foobar',30,1);1,8,2)",8);
    h.checkBadExpression("mid('foobar',2,1):='a'");                 // FIXME: This may get legalized someday

    // Left
    h.checkStringExpression("left('foobar',-3)", "");
    h.checkStringExpression("left('foobar',-2)", "");
    h.checkStringExpression("left('foobar',-1)", "");
    h.checkStringExpression("left('foobar',0)", "");
    h.checkStringExpression("left('foobar',1)", "f");
    h.checkStringExpression("left('foobar',2)", "fo");
    h.checkStringExpression("left('foobar',3)", "foo");
    h.checkStringExpression("left('foobar',4)", "foob");
    h.checkStringExpression("left('foobar',5)", "fooba");
    h.checkStringExpression("left('foobar',6)", "foobar");
    h.checkStringExpression("left('foobar',7)", "foobar");
    h.checkStringExpression("left('foobar',8)", "foobar");

    h.checkNullExpression("left(z(0),1)");
    h.checkNullExpression("left(z(0),z(0))");
    h.checkNullExpression("left('foo',z(0))");

    h.checkIntegerExpression("if(left('foobar',3),8,2)",8);
    h.checkIntegerExpression("if(left('foobar',30);0,8,2)",2);
    h.checkBadExpression("left('foobar',2):='a'");                 // FIXME: This may get legalized someday

    // Right
    h.checkStringExpression("right('foobar',-3)", "");
    h.checkStringExpression("right('foobar',-2)", "");
    h.checkStringExpression("right('foobar',-1)", "");
    h.checkStringExpression("right('foobar',0)", "");
    h.checkStringExpression("right('foobar',1)", "r");
    h.checkStringExpression("right('foobar',2)", "ar");
    h.checkStringExpression("right('foobar',3)", "bar");
    h.checkStringExpression("right('foobar',4)", "obar");
    h.checkStringExpression("right('foobar',5)", "oobar");
    h.checkStringExpression("right('foobar',6)", "foobar");
    h.checkStringExpression("right('foobar',7)", "foobar");
    h.checkStringExpression("right('foobar',8)", "foobar");

    h.checkNullExpression("right(z(0),1)");
    h.checkNullExpression("right(z(0),z(0))");
    h.checkNullExpression("right('foo',z(0))");

    h.checkIntegerExpression("if(right('foobar',3),8,2)",8);
    h.checkIntegerExpression("if(right('foobar',30);0,8,2)",2);
    h.checkBadExpression("right('foobar',2):='a'");                 // FIXME: This may get legalized someday

    // Type errors
    h.checkFailureExpression("mid(10,1,1)");
    h.checkFailureExpression("mid('foo','1','1')");
    h.checkFailureExpression("mid('foo','1')");

    h.checkFailureExpression("left(10, 1)");
    h.checkFailureExpression("left('foo', '1')");

    h.checkFailureExpression("right(10, 1)");
    h.checkFailureExpression("right('foo', '1')");
}

/** Test trim functions: Trim, LTrim, RTrim.
    Instructions: ulrtrim, ultrim, urtrim */
void
TestInterpreterExprBuiltinFunction::testTrim()
{
    // ex IntBuiltinTestSuite::testTrim
    ExpressionTestHelper h;
    // Trim
    h.checkStringExpression("trim('foo')", "foo");
    h.checkStringExpression("trim('  foo')", "foo");
    h.checkStringExpression("trim('foo  ')", "foo");
    h.checkStringExpression("trim('  foo  ')", "foo");
    h.checkStringExpression("trim('    ')", "");
    h.checkNullExpression("trim(z(0))");
    h.checkFailureExpression("trim(1)");
    h.checkFailureExpression("trim(1.0)");

    h.checkIntegerExpression("if(trim(' x'),7,6)",7);
    h.checkIntegerExpression("if(trim(' ');1,7,6)",7);
    h.checkBadExpression("trim(''):=9");

    // LTrim
    h.checkStringExpression("ltrim('foo')", "foo");
    h.checkStringExpression("ltrim('  foo')", "foo");
    h.checkStringExpression("ltrim('foo  ')", "foo  ");
    h.checkStringExpression("ltrim('  foo  ')", "foo  ");
    h.checkStringExpression("ltrim('    ')", "");
    h.checkNullExpression("ltrim(z(0))");
    h.checkFailureExpression("ltrim(1)");
    h.checkFailureExpression("ltrim(1.0)");

    h.checkIntegerExpression("if(ltrim(' x'),7,6)",7);
    h.checkIntegerExpression("if(ltrim(' ');1,7,6)",7);
    h.checkBadExpression("ltrim(''):=9");

    // RTrim
    h.checkStringExpression("rtrim('foo')", "foo");
    h.checkStringExpression("rtrim('  foo')", "  foo");
    h.checkStringExpression("rtrim('foo  ')", "foo");
    h.checkStringExpression("rtrim('  foo  ')", "  foo");
    h.checkStringExpression("rtrim('    ')", "");
    h.checkNullExpression("rtrim(z(0))");
    h.checkFailureExpression("rtrim(1)");
    h.checkFailureExpression("rtrim(1.0)");

    h.checkIntegerExpression("if(rtrim(' x'),7,6)",7);
    h.checkIntegerExpression("if(rtrim(' ');1,7,6)",7);
    h.checkBadExpression("rtrim(''):=9");
}

/** Test square root (Sqr/Sqrt).
    Instructions: usqrt */
void
TestInterpreterExprBuiltinFunction::testSqrt()
{
    // ex IntBuiltinTestSuite::testSqrt
    ExpressionTestHelper h;
    // Sqrt
    h.checkFloatExpression("sqrt(0)", 0);
    h.checkFloatExpression("sqrt(1)", 1);
    h.checkFloatExpression("sqrt(2)", 1.4142);
    h.checkFloatExpression("sqrt(3)", 1.73205);
    h.checkFloatExpression("sqrt(4)", 2);

    h.checkFloatExpression("sqrt(0.0)", 0);
    h.checkFloatExpression("sqrt(1.0)", 1);
    h.checkFloatExpression("sqrt(2.0)", 1.4142);
    h.checkFloatExpression("sqrt(3.0)", 1.73205);
    h.checkFloatExpression("sqrt(4.0)", 2);

    h.checkFloatExpression("sqrt(1.23456^2)", 1.23456);
    h.checkFloatExpression("sqrt(12345)^2", 12345);

    h.checkNullExpression("sqrt(z(0))");
    h.checkNullExpression("sqrt(z(0)^2)");

    h.checkFailureExpression("sqrt(-1)");
    h.checkFailureExpression("sqrt('a')");

    h.checkIntegerExpression("if(sqrt(9),4,3)", 4);
    h.checkIntegerExpression("if(sqrt(9);0,2,1)", 1);
    h.checkBadExpression("sqrt(7):=9");

    // Same things again with Sqr
    h.checkFloatExpression("sqr(0)", 0);
    h.checkFloatExpression("sqr(1)", 1);
    h.checkFloatExpression("sqr(2)", 1.4142);
    h.checkFloatExpression("sqr(3)", 1.73205);
    h.checkFloatExpression("sqr(4)", 2);

    h.checkFloatExpression("sqr(0.0)", 0);
    h.checkFloatExpression("sqr(1.0)", 1);
    h.checkFloatExpression("sqr(2.0)", 1.4142);
    h.checkFloatExpression("sqr(3.0)", 1.73205);
    h.checkFloatExpression("sqr(4.0)", 2);

    h.checkFloatExpression("sqr(1.23456^2)", 1.23456);
    h.checkFloatExpression("sqr(12345)^2", 12345);

    h.checkNullExpression("sqr(z(0))");
    h.checkNullExpression("sqr(z(0)^2)");

    h.checkFailureExpression("sqr(-1)");
    h.checkFailureExpression("sqr('a')");
}

/** Test rounding functions: Int, Round.
    Instructions: utrunc, uround */
void
TestInterpreterExprBuiltinFunction::testRound()
{
    // ex IntBuiltinTestSuite::testRound
    ExpressionTestHelper h;
    // Integers
    h.checkIntegerExpression("int(-1)", -1);
    h.checkIntegerExpression("int(0)", 0);
    h.checkIntegerExpression("int(+1)", 1);
    h.checkIntegerExpression("int(true)", 1);
    h.checkIntegerExpression("round(-1)", -1);
    h.checkIntegerExpression("round(0)", 0);
    h.checkIntegerExpression("round(+1)", 1);
    h.checkIntegerExpression("round(true)", 1);

    h.checkIntegerExpression("if(round(9),4,3)", 4);
    h.checkIntegerExpression("if(round(9);0,2,1)", 1);
    h.checkBadExpression("round(7):=9");

    // Floats, Int
    h.checkIntegerExpression("int(1.9)", 1);
    h.checkIntegerExpression("int(1.5)", 1);
    h.checkIntegerExpression("int(1.1)", 1);
    h.checkIntegerExpression("int(0.4)", 0);
    h.checkIntegerExpression("int(0.0)", 0);
    h.checkIntegerExpression("int(-0.4)", 0);
    h.checkIntegerExpression("int(-1.1)", -1);
    h.checkIntegerExpression("int(-1.5)", -1);
    h.checkIntegerExpression("int(-1.9)", -1);
    h.checkIntegerExpression("int(2147483647)", 2147483647);
    h.checkNullExpression("int(z(0))");
    h.checkFailureExpression("int('a')");
    h.checkFailureExpression("int(2147483648)");

    h.checkIntegerExpression("if(int(9),4,3)", 4);
    h.checkIntegerExpression("if(int(9);0,2,1)", 1);
    h.checkBadExpression("int(7):=9");

    // Floats, Round
    h.checkIntegerExpression("round(1.9)", 2);
    h.checkIntegerExpression("round(1.5)", 2);
    h.checkIntegerExpression("round(1.1)", 1);
    h.checkIntegerExpression("round(0.4)", 0);
    h.checkIntegerExpression("round(0.0)", 0);
    h.checkIntegerExpression("round(-0.4)", 0);
    h.checkIntegerExpression("round(-1.1)", -1);
    h.checkIntegerExpression("round(-1.5)", -2);
    h.checkIntegerExpression("round(-1.9)", -2);
    h.checkIntegerExpression("round(2147483647)", 2147483647);
    h.checkNullExpression("round(z(0))");
    h.checkFailureExpression("round('a')");
    h.checkFailureExpression("round(2147483648)");
}

/** Test If function.
    Several other tests can be found in IntParseExprTestSuite::testAnd etc. */
void
TestInterpreterExprBuiltinFunction::testIf()
{
    // ex IntBuiltinTestSuite::testIf
    ExpressionTestHelper h;
    h.checkIntegerExpression("if(1,2,3)", 2);
    h.checkIntegerExpression("if(0,2,3)", 3);
    h.checkNullExpression("if(0,2)");
    h.checkIntegerExpression("if(1,2)", 2);

    h.checkIntegerExpression("if(1,2,'a')",2);
    h.checkStringExpression("if(0,2,'a')", "a");

    h.checkIntegerExpression("if(if(1,2,0),3,4)", 3);
    h.checkIntegerExpression("if(if(1,2,0);0,3,4)", 4);
    h.checkBadExpression("if(1,2,3):=9");
}

/** Test Str function.
    Instructions: ustr, bstr */
void
TestInterpreterExprBuiltinFunction::testStr()
{
    // ex IntBuiltinTestSuite::testStr
    ExpressionTestHelper h;
    // Unary
    h.checkStringExpression("str(1)", "1");
    h.checkStringExpression("str(123456789)", "123456789");
    h.checkStringExpression("str(1.0)", "1");
    h.checkStringExpression("str(1.01)", "1.01");
    // FIXME: the following holds for PCC1, but not for PCC2:
    // checkStringExpression("str(1.001)", "1");
    h.checkStringExpression("str('a')", "a");
    h.checkStringExpression("str(true)", "YES");
    h.checkStringExpression("str(false)", "NO");
    h.checkNullExpression("str(z(0))");

    h.checkIntegerExpression("if(str(9),4,3)", 4);
    h.checkIntegerExpression("if(str(9);0,2,1)", 1);
    h.checkBadExpression("str(7):=9");

    // Binary
    h.checkStringExpression("str(1,3)", "1.000");
    h.checkStringExpression("str(123456789,3)", "123456789.000");
    h.checkStringExpression("str(1.0,3)", "1.000");
    h.checkStringExpression("str(1.01,3)", "1.010");
    h.checkStringExpression("str(1.001,3)", "1.001");
    h.checkFailureExpression("str('a',3)");
    h.checkStringExpression("str(true,3)", "YES");
    h.checkStringExpression("str(false,3)", "NO");
    h.checkNullExpression("str(z(0),3)");
    h.checkNullExpression("str(1,z(0))");
    h.checkFailureExpression("str(1, 'a')");

    h.checkIntegerExpression("if(str(9,2),4,3)", 4);
    h.checkIntegerExpression("if(str(9,2);0,2,1)", 1);
    h.checkBadExpression("str(7,2):=9");
}

/** Test Val function.
    Instruction: uval */
void
TestInterpreterExprBuiltinFunction::testVal()
{
    // ex IntBuiltinTestSuite::testVal
    ExpressionTestHelper h;
    h.checkIntegerExpression("val('1')", 1);
    h.checkIntegerExpression("val('99')", 99);
    h.checkIntegerExpression("val('-99')", -99);
    h.checkIntegerExpression("val('   1 ')", 1);
    h.checkFloatExpression("val('1.0')", 1);
    h.checkFloatExpression("val('99.0')", 99);
    h.checkFloatExpression("val('-99.0')", -99);
    h.checkFloatExpression("val('.5')", 0.5);
    h.checkFloatExpression("val('1.')", 1.0);
    h.checkFloatExpression("val('   1.0 ')", 1);

    h.checkNullExpression("val('')");
    h.checkNullExpression("val('a')");

    h.checkFailureExpression("val(1)");
    h.checkFailureExpression("val(true)");

    h.checkIntegerExpression("if(val('9'),4,3)", 4);
    h.checkIntegerExpression("if(val('9');0,2,1)", 1);
    h.checkBadExpression("val('7'):=9");
}

/** Test Z/Zap function.
    Instruction: uzap */
void
TestInterpreterExprBuiltinFunction::testZap()
{
    // ex IntBuiltinTestSuite::testZap
    ExpressionTestHelper h;
    // Null
    h.checkNullExpression("z(0)");
    h.checkNullExpression("z('')");
    h.checkNullExpression("z(0.0)");
    h.checkNullExpression("z(0.00000000000001)");
    h.checkNullExpression("z(false)");

    // Non-Null
    h.checkIntegerExpression("z(1)", 1);
    h.checkIntegerExpression("z(999)", 999);
    h.checkStringExpression("z('a')", "a");
    h.checkFloatExpression("z(0.1)", 0.1);
    h.checkBooleanExpression("z(true)", true);

    // Null, using Zap
    h.checkNullExpression("zap(0)");
    h.checkNullExpression("zap('')");
    h.checkNullExpression("zap(0.0)");
    h.checkNullExpression("zap(0.00000000000001)");
    h.checkNullExpression("zap(false)");

    // Non-Null, using Zap
    h.checkIntegerExpression("zap(1)", 1);
    h.checkIntegerExpression("zap(999)", 999);
    h.checkStringExpression("zap('a')", "a");
    h.checkFloatExpression("zap(0.1)", 0.1);
    h.checkBooleanExpression("zap(true)", true);

    // Variants
    h.checkIntegerExpression("if(z(9),4,3)", 4);
    h.checkIntegerExpression("if(z(9);0,2,1)", 1);
    h.checkBadExpression("z(7):=9");
}

/** Test Len function.
    Instruction: ulen */
void
TestInterpreterExprBuiltinFunction::testLen()
{
    // ex IntBuiltinTestSuite::testLen
    ExpressionTestHelper h;
    h.checkIntegerExpression("len('')", 0);
    h.checkIntegerExpression("len('a')", 1);
    h.checkIntegerExpression("len('foobar')", 6);
    h.checkNullExpression("len(z(0))");

    // FIXME: those yield 1 and 2, respectively, in PCC1:
    h.checkFailureExpression("len(2)");
    h.checkFailureExpression("len(12)");

    // Variants
    h.checkIntegerExpression("if(len('x'),4,3)", 4);
    h.checkIntegerExpression("if(len('');1,2,1)", 2);
    h.checkBadExpression("len(''):=9");
}

/** Test String/String$ function.
    Instruction: bstrmult */
void
TestInterpreterExprBuiltinFunction::testStrMult()
{
    // ex IntBuiltinTestSuite::testStrMult
    ExpressionTestHelper h;
    // String
    h.checkStringExpression("string(10)", "          ");
    h.checkStringExpression("string(0)", "");
    h.checkStringExpression("string(-1)", "");

    h.checkStringExpression("string(10, '')", "");
    h.checkStringExpression("string(10, 'a')", "aaaaaaaaaa");
    h.checkStringExpression("string(5, 'ab')", "ababababab");

    h.checkNullExpression("string(z(0), 10)");
    h.checkNullExpression("string(10, z(0))");
    h.checkNullExpression("string(z(0), z(0))");
    h.checkNullExpression("string('a', z(0))");
    h.checkNullExpression("string(z(0))");

    h.checkFailureExpression("string('a', 10)");
    h.checkFailureExpression("string('a', 'b')");
    h.checkFailureExpression("string(1,2)");

    h.checkIntegerExpression("if(string(10,'a'),9,8)", 9);
    h.checkIntegerExpression("if(string(10,'a');0,9,8)", 8);
    h.checkBadExpression("string(10,'a'):='y'");

    h.checkIntegerExpression("if(string(10),9,8)", 9);
    h.checkIntegerExpression("if(string(10);0,9,8)", 8);
    h.checkBadExpression("string(10):='y'");

    // Same thing using String$
    h.checkStringExpression("string$(10)", "          ");
    h.checkStringExpression("string$(0)", "");
    h.checkStringExpression("string$(-1)", "");

    h.checkStringExpression("string$(10, '')", "");
    h.checkStringExpression("string$(10, 'a')", "aaaaaaaaaa");
    h.checkStringExpression("string$(5, 'ab')", "ababababab");

    h.checkNullExpression("string$(z(0), 10)");
    h.checkNullExpression("string$(10, z(0))");
    h.checkNullExpression("string$(z(0), z(0))");
    h.checkNullExpression("string$('a', z(0))");
    h.checkNullExpression("string$(z(0))");

    h.checkFailureExpression("string$('a', 10)");
    h.checkFailureExpression("string$('a', 'b')");
    h.checkFailureExpression("string$(1,2)");
}

/** Test StrCase function.
    Instructions affected by StrCase have already been tested elsewhere,
    so this tests that StrCase doesn't affect too much. */
void
TestInterpreterExprBuiltinFunction::testStrCase()
{
    // IntBuiltinTestSuite::testStrCase
    ExpressionTestHelper h;
    h.checkIntegerExpression("strcase(1+1)", 2);

    h.checkIntegerExpression("strcase(instr('foo','O')) + instr('foo','O')", 2);
    h.checkIntegerExpression("instr('foo','O') + strcase(instr('foo','O'))", 2);
    h.checkIntegerExpression("strcase(instr('foo','O')) + strcase(instr('foo','O'))", 0);
    h.checkIntegerExpression("instr('foo','O') + instr('foo','O')", 4);
    h.checkIntegerExpression("strcase(instr('foo','O') + instr('foo','O'))", 0);
    h.checkIntegerExpression("instr(strcase('foo'),'O')", 2);
    h.checkIntegerExpression("instr('foo',strcase('O'))", 2);

    h.checkIntegerExpression("if(strcase(1 or 2),3,4)", 3);

    h.checkIntegerExpression("if(strcase(instr('foo','O') or instr('foo','O')),3,4)", 4);
    h.checkIntegerExpression("if(strcase(instr('foo','O') or instr('foo','o')),3,4)", 3);
    h.checkIntegerExpression("if(strcase(instr('foo','o') or instr('foo','O')),3,4)", 3);
    h.checkIntegerExpression("strcase(if(instr('foo','O') or instr('foo','O'),3,4))", 4);

    h.checkIntegerExpression("if('a' = 'A', 3, 4)", 3);
    h.checkIntegerExpression("if('a' <> 'A', 3, 4)", 4);

    h.checkIntegerExpression("if(strcase(instr('foo','O'));1,9,2)", 9);
    h.checkBadExpression("strcase('x'):=9");
}

/** Test atom functions: Atom, AtomStr.
    Instructions: uatom, uatomstr */
void
TestInterpreterExprBuiltinFunction::testAtom()
{
    // ex IntBuiltinTestSuite::testAtom
    ExpressionTestHelper h;
    h.checkIntegerExpression("atom('')", 0);
    h.checkStringExpression("atomstr(0)", "");

    h.checkNullExpression("atom(z(0))");
    h.checkNullExpression("atomstr(z(0))");

    h.checkStringExpression("atomstr(atom(1))", "1");
    h.checkStringExpression("atomstr(atom('haha'))", "haha");

    h.checkIntegerExpression("if(atom('x'),3,5)", 3);
    h.checkIntegerExpression("if(atom('');1,3,5)", 3);
    h.checkBadExpression("atom('y'):=3");

    h.checkIntegerExpression("if(atomstr(atom('x')),3,5)", 3);
    h.checkIntegerExpression("if(atomstr(0);1,3,5)", 3);
    h.checkBadExpression("atomstr(77):=3");
}

/** Test Eval function.
    Instructions: sevalx */
void
TestInterpreterExprBuiltinFunction::testEval()
{
    // ex IntBuiltinTestSuite::testEval
    ExpressionTestHelper h;
    h.checkIntegerExpression("eval(1)", 1);
    h.checkIntegerExpression("eval('1')", 1);
    h.checkIntegerExpression("eval('1+1')", 2);
    h.checkIntegerExpression("eval('1;2;3')", 3);
    h.checkNullExpression("eval(z(0))");
    h.checkNullExpression("eval('z(0)')");
    h.checkStringExpression("eval('\"foo\"')", "foo");

    h.checkIntegerExpression("if(eval(1),2,3)", 2);
    h.checkIntegerExpression("if(eval(0);1,2,3)", 2);
    h.checkBadExpression("eval(1):=2");

    // Two-argument forms
    // FIXME: cannot test the actual eval(expr,context) form yet
    h.checkNullExpression("eval('1',z(0))");
    h.checkFailureExpression("eval('1',1)");
}

/** Test miscellaneous. */
void
TestInterpreterExprBuiltinFunction::testMisc()
{
    ExpressionTestHelper h;

    // This does not execute because we don't have a user-defined function (but it compiles)
    h.checkFailureExpression("udf(9)");
}

