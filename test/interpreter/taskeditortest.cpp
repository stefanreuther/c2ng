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
        interpreter::TaskEditor testee(h.proc, false);
        a.checkEqual("11. process", &testee.process(), &h.proc);
        a.checkEqual("12. changed", testee.isChanged(), false);
    }

    // Process needs to be unchanged
    a.checkEqual("21. getNumActiveFrames", h.proc.getNumActiveFrames(), 0U);
    a.checkNull("22. getFreezer", h.proc.getFreezer());
}

/** Test adding to empty process.
    Process must report updated content. */
AFL_TEST("interpreter.TaskEditor:add-to-empty", a)
{
    // Create a blank process
    TestHarness h;
    a.checkEqual("01. getNumActiveFrames", h.proc.getNumActiveFrames(), 0U);

    // Use TaskEditor to add a line of code
    interpreter::TaskEditor(h.proc, false).addAtEnd(Commands_t::fromSingleObject("whatever"));

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
        interpreter::TaskEditor ed(h.proc, false);
        ed.addAtEnd(Commands_t::fromSingleObject("one"));
        ed.addAtEnd(Commands_t::fromSingleObject("two"));
        ed.addAtEnd(Commands_t::fromSingleObject("restart"));
        a.checkEqual("11. getNumInstructions", ed.getNumInstructions(), 3U);
        a.checkEqual("12. getPC", ed.getPC(), 0U);
        a.checkEqual("13. isInSubroutineCall", ed.isInSubroutineCall(), false);
        a.checkEqual("14. changed", ed.isChanged(), true);
    }

    // Verify process content: must be one frame
    a.checkEqual("21. getNumActiveFrames", h.proc.getNumActiveFrames(), 1U);
    a.checkNonNull("22. getOutermostFrame", h.proc.getOutermostFrame());
    a.check("23. getOutermostFrame", h.proc.getOutermostFrame()->bco->getNumInstructions() > 0);

    // Create another TaskEditor and verify its content
    interpreter::TaskEditor testee(h.proc, false);
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
    interpreter::TaskEditor ed(h.proc, false);

    // Another one cannot be made
    std::auto_ptr<interpreter::TaskEditor> other;
    AFL_CHECK_THROWS(a, other.reset(new interpreter::TaskEditor(h.proc, false)), interpreter::Error);
}

/** Test move(): forward. */
AFL_TEST("interpreter.TaskEditor:move:forward", a)
{
    TestHarness h;
    interpreter::TaskEditor ed(h.proc, false);
    ed.addAtEnd(Commands_t::fromSingleObject("one"));
    ed.addAtEnd(Commands_t::fromSingleObject("two"));
    ed.addAtEnd(Commands_t::fromSingleObject("three"));
    ed.addAtEnd(Commands_t::fromSingleObject("four"));
    ed.addAtEnd(Commands_t::fromSingleObject("five"));
    ed.setPC(1);
    ed.setCursor(4);

    // Move
    //    three
    //    four
    //    one
    // => two
    // c. five
    ed.move(0, 4, 2);
    a.checkEqual("01. getPC",              ed.getPC(), 3U);
    a.checkEqual("02. getCursor",          ed.getCursor(), 4U);
    a.checkEqual("03. getNumInstructions", ed.getNumInstructions(), 5U);

    a.checkEqual("11. content", ed[0], "three");
    a.checkEqual("12. content", ed[1], "four");
    a.checkEqual("13. content", ed[2], "one");
    a.checkEqual("14. content", ed[3], "two");
    a.checkEqual("15. content", ed[4], "five");
}

/** Test move(): forward, including counter. */
AFL_TEST("interpreter.TaskEditor:move:forward-counter", a)
{
    TestHarness h;
    interpreter::TaskEditor ed(h.proc, false);
    ed.addAtEnd(Commands_t::fromSingleObject("one"));
    ed.addAtEnd(Commands_t::fromSingleObject("two"));
    ed.addAtEnd(Commands_t::fromSingleObject("three"));
    ed.addAtEnd(Commands_t::fromSingleObject("four"));
    ed.addAtEnd(Commands_t::fromSingleObject("five"));
    ed.setPC(3);
    ed.setCursor(4);

    // Move
    //    one
    //    two
    // => four
    //    three
    // c. five
    ed.move(2, 4, 1);
    a.checkEqual("01. getPC",              ed.getPC(), 2U);
    a.checkEqual("02. getCursor",          ed.getCursor(), 4U);
    a.checkEqual("03. getNumInstructions", ed.getNumInstructions(), 5U);

    a.checkEqual("11. content", ed[0], "one");
    a.checkEqual("12. content", ed[1], "two");
    a.checkEqual("13. content", ed[2], "four");
    a.checkEqual("14. content", ed[3], "three");
    a.checkEqual("15. content", ed[4], "five");
}

