/**
  *  \file test/interpreter/taskpredictortest.cpp
  *  \brief Test for interpreter::TaskPredictor
  */

#include "interpreter/taskpredictor.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/basetaskeditor.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

namespace {
    class TestPredictor : public interpreter::TaskPredictor {
     public:
        virtual bool predictInstruction(const String_t& name, interpreter::Arguments& args)
            {
                if (!m_accumulator.empty()) {
                    m_accumulator += ",";
                }
                m_accumulator += name;
                m_accumulator += "(";
                while (args.getNumArgs() != 0) {
                    m_accumulator += interpreter::toString(args.getNext(), true);
                    if (args.getNumArgs() != 0) {
                        m_accumulator += ",";
                    }
                }
                m_accumulator += ")";
                return true;
            }

        String_t get() const
            { return m_accumulator; }

     private:
        String_t m_accumulator;
    };


    class TestHarness {
     public:
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        interpreter::BaseTaskEditor editor;

        TestHarness()
            : log(), tx(), fs(), world(log, tx, fs), editor()
            { }
    };
}


/** Task prediction, regular case. */
AFL_TEST("interpreter.TaskPredictor:predictTask", a)
{
    // Prepare a task editor
    TestHarness h;
    String_t task[] = {
        "first",
        "second",     // <- PC
        "third",
        "fourth"
    };
    h.editor.addAtEnd(task);
    h.editor.setPC(1);

    // Predict from PC to end
    {
        TestPredictor p;
        p.predictTask(h.editor);
        a.checkEqual("01. get", p.get(), "SECOND(),THIRD(),FOURTH()");
    }

    // Predict from PC to given location, inclusive
    {
        TestPredictor p;
        p.predictTask(h.editor, 3);
        a.checkEqual("11. get", p.get(), "SECOND(),THIRD()");
    }

    // Predict single statement
    {
        TestPredictor p;
        p.predictStatement(h.editor, 2);
        a.checkEqual("21. get", p.get(), "THIRD()");
    }

    // Predict out-of-range statement
    {
        TestPredictor p;
        p.predictStatement(h.editor, 4);
        a.checkEqual("31. get", p.get(), "");
    }
}

/** Task prediction, "Restart". */
AFL_TEST("interpreter.TaskPredictor:predictTask:Restart", a)
{
    // Prepare a task editor
    TestHarness h;
    String_t task[] = {
        "a",
        "b",     // <- PC
        "c",
        "restart"
    };
    h.editor.addAtEnd(task);
    h.editor.setPC(1);

    // Predict from PC, one loop
    {
        TestPredictor p;
        p.predictTask(h.editor);
        a.checkEqual("01. get", p.get(), "B(),C(),A()");
    }

    // Predict from PC to given location, inclusive
    {
        TestPredictor p;
        p.predictTask(h.editor, 3);
        a.checkEqual("11. get", p.get(), "B(),C()");
    }

    // Predict single "Restart" statement
    {
        TestPredictor p;
        p.predictStatement(h.editor, 3);
        a.checkEqual("21. get", p.get(), "");
    }
}

/** Task prediction, more "Restart" cases. */
AFL_TEST("interpreter.TaskPredictor:predictTask:cursor-at-Restart", a)
{
    // Prepare a task editor
    TestHarness h;
    String_t task[] = {
        "a",
        "b",
        "c",
        "% com",
        "restart",    // <- PC
        "xx"
    };
    h.editor.addAtEnd(task);
    h.editor.setPC(3);

    // Predict from PC at "Restart" instruction, one loop
    {
        TestPredictor p;
        p.predictTask(h.editor);
        a.checkEqual("01. get", p.get(), "A(),B(),C()");
    }

    // Predict from PC to given location; nothing predicted as PC already beyond
    {
        TestPredictor p;
        p.predictTask(h.editor, 2);
        a.checkEqual("11. get", p.get(), "");
    }
}

