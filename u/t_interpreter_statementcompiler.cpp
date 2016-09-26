/**
  *  \file u/t_interpreter_statementcompiler.cpp
  *  \brief Test for interpreter::StatementCompiler
  */

#include "interpreter/statementcompiler.hpp"

#include "t_interpreter.hpp"

/** Test expression statements.
    This tests just the parser.
    The expression interpreter is tested in detail in t_int_parseexpr.cc,
    therefore the expressions can be simple,
    and we limit ourselves to testing expressions yielding integers.
    The idea is to simply make sure that we correctly compile syntactically ambiguous statements. */
void
TestInterpreterStatementCompiler::testExprStatement()
{
    // ex IntStatementTestSuite::testExprStatement
    ExpressionTestHelper h;

    // Operators: ";"
    h.checkIntegerExpressionStatement("a;97", 97);

    // Operators: ":="
    h.checkIntegerExpressionStatement("a:=3", 3);
    h.checkIntegerExpressionStatement("b:=c:=0", 0);
    TS_ASSERT_EQUALS(h.a, 3);
    TS_ASSERT_EQUALS(h.b, 0);
    TS_ASSERT_EQUALS(h.c, 0);

    // Operators: "Or", "Xor"
    h.checkIntegerExpressionStatement("a or b", 1);
    h.checkIntegerExpressionStatement("a xor a", 0);

    // Operators: "And"
    h.checkIntegerExpressionStatement("a and a", 1);

    // Operators: "Not"
    h.checkIntegerExpressionStatement("not a", 0);

    // Operators: comparisons
    h.checkIntegerExpressionStatement("a>0", 1);
    h.checkIntegerExpressionStatement("a<10", 1);
    h.checkIntegerExpressionStatement("a<3", 0);
    h.checkIntegerExpressionStatement("a>=0", 1);
    h.checkIntegerExpressionStatement("a<=3", 1);
    h.checkIntegerExpressionStatement("a<>99", 1);
    h.checkIntegerExpressionStatement("a=7", 7);               // assignment
    TS_ASSERT_EQUALS(h.a, 7);
    h.checkIntegerExpressionStatement("a=3 or 2", 1);          // comparison

    // Operators: "#", "&"
    h.checkIntegerExpressionStatement("a&b;9", 9);
    h.checkIntegerExpressionStatement("a#b;9", 9);

    // Operators: "+", "-"
    h.checkIntegerExpressionStatement("a+3", 10);
    h.checkIntegerExpressionStatement("a-3", 4);

    // Operators: "*", "/", "\", "Mod"
    h.checkIntegerExpressionStatement("a*3", 21);
    h.checkIntegerExpressionStatement("a/1;12", 12);
    h.checkIntegerExpressionStatement("a\\2", 3);
    h.checkIntegerExpressionStatement("a mod 2", 1);

    // Operators: unary "+", "-"
    h.checkIntegerExpressionStatement("-3", -3);
    h.checkIntegerExpressionStatement("+3", +3);

    // Operators: "^"
    h.checkIntegerExpressionStatement("a^2", 49);

    // Operators: "(...)"
    h.checkIntegerExpressionStatement("(9)", 9);
    h.checkIntegerExpressionStatement("(9)*2", 18);

    // Operators: function call
    h.checkIntegerExpressionStatement("isempty(z(0))", 1);

    // Firsts: identifiers
    h.checkIntegerExpressionStatement("a", 7);

    // Firsts: numbers
    h.checkIntegerExpressionStatement("1+1", 2);
    h.checkIntegerExpressionStatement("1.3*99;5", 5);

    // Firsts: strings
    h.checkIntegerExpressionStatement("'a';99", 99);
    h.checkIntegerExpressionStatement("'a'+'b';98", 98);
}

