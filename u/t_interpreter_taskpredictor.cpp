/**
  *  \file u/t_interpreter_taskpredictor.cpp
  *  \brief Test for interpreter::TaskPredictor
  */

#include "interpreter/taskpredictor.hpp"

#include "t_interpreter.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/taskeditor.hpp"
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
        interpreter::Process process;
        interpreter::TaskEditor editor;

        TestHarness()
            : log(), tx(), fs(), world(log, tx, fs), process(world, "pro", 99), editor(process)
            { }
    };
}


/** Task prediction, regular case. */
void
TestInterpreterTaskPredictor::testPredictTask()
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
        TS_ASSERT_EQUALS(p.get(), "SECOND(),THIRD(),FOURTH()");
    }

    // Predict from PC to given location, inclusive
    {
        TestPredictor p;
        p.predictTask(h.editor, 3);
        TS_ASSERT_EQUALS(p.get(), "SECOND(),THIRD()");
    }

    // Predict single statement
    {
        TestPredictor p;
        p.predictStatement(h.editor, 2);
        TS_ASSERT_EQUALS(p.get(), "THIRD()");
    }

    // Predict out-of-range statement
    {
        TestPredictor p;
        p.predictStatement(h.editor, 4);
        TS_ASSERT_EQUALS(p.get(), "");
    }
}

/** Task prediction, "Restart". */
void
TestInterpreterTaskPredictor::testPredictRestart()
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
        TS_ASSERT_EQUALS(p.get(), "B(),C(),A()");
    }

    // Predict from PC to given location, inclusive
    {
        TestPredictor p;
        p.predictTask(h.editor, 3);
        TS_ASSERT_EQUALS(p.get(), "B(),C()");
    }

    // Predict single "Restart" statement
    {
        TestPredictor p;
        p.predictStatement(h.editor, 3);
        TS_ASSERT_EQUALS(p.get(), "");
    }
}

/** Task prediction, more "Restart" cases. */
void
TestInterpreterTaskPredictor::testPredictRestart2()
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
        TS_ASSERT_EQUALS(p.get(), "A(),B(),C()");
    }

    // Predict from PC to given location; nothing predicted as PC already beyond
    {
        TestPredictor p;
        p.predictTask(h.editor, 2);
        TS_ASSERT_EQUALS(p.get(), "");
    }
}

/** Task prediction, error case. */
void
TestInterpreterTaskPredictor::testPredictError()
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
        TS_ASSERT_EQUALS(p.get(), "B()");
    }

    // Predict erroneous line
    {
        TestPredictor p;
        p.predictStatement(h.editor, 2);
        TS_ASSERT_EQUALS(p.get(), "");
    }

    // Predict erroneous line
    {
        TestPredictor p;
        p.predictStatement(h.editor, 3);
        TS_ASSERT_EQUALS(p.get(), "");
    }

    // Predict comment line
    {
        TestPredictor p;
        p.predictStatement(h.editor, 4);
        TS_ASSERT_EQUALS(p.get(), "");
    }
}

/** Single statement prediction (without a task). */
void
TestInterpreterTaskPredictor::testPredictStatement()
{
    // Base case
    {
        TestPredictor p;
        p.predictStatement("hi");
        TS_ASSERT_EQUALS(p.get(), "HI()");
    }

    // One arg
    {
        TestPredictor p;
        p.predictStatement("hi 1");
        TS_ASSERT_EQUALS(p.get(), "HI(1)");
    }

    // Two args
    {
        TestPredictor p;
        p.predictStatement("hi 1,2");
        TS_ASSERT_EQUALS(p.get(), "HI(1,2)");
    }

    // Signed ints
    {
        TestPredictor p;
        p.predictStatement("hi -1,+2");
        TS_ASSERT_EQUALS(p.get(), "HI(-1,2)");
    }

    // Signed float
    {
        TestPredictor p;
        p.predictStatement("hi -1.5,+3.5");
        TS_ASSERT_EQUALS(p.get(), "HI(-1.5,3.5)");
    }

    // Bool
    {
        TestPredictor p;
        p.predictStatement("hi false,true");
        TS_ASSERT_EQUALS(p.get(), "HI(False,True)");
    }

    // String
    {
        TestPredictor p;
        p.predictStatement("set 'ho'");
        TS_ASSERT_EQUALS(p.get(), "SET(\"ho\")");
    }

    // "Restart" special case
    {
        TestPredictor p;
        p.predictStatement("restart");
        TS_ASSERT_EQUALS(p.get(), "");
    }
}

/** Single statement prediction, error cases.
    These all cause the call to be ignored. */
void
TestInterpreterTaskPredictor::testPredictStatementError()
{
    // Partial arg
    {
        TestPredictor p;
        p.predictStatement("hi +");
        TS_ASSERT_EQUALS(p.get(), "");
    }

    // Lexer error
    {
        TestPredictor p;
        p.predictStatement("hi '");
        TS_ASSERT_EQUALS(p.get(), "");
    }

    // Unsupported arg
    {
        TestPredictor p;
        p.predictStatement("hi ho");
        TS_ASSERT_EQUALS(p.get(), "");
    }

    // Unsupported arg
    {
        TestPredictor p;
        p.predictStatement("hi 1,ho");
        TS_ASSERT_EQUALS(p.get(), "");
    }

    // Unsupported arg
    {
        TestPredictor p;
        p.predictStatement("hi 5*9");
        TS_ASSERT_EQUALS(p.get(), "");
    }
}
