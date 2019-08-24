/**
  *  \file u/t_interpreter_expr_parser.cpp
  *  \brief Test for interpreter::expr::Parser
  *
  *  This module assumes absence of constant folding. With constant
  *  folding, the checkFailureExpression tests might turn into compilation
  *  failures. A possible workaround would be to place the literals in
  *  global variables.
  */

#include "interpreter/expr/parser.hpp"

#include "t_interpreter_expr.hpp"
#include "t_interpreter.hpp"

/** Test all sorts of literals.
    Also tests the builtin 'Z' function which is used later on.
    Instructions: pushint, pushbool, pushlit, (pushe) */
void
TestInterpreterExprParser::testLiterals()
{
    // ex IntParseExprTestSuite::testLiterals
    ExpressionTestHelper h;
    h.checkBooleanExpression("true", 1);
    h.checkBooleanExpression("false", 0);
    h.checkIntegerExpression("1", 1);
    h.checkIntegerExpression("0", 0);
    h.checkIntegerExpression("99999", 99999);
    h.checkStringExpression("''", "");
    h.checkStringExpression("'foo'", "foo");
    h.checkStringExpression("\"\"", "");
    h.checkStringExpression("\"bar\"", "bar");
    h.checkFloatExpression("pi", 3.14159265);
    h.checkNullExpression("z(0)");
    h.checkNullExpression("#z(0)");
    h.checkNullExpression("# # # z(0)");
    h.checkFailureExpression("#'foo'");
    h.checkFailureExpression("#2+3");        // means plus(file(2),3) in PCC2
    h.checkFileExpression("#1", 1);
    h.checkFileExpression("#2", 2);
    h.checkFileExpression("#42", 42);
    h.checkFileExpression("# # # # 23", 23);
}

/** Test sequence ";" operator. */
void
TestInterpreterExprParser::testSequence()
{
    // ex IntParseExprTestSuite::testSequence
    // operator ";"
    ExpressionTestHelper h;
    h.checkIntegerExpression("1;2", 2);
    h.checkIntegerExpression("1;2;3;4;5", 5);
    h.checkIntegerExpression("(1;2);3", 3);
    h.checkIntegerExpression("1;(3;4)", 4);
}

/** Test assignment ":=" operator. */
void
TestInterpreterExprParser::testAssignment()
{
    // ex IntParseExprTestSuite::testAssignment
    // operator ":="
    ExpressionTestHelper h;
    h.checkIntegerExpression("a:=1", 1);
    TS_ASSERT_EQUALS(h.a, 1);
    h.a = h.b = h.c = 0;

    h.checkIntegerExpression("a:=b:=1", 1);
    TS_ASSERT_EQUALS(h.a, 1);
    TS_ASSERT_EQUALS(h.b, 1);
    h.a = h.b = h.c = 0;

    h.checkIntegerExpression("(((a))):=17", 17);
    TS_ASSERT_EQUALS(h.a, 17);
    h.a = h.b = h.c = 0;

    h.checkIntegerExpression("if(a:=b,2,3)", 3);
    TS_ASSERT_EQUALS(h.a, 0);
    h.a = h.b = h.c = 0;
}

/** Test logical "Or" operator.
    Also uses ":=" and parentheses. */