/** Test various flavours of "If", "Else", "Else If". */
void
TestInterpreterStatementCompiler::testIf()
{
    // ex IntStatementTestSuite::testIf
    ExpressionTestHelper h;
    h.checkStatement("a:=3");
    TS_ASSERT_EQUALS(h.a, 3);

    h.checkStatement("if a=4 then\n"
                     "  a:=5\n"
                     "else\n"
                     "  a:=6\n"
                     "endif");
    TS_ASSERT_EQUALS(h.a, 6);

    h.checkStatement("if a=5 then\n"
                     "  a:=6\n"
                     "else if a=6 then\n"
                     "  a=7\n"
                     "endif");
    TS_ASSERT_EQUALS(h.a, 7);

    h.checkStatement("if a=5 then\n"
                     "  a:=6\n"
                     "else if a=6 then\n"
                     "  a=7\n"
                     "else if a=7 then\n"
                     "  a=8\n"
                     "else\n"
                     "  a:=9\n"
                     "endif");
    TS_ASSERT_EQUALS(h.a, 8);

    h.checkStatement("if a=5 then %second\n"
                     "  a:=6\n"
                     "else if a=6 then\n"
                     "  a=7\n"
                     "else if a=7 then\n"
                     "  a=8\n"
                     "else\n"
                     "  a:=9\n"
                     "endif");
    TS_ASSERT_EQUALS(h.a, 9);

    h.checkStatement("if a=9 then a:=10");
    TS_ASSERT_EQUALS(h.a, 10);
}

/** Test 'For' statement. */
void
TestInterpreterStatementCompiler::testFor()
{
    // ex IntStatementTestSuite::testFor
    ExpressionTestHelper h;

    // Basic iteration
    h.checkStatement("for b:=1 to 10 do a:=a+b");
    TS_ASSERT_EQUALS(h.a, 55);

    // Backward iteration: body must not be entered
    h.checkStatement("for b:=10 to 1 do abort 1");

    // Body must be entered once
    h.a = 0;
    h.checkStatement("for b:=20 to 20 do a:=a+99");
    TS_ASSERT_EQUALS(h.a, 99);

    h.a = 0;
    h.checkStatement("for b:=-20 to -20 do a:=a+b");
    TS_ASSERT_EQUALS(h.a, -20);

    // Basic iteration, multi-line
    h.checkStatement("a:=0\n"
                  "for b:=1 to 10\n"
                  "  a:=a+b\n"
                  "next");
    TS_ASSERT_EQUALS(h.a, 55);

    // Basic iteration, multi-line, optional 'do' keyword
    h.checkStatement("a:=10\n"
                  "for b:=1 to 10 do\n"
                  "  a:=a+b\n"
                  "next");
    TS_ASSERT_EQUALS(h.a, 65);

    // Continue
    h.checkStatement("a:=0\n"
                  "for b:=1 to 10 do\n"
                  "  if b mod 2 = 0 then continue\n"
                  "  a:=a+b\n"
                  "next");
    TS_ASSERT_EQUALS(h.a, 25);

    // Break
    h.checkStatement("a:=0\n"
                  "for b:=1 to 10 do\n"
                  "  if b mod 2 = 0 then break\n"
                  "  a:=a+b\n"
                  "next");
    TS_ASSERT_EQUALS(h.a, 1);

    // Varying limit (must not affect loop)
    h.checkStatement("c:=10; a:=0\n"
                  "for b:=1 to c do\n"
                  "  c:=3\n"
                  "  a:=a+1\n"
                  "next");
    TS_ASSERT_EQUALS(h.c, 3);
    TS_ASSERT_EQUALS(h.a, 10);
}

