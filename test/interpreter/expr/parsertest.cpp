/**
  *  \file test/interpreter/expr/parsertest.cpp
  *  \brief Test for interpreter::expr::Parser
  *
  *  This module assumes absence of constant folding. With constant
  *  folding, the verifyExecutionError tests might turn into compilation
  *  failures. A possible workaround would be to place the literals in
  *  global variables.
  */

#include "interpreter/expr/parser.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/test/expressionverifier.hpp"

using interpreter::test::ExpressionVerifier;

/** Test all sorts of literals.
    Also tests the builtin 'Z' function which is used later on.
    Instructions: pushint, pushbool, pushlit, (pushe) */
AFL_TEST("interpreter.expr.Parser:literals", a)
{
    // ex IntParseExprTestSuite::testLiterals
    ExpressionVerifier h(a);
    h.verifyBoolean("true", 1);
    h.verifyBoolean("false", 0);
    h.verifyInteger("1", 1);
    h.verifyInteger("0", 0);
    h.verifyInteger("99999", 99999);
    h.verifyString("''", "");
    h.verifyString("'foo'", "foo");
    h.verifyString("\"\"", "");
    h.verifyString("\"bar\"", "bar");
    h.verifyFloat("pi", 3.14159265);
    h.verifyNull("z(0)");
    h.verifyNull("#z(0)");
    h.verifyNull("# # # z(0)");
    h.verifyExecutionError("#'foo'");
    h.verifyExecutionError("#2+3");        // means plus(file(2),3) in PCC2
    h.verifyFile("#1", 1);
    h.verifyFile("#2", 2);
    h.verifyFile("#42", 42);
    h.verifyFile("# # # # 23", 23);
}

/** Test sequence ";" operator. */
AFL_TEST("interpreter.expr.Parser:sequence", a)
{
    // ex IntParseExprTestSuite::testSequence
    // operator ";"
    ExpressionVerifier h(a);
    h.verifyInteger("1;2", 2);
    h.verifyInteger("1;2;3;4;5", 5);
    h.verifyInteger("(1;2);3", 3);
    h.verifyInteger("1;(3;4)", 4);
}

/** Test assignment ":=" operator. */
AFL_TEST("interpreter.expr.Parser:assignment", a)
{
    // ex IntParseExprTestSuite::testAssignment
    // operator ":="
    ExpressionVerifier h(a);
    h.verifyInteger("a:=1", 1);
    a.checkEqual("01", h.get(0), 1);
    h.clear();

    h.verifyInteger("a:=b:=1", 1);
    a.checkEqual("11", h.get(0), 1);
    a.checkEqual("12", h.get(1), 1);
    h.clear();

    h.verifyInteger("(((a))):=17", 17);
    a.checkEqual("21", h.get(0), 17);
    h.clear();

    h.verifyInteger("if(a:=b,2,3)", 3);
    a.checkEqual("31", h.get(0), 0);
    h.clear();
}

/** Test logical "Or" operator.
    Also uses ":=" and parentheses. */