void
TestInterpreterExprParser::testOr()
{
    // ex IntParseExprTestSuite::testOr
    ExpressionTestHelper h;

    // Test ternary logic
    h.checkBooleanExpression("0 or 0",       false);
    h.checkBooleanExpression("0 or 1",       true);
    h.checkBooleanExpression("0 or 17",      true);
    h.checkNullExpression("0 or z(0)");
    h.checkBooleanExpression("1 or 0",       true);
    h.checkBooleanExpression("1 or 1",       true);
    h.checkBooleanExpression("1 or z(0)",    true);
    h.checkBooleanExpression("17 or 0",      true);
    h.checkBooleanExpression("17 or 1",      true);
    h.checkBooleanExpression("17 or z(0)",   true);
    h.checkNullExpression("z(0) or 0");
    h.checkBooleanExpression("z(0) or 19",   true);
    h.checkNullExpression("z(0) or z(0)");

    // Check lazy evaluation
    h.checkBooleanExpression("(a:=1; 0) or (b:=1; 0)", false);
    TS_ASSERT_EQUALS(h.a, 1);
    TS_ASSERT_EQUALS(h.b, 1);

    h.checkBooleanExpression("(a:=2; 0) or (b:=2; 1)", true);
    TS_ASSERT_EQUALS(h.a, 2);
    TS_ASSERT_EQUALS(h.b, 2);

    h.checkBooleanExpression("(a:=3; 1) or (b:=3; 0)", true);
    TS_ASSERT_EQUALS(h.a, 3);
    TS_ASSERT_EQUALS(h.b, 2);

    h.checkNullExpression("(a:=4; z(0)) or (b:=4; 0)");
    TS_ASSERT_EQUALS(h.a, 4);
    TS_ASSERT_EQUALS(h.b, 4);

    // Test 'if'
    h.checkIntegerExpression("if((a:=5; 0) or (b:=5; 0),    333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 5);
    TS_ASSERT_EQUALS(h.b, 5);
    h.checkIntegerExpression("if((a:=6; 0) or (b:=6; 1),    333, 444)", 333);
    TS_ASSERT_EQUALS(h.a, 6);
    TS_ASSERT_EQUALS(h.b, 6);
    h.checkIntegerExpression("if((a:=7; 0) or (b:=7; z(0)), 333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 7);
    TS_ASSERT_EQUALS(h.b, 7);

    h.checkIntegerExpression("if((a:=8; 1) or (b:=8; 0),    333, 444)", 333);
    TS_ASSERT_EQUALS(h.a, 8);
    TS_ASSERT_EQUALS(h.b, 7);
    h.checkIntegerExpression("if((a:=9; 1) or (b:=9; 1),    333, 444)", 333);
    TS_ASSERT_EQUALS(h.a, 9);
    TS_ASSERT_EQUALS(h.b, 7);
    h.checkIntegerExpression("if((a:=10; 1) or (b:=10; z(0)), 333, 444)", 333);
    TS_ASSERT_EQUALS(h.a, 10);
    TS_ASSERT_EQUALS(h.b, 7);

    h.checkIntegerExpression("if((a:=11; z(0)) or (b:=11; 0),    333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 11);
    TS_ASSERT_EQUALS(h.b, 11);
    h.checkIntegerExpression("if((a:=12; z(0)) or (b:=12; 1),    333, 444)", 333);
    TS_ASSERT_EQUALS(h.a, 12);
    TS_ASSERT_EQUALS(h.b, 12);
    h.checkIntegerExpression("if((a:=13; z(0)) or (b:=13; z(0)), 333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 13);
    TS_ASSERT_EQUALS(h.b, 13);
}

/** Test logical "And" operator.
    Also uses ":=" and parentheses. */
void
TestInterpreterExprParser::testAnd()
{
    // ex IntParseExprTestSuite::testAnd
    ExpressionTestHelper h;

    // Test ternary logic
    h.checkBooleanExpression("0 and 0",       false);
    h.checkBooleanExpression("0 and 1",       false);
    h.checkBooleanExpression("0 and 17",      false);
    h.checkBooleanExpression("0 and z(0)",    false);
    h.checkBooleanExpression("1 and 0",       false);
    h.checkBooleanExpression("1 and 1",       true);
    h.checkNullExpression("1 and z(0)");
    h.checkBooleanExpression("17 and 0",      false);
    h.checkBooleanExpression("17 and 1",      true);
    h.checkNullExpression("17 and z(0)");
    h.checkBooleanExpression("z(0) and 0",    false);
    h.checkNullExpression("z(0) and 19");
    h.checkNullExpression("z(0) and z(0)");

    // Check lazy evaluation
    h.checkBooleanExpression("(a:=1; 0) and (b:=1; 0)", false);
    TS_ASSERT_EQUALS(h.a, 1);
    TS_ASSERT_EQUALS(h.b, 0);

    h.checkBooleanExpression("(a:=2; 0) and (b:=2; 1)", false);
    TS_ASSERT_EQUALS(h.a, 2);
    TS_ASSERT_EQUALS(h.b, 0);

    h.checkBooleanExpression("(a:=3; 1) and (b:=3; 0)", false);
    TS_ASSERT_EQUALS(h.a, 3);
    TS_ASSERT_EQUALS(h.b, 3);

    h.checkBooleanExpression("(a:=4; z(0)) and (b:=4; 0)", false);
    TS_ASSERT_EQUALS(h.a, 4);
    TS_ASSERT_EQUALS(h.b, 4);

    h.checkNullExpression("(a:=5; z(0)) and (b:=5; 77)");
    TS_ASSERT_EQUALS(h.a, 5);
    TS_ASSERT_EQUALS(h.b, 5);

    // Test 'if'
    h.a = h.b = 0;
    h.checkIntegerExpression("if((a:=5; 0) and (b:=5; 0),    333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 5);
    TS_ASSERT_EQUALS(h.b, 0);
    h.checkIntegerExpression("if((a:=6; 0) and (b:=6; 1),    333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 6);
    TS_ASSERT_EQUALS(h.b, 0);
    h.checkIntegerExpression("if((a:=7; 0) and (b:=7; z(0)), 333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 7);
    TS_ASSERT_EQUALS(h.b, 0);

    h.checkIntegerExpression("if((a:=8; 1) and (b:=8; 0),    333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 8);
    TS_ASSERT_EQUALS(h.b, 8);
    h.checkIntegerExpression("if((a:=9; 1) and (b:=9; 1),    333, 444)", 333);
    TS_ASSERT_EQUALS(h.a, 9);
    TS_ASSERT_EQUALS(h.b, 9);
    h.checkIntegerExpression("if((a:=10; 1) and (b:=10; z(0)), 333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 10);
    TS_ASSERT_EQUALS(h.b, 10);

    // NOTE: here, the second part is not evaluated whereas in the similar 'a:=4' test, it is.
    // The reason is that we don't need an exact value here, and don't care whether the result
    // is empty or false.
    h.checkIntegerExpression("if((a:=11; z(0)) and (b:=11; 0),    333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 11);
    TS_ASSERT_EQUALS(h.b, 10);
    h.checkIntegerExpression("if((a:=12; z(0)) and (b:=12; 1),    333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 12);
    TS_ASSERT_EQUALS(h.b, 10);
    h.checkIntegerExpression("if((a:=13; z(0)) and (b:=13; z(0)), 333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 13);
    TS_ASSERT_EQUALS(h.b, 10);
}

/** Test logical "Xor" operator.
    Also uses ":=" and parentheses. */
void
TestInterpreterExprParser::testXor()
{
    // ex IntParseExprTestSuite::testXor
    ExpressionTestHelper h;
    // Test ternary logic
    h.checkBooleanExpression("0 xor 0",       false);
    h.checkBooleanExpression("0 xor 1",       true);
    h.checkBooleanExpression("0 xor 17",      true);
    h.checkNullExpression("0 xor z(0)");
    h.checkBooleanExpression("1 xor 0",       true);
    h.checkBooleanExpression("1 xor 1",       false);
    h.checkNullExpression("1 xor z(0)");
    h.checkBooleanExpression("17 xor 0",      true);
    h.checkBooleanExpression("17 xor 1",      false);
    h.checkNullExpression("17 xor z(0)");
    h.checkNullExpression("z(0) xor 0");
    h.checkNullExpression("z(0) xor 19");
    h.checkNullExpression("z(0) xor z(0)");

    // Check lazy evaluation
    h.checkBooleanExpression("(a:=1; 0) xor (b:=1; 0)", false);
    TS_ASSERT_EQUALS(h.a, 1);
    TS_ASSERT_EQUALS(h.b, 1);

    h.checkBooleanExpression("(a:=2; 0) xor (b:=2; 1)", true);
    TS_ASSERT_EQUALS(h.a, 2);
    TS_ASSERT_EQUALS(h.b, 2);

    h.checkBooleanExpression("(a:=3; 1) xor (b:=3; 0)", true);
    TS_ASSERT_EQUALS(h.a, 3);
    TS_ASSERT_EQUALS(h.b, 3);

    h.checkNullExpression("(a:=4; z(0)) xor (b:=4; 0)");
    TS_ASSERT_EQUALS(h.a, 4);
    TS_ASSERT_EQUALS(h.b, 3);

    // Test 'if'
    h.checkIntegerExpression("if((a:=5; 0) xor (b:=5; 0),    333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 5);
    TS_ASSERT_EQUALS(h.b, 5);
    h.checkIntegerExpression("if((a:=6; 0) xor (b:=6; 1),    333, 444)", 333);
    TS_ASSERT_EQUALS(h.a, 6);
    TS_ASSERT_EQUALS(h.b, 6);
    h.checkIntegerExpression("if((a:=7; 0) xor (b:=7; z(0)), 333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 7);
    TS_ASSERT_EQUALS(h.b, 7);

    h.checkIntegerExpression("if((a:=8; 1) xor (b:=8; 0),    333, 444)", 333);
    TS_ASSERT_EQUALS(h.a, 8);
    TS_ASSERT_EQUALS(h.b, 8);
    h.checkIntegerExpression("if((a:=9; 1) xor (b:=9; 1),    333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 9);
    TS_ASSERT_EQUALS(h.b, 9);
    h.checkIntegerExpression("if((a:=10; 1) xor (b:=10; z(0)), 333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 10);
    TS_ASSERT_EQUALS(h.b, 10);

    h.checkIntegerExpression("if((a:=11; z(0)) xor (b:=11; 0),    333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 11);
    TS_ASSERT_EQUALS(h.b, 10);
    h.checkIntegerExpression("if((a:=12; z(0)) xor (b:=12; 1),    333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 12);
    TS_ASSERT_EQUALS(h.b, 10);
    h.checkIntegerExpression("if((a:=13; z(0)) xor (b:=13; z(0)), 333, 444)", 444);
    TS_ASSERT_EQUALS(h.a, 13);
    TS_ASSERT_EQUALS(h.b, 10);
}

/** Test logical "Not" operator.
    Instructions: unot, ubool. */
void
TestInterpreterExprParser::testNot()
{
    // ex IntParseExprTestSuite::testNot
    ExpressionTestHelper h;
    // Not
    h.checkBooleanExpression("not 1", false);
    h.checkBooleanExpression("not 0", true);
    h.checkNullExpression("not z(0)");

    h.checkBooleanExpression("not not 1", true);
    h.checkBooleanExpression("not not 99", true);
    h.checkBooleanExpression("not not 0", false);
    h.checkNullExpression("not not z(0)");

    h.checkBooleanExpression("not not not 1",  false);
    h.checkBooleanExpression("not not not 99", false);
    h.checkBooleanExpression("not not not 0",  true);
    h.checkNullExpression("not not not z(0)");

    // Test 'if'
    h.checkIntegerExpression("if(not 1, 333, 444)", 444);
    h.checkIntegerExpression("if(not 0, 333, 444)", 333);
    h.checkIntegerExpression("if(not z(0), 333, 444)", 444);

    h.checkIntegerExpression("if(not not 1, 333, 444)", 333);
    h.checkIntegerExpression("if(not not 0, 333, 444)", 444);
    h.checkIntegerExpression("if(not not z(0), 333, 444)", 444);
}

/** Test comparison operators: "=", "<>", "<", ">", "<=", ">=".
    Also tests the StrCase function.
    Instructions: bcmpeq, bcmpne, bcmplt, bcmpgt, bcmple, bcmpge, and bcmpXX_nc" versions of those. */
void
TestInterpreterExprParser::testComparison()
{
    // ex IntParseExprTestSuite::testComparison
    ExpressionTestHelper h;
    // "="
    h.checkBooleanExpression("1=1", true);
    h.checkBooleanExpression("1=2", false);
    h.checkBooleanExpression("1=1.0", true);
    h.checkBooleanExpression("1.1=1.0", false);
    h.checkBooleanExpression("1.0=1", true);
    h.checkBooleanExpression("1.0=1.0", true);
    h.checkBooleanExpression("'a'='A'", true);
    h.checkBooleanExpression("'A'='A'", true);
    h.checkBooleanExpression("strcase('a'='A')", false);
    h.checkBooleanExpression("strcase('A'='A')", true);
    h.checkBooleanExpression("''=''", true);
    h.checkNullExpression("z(0)=1");
    h.checkNullExpression("1=z(0)");
    h.checkNullExpression("0=z(0)");
    h.checkNullExpression("z(0)=z(0)");
    h.checkFailureExpression("1='a'");
    h.checkFailureExpression("'a'=1");
    h.checkFailureExpression("'a'=1.0");
    h.checkFailureExpression("'a'=a");

    h.checkBooleanExpression("StrCase(1=1)", true);
    h.checkBooleanExpression("StrCase(1=2)", false);
    h.checkBooleanExpression("StrCase(1=1.0)", true);
    h.checkBooleanExpression("StrCase(1.1=1.0)", false);
    h.checkBooleanExpression("StrCase(1.0=1)", true);
    h.checkBooleanExpression("StrCase(1.0=1.0)", true);
    h.checkNullExpression("StrCase(0=z(0))");
    h.checkNullExpression("StrCase(z(0)=z(0))");
    h.checkFailureExpression("StrCase(1='a')");
    h.checkFailureExpression("StrCase('a'=1)");
    
    // "<>"
    h.checkBooleanExpression("1<>1", false);
    h.checkBooleanExpression("1<>2", true);
    h.checkBooleanExpression("1<>1.0", false);
    h.checkBooleanExpression("1.1<>1.0", true);
    h.checkBooleanExpression("1.0<>1", false);
    h.checkBooleanExpression("1.0<>1.0", false);
    h.checkBooleanExpression("'a'<>'A'", false);
    h.checkBooleanExpression("'A'<>'A'", false);
    h.checkBooleanExpression("strcase('a'<>'A')", true);
    h.checkBooleanExpression("strcase('A'<>'A')", false);
    h.checkBooleanExpression("''<>''", false);
    h.checkNullExpression("z(0)<>1");
    h.checkNullExpression("1<>z(0)");
    h.checkNullExpression("0<>z(0)");
    h.checkNullExpression("z(0)<>z(0)");
    h.checkFailureExpression("1<>'a'");
    h.checkFailureExpression("'a'<>1");
    h.checkFailureExpression("'a'<>1.0");
    h.checkFailureExpression("'a'<>a");

    // "<"
    h.checkBooleanExpression("1<1", false);
    h.checkBooleanExpression("1<2", true);
    h.checkBooleanExpression("1<1.0", false);
    h.checkBooleanExpression("1.1<1.0", false);
    h.checkBooleanExpression("1.0<1.1", true);
    h.checkBooleanExpression("1.0<1", false);
    h.checkBooleanExpression("1.0<1.0", false);
    h.checkBooleanExpression("'a'<'A'", false);
    h.checkBooleanExpression("'A'<'A'", false);
    h.checkBooleanExpression("strcase('a'<'A')", false);
    h.checkBooleanExpression("strcase('A'<'A')", false);
    h.checkBooleanExpression("''<''", false);
    h.checkBooleanExpression("'a'<'b'", true);
    h.checkBooleanExpression("'a'<''", false);
    h.checkNullExpression("z(0)<1");
    h.checkNullExpression("1<z(0)");
    h.checkNullExpression("0<z(0)");
    h.checkNullExpression("z(0)<z(0)");
    h.checkFailureExpression("1<'a'");
    h.checkFailureExpression("'a'<1");
    h.checkFailureExpression("'a'<1.0");
    h.checkFailureExpression("'a'<a");

    // ">"
    h.checkBooleanExpression("1>1", false);
    h.checkBooleanExpression("1>2", false);
    h.checkBooleanExpression("1>1.0", false);
    h.checkBooleanExpression("1.1>1.0", true);
    h.checkBooleanExpression("1.0>1.1", false);
    h.checkBooleanExpression("1.0>1", false);
    h.checkBooleanExpression("1.0>1.0", false);
    h.checkBooleanExpression("'a'>'A'", false);
    h.checkBooleanExpression("'A'>'A'", false);
    h.checkBooleanExpression("strcase('a'>'A')", true);
    h.checkBooleanExpression("strcase('A'>'A')", false);
    h.checkBooleanExpression("''>''", false);
    h.checkBooleanExpression("'a'>'b'", false);
    h.checkBooleanExpression("'a'>''", true);
    h.checkNullExpression("z(0)>1");
    h.checkNullExpression("1>z(0)");
    h.checkNullExpression("0>z(0)");
    h.checkNullExpression("z(0)>z(0)");
    h.checkFailureExpression("1>'a'");
    h.checkFailureExpression("'a'>1");
    h.checkFailureExpression("'a'>1.0");
    h.checkFailureExpression("'a'>a");

    // "<="
    h.checkBooleanExpression("1<=1", true);
    h.checkBooleanExpression("1<=2", true);
    h.checkBooleanExpression("1<=1.0", true);
    h.checkBooleanExpression("1.1<=1.0", false);
    h.checkBooleanExpression("1.0<=1.1", true);
    h.checkBooleanExpression("1.0<=1", true);
    h.checkBooleanExpression("1.0<=1.0", true);
    h.checkBooleanExpression("'a'<='A'", true);
    h.checkBooleanExpression("'A'<='A'", true);
    h.checkBooleanExpression("strcase('a'<='A')", false);
    h.checkBooleanExpression("strcase('A'<='A')", true);
    h.checkBooleanExpression("''<=''", true);
    h.checkBooleanExpression("'a'<='b'", true);
    h.checkBooleanExpression("'a'<=''", false);
    h.checkNullExpression("z(0)<=1");
    h.checkNullExpression("1<=z(0)");
    h.checkNullExpression("0<=z(0)");
    h.checkNullExpression("z(0)<=z(0)");
    h.checkFailureExpression("1<='a'");
    h.checkFailureExpression("'a'<=1");
    h.checkFailureExpression("'a'<=1.0");
    h.checkFailureExpression("'a'<=a");

    // ">="
    h.checkBooleanExpression("1>=1", true);
    h.checkBooleanExpression("1>=2", false);
    h.checkBooleanExpression("1>=1.0", true);
    h.checkBooleanExpression("1.1>=1.0", true);
    h.checkBooleanExpression("1.0>=1.1", false);
    h.checkBooleanExpression("1.0>=1", true);
    h.checkBooleanExpression("1.0>=1.0", true);
    h.checkBooleanExpression("'a'>='A'", true);
    h.checkBooleanExpression("'A'>='A'", true);
    h.checkBooleanExpression("strcase('a'>='A')", true);
    h.checkBooleanExpression("strcase('A'>='A')", true);
    h.checkBooleanExpression("''>=''", true);
    h.checkBooleanExpression("'a'>='b'", false);
    h.checkBooleanExpression("'a'>=''", true);
    h.checkNullExpression("z(0)>=1");
    h.checkNullExpression("1>=z(0)");
    h.checkNullExpression("0>=z(0)");
    h.checkNullExpression("z(0)>=z(0)");
    h.checkFailureExpression("1>='a'");
    h.checkFailureExpression("'a'>=1");
    h.checkFailureExpression("'a'>=1.0");
    h.checkFailureExpression("'a'>=a");
}

/** Test concatenation operators: "&", "#".
    Instructions: bconcat, bconcatempty. */
void
TestInterpreterExprParser::testConcat()
{
    // ex IntParseExprTestSuite::testConcat
    ExpressionTestHelper h;
    // "&", interpolates Empty as ''
    h.checkStringExpression("1 & 2", "12");
    h.checkStringExpression("1 & 'a'", "1a");
    h.checkStringExpression("1 & z(0)", "1");
    h.checkNullExpression("z(0) & z(0)");
    h.checkStringExpression("'a' & 'b' & 'c' & 'd' & z(0)", "abcd");
    h.checkStringExpression("'a' & 'b' & 'c' & z(0) & 'd'", "abcd");
    h.checkStringExpression("'a' & 'b' & z(0) & 'c' & 'd'", "abcd");
    h.checkStringExpression("'a' & z(0) & 'b' & 'c' & 'd'", "abcd");
    h.checkStringExpression("z(0) & 'a' & 'b' & 'c' & 'd'", "abcd");

    // "#", Empty annihilates expression
    h.checkStringExpression("1 # 2", "12");
    h.checkStringExpression("1 # 'a'", "1a");
    h.checkNullExpression("1 # z(0)");
    h.checkNullExpression("z(0) # z(0)");
    h.checkNullExpression("'a' # 'b' # 'c' # 'd' # z(0)");
    h.checkNullExpression("'a' # 'b' # 'c' # z(0) # 'd'");
    h.checkNullExpression("'a' # 'b' # z(0) # 'c' # 'd'");
    h.checkNullExpression("'a' # z(0) # 'b' # 'c' # 'd'");
    h.checkNullExpression("z(0) # 'a' # 'b' # 'c' # 'd'");
}

/** Test addition operator "+".
    Instructions: badd. */
void
TestInterpreterExprParser::testAdd()
{
    // ex IntParseExprTestSuite::testAdd
    ExpressionTestHelper h;

    // Integers
    h.checkIntegerExpression("1 + 1", 2);
    h.checkIntegerExpression("0 + 1000000", 1000000);
    h.checkNullExpression("1 + z(0)");
    h.checkNullExpression("z(0) + 1");

    // Floats
    h.checkFloatExpression("1.0 + 2.0", 3.0);
    h.checkFloatExpression("0.0 + 1000000.0", 1000000.0);
    h.checkNullExpression("1.0 + z(0)");
    h.checkNullExpression("z(0) + 1.0");

    // Mixed
    h.checkFloatExpression("1 + 2.0", 3.0);
    h.checkFloatExpression("0.0 + 0", 0.0);

    // Strings
    h.checkStringExpression("'a' + 'b'", "ab");
    h.checkNullExpression("'a' + z(0)");
    h.checkNullExpression("z(0) + 'a'");

    // Errors
    h.checkFailureExpression("'a' + 1");
    h.checkFailureExpression("1 + 'a'");
}

/** Test subtraction operator: "-".
    Instruction: bsub. */
void
TestInterpreterExprParser::testSubtract()
{
    // ex IntParseExprTestSuite::testSubtract
    ExpressionTestHelper h;

    // Integers
    h.checkIntegerExpression("1 - 1", 0);
    h.checkIntegerExpression("100 - 1", 99);
    h.checkIntegerExpression("0 - 1000000", -1000000);
    h.checkNullExpression("1 - z(0)");
    h.checkNullExpression("z(0) - 1");

    // Floats
    h.checkFloatExpression("1.0 - 2.0", -1.0);
    h.checkFloatExpression("0.0 - 1000000.0", -1000000.0);
    h.checkNullExpression("1.0 - z(0)");
    h.checkNullExpression("z(0) - 1.0");

    // Mixed
    h.checkFloatExpression("12 - 2.0", 10.0);
    h.checkFloatExpression("0.0 - 0", 0.0);

    // Errors
    h.checkFailureExpression("'a' - 1");
    h.checkFailureExpression("'ab' - 'a'");
    // checkFailureExpression("'a' - z(0)");  Not an error - should it?
    // checkFailureExpression("z(0) - 'a'");  Not an error - should it?
}

/** Test multiplication operator: "*".
    Instruction: bmul. */
void
TestInterpreterExprParser::testMultiply()
{
    // ex IntParseExprTestSuite::testMultiply
    ExpressionTestHelper h;

    // Integers
    h.checkIntegerExpression("2*3*4", 24);
    h.checkIntegerExpression("10*0", 0);
    h.checkNullExpression("z(0) * 10");
    h.checkNullExpression("10 * z(0)");

    // Floats
    h.checkFloatExpression("2.0*3.0*4.0", 24.0);
    h.checkFloatExpression("10.0 * 0", 0.0);
    h.checkNullExpression("z(0) * 10.0");
    h.checkNullExpression("10.0 * z(0)");

    // Mixed
    h.checkFloatExpression("2*3.0", 6);
    h.checkFloatExpression("2.0*3", 6);

    // Errors
    h.checkFailureExpression("10*'a'");
    h.checkFailureExpression("'a'*10");
    // checkFailureExpression("'a'*z(0)");  Not an error - should it?
    // checkFailureExpression("z(0)*'a'");  Not an error - should it?
}

/** Test real division operator "/".
    Instruction: bdiv. */
void
TestInterpreterExprParser::testDivide()
{
    // ex IntParseExprTestSuite::testDivide
    ExpressionTestHelper h;
    // Integers
    h.checkFloatExpression("16/4", 4.0);
    h.checkFloatExpression("10/1", 10.0);
    h.checkNullExpression("z(0) / 10");
    h.checkNullExpression("10 / z(0)");

    // Floats
    h.checkFloatExpression("2.0/4.0", 0.5);
    h.checkFloatExpression("10.0/4.0", 2.5);
    h.checkNullExpression("z(0) / 10.0");
    h.checkNullExpression("10.0 / z(0)");

    // Mixed
    h.checkFloatExpression("2 / 4.0", 0.5);
    h.checkFloatExpression("2.0 / 4", 0.5);

    // Errors
    h.checkFailureExpression("10/'a'");
    h.checkFailureExpression("'a'/10");
    // checkFailureExpression("'a'/z(0)");  Not an error - should it?
    // checkFailureExpression("z(0)/'a'");  Not an error - should it?

    h.checkFailureExpression("10/0");
    h.checkFailureExpression("10.0/0");
    h.checkFailureExpression("10.0/0.0");
}

/** Test integral division operators: "\", "Mod".
    Instructions: bidiv, brem. */
void
TestInterpreterExprParser::testIntegerDivide()
{
    // ex IntParseExprTestSuite::testIntegerDivide
    ExpressionTestHelper h;
    // Integers
    h.checkIntegerExpression("15 \\ 3", 5);
    h.checkIntegerExpression("16 \\ 3", 5);
    h.checkIntegerExpression("17 \\ 3", 5);
    h.checkIntegerExpression("18 \\ 3", 6);
    h.checkIntegerExpression("15 mod 3", 0);
    h.checkIntegerExpression("16 mod 3", 1);
    h.checkIntegerExpression("17 mod 3", 2);
    h.checkIntegerExpression("18 mod 3", 0);

    h.checkNullExpression("z(0) \\ 3");
    h.checkNullExpression("15 \\ z(0)");
    h.checkNullExpression("z(0) mod 3");
    h.checkNullExpression("15 mod z(0)");

    // Floats
    h.checkFailureExpression("15.0 \\ 3");
    h.checkFailureExpression("15 \\ 3.0");
    h.checkFailureExpression("15.0 mod 3");
    h.checkFailureExpression("15 mod 3.0");

    // Errors
    h.checkFailureExpression("'a' \\ 3");
    h.checkFailureExpression("3 \\ 'a'");
    h.checkFailureExpression("'a' mod 3");
    h.checkFailureExpression("3 mod 'a'");
    // checkFailureExpression("'a' \\ z(0)");  Not an error - should it?
    // checkFailureExpression("z(0) \\ 'a'");  Not an error - should it?
}

/** Test unary signs "+", "-".
    Instructions: uneg, upos. */
void
TestInterpreterExprParser::testNegation()
{
    // ex IntParseExprTestSuite::testNegation
    ExpressionTestHelper h;
    // Integers
    h.checkIntegerExpression("-1", -1);
    h.checkIntegerExpression("+1", 1);

    h.checkIntegerExpression("--1", 1);
    h.checkIntegerExpression("+-1", -1);
    h.checkIntegerExpression("-+1", -1);
    h.checkIntegerExpression("++1", 1);

    h.checkIntegerExpression("---1", -1);
    h.checkIntegerExpression("+--1", 1);
    h.checkIntegerExpression("-+-1", 1);
    h.checkIntegerExpression("++-1", -1);
    h.checkIntegerExpression("--+1", 1);
    h.checkIntegerExpression("+-+1", -1);
    h.checkIntegerExpression("-++1", -1);
    h.checkIntegerExpression("+++1", 1);

    // Floats
    h.checkFloatExpression("-1.0", -1.0);
    h.checkFloatExpression("+1.0", 1.0);

    h.checkFloatExpression("--1.0", 1.0);
    h.checkFloatExpression("+-1.0", -1.0);
    h.checkFloatExpression("-+1.0", -1.0);
    h.checkFloatExpression("++1.0", 1.0);

    h.checkFloatExpression("---1.0", -1.0);
    h.checkFloatExpression("+--1.0", 1.0);
    h.checkFloatExpression("-+-1.0", 1.0);
    h.checkFloatExpression("++-1.0", -1.0);
    h.checkFloatExpression("--+1.0", 1.0);
    h.checkFloatExpression("+-+1.0", -1.0);
    h.checkFloatExpression("-++1.0", -1.0);
    h.checkFloatExpression("+++1.0", 1.0);

    // Strings
    h.checkFailureExpression("+'a'");
    h.checkFailureExpression("-'a'");
    h.checkFailureExpression("+-'a'");
    h.checkFailureExpression("--'a'");
    h.checkFailureExpression("++'a'");
    h.checkFailureExpression("-+'a'");

    // Null
    h.checkNullExpression("-z(0)");
    h.checkNullExpression("+z(0)");
    h.checkNullExpression("-+z(0)");
    h.checkNullExpression("++z(0)");
    h.checkNullExpression("+-z(0)");
    h.checkNullExpression("--z(0)");
}

/** Test exponentiation operator "^".
    Instruction: bpow. */
void
TestInterpreterExprParser::testPower()
{
    // ex IntParseExprTestSuite::testPower
    ExpressionTestHelper h;
    // Integers
    h.checkIntegerExpression("2^8", 256);
    h.checkIntegerExpression("0^10", 0);
    h.checkIntegerExpression("10^0", 1);
    h.checkIntegerExpression("61^2", 3721);
    h.checkIntegerExpression("-61^2", -3721);
    h.checkIntegerExpression("(-61)^2", 3721);

    // Boundaries
    h.checkIntegerExpression("46340^2", 2147395600);
    h.checkFloatExpression("46341^2", 2147488281.0);
    h.checkIntegerExpression("2^20", 1048576);
    h.checkFloatExpression("3^20", 3486784401.0);
    h.checkFloatExpression("3^31", 617673396283947.0);

    // Floats
    h.checkFloatExpression("10^12", 1000000000000.0);
    // checkFloatExpression("4^0.5", 2);
    h.checkFloatExpression("0.5^2", 0.25);
    // checkFloatExpression("4^2.5", 32);

    // Null
    h.checkNullExpression("2^z(0)");
    h.checkNullExpression("z(0)^2");
    // checkNullExpression("z(0)^2.5");
    h.checkNullExpression("z(0)^3");
    h.checkNullExpression("z(0)^z(0)");

    // Strings
    h.checkFailureExpression("2^'a'");
    h.checkFailureExpression("'a'^3");
    h.checkFailureExpression("'a'^'b'");
    // checkFailureExpression("'a' ^ z(0)");
    // checkFailureExpression("z(0) ^ 'a'");

    // Parsing
    h.checkIntegerExpression("-3^2", -9);
    h.checkIntegerExpression("(-3)^2", 9);
    h.checkFloatExpression("3^-2", 0.1111111111111111111111111111111111111111111111111111);
}

void
TestInterpreterExprParser::testPrecedence()
{
    // ex IntParseExprTestSuite::testPrecedence
    ExpressionTestHelper h;
    h.checkIntegerExpression("1+2*3", 7);
    h.checkIntegerExpression("1*2+3", 5);
    h.checkIntegerExpression("(1+2)*3", 9);
    h.checkIntegerExpression("1+2^3*4", 33);
    h.checkBooleanExpression("1 or 1 and 0", true);
    h.checkBooleanExpression("(1 or 1) and 0", false);
    h.checkBooleanExpression("1 or (1 and 0)", true);

    // Negation vs. NOT
    h.checkIntegerExpression("-NOT 0", -1);
    h.checkIntegerExpression("-NOT 1", 0);
    h.checkIntegerExpression("+NOT 0", 1);
    h.checkBooleanExpression("not -1", false);
    h.checkBooleanExpression("not +0", true);
}

void
TestInterpreterExprParser::testErrors()
{
    ExpressionTestHelper h;

    // Parens
    h.checkRejectedExpression("(1+2");
    h.checkRejectedExpression("(3*(1+2)");
    h.checkRejectedExpression("z(1");

    // Argument count for builtin
    h.checkRejectedExpression("z()");
    h.checkIntegerExpression("z(1)", 1);
    h.checkRejectedExpression("z(1,2)");
    h.checkRejectedExpression("z(1,,2)");

    // Assignment
    h.checkBadExpression("sin(1) := 2");

    // Member reference
    h.checkNullExpression("z(0).foo");
    h.checkNullExpression("z(0)->foo");
    h.checkRejectedExpression("z(0).'x'");
    h.checkRejectedExpression("z(0)->3");

    // Bad syntax
    h.checkRejectedExpression(",");
}
