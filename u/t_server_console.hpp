/**
  *  \file u/t_server_console.hpp
  *  \brief Tests for server::console
  */
#ifndef C2NG_U_T_SERVER_CONSOLE_HPP
#define C2NG_U_T_SERVER_CONSOLE_HPP

#include <cxxtest/TestSuite.h>

class TestServerConsoleColorTerminal : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerConsoleCommandHandler : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerConsoleContext : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerConsoleContextFactory : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerConsoleDumbTerminal : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerConsoleEnvironment : public CxxTest::TestSuite {
 public:
    void testNamed();
    void testNameError();
    void testPositional();
};

class TestServerConsoleFundamentalCommandHandler : public CxxTest::TestSuite {
 public:
    void testForEach();
    void testForEachPreserve();
    void testForEachError();
    void testForEachUnrecognized();
    void testIf();
    void testIfFalse();
    void testIfElse();
    void testIfElseFalse();
    void testIfElsif();
    void testIfElsifFalse();
    void testIfMultiline();
    void testSetenv();
    void testEnv();
    void testEcho();
    void testErrors();
};

class TestServerConsoleIntegerCommandHandler : public CxxTest::TestSuite {
 public:
    void testInt();
    void testIntNot();
    void testIntAdd();
    void testIntSeq();
    void testError();
};

class TestServerConsoleNullTerminal : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerConsoleParser : public CxxTest::TestSuite {
 public:
    void testEval();
    void testString();
    void testPipe();
    void testVar();
    void testErrors();
    void testEvalBool();
    void testEmptyPipe();
    void testTypedPipe();
};

class TestServerConsolePipeTerminal : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerConsoleRouterContextFactory : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerConsoleStringCommandHandler : public CxxTest::TestSuite {
 public:
    void testStr();
    void testStrEq();
    void testStrEmpty();
    void testError();
};

class TestServerConsoleTerminal : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testPack();
};

#endif