AFL_TEST("interpreter.expr.Parser:or", a)
{
    // ex IntParseExprTestSuite::testOr
    ExpressionVerifier h(a);

    // Test ternary logic
    h.verifyBoolean("0 or 0",       false);
    h.verifyBoolean("0 or 1",       true);
    h.verifyBoolean("0 or 17",      true);
    h.verifyNull("0 or z(0)");
    h.verifyBoolean("1 or 0",       true);
    h.verifyBoolean("1 or 1",       true);
    h.verifyBoolean("1 or z(0)",    true);
    h.verifyBoolean("17 or 0",      true);
    h.verifyBoolean("17 or 1",      true);
    h.verifyBoolean("17 or z(0)",   true);
    h.verifyNull("z(0) or 0");
    h.verifyBoolean("z(0) or 19",   true);
    h.verifyNull("z(0) or z(0)");

    // Check lazy evaluation
    h.verifyBoolean("(a:=1; 0) or (b:=1; 0)", false);
    a.checkEqual("01", h.get(0), 1);
    a.checkEqual("02", h.get(1), 1);

    h.verifyBoolean("(a:=2; 0) or (b:=2; 1)", true);
    a.checkEqual("11", h.get(0), 2);
    a.checkEqual("12", h.get(1), 2);

    h.verifyBoolean("(a:=3; 1) or (b:=3; 0)", true);
    a.checkEqual("21", h.get(0), 3);
    a.checkEqual("22", h.get(1), 2);

    h.verifyNull("(a:=4; z(0)) or (b:=4; 0)");
    a.checkEqual("31", h.get(0), 4);
    a.checkEqual("32", h.get(1), 4);

    h.verifyInteger("(a:=5; 0) or (b:=5; 0); a:=6", 6);
    a.checkEqual("41", h.get(0), 6);
    a.checkEqual("42", h.get(1), 5);

    // Test 'if'
    h.verifyInteger("if((a:=5; 0) or (b:=5; 0),    333, 444)", 444);
    a.checkEqual("51", h.get(0), 5);
    a.checkEqual("52", h.get(1), 5);
    h.verifyInteger("if((a:=6; 0) or (b:=6; 1),    333, 444)", 333);
    a.checkEqual("53", h.get(0), 6);
    a.checkEqual("54", h.get(1), 6);
    h.verifyInteger("if((a:=7; 0) or (b:=7; z(0)), 333, 444)", 444);
    a.checkEqual("55", h.get(0), 7);
    a.checkEqual("56", h.get(1), 7);

    h.verifyInteger("if((a:=8; 1) or (b:=8; 0),    333, 444)", 333);
    a.checkEqual("61", h.get(0), 8);
    a.checkEqual("62", h.get(1), 7);
    h.verifyInteger("if((a:=9; 1) or (b:=9; 1),    333, 444)", 333);
    a.checkEqual("63", h.get(0), 9);
    a.checkEqual("64", h.get(1), 7);
    h.verifyInteger("if((a:=10; 1) or (b:=10; z(0)), 333, 444)", 333);
    a.checkEqual("65", h.get(0), 10);
    a.checkEqual("66", h.get(1), 7);

    h.verifyInteger("if((a:=11; z(0)) or (b:=11; 0),    333, 444)", 444);
    a.checkEqual("71", h.get(0), 11);
    a.checkEqual("72", h.get(1), 11);
    h.verifyInteger("if((a:=12; z(0)) or (b:=12; 1),    333, 444)", 333);
    a.checkEqual("73", h.get(0), 12);
    a.checkEqual("74", h.get(1), 12);
    h.verifyInteger("if((a:=13; z(0)) or (b:=13; z(0)), 333, 444)", 444);
    a.checkEqual("75", h.get(0), 13);
    a.checkEqual("76", h.get(1), 13);
}

/** Test logical "And" operator.
    Also uses ":=" and parentheses. */
AFL_TEST("interpreter.expr.Parser:and", a)
{
    // ex IntParseExprTestSuite::testAnd
    ExpressionVerifier h(a);

    // Test ternary logic
    h.verifyBoolean("0 and 0",       false);
    h.verifyBoolean("0 and 1",       false);
    h.verifyBoolean("0 and 17",      false);
    h.verifyBoolean("0 and z(0)",    false);
    h.verifyBoolean("1 and 0",       false);
    h.verifyBoolean("1 and 1",       true);
    h.verifyNull("1 and z(0)");
    h.verifyBoolean("17 and 0",      false);
    h.verifyBoolean("17 and 1",      true);
    h.verifyNull("17 and z(0)");
    h.verifyBoolean("z(0) and 0",    false);
    h.verifyNull("z(0) and 19");
    h.verifyNull("z(0) and z(0)");

    // Check lazy evaluation
    h.verifyBoolean("(a:=1; 0) and (b:=1; 0)", false);
    a.checkEqual("01", h.get(0), 1);
    a.checkEqual("02", h.get(1), 0);

    h.verifyBoolean("(a:=2; 0) and (b:=2; 1)", false);
    a.checkEqual("11", h.get(0), 2);
    a.checkEqual("12", h.get(1), 0);

    h.verifyBoolean("(a:=3; 1) and (b:=3; 0)", false);
    a.checkEqual("21", h.get(0), 3);
    a.checkEqual("22", h.get(1), 3);

    h.verifyBoolean("(a:=4; z(0)) and (b:=4; 0)", false);
    a.checkEqual("31", h.get(0), 4);
    a.checkEqual("32", h.get(1), 4);

    h.verifyNull("(a:=5; z(0)) and (b:=5; 77)");
    a.checkEqual("41", h.get(0), 5);
    a.checkEqual("42", h.get(1), 5);

    h.verifyInteger("(a:=6) and (b:=6; 0); a:=7", 7);
    a.checkEqual("51", h.get(0), 7);
    a.checkEqual("52", h.get(1), 6);

    // Test 'if'
    h.clear();
    h.verifyInteger("if((a:=5; 0) and (b:=5; 0),    333, 444)", 444);
    a.checkEqual("61", h.get(0), 5);
    a.checkEqual("62", h.get(1), 0);
    h.verifyInteger("if((a:=6; 0) and (b:=6; 1),    333, 444)", 444);
    a.checkEqual("63", h.get(0), 6);
    a.checkEqual("64", h.get(1), 0);
    h.verifyInteger("if((a:=7; 0) and (b:=7; z(0)), 333, 444)", 444);
    a.checkEqual("65", h.get(0), 7);
    a.checkEqual("66", h.get(1), 0);

    h.verifyInteger("if((a:=8; 1) and (b:=8; 0),    333, 444)", 444);
    a.checkEqual("71", h.get(0), 8);
    a.checkEqual("72", h.get(1), 8);
    h.verifyInteger("if((a:=9; 1) and (b:=9; 1),    333, 444)", 333);
    a.checkEqual("73", h.get(0), 9);
    a.checkEqual("74", h.get(1), 9);
    h.verifyInteger("if((a:=10; 1) and (b:=10; z(0)), 333, 444)", 444);
    a.checkEqual("75", h.get(0), 10);
    a.checkEqual("76", h.get(1), 10);

    // NOTE: here, the second part is not evaluated whereas in the similar 'a:=4' test, it is.
    // The reason is that we don't need an exact value here, and don't care whether the result
    // is empty or false.
    h.verifyInteger("if((a:=11; z(0)) and (b:=11; 0),    333, 444)", 444);
    a.checkEqual("81", h.get(0), 11);
    a.checkEqual("82", h.get(1), 10);
    h.verifyInteger("if((a:=12; z(0)) and (b:=12; 1),    333, 444)", 444);
    a.checkEqual("83", h.get(0), 12);
    a.checkEqual("84", h.get(1), 10);
    h.verifyInteger("if((a:=13; z(0)) and (b:=13; z(0)), 333, 444)", 444);
    a.checkEqual("85", h.get(0), 13);
    a.checkEqual("86", h.get(1), 10);
}

