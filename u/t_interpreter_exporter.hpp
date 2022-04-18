/**
  *  \file u/t_interpreter_exporter.hpp
  *  \brief Tests for interpreter::exporter
  */
#ifndef C2NG_U_T_INTERPRETER_EXPORTER_HPP
#define C2NG_U_T_INTERPRETER_EXPORTER_HPP

#include <cxxtest/TestSuite.h>

class TestInterpreterExporterConfiguration : public CxxTest::TestSuite {
 public:
    void testIt();
    void testLoad();
};

class TestInterpreterExporterDbfExporter : public CxxTest::TestSuite {
 public:
    void testIt();
    void testIt2();
    void testCharset();
};

class TestInterpreterExporterExporter : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testIt();
    void testError();
};

class TestInterpreterExporterFieldList : public CxxTest::TestSuite {
 public:
    void testAdd();
    void testAddList();
    void testModify();
    void testCopy();
};

class TestInterpreterExporterFormat : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterExporterHtmlExporter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterExporterJsonExporter : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
    void testArray();
};

class TestInterpreterExporterSeparatedTextExporter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterExporterTextExporter : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testSimple();
    void testSimpleLong();
    void testEmptyBox();
    void testBox();
    void testLongBox();
};

#endif
