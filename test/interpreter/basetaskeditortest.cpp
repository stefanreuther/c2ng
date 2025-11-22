/**
  *  \file test/interpreter/basetaskeditortest.cpp
  *  \brief Test for interpreter::BaseTaskEditor
  */

#include "interpreter/basetaskeditor.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"

using interpreter::BaseTaskEditor;

/*
 *  For historical reasons, many tests are in TaskEditorTest.
 *  (In particular, everything involving load/save.)
 */

/** Basic test. */
AFL_TEST("interpreter.BaseTaskEditor:basic", a)
{
    BaseTaskEditor ed;
    a.check("01. isChanged", !ed.isChanged());

    String_t lines[] = { "a", "b" };
    ed.addAtEnd(lines);
    a.check("11. isChanged", ed.isChanged());
    a.checkEqual("12. getNumInstructions", ed.getNumInstructions(), 2U);
    a.checkEqual("13. index 0", ed[0], "a");
    a.checkEqual("14. index 1", ed[1], "b");

    ed.clear();
    a.check("21. isChanged", ed.isChanged());
    a.checkEqual("22. getNumInstructions", ed.getNumInstructions(), 0U);
}

/** save() with excessive size. */
AFL_TEST("interpreter.BaseTaskEditor:save-too-large", a)
{
    // Create
    BaseTaskEditor ed;
    a.check("01. isChanged", !ed.isChanged());

    // Add 70k lines of code. An auto task cannot be that long.
    for (int32_t i = 0; i < 70000; ++i) {
        String_t lines[] = { afl::string::Format("a%d", i) };
        ed.addAtEnd(lines);
    }
    a.check("11. isChanged", ed.isChanged());

    // Do it. Call must succeed.
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::Process proc(world, "proc", 99);
    AFL_CHECK_SUCCEEDS(a("21. save"), ed.save(proc, false));
    a.check("22. isChanged", !ed.isChanged());
}

/** Test isValidCommand(). */
AFL_TEST("interpreter.BaseTaskEditor:isValidCommand", a)
{
    a.check("01", BaseTaskEditor::isValidCommand("MoveTo 1,2"));
    a.check("02", BaseTaskEditor::isValidCommand("Print \"Hi mom\""));
    a.check("03", BaseTaskEditor::isValidCommand(""));

    a.check("11", !BaseTaskEditor::isValidCommand("If x Then Print \"Hi mom\""));
    a.check("12", !BaseTaskEditor::isValidCommand("If x"));
    a.check("13", !BaseTaskEditor::isValidCommand("Break"));
    a.check("14", !BaseTaskEditor::isValidCommand("Function f"));
    a.check("15", !BaseTaskEditor::isValidCommand("(x+1)"));
    a.check("16", !BaseTaskEditor::isValidCommand("Print 'hi"));   // unbalanced quotes
    a.check("17", !BaseTaskEditor::isValidCommand("'foo'"));
    a.check("18", !BaseTaskEditor::isValidCommand("~"));           // invalid token
    a.check("19", !BaseTaskEditor::isValidCommand("Print ~"));     // invalid token
}

/** Test isBlankCommand(). */
AFL_TEST("interpreter.BaseTaskEditor:isBlankCommand", a)
{
    a.check("01", BaseTaskEditor::isBlankCommand(""));
    a.check("02", BaseTaskEditor::isBlankCommand("         "));
    a.check("03", BaseTaskEditor::isBlankCommand("   %foo"));

    a.check("11", !BaseTaskEditor::isBlankCommand("Print 'hi'"));
    a.check("12", !BaseTaskEditor::isBlankCommand("Print 'hi"));   // unbalanced quotes
    a.check("13", !BaseTaskEditor::isBlankCommand("'hi"));         // unbalanced quotes (throw on first token)
    a.check("14", !BaseTaskEditor::isBlankCommand("~"));           // invalid token
}

/** Test isRestartCommand(). */
AFL_TEST("interpreter.BaseTaskEditor:isRestartCommand", a)
{
    a.check("01", BaseTaskEditor::isRestartCommand("Restart"));
    a.check("02", BaseTaskEditor::isRestartCommand("      Restart   "));

    a.check("11", !BaseTaskEditor::isRestartCommand("%Restart"));
    a.check("12", !BaseTaskEditor::isRestartCommand("'hi"));       // unbalanced quotes
    a.check("13", !BaseTaskEditor::isRestartCommand("~"));         // invalid token
}