/** Test logical "Xor" operator.
    Also uses ":=" and parentheses. */
AFL_TEST("interpreter.expr.Parser:xor", a)
{
    // ex IntParseExprTestSuite::testXor
    ExpressionVerifier h(a);
    // Test ternary logic
    h.verifyBoolean("0 xor 0",       false);
    h.verifyBoolean("0 xor 1",       true);
    h.verifyBoolean("0 xor 17",      true);
    h.verifyNull("0 xor z(0)");
    h.verifyBoolean("1 xor 0",       true);
    h.verifyBoolean("1 xor 1",       false);
    h.verifyNull("1 xor z(0)");
    h.verifyBoolean("17 xor 0",      true);
    h.verifyBoolean("17 xor 1",      false);
    h.verifyNull("17 xor z(0)");
    h.verifyNull("z(0) xor 0");
    h.verifyNull("z(0) xor 19");
    h.verifyNull("z(0) xor z(0)");

    // Check lazy evaluation
    h.verifyBoolean("(a:=1; 0) xor (b:=1; 0)", false);
    a.checkEqual("01", h.get(0), 1);
    a.checkEqual("02", h.get(1), 1);

    h.verifyBoolean("(a:=2; 0) xor (b:=2; 1)", true);
    a.checkEqual("11", h.get(0), 2);
    a.checkEqual("12", h.get(1), 2);

    h.verifyBoolean("(a:=3; 1) xor (b:=3; 0)", true);
    a.checkEqual("21", h.get(0), 3);
    a.checkEqual("22", h.get(1), 3);

    h.verifyNull("(a:=4; z(0)) xor (b:=4; 0)");
    a.checkEqual("31", h.get(0), 4);
    a.checkEqual("32", h.get(1), 3);

    // Test 'if'
    h.verifyInteger("if((a:=5; 0) xor (b:=5; 0),    333, 444)", 444);
    a.checkEqual("41", h.get(0), 5);
    a.checkEqual("42", h.get(1), 5);
    h.verifyInteger("if((a:=6; 0) xor (b:=6; 1),    333, 444)", 333);
    a.checkEqual("43", h.get(0), 6);
    a.checkEqual("44", h.get(1), 6);
    h.verifyInteger("if((a:=7; 0) xor (b:=7; z(0)), 333, 444)", 444);
    a.checkEqual("45", h.get(0), 7);
    a.checkEqual("46", h.get(1), 7);

    h.verifyInteger("if((a:=8; 1) xor (b:=8; 0),    333, 444)", 333);
    a.checkEqual("51", h.get(0), 8);
    a.checkEqual("52", h.get(1), 8);
    h.verifyInteger("if((a:=9; 1) xor (b:=9; 1),    333, 444)", 444);
    a.checkEqual("53", h.get(0), 9);
    a.checkEqual("54", h.get(1), 9);
    h.verifyInteger("if((a:=10; 1) xor (b:=10; z(0)), 333, 444)", 444);
    a.checkEqual("55", h.get(0), 10);
    a.checkEqual("56", h.get(1), 10);

    h.verifyInteger("if((a:=11; z(0)) xor (b:=11; 0),    333, 444)", 444);
    a.checkEqual("61", h.get(0), 11);
    a.checkEqual("62", h.get(1), 10);
    h.verifyInteger("if((a:=12; z(0)) xor (b:=12; 1),    333, 444)", 444);
    a.checkEqual("63", h.get(0), 12);
    a.checkEqual("64", h.get(1), 10);
    h.verifyInteger("if((a:=13; z(0)) xor (b:=13; z(0)), 333, 444)", 444);
    a.checkEqual("65", h.get(0), 13);
    a.checkEqual("66", h.get(1), 10);
}