/** Test "Do"/"Loop" statements. */
void
TestInterpreterStatementCompiler::testDo()
{
    // ex IntStatementTestSuite::testDo
    ExpressionTestHelper h;

    // Basic Do/While loop
    h.checkStatement("a:=1; b:=0\n"
                  "do while a<10\n"
                  "  a:=a+1\n"
                  "  b:=b+1\n"
                  "loop");
    TS_ASSERT_EQUALS(h.a, 10);
    TS_ASSERT_EQUALS(h.b, 9);

    // Basic Do/Until loop
    h.checkStatement("a:=1; b:=0\n"
                  "do until a>10\n"
                  "  a:=a+1\n"
                  "  b:=b+1\n"
                  "loop");
    TS_ASSERT_EQUALS(h.a, 11);
    TS_ASSERT_EQUALS(h.b, 10);

    // Basic Do/Loop/While loop
    h.checkStatement("a:=1; b:=0\n"
                  "do\n"
                  "  a:=a+1\n"
                  "  b:=b+1\n"
                  "loop while a<10");
    TS_ASSERT_EQUALS(h.a, 10);
    TS_ASSERT_EQUALS(h.b, 9);

    // Basic Do/Loop/Until loop
    h.checkStatement("a:=1; b:=0\n"
                  "do\n"
                  "  a:=a+1\n"
                  "  b:=b+1\n"
                  "loop until a>10");
    TS_ASSERT_EQUALS(h.a, 11);
    TS_ASSERT_EQUALS(h.b, 10);

    // Do/While entered with wrong condition
    h.checkStatement("a:=1; b:=0\n"
                  "do while a<1\n"
                  "  b:=99\n"
                  "loop");
    TS_ASSERT_EQUALS(h.a, 1);
    TS_ASSERT_EQUALS(h.b, 0);

    // Do/Loop/While entered with wrong condition
    h.checkStatement("a:=1; b:=0\n"
                  "do\n"
                  "  b:=b+99\n"
                  "loop while a<1");
    TS_ASSERT_EQUALS(h.a, 1);
    TS_ASSERT_EQUALS(h.b, 99);

    // Condition with side-effect
    h.checkStatement("a:=1; b:=0\n"
                  "do\n"
                  "  b:=b+1\n"
                  "loop while (a:=a+1)<10");
    TS_ASSERT_EQUALS(h.a, 10);
    TS_ASSERT_EQUALS(h.b, 9);

    // Continue
    h.checkStatement("a:=1; b:=0\n"
                  "do\n"
                  "  continue\n"
                  "  b:=b+1\n"
                  "loop while (a:=a+1)<10");
    TS_ASSERT_EQUALS(h.a, 10);
    TS_ASSERT_EQUALS(h.b, 0);

    // Break
    h.checkStatement("a:=1; b:=0\n"
                  "do\n"
                  "  break\n"
                  "  b:=b+1\n"
                  "loop while (a:=a+1)<10");
    TS_ASSERT_EQUALS(h.a, 1);
    TS_ASSERT_EQUALS(h.b, 0);
}