/** Task prediction, error case. */
AFL_TEST("interpreter.TaskPredictor:predictTask:error", a)
{
    // Prepare a task editor
    TestHarness h;
    String_t task[] = {
        "a",
        "b",     // <- PC
        "c'd",
        "e +",
        "% com",
        "f",
    };
    h.editor.addAtEnd(task);
    h.editor.setPC(1);

    // Predict from PC to end, stop at error
    {
        TestPredictor p;
        p.predictTask(h.editor);
        a.checkEqual("01. get", p.get(), "B()");
    }

    // Predict erroneous line
    {
        TestPredictor p;
        p.predictStatement(h.editor, 2);
        a.checkEqual("11. get", p.get(), "");
    }

    // Predict erroneous line
    {
        TestPredictor p;
        p.predictStatement(h.editor, 3);
        a.checkEqual("21. get", p.get(), "");
    }

    // Predict comment line
    {
        TestPredictor p;
        p.predictStatement(h.editor, 4);
        a.checkEqual("31. get", p.get(), "");
    }
}

/*
 *  Single statement prediction (without a task).
 */

// Base case
AFL_TEST("interpreter.TaskPredictor:predictStatement", a)
{
    TestPredictor p;
    p.predictStatement("hi");
    a.checkEqual("get", p.get(), "HI()");
}

// One arg
AFL_TEST("interpreter.TaskPredictor:predictStatement:one-arg", a)
{
    TestPredictor p;
    p.predictStatement("hi 1");
    a.checkEqual("get", p.get(), "HI(1)");
}

// Two args
AFL_TEST("interpreter.TaskPredictor:predictStatement:two-args", a)
{
    TestPredictor p;
    p.predictStatement("hi 1,2");
    a.checkEqual("get", p.get(), "HI(1,2)");
}

// Signed ints
AFL_TEST("interpreter.TaskPredictor:predictStatement:signed-int", a)
{
    TestPredictor p;
    p.predictStatement("hi -1,+2");
    a.checkEqual("get", p.get(), "HI(-1,2)");
}

// Signed float
AFL_TEST("interpreter.TaskPredictor:predictStatement:signed-float", a)
{
    TestPredictor p;
    p.predictStatement("hi -1.5,+3.5");
    a.checkEqual("get", p.get(), "HI(-1.5,3.5)");
}

// Bool
AFL_TEST("interpreter.TaskPredictor:predictStatement:bool", a)
{
    TestPredictor p;
    p.predictStatement("hi false,true");
    a.checkEqual("get", p.get(), "HI(False,True)");
}

// String
AFL_TEST("interpreter.TaskPredictor:predictStatement:string", a)
{
    TestPredictor p;
    p.predictStatement("set 'ho'");
    a.checkEqual("get", p.get(), "SET(\"ho\")");
}

// "Restart" special case
AFL_TEST("interpreter.TaskPredictor:predictStatement:Restart", a)
{
    TestPredictor p;
    p.predictStatement("restart");
    a.checkEqual("get", p.get(), "");
}


/** Single statement prediction, error cases.
    These all cause the call to be ignored. */

// Partial arg
AFL_TEST("interpreter.TaskPredictor:predictStatement:error:partial-arg", a)
{
    TestPredictor p;
    p.predictStatement("hi +");
    a.checkEqual("get", p.get(), "");
}

// Lexer error
AFL_TEST("interpreter.TaskPredictor:predictStatement:error:lexer-error", a)
{
    TestPredictor p;
    p.predictStatement("hi '");
    a.checkEqual("get", p.get(), "");
}

// Unsupported arg
AFL_TEST("interpreter.TaskPredictor:predictStatement:error:unsupported-arg", a)
{
    TestPredictor p;
    p.predictStatement("hi ho");
    a.checkEqual("get", p.get(), "");
}

// Unsupported arg
AFL_TEST("interpreter.TaskPredictor:predictStatement:error:unsupported-arg-2", a)
{
    TestPredictor p;
    p.predictStatement("hi 1,ho");
    a.checkEqual("get", p.get(), "");
}

// Unsupported arg
AFL_TEST("interpreter.TaskPredictor:predictStatement:error:unsupported-arg-3", a)
{
    TestPredictor p;
    p.predictStatement("hi 5*9");
    a.checkEqual("get", p.get(), "");
}