/** Test logical "Not" operator.
    Instructions: unot, ubool. */
AFL_TEST("interpreter.expr.Parser:not", a)
{
    // ex IntParseExprTestSuite::testNot
    ExpressionVerifier h(a);
    // Not
    h.verifyBoolean("not 1", false);
    h.verifyBoolean("not 0", true);
    h.verifyNull("not z(0)");

    h.verifyBoolean("not not 1", true);
    h.verifyBoolean("not not 99", true);
    h.verifyBoolean("not not 0", false);
    h.verifyNull("not not z(0)");

    h.verifyBoolean("not not not 1",  false);
    h.verifyBoolean("not not not 99", false);
    h.verifyBoolean("not not not 0",  true);
    h.verifyNull("not not not z(0)");

    // Test 'if'
    h.verifyInteger("if(not 1, 333, 444)", 444);
    h.verifyInteger("if(not 0, 333, 444)", 333);
    h.verifyInteger("if(not z(0), 333, 444)", 444);

    h.verifyInteger("if(not not 1, 333, 444)", 333);
    h.verifyInteger("if(not not 0, 333, 444)", 444);
    h.verifyInteger("if(not not z(0), 333, 444)", 444);
}

/** Test comparison operators: "=", "<>", "<", ">", "<=", ">=".
    Also tests the StrCase function.
    Instructions: bcmpeq, bcmpne, bcmplt, bcmpgt, bcmple, bcmpge, and bcmpXX_nc" versions of those. */