/** Test "Select Case" statements. */
void
TestInterpreterStatementCompiler::testSelect()
{
    // ex IntStatementTestSuite::testSelect
    ExpressionTestHelper h;

    // Basic Select Case
    h.a = 1;
    h.checkStatement("select case a\n"
                  "  case 0\n"
                  "    b:=9\n"
                  "  case 1\n"
                  "    b:=8\n"
                  "  case 2\n"
                  "    b:=7\n"
                  "endselect");
    TS_ASSERT_EQUALS(h.b, 8);

    // No matching case
    h.a = 1;
    h.b = 0;
    h.checkStatement("select case a\n"
                  "  case 10\n"
                  "    b:=9\n"
                  "  case 11\n"
                  "    b:=8\n"
                  "  case 12\n"
                  "    b:=7\n"
                  "endselect");
    TS_ASSERT_EQUALS(h.b, 0);

    // No matching case, matching else
    h.a = 1;
    h.b = 0;
    h.checkStatement("select case a\n"
                  "  case 10\n"
                  "    b:=9\n"
                  "  case 11\n"
                  "    b:=8\n"
                  "  case 12\n"
                  "    b:=7\n"
                  "  case else\n"
                  "    b:=6\n"
                  "endselect");
    TS_ASSERT_EQUALS(h.b, 6);

    // Ranges
    h.a = 5;
    h.b = 0;
    h.checkStatement("select case a\n"
                  "  case 1,2,3\n"
                  "    b:=1\n"
                  "  case 4,5,6\n"
                  "    b:=2\n"
                  "  case 7,8,9\n"
                  "    b:=3\n"
                  "endselect");
    TS_ASSERT_EQUALS(h.b, 2);

    // Match first in range
    h.a = 1;
    h.b = 0;
    h.checkStatement("select case a\n"
                  "  case 1,2,3\n"
                  "    b:=1\n"
                  "  case 4,5,6\n"
                  "    b:=2\n"
                  "  case 7,8,9\n"
                  "    b:=3\n"
                  "endselect");
    TS_ASSERT_EQUALS(h.b, 1);

    // Match last in range
    h.a = 3;
    h.b = 0;
    h.checkStatement("select case a\n"
                  "  case 1,2,3\n"
                  "    b:=1\n"
                  "  case 4,5,6\n"
                  "    b:=2\n"
                  "  case 7,8,9\n"
                  "    b:=3\n"
                  "endselect");
    TS_ASSERT_EQUALS(h.b, 1);

    // Match last item
    h.a = 9;
    h.b = 0;
    h.checkStatement("select case a\n"
                  "  case 1,2,3\n"
                  "    b:=1\n"
                  "  case 4,5,6\n"
                  "    b:=2\n"
                  "  case 7,8,9\n"
                  "    b:=3\n"
                  "endselect");
    TS_ASSERT_EQUALS(h.b, 3);

    // Relations
    h.a = 5;
    h.b = 0;
    h.checkStatement("select case a\n"
                  "  case is <5\n"
                  "    b:=1\n"
                  "  case is >=5\n"
                  "    b:=2\n"
                  "endselect");
    TS_ASSERT_EQUALS(h.b, 2);

    // Empty
    h.a = 0;
    h.b = 0;
    h.checkStatement("select case a\n"
                  "endselect");

    // Empty with Else
    h.a = 0;
    h.b = 0;
    h.checkStatement("select case a\n"
                  "  case else\n"
                  "    b:=3\n"
                  "endselect");
    TS_ASSERT_EQUALS(h.b, 3);

    // Break from switch (interesting because both for and select place stuff on the stack)
    h.a = 0;
    h.b = 0;
    h.checkStatement("for c:=1 to 10 do\n"
                  "  select case c\n"
                  "    case 1,3,5,7,9\n"
                  "      b:=b+c\n"            // 1,3,5,7
                  "    case is <5\n"
                  "      b:=b+2*c\n"          // 2*2, 2*4
                  "    case is =8\n"
                  "      break\n"
                  "    case else\n"           // 3*6
                  "      b:=b+3*c\n"
                  "  endselect\n"
                  "next");
    TS_ASSERT_EQUALS(h.b, 46);

    // Continue from switch
    h.a = 0;
    h.b = 0;
    h.checkStatement("for c:=1 to 10 do\n"
                  "  select case c\n"
                  "    case 1,3,5,7,9\n"
                  "      b:=b+c\n"            // 1,3,5,7,9
                  "    case is =8\n"
                  "      continue\n"
                  "  endselect\n"
                  "  b:=b+1\n"
                  "next");
    TS_ASSERT_EQUALS(h.b, 34);
}

/** Test Eval statement. */
void
TestInterpreterStatementCompiler::testEval()
{
    // ex IntStatementTestSuite::testEval
    ExpressionTestHelper h;
    h.checkStatement("Eval 'a:=1'");
    TS_ASSERT_EQUALS(h.a, 1);

    h.checkStatement("Eval 'a:=2', 'b:=a+3'");
    TS_ASSERT_EQUALS(h.a, 2);
    TS_ASSERT_EQUALS(h.b, 5);

    h.checkStatement("Eval 'for a:=1 to 10', 'b:=a+5', 'next'");
    TS_ASSERT_EQUALS(h.b, 15);
}
