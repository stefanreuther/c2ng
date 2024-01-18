/**
  *  \file test/game/interface/completionlisttest.cpp
  *  \brief Test for game::interface::CompletionList
  */

#include "game/interface/completionlist.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"

namespace {
    bool hasCompletion(const game::interface::CompletionList& list, const String_t& what)
    {
        for (game::interface::CompletionList::Iterator_t it = list.begin(), end = list.end(); it != end; ++it) {
            if (*it == what) {
                return true;
            }
        }
        return false;
    }
}

/** Test initialisation. */
AFL_TEST("game.interface.CompletionList:init", a)
{
    game::interface::CompletionList testee("x");
    a.checkEqual("01. getStem", testee.getStem(), "x");
    a.checkEqual("02. isEmpty", testee.isEmpty(), true);
    a.checkEqual("03. getImmediateCompletion", testee.getImmediateCompletion(), "");

    testee.setStem("yy");
    a.checkEqual("11. getStem", testee.getStem(), "yy");
    a.checkEqual("12. isEmpty", testee.isEmpty(), true);
    a.checkEqual("13. getImmediateCompletion", testee.getImmediateCompletion(), "");

    a.check("21. empty", testee.begin() == testee.end());
}

/** Test addCandidate, normal case. */
AFL_TEST("game.interface.CompletionList:addCandidate", a)
{
    game::interface::CompletionList testee("se");
    testee.addCandidate("six");
    testee.addCandidate("seven");
    testee.addCandidate("several");

    a.checkEqual("01. isEmpty", testee.isEmpty(), false);
    a.checkEqual("02. getImmediateCompletion", testee.getImmediateCompletion(), "seve");

    game::interface::CompletionList::Iterator_t it = testee.begin();
    a.check("11. not end", it != testee.end());
    a.checkEqual("12. value", *it, "seven");

    ++it;
    a.check("21. not end", it != testee.end());
    a.checkEqual("22. value", *it, "several");

    ++it;
    a.check("31. end", it == testee.end());
}

/** Test addCandidate, handling of '$'. */
// Completion does not add $ in the middle
AFL_TEST("game.interface.CompletionList:addCandidate:middle-dollar", a)
{
    game::interface::CompletionList testee("a");
    testee.addCandidate("a$b");
    a.checkEqual("isEmpty", testee.isEmpty(), true);
}

// Stem containing a $ is completed normally
AFL_TEST("game.interface.CompletionList:addCandidate:entered-dollar", a)
{
    game::interface::CompletionList testee("a$");
    testee.addCandidate("a$b");
    a.checkEqual("isEmpty", testee.isEmpty(), false);
    a.checkEqual("result", *testee.begin(), "a$b");
}

// Word ending in $ is completed normally
AFL_TEST("game.interface.CompletionList:addCandidate:final-dollar", a)
{
    game::interface::CompletionList testee("a");
    testee.addCandidate("abc$");
    a.checkEqual("isEmpty", testee.isEmpty(), false);
    a.checkEqual("result", *testee.begin(), "abc$");
}

/** Test addCandidate, handling of mixed-case candidates. */
AFL_TEST("game.interface.CompletionList:addCandidate:mixed-case", a)
{
    game::interface::CompletionList testee("Se");
    testee.addCandidate("Six");
    testee.addCandidate("Seven");
    testee.addCandidate("sEvEral");

    a.checkEqual("01. isEmpty", testee.isEmpty(), false);
    a.checkEqual("02. getImmediateCompletion", testee.getImmediateCompletion(), "Sev");

    game::interface::CompletionList::Iterator_t it = testee.begin();
    a.check("11. not end", it != testee.end());
    a.checkEqual("12. value", *it, "Seven");

    ++it;
    a.check("21. not end", it != testee.end());
    a.checkEqual("22. value", *it, "sEvEral");

    ++it;
    a.check("31. end", it == testee.end());
}