AFL_TEST("interpreter.expr.Parser:comparison", a)
{
    // ex IntParseExprTestSuite::testComparison
    ExpressionVerifier h(a);

    // "="
    h.verifyBoolean("1=1", true);
    h.verifyBoolean("1=2", false);
    h.verifyBoolean("1=1.0", true);
    h.verifyBoolean("1.1=1.0", false);
    h.verifyBoolean("1.0=1", true);
    h.verifyBoolean("1.0=1.0", true);
    h.verifyBoolean("'a'='A'", true);
    h.verifyBoolean("'A'='A'", true);
    h.verifyBoolean("strcase('a'='A')", false);
    h.verifyBoolean("strcase('A'='A')", true);
    h.verifyBoolean("''=''", true);
    h.verifyNull("z(0)=1");
    h.verifyNull("1=z(0)");
    h.verifyNull("0=z(0)");
    h.verifyNull("z(0)=z(0)");
    h.verifyExecutionError("1='a'");
    h.verifyExecutionError("'a'=1");
    h.verifyExecutionError("'a'=1.0");
    h.verifyExecutionError("'a'=a");

    h.verifyBoolean("StrCase(1=1)", true);
    h.verifyBoolean("StrCase(1=2)", false);
    h.verifyBoolean("StrCase(1=1.0)", true);
    h.verifyBoolean("StrCase(1.1=1.0)", false);
    h.verifyBoolean("StrCase(1.0=1)", true);
    h.verifyBoolean("StrCase(1.0=1.0)", true);
    h.verifyNull("StrCase(0=z(0))");
    h.verifyNull("StrCase(z(0)=z(0))");
    h.verifyExecutionError("StrCase(1='a')");
    h.verifyExecutionError("StrCase('a'=1)");

    h.verifyInteger("strcase('a'='A');3", 3);
    h.verifyInteger("'a'='A';3", 3);

    // "<>"
    h.verifyBoolean("1<>1", false);
    h.verifyBoolean("1<>2", true);
    h.verifyBoolean("1<>1.0", false);
    h.verifyBoolean("1.1<>1.0", true);
    h.verifyBoolean("1.0<>1", false);
    h.verifyBoolean("1.0<>1.0", false);
    h.verifyBoolean("'a'<>'A'", false);
    h.verifyBoolean("'A'<>'A'", false);
    h.verifyBoolean("strcase('a'<>'A')", true);
    h.verifyBoolean("strcase('A'<>'A')", false);
    h.verifyBoolean("''<>''", false);
    h.verifyNull("z(0)<>1");
    h.verifyNull("1<>z(0)");
    h.verifyNull("0<>z(0)");
    h.verifyNull("z(0)<>z(0)");
    h.verifyExecutionError("1<>'a'");
    h.verifyExecutionError("'a'<>1");
    h.verifyExecutionError("'a'<>1.0");
    h.verifyExecutionError("'a'<>a");

    // "<"
    h.verifyBoolean("1<1", false);
    h.verifyBoolean("1<2", true);
    h.verifyBoolean("1<1.0", false);
    h.verifyBoolean("1.1<1.0", false);
    h.verifyBoolean("1.0<1.1", true);
    h.verifyBoolean("1.0<1", false);
    h.verifyBoolean("1.0<1.0", false);
    h.verifyBoolean("'a'<'A'", false);
    h.verifyBoolean("'A'<'A'", false);
    h.verifyBoolean("strcase('a'<'A')", false);
    h.verifyBoolean("strcase('A'<'A')", false);
    h.verifyBoolean("''<''", false);
    h.verifyBoolean("'a'<'b'", true);
    h.verifyBoolean("'a'<''", false);
    h.verifyNull("z(0)<1");
    h.verifyNull("1<z(0)");
    h.verifyNull("0<z(0)");
    h.verifyNull("z(0)<z(0)");
    h.verifyExecutionError("1<'a'");
    h.verifyExecutionError("'a'<1");
    h.verifyExecutionError("'a'<1.0");
    h.verifyExecutionError("'a'<a");

    // ">"
    h.verifyBoolean("1>1", false);
    h.verifyBoolean("1>2", false);
    h.verifyBoolean("1>1.0", false);
    h.verifyBoolean("1.1>1.0", true);
    h.verifyBoolean("1.0>1.1", false);
    h.verifyBoolean("1.0>1", false);
    h.verifyBoolean("1.0>1.0", false);
    h.verifyBoolean("'a'>'A'", false);
    h.verifyBoolean("'A'>'A'", false);
    h.verifyBoolean("strcase('a'>'A')", true);
    h.verifyBoolean("strcase('A'>'A')", false);
    h.verifyBoolean("''>''", false);
    h.verifyBoolean("'a'>'b'", false);
    h.verifyBoolean("'a'>''", true);
    h.verifyNull("z(0)>1");
    h.verifyNull("1>z(0)");
    h.verifyNull("0>z(0)");
    h.verifyNull("z(0)>z(0)");
    h.verifyExecutionError("1>'a'");
    h.verifyExecutionError("'a'>1");
    h.verifyExecutionError("'a'>1.0");
    h.verifyExecutionError("'a'>a");

    // "<="
    h.verifyBoolean("1<=1", true);
    h.verifyBoolean("1<=2", true);
    h.verifyBoolean("1<=1.0", true);
    h.verifyBoolean("1.1<=1.0", false);
    h.verifyBoolean("1.0<=1.1", true);
    h.verifyBoolean("1.0<=1", true);
    h.verifyBoolean("1.0<=1.0", true);
    h.verifyBoolean("'a'<='A'", true);
    h.verifyBoolean("'A'<='A'", true);
    h.verifyBoolean("strcase('a'<='A')", false);
    h.verifyBoolean("strcase('A'<='A')", true);
    h.verifyBoolean("''<=''", true);
    h.verifyBoolean("'a'<='b'", true);
    h.verifyBoolean("'a'<=''", false);
    h.verifyNull("z(0)<=1");
    h.verifyNull("1<=z(0)");
    h.verifyNull("0<=z(0)");
    h.verifyNull("z(0)<=z(0)");
    h.verifyExecutionError("1<='a'");
    h.verifyExecutionError("'a'<=1");
    h.verifyExecutionError("'a'<=1.0");
    h.verifyExecutionError("'a'<=a");

    // ">="
    h.verifyBoolean("1>=1", true);
    h.verifyBoolean("1>=2", false);
    h.verifyBoolean("1>=1.0", true);
    h.verifyBoolean("1.1>=1.0", true);
    h.verifyBoolean("1.0>=1.1", false);
    h.verifyBoolean("1.0>=1", true);
    h.verifyBoolean("1.0>=1.0", true);
    h.verifyBoolean("'a'>='A'", true);
    h.verifyBoolean("'A'>='A'", true);
    h.verifyBoolean("strcase('a'>='A')", true);
    h.verifyBoolean("strcase('A'>='A')", true);
    h.verifyBoolean("''>=''", true);
    h.verifyBoolean("'a'>='b'", false);
    h.verifyBoolean("'a'>=''", true);
    h.verifyNull("z(0)>=1");
    h.verifyNull("1>=z(0)");
    h.verifyNull("0>=z(0)");
    h.verifyNull("z(0)>=z(0)");
    h.verifyExecutionError("1>='a'");
    h.verifyExecutionError("'a'>=1");
    h.verifyExecutionError("'a'>=1.0");
    h.verifyExecutionError("'a'>=a");
}

