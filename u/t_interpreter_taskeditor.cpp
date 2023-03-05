/**
  *  \file u/t_interpreter_taskeditor.cpp
  *  \brief Test for interpreter::TaskEditor
  */

#include <memory>
#include "interpreter/taskeditor.hpp"

#include "t_interpreter.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/error.hpp"
#include "interpreter/world.hpp"

namespace {
    struct TestHarness {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        interpreter::Process proc;

        TestHarness()
            : log(), tx(), fs(),
              world(log, tx, fs),
              proc(world, "proc", 77)
            { }
    };

    typedef interpreter::TaskEditor::Commands_t Commands_t;
}


/** Test empty process.
    Process must remain unchanged. */
void
TestInterpreterTaskEditor::testEmpty()
{
    // Create a blank process
    TestHarness h;
    TS_ASSERT_EQUALS(h.proc.getNumActiveFrames(), 0U);

    // Create and destroy editor
    {
        interpreter::TaskEditor testee(h.proc);
        TS_ASSERT_EQUALS(&testee.process(), &h.proc);
    }

    // Process needs to be unchanged
    TS_ASSERT_EQUALS(h.proc.getNumActiveFrames(), 0U);
}

/** Test adding to empty process.
    Process must report updated content. */
void
TestInterpreterTaskEditor::testAddToEmpty()
{
    // Create a blank process
    TestHarness h;
    TS_ASSERT_EQUALS(h.proc.getNumActiveFrames(), 0U);

    // Use TaskEditor to add a line of code
    interpreter::TaskEditor(h.proc).addAtEnd(Commands_t::fromSingleObject("whatever"));

    // Verify process content: must be one frame
    TS_ASSERT_EQUALS(h.proc.getNumActiveFrames(), 1U);
    TS_ASSERT(h.proc.getOutermostFrame() != 0);
    TS_ASSERT(h.proc.getOutermostFrame()->bco->getNumInstructions() > 0);
}

/** Test round-trip operation.
    TaskEditor must be able to parse what it created. */
void
TestInterpreterTaskEditor::testRoundtrip()
{
    // Create a blank process
    TestHarness h;
    TS_ASSERT_EQUALS(h.proc.getNumActiveFrames(), 0U);

    // Use TaskEditor to add some code
    {
        interpreter::TaskEditor ed(h.proc);
        ed.addAtEnd(Commands_t::fromSingleObject("one"));
        ed.addAtEnd(Commands_t::fromSingleObject("two"));
        ed.addAtEnd(Commands_t::fromSingleObject("restart"));
        TS_ASSERT_EQUALS(ed.getNumInstructions(), 3U);
        TS_ASSERT_EQUALS(ed.getPC(), 0U);
        TS_ASSERT_EQUALS(ed.isInSubroutineCall(), false);
    }

    // Verify process content: must be one frame
    TS_ASSERT_EQUALS(h.proc.getNumActiveFrames(), 1U);
    TS_ASSERT(h.proc.getOutermostFrame() != 0);
    TS_ASSERT(h.proc.getOutermostFrame()->bco->getNumInstructions() > 0);

    // Create another TaskEditor and verify its content
    interpreter::TaskEditor testee(h.proc);
    TS_ASSERT_EQUALS(testee.getNumInstructions(), 3U);
    TS_ASSERT_EQUALS(testee[0], "one");
    TS_ASSERT_EQUALS(testee[1], "two");
    TS_ASSERT_EQUALS(testee[2], "Restart");   // note how spelling has been normalized
    TS_ASSERT_EQUALS(testee.getPC(), 0U);
    TS_ASSERT_EQUALS(testee.isInSubroutineCall(), false);
}

/** Test conflict.
    We cannot create two TaskEditor for one process. */
void
TestInterpreterTaskEditor::testConflict()
{
    // Create a blank process
    TestHarness h;

    // Create TaskEditor
    interpreter::TaskEditor ed(h.proc);

    // Another one cannot be made
    std::auto_ptr<interpreter::TaskEditor> other;
    TS_ASSERT_THROWS(other.reset(new interpreter::TaskEditor(h.proc)), interpreter::Error);
}

/** Test format error.
    Test error handling if process cannot be parsed. */
void
TestInterpreterTaskEditor::testFormat()
{
    // Create a blank process
    TestHarness h;

    // Create a BCO and push a frame
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 42);
    h.proc.pushFrame(bco, true);

    // Creating a TaskEditor will fail
    std::auto_ptr<interpreter::TaskEditor> ed;
    TS_ASSERT_THROWS(ed.reset(new interpreter::TaskEditor(h.proc)), interpreter::Error);
}

/** Test isValidCommand(). */
void
TestInterpreterTaskEditor::testIsValidCommand()
{
    using interpreter::TaskEditor;
    TS_ASSERT(TaskEditor::isValidCommand("MoveTo 1,2"));
    TS_ASSERT(TaskEditor::isValidCommand("Print \"Hi mom\""));
    TS_ASSERT(TaskEditor::isValidCommand(""));

    TS_ASSERT(!TaskEditor::isValidCommand("If x Then Print \"Hi mom\""));
    TS_ASSERT(!TaskEditor::isValidCommand("If x"));
    TS_ASSERT(!TaskEditor::isValidCommand("Break"));
    TS_ASSERT(!TaskEditor::isValidCommand("Function f"));
    TS_ASSERT(!TaskEditor::isValidCommand("(x+1)"));
    TS_ASSERT(!TaskEditor::isValidCommand("Print 'hi"));       // unbalanced quotes
    TS_ASSERT(!TaskEditor::isValidCommand("'foo'"));
    TS_ASSERT(!TaskEditor::isValidCommand("~"));               // invalid token
}

