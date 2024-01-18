/**
  *  \file test/interpreter/taskeditortest.cpp
  *  \brief Test for interpreter::TaskEditor
  */

#include "interpreter/taskeditor.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/world.hpp"
#include <memory>

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
AFL_TEST("interpreter.TaskEditor:empty", a)
{
    // Create a blank process
    TestHarness h;
    a.checkEqual("01. getNumActiveFrames", h.proc.getNumActiveFrames(), 0U);

    // Create and destroy editor
    {
        interpreter::TaskEditor testee(h.proc);
        a.checkEqual("11. process", &testee.process(), &h.proc);
    }

    // Process needs to be unchanged
    a.checkEqual("21. getNumActiveFrames", h.proc.getNumActiveFrames(), 0U);
}

/** Test adding to empty process.
    Process must report updated content. */
AFL_TEST("interpreter.TaskEditor:add-to-empty", a)
{
    // Create a blank process
    TestHarness h;
    a.checkEqual("01. getNumActiveFrames", h.proc.getNumActiveFrames(), 0U);

    // Use TaskEditor to add a line of code
    interpreter::TaskEditor(h.proc).addAtEnd(Commands_t::fromSingleObject("whatever"));

    // Verify process content: must be one frame
    a.checkEqual("11. getNumActiveFrames", h.proc.getNumActiveFrames(), 1U);
    a.checkNonNull("12. getOutermostFrame", h.proc.getOutermostFrame());
    a.check("13. getOutermostFrame", h.proc.getOutermostFrame()->bco->getNumInstructions() > 0);
}

/** Test round-trip operation.
    TaskEditor must be able to parse what it created. */
AFL_TEST("interpreter.TaskEditor:roundtrip", a)
{
    // Create a blank process
    TestHarness h;
    a.checkEqual("01. getNumActiveFrames", h.proc.getNumActiveFrames(), 0U);

    // Use TaskEditor to add some code
    {
        interpreter::TaskEditor ed(h.proc);
        ed.addAtEnd(Commands_t::fromSingleObject("one"));
        ed.addAtEnd(Commands_t::fromSingleObject("two"));
        ed.addAtEnd(Commands_t::fromSingleObject("restart"));
        a.checkEqual("11. getNumInstructions", ed.getNumInstructions(), 3U);
        a.checkEqual("12. getPC", ed.getPC(), 0U);
        a.checkEqual("13. isInSubroutineCall", ed.isInSubroutineCall(), false);
    }

    // Verify process content: must be one frame
    a.checkEqual("21. getNumActiveFrames", h.proc.getNumActiveFrames(), 1U);
    a.checkNonNull("22. getOutermostFrame", h.proc.getOutermostFrame());
    a.check("23. getOutermostFrame", h.proc.getOutermostFrame()->bco->getNumInstructions() > 0);

    // Create another TaskEditor and verify its content
    interpreter::TaskEditor testee(h.proc);
    a.checkEqual("31. getNumInstructions", testee.getNumInstructions(), 3U);
    a.checkEqual("32. content", testee[0], "one");
    a.checkEqual("33. content", testee[1], "two");
    a.checkEqual("34. content", testee[2], "Restart");   // note how spelling has been normalized
    a.checkEqual("35. getPC", testee.getPC(), 0U);
    a.checkEqual("36. isInSubroutineCall", testee.isInSubroutineCall(), false);
}

/** Test conflict.
    We cannot create two TaskEditor for one process. */
AFL_TEST("interpreter.TaskEditor:conflict", a)
{
    // Create a blank process
    TestHarness h;

    // Create TaskEditor
    interpreter::TaskEditor ed(h.proc);

    // Another one cannot be made
    std::auto_ptr<interpreter::TaskEditor> other;
    AFL_CHECK_THROWS(a, other.reset(new interpreter::TaskEditor(h.proc)), interpreter::Error);
}

/** Test format error.
    Test error handling if process cannot be parsed. */
AFL_TEST("interpreter.TaskEditor:process-format", a)
{
    // Create a blank process
    TestHarness h;

    // Create a BCO and push a frame
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 42);
    h.proc.pushFrame(bco, true);

    // Creating a TaskEditor will fail
    std::auto_ptr<interpreter::TaskEditor> ed;
    AFL_CHECK_THROWS(a, ed.reset(new interpreter::TaskEditor(h.proc)), interpreter::Error);
}

/** Test isValidCommand(). */
AFL_TEST("interpreter.TaskEditor:isValidCommand", a)
{
    using interpreter::TaskEditor;
    a.check("01", TaskEditor::isValidCommand("MoveTo 1,2"));
    a.check("02", TaskEditor::isValidCommand("Print \"Hi mom\""));
    a.check("03", TaskEditor::isValidCommand(""));

    a.check("11", !TaskEditor::isValidCommand("If x Then Print \"Hi mom\""));
    a.check("12", !TaskEditor::isValidCommand("If x"));
    a.check("13", !TaskEditor::isValidCommand("Break"));
    a.check("14", !TaskEditor::isValidCommand("Function f"));
    a.check("15", !TaskEditor::isValidCommand("(x+1)"));
    a.check("16", !TaskEditor::isValidCommand("Print 'hi"));       // unbalanced quotes
    a.check("17", !TaskEditor::isValidCommand("'foo'"));
    a.check("18", !TaskEditor::isValidCommand("~"));               // invalid token
}