/** Test concatenation operators: "&", "#".
    Instructions: bconcat, bconcatempty. */
AFL_TEST("interpreter.expr.Parser:concat", a)
{
    // ex IntParseExprTestSuite::testConcat
    ExpressionVerifier h(a);
    // "&", interpolates Empty as ''
    h.verifyString("1 & 2", "12");
    h.verifyString("1 & 'a'", "1a");
    h.verifyString("1 & z(0)", "1");
    h.verifyNull("z(0) & z(0)");
    h.verifyString("'a' & 'b' & 'c' & 'd' & z(0)", "abcd");
    h.verifyString("'a' & 'b' & 'c' & z(0) & 'd'", "abcd");
    h.verifyString("'a' & 'b' & z(0) & 'c' & 'd'", "abcd");
    h.verifyString("'a' & z(0) & 'b' & 'c' & 'd'", "abcd");
    h.verifyString("z(0) & 'a' & 'b' & 'c' & 'd'", "abcd");

    // "#", Empty annihilates expression
    h.verifyString("1 # 2", "12");
    h.verifyString("1 # 'a'", "1a");
    h.verifyNull("1 # z(0)");
    h.verifyNull("z(0) # z(0)");
    h.verifyNull("'a' # 'b' # 'c' # 'd' # z(0)");
    h.verifyNull("'a' # 'b' # 'c' # z(0) # 'd'");
    h.verifyNull("'a' # 'b' # z(0) # 'c' # 'd'");
    h.verifyNull("'a' # z(0) # 'b' # 'c' # 'd'");
    h.verifyNull("z(0) # 'a' # 'b' # 'c' # 'd'");
}

/** Test addition operator "+".
    Instructions: badd. */
AFL_TEST("interpreter.expr.Parser:add", a)
{
    // ex IntParseExprTestSuite::testAdd
    ExpressionVerifier h(a);

    // Integers
    h.verifyInteger("1 + 1", 2);
    h.verifyInteger("0 + 1000000", 1000000);
    h.verifyNull("1 + z(0)");
    h.verifyNull("z(0) + 1");

    // Floats
    h.verifyFloat("1.0 + 2.0", 3.0);
    h.verifyFloat("0.0 + 1000000.0", 1000000.0);
    h.verifyNull("1.0 + z(0)");
    h.verifyNull("z(0) + 1.0");

    // Mixed
    h.verifyFloat("1 + 2.0", 3.0);
    h.verifyFloat("0.0 + 0", 0.0);

    // Strings
    h.verifyString("'a' + 'b'", "ab");
    h.verifyNull("'a' + z(0)");
    h.verifyNull("z(0) + 'a'");

    // Errors
    h.verifyExecutionError("'a' + 1");
    h.verifyExecutionError("1 + 'a'");

    // In 'ignore' position
    h.verifyInteger("1 + 2; 9", 9);

    // In 'condition' position
    h.verifyInteger("If(1+2, 7, 8)", 7);
}

/** Test subtraction operator: "-".
    Instruction: bsub. */
AFL_TEST("interpreter.expr.Parser:sub", a)
{
    // ex IntParseExprTestSuite::testSubtract
    ExpressionVerifier h(a);

    // Integers
    h.verifyInteger("1 - 1", 0);
    h.verifyInteger("100 - 1", 99);
    h.verifyInteger("0 - 1000000", -1000000);
    h.verifyNull("1 - z(0)");
    h.verifyNull("z(0) - 1");

    // Floats
    h.verifyFloat("1.0 - 2.0", -1.0);
    h.verifyFloat("0.0 - 1000000.0", -1000000.0);
    h.verifyNull("1.0 - z(0)");
    h.verifyNull("z(0) - 1.0");

    // Mixed
    h.verifyFloat("12 - 2.0", 10.0);
    h.verifyFloat("0.0 - 0", 0.0);

    // Errors
    h.verifyExecutionError("'a' - 1");
    h.verifyExecutionError("'ab' - 'a'");
    // verifyExecutionError("'a' - z(0)");  Not an error - should it?
    // verifyExecutionError("z(0) - 'a'");  Not an error - should it?
}

/** Test multiplication operator: "*".
    Instruction: bmul. */
AFL_TEST("interpreter.expr.Parser:mul", a)
{
    // ex IntParseExprTestSuite::testMultiply
    ExpressionVerifier h(a);

    // Integers
    h.verifyInteger("2*3*4", 24);
    h.verifyInteger("10*0", 0);
    h.verifyNull("z(0) * 10");
    h.verifyNull("10 * z(0)");

    // Floats
    h.verifyFloat("2.0*3.0*4.0", 24.0);
    h.verifyFloat("10.0 * 0", 0.0);
    h.verifyNull("z(0) * 10.0");
    h.verifyNull("10.0 * z(0)");

    // Mixed
    h.verifyFloat("2*3.0", 6);
    h.verifyFloat("2.0*3", 6);

    // Errors
    h.verifyExecutionError("10*'a'");
    h.verifyExecutionError("'a'*10");
    // verifyExecutionError("'a'*z(0)");  Not an error - should it?
    // verifyExecutionError("z(0)*'a'");  Not an error - should it?
}