/** Test buildCompletionList() for a session. */
AFL_TEST("game.interface.CompletionList:buildCompletionList", a)
{
    afl::string::NullTranslator tx;
    afl::io::InternalFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    afl::container::PtrVector<interpreter::Context> ctx;
    session.world().keymaps().createKeymap("KEYBOARD");
    session.world().keymaps().createKeymap("KEYMAP");
    fs.createDirectory("/root");
    fs.createDirectory("/home");
    fs.openFile("/home/file1", afl::io::FileSystem::Create);
    fs.openFile("/home/file2", afl::io::FileSystem::Create);

    // Regular command
    game::interface::CompletionList list;
    buildCompletionList(list, "pla", session, false, ctx);
    a.checkEqual("01", list.getStem(), "pla");
    a.check("02", hasCompletion(list, "Player"));
    a.check("03", hasCompletion(list, "Planet"));

    // Word in command
    buildCompletionList(list, "if pla", session, false, ctx);
    a.checkEqual("11", list.getStem(), "pla");
    a.check("12", hasCompletion(list, "Player"));
    a.check("13", hasCompletion(list, "Planet"));

    // Configuration
    buildCompletionList(list, "cfg(\"allo", session, false, ctx);
    a.checkEqual("21", list.getStem(), "allo");
    a.checkEqual("22", list.getImmediateCompletion(), "Allow");
    a.check("23", hasCompletion(list, "AllowPlanetAttacks"));
    a.check("24", hasCompletion(list, "AllowChunneling"));

    // Configuration
    buildCompletionList(list, "addconfig 'defen", session, false, ctx);
    a.checkEqual("31", list.getStem(), "defen");
    a.checkEqual("32", list.getImmediateCompletion(), "Defense");
    a.check("33", hasCompletion(list, "DefenseForUndetectable"));

    // Preferences
    buildCompletionList(list, "pref(\"displ", session, false, ctx);
    a.checkEqual("41", list.getStem(), "displ");
    a.checkEqual("42", list.getImmediateCompletion(), "Display.");
    a.check("43", hasCompletion(list, "Display.Clans"));

    // Preferences
    buildCompletionList(list, "addpref \"displ", session, false, ctx);
    a.checkEqual("51", list.getStem(), "displ");
    a.checkEqual("52", list.getImmediateCompletion(), "Display.");
    a.check("53", hasCompletion(list, "Display.Clans"));

    // Keymaps
    buildCompletionList(list, "bind k", session, false, ctx);
    a.checkEqual("61", list.getStem(), "k");
    a.checkEqual("62", list.getImmediateCompletion(), "Key");
    a.check("63", hasCompletion(list, "Keyboard"));
    a.check("64", hasCompletion(list, "Keymap"));

    // Keymaps
    buildCompletionList(list, "usekeymap Keyb", session, false, ctx);
    a.checkEqual("71", list.getStem(), "Keyb");
    a.checkEqual("72", list.getImmediateCompletion(), "Keyboard");
    a.check("73", hasCompletion(list, "Keyboard"));

    // Empty
    buildCompletionList(list, "", session, false, ctx);
    a.checkEqual("81", list.getStem(), "");
    a.check("82", list.isEmpty());

    // Space after
    buildCompletionList(list, "pla ", session, false, ctx);
    a.checkEqual("91", list.getStem(), "");
    a.check("92", list.isEmpty());

    // Directory
    buildCompletionList(list, "open \"/r", session, false, ctx);
    a.checkEqual("101", list.getStem(), "/r");
    a.checkEqual("102", list.getImmediateCompletion(), "/root/");
    a.check("103", hasCompletion(list, "/root/"));

    // File
    buildCompletionList(list, "open \"/home/f", session, false, ctx);
    a.checkEqual("111", list.getStem(), "/home/f");
    a.checkEqual("112", list.getImmediateCompletion(), "/home/file");
    a.check("113", hasCompletion(list, "/home/file1"));
    a.check("114", hasCompletion(list, "/home/file2"));
}