/** Test move(): forward, range limit.
    If distance between from and to is less than n, n is adjusted accordingly.
    This means the operation becomes a no-op. */
AFL_TEST("interpreter.TaskEditor:move:forward-limit", a)
{
    TestHarness h;
    interpreter::TaskEditor ed(h.proc, false);
    ed.addAtEnd(Commands_t::fromSingleObject("one"));
    ed.addAtEnd(Commands_t::fromSingleObject("two"));
    ed.addAtEnd(Commands_t::fromSingleObject("three"));
    ed.addAtEnd(Commands_t::fromSingleObject("four"));
    ed.addAtEnd(Commands_t::fromSingleObject("five"));
    ed.setPC(3);
    ed.setCursor(1);

    // Move
    //    one
    // c. two
    //    three
    // => four
    //    five
    ed.move(2, 4, 4);
    a.checkEqual("01. getPC",              ed.getPC(), 3U);
    a.checkEqual("02. getCursor",          ed.getCursor(), 1U);
    a.checkEqual("03. getNumInstructions", ed.getNumInstructions(), 5U);

    a.checkEqual("11. content", ed[0], "one");
    a.checkEqual("12. content", ed[1], "two");
    a.checkEqual("13. content", ed[2], "three");
    a.checkEqual("14. content", ed[3], "four");
    a.checkEqual("15. content", ed[4], "five");
}

/** Test move(): backward. */
AFL_TEST("interpreter.TaskEditor:move:backward", a)
{
    TestHarness h;
    interpreter::TaskEditor ed(h.proc, false);
    ed.addAtEnd(Commands_t::fromSingleObject("one"));
    ed.addAtEnd(Commands_t::fromSingleObject("two"));
    ed.addAtEnd(Commands_t::fromSingleObject("three"));
    ed.addAtEnd(Commands_t::fromSingleObject("four"));
    ed.addAtEnd(Commands_t::fromSingleObject("five"));
    ed.setPC(4);
    ed.setCursor(2);

    // Move
    //    one
    // c. three
    //    four
    //    two
    // => five
    ed.move(2, 1, 2);
    a.checkEqual("01. getPC",              ed.getPC(), 4U);
    a.checkEqual("02. getCursor",          ed.getCursor(), 1U);
    a.checkEqual("03. getNumInstructions", ed.getNumInstructions(), 5U);

    a.checkEqual("11. content", ed[0], "one");
    a.checkEqual("12. content", ed[1], "three");
    a.checkEqual("13. content", ed[2], "four");
    a.checkEqual("14. content", ed[3], "two");
    a.checkEqual("15. content", ed[4], "five");
}

/** Test move(): backward across PC. */
AFL_TEST("interpreter.TaskEditor:move:backward:pc", a)
{
    TestHarness h;
    interpreter::TaskEditor ed(h.proc, false);
    ed.addAtEnd(Commands_t::fromSingleObject("one"));
    ed.addAtEnd(Commands_t::fromSingleObject("two"));
    ed.addAtEnd(Commands_t::fromSingleObject("three"));
    ed.setPC(1);
    ed.setCursor(2);

    // Move
    //    one
    // c. three
    // => two
    ed.move(2, 1, 1);
    a.checkEqual("01. getPC",              ed.getPC(), 2U);
    a.checkEqual("02. getCursor",          ed.getCursor(), 1U);
    a.checkEqual("03. getNumInstructions", ed.getNumInstructions(), 3U);

    a.checkEqual("11. content", ed[0], "one");
    a.checkEqual("12. content", ed[1], "three");
    a.checkEqual("13. content", ed[2], "two");
}

/** Test move(): out-of-range from. */
AFL_TEST("interpreter.TaskEditor:move:range:from", a)
{
    TestHarness h;
    interpreter::TaskEditor ed(h.proc, false);
    ed.addAtEnd(Commands_t::fromSingleObject("one"));
    ed.addAtEnd(Commands_t::fromSingleObject("two"));

    // Move
    ed.move(2, 1, 2);
    a.checkEqual("01. getNumInstructions", ed.getNumInstructions(), 2U);
    a.checkEqual("02. content", ed[0], "one");
    a.checkEqual("03. content", ed[1], "two");
}

/** Test move(): out-of-range to. */
AFL_TEST("interpreter.TaskEditor:move:range:to", a)
{
    TestHarness h;
    interpreter::TaskEditor ed(h.proc, false);
    ed.addAtEnd(Commands_t::fromSingleObject("one"));
    ed.addAtEnd(Commands_t::fromSingleObject("two"));

    // Move
    ed.move(1, 2, 2);
    a.checkEqual("01. getNumInstructions", ed.getNumInstructions(), 2U);
    a.checkEqual("02. content", ed[0], "one");
    a.checkEqual("03. content", ed[1], "two");
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
    AFL_CHECK_THROWS(a, ed.reset(new interpreter::TaskEditor(h.proc, false)), interpreter::Error);
}