/** Test real division operator "/".
    Instruction: bdiv. */
AFL_TEST("interpreter.expr.Parser:div", a)
{
    // ex IntParseExprTestSuite::testDivide
    ExpressionVerifier h(a);
    // Integers
    h.verifyInteger("16/4", 4);
    h.verifyInteger("10/1", 10);
    h.verifyFloat("5/2", 2.5);
    h.verifyNull("z(0) / 10");
    h.verifyNull("10 / z(0)");

    // Floats
    h.verifyFloat("16.0/4.0", 4.0);
    h.verifyFloat("2.0/4.0", 0.5);
    h.verifyFloat("10.0/4.0", 2.5);
    h.verifyNull("z(0) / 10.0");
    h.verifyNull("10.0 / z(0)");

    // Mixed
    h.verifyFloat("2 / 4.0", 0.5);
    h.verifyFloat("2.0 / 4", 0.5);
    h.verifyFloat("4 / 2.0", 2.0);

    // Errors
    h.verifyExecutionError("10/'a'");
    h.verifyExecutionError("'a'/10");
    // verifyExecutionError("'a'/z(0)");  Not an error - should it?
    // verifyExecutionError("z(0)/'a'");  Not an error - should it?

    h.verifyExecutionError("10/0");
    h.verifyExecutionError("10.0/0");
    h.verifyExecutionError("10.0/0.0");
}

/** Test integral division operators: "\", "Mod".
    Instructions: bidiv, brem. */
AFL_TEST("interpreter.expr.Parser:integer-divide", a)
{
    // ex IntParseExprTestSuite::testIntegerDivide
    ExpressionVerifier h(a);
    // Integers
    h.verifyInteger("15 \\ 3", 5);
    h.verifyInteger("16 \\ 3", 5);
    h.verifyInteger("17 \\ 3", 5);
    h.verifyInteger("18 \\ 3", 6);
    h.verifyInteger("15 mod 3", 0);
    h.verifyInteger("16 mod 3", 1);
    h.verifyInteger("17 mod 3", 2);
    h.verifyInteger("18 mod 3", 0);

    h.verifyNull("z(0) \\ 3");
    h.verifyNull("15 \\ z(0)");
    h.verifyNull("z(0) mod 3");
    h.verifyNull("15 mod z(0)");

    // Floats
    h.verifyExecutionError("15.0 \\ 3");
    h.verifyExecutionError("15 \\ 3.0");
    h.verifyExecutionError("15.0 mod 3");
    h.verifyExecutionError("15 mod 3.0");

    // Errors
    h.verifyExecutionError("'a' \\ 3");
    h.verifyExecutionError("3 \\ 'a'");
    h.verifyExecutionError("'a' mod 3");
    h.verifyExecutionError("3 mod 'a'");
    // verifyExecutionError("'a' \\ z(0)");  Not an error - should it?
    // verifyExecutionError("z(0) \\ 'a'");  Not an error - should it?
}

/** Test unary signs "+", "-".
    Instructions: uneg, upos. */
AFL_TEST("interpreter.expr.Parser:neg", a)
{
    // ex IntParseExprTestSuite::testNegation
    ExpressionVerifier h(a);
    // Integers
    h.verifyInteger("-1", -1);
    h.verifyInteger("+1", 1);

    h.verifyInteger("--1", 1);
    h.verifyInteger("+-1", -1);
    h.verifyInteger("-+1", -1);
    h.verifyInteger("++1", 1);

    h.verifyInteger("---1", -1);
    h.verifyInteger("+--1", 1);
    h.verifyInteger("-+-1", 1);
    h.verifyInteger("++-1", -1);
    h.verifyInteger("--+1", 1);
    h.verifyInteger("+-+1", -1);
    h.verifyInteger("-++1", -1);
    h.verifyInteger("+++1", 1);

    // Floats
    h.verifyFloat("-1.0", -1.0);
    h.verifyFloat("+1.0", 1.0);

    h.verifyFloat("--1.0", 1.0);
    h.verifyFloat("+-1.0", -1.0);
    h.verifyFloat("-+1.0", -1.0);
    h.verifyFloat("++1.0", 1.0);

    h.verifyFloat("---1.0", -1.0);
    h.verifyFloat("+--1.0", 1.0);
    h.verifyFloat("-+-1.0", 1.0);
    h.verifyFloat("++-1.0", -1.0);
    h.verifyFloat("--+1.0", 1.0);
    h.verifyFloat("+-+1.0", -1.0);
    h.verifyFloat("-++1.0", -1.0);
    h.verifyFloat("+++1.0", 1.0);

    // Strings
    h.verifyExecutionError("+'a'");
    h.verifyExecutionError("-'a'");
    h.verifyExecutionError("+-'a'");
    h.verifyExecutionError("--'a'");
    h.verifyExecutionError("++'a'");
    h.verifyExecutionError("-+'a'");

    // Null
    h.verifyNull("-z(0)");
    h.verifyNull("+z(0)");
    h.verifyNull("-+z(0)");
    h.verifyNull("++z(0)");
    h.verifyNull("+-z(0)");
    h.verifyNull("--z(0)");

    // In 'ignore' position
    h.verifyInteger("+1; 9", 9);
    h.verifyExecutionError("+'a'; 9");

    // In 'condition' position
    h.verifyInteger("If(+2, 7, 8)", 7);
    h.verifyExecutionError("If(+'a', 7, 8)");
}

/** Test exponentiation operator "^".
    Instruction: bpow. */
AFL_TEST("interpreter.expr.Parser:pow", a)
{
    // ex IntParseExprTestSuite::testPower
    ExpressionVerifier h(a);
    // Integers
    h.verifyInteger("2^8", 256);
    h.verifyInteger("0^10", 0);
    h.verifyInteger("10^0", 1);
    h.verifyInteger("61^2", 3721);
    h.verifyInteger("-61^2", -3721);
    h.verifyInteger("(-61)^2", 3721);

    // Boundaries
    h.verifyInteger("46340^2", 2147395600);
    h.verifyFloat("46341^2", 2147488281.0);
    h.verifyInteger("2^20", 1048576);
    h.verifyFloat("3^20", 3486784401.0);
    h.verifyFloat("3^31", 617673396283947.0);

    // Floats
    h.verifyFloat("10^12", 1000000000000.0);
    // verifyFloat("4^0.5", 2);
    h.verifyFloat("0.5^2", 0.25);
    // verifyFloat("4^2.5", 32);

    // Null
    h.verifyNull("2^z(0)");
    h.verifyNull("z(0)^2");
    // verifyNull("z(0)^2.5");
    h.verifyNull("z(0)^3");
    h.verifyNull("z(0)^z(0)");

    // Strings
    h.verifyExecutionError("2^'a'");
    h.verifyExecutionError("'a'^3");
    h.verifyExecutionError("'a'^'b'");
    // verifyExecutionError("'a' ^ z(0)");
    // verifyExecutionError("z(0) ^ 'a'");

    // Parsing
    h.verifyInteger("-3^2", -9);
    h.verifyInteger("(-3)^2", 9);
    h.verifyFloat("3^-2", 0.1111111111111111111111111111111111111111111111111111);
}

AFL_TEST("interpreter.expr.Parser:precedence", a)
{
    // ex IntParseExprTestSuite::testPrecedence
    ExpressionVerifier h(a);
    h.verifyInteger("1+2*3", 7);
    h.verifyInteger("1*2+3", 5);
    h.verifyInteger("(1+2)*3", 9);
    h.verifyInteger("1+2^3*4", 33);
    h.verifyBoolean("1 or 1 and 0", true);
    h.verifyBoolean("(1 or 1) and 0", false);
    h.verifyBoolean("1 or (1 and 0)", true);

    // Negation vs. NOT
    h.verifyInteger("-NOT 0", -1);
    h.verifyInteger("-NOT 1", 0);
    h.verifyInteger("+NOT 0", 1);
    h.verifyBoolean("not -1", false);
    h.verifyBoolean("not +0", true);
}

AFL_TEST("interpreter.expr.Parser:errors", a)
{
    ExpressionVerifier h(a);

    // Parens
    h.verifyParseError("(1+2");
    h.verifyParseError("(3*(1+2)");
    h.verifyParseError("z(1");

    // Argument count for builtin
    h.verifyParseError("z()");
    h.verifyInteger("z(1)", 1);
    h.verifyParseError("z(1,2)");
    h.verifyParseError("z(1,,2)");

    // Assignment
    h.verifyCompileError("sin(1) := 2");

    // Member reference
    h.verifyNull("z(0).foo");
    h.verifyNull("z(0)->foo");
    h.verifyParseError("z(0).'x'");
    h.verifyParseError("z(0)->3");
    h.verifyInteger("z(0).foo; 3", 3);
    h.verifyInteger("if(z(0).foo, 7, 8)", 8);

    // Bad syntax
    h.verifyParseError(",");
}
