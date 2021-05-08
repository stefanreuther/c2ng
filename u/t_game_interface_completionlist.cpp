/**
  *  \file u/t_game_interface_completionlist.cpp
  *  \brief Test for game::interface::CompletionList
  */

#include "game/interface/completionlist.hpp"

#include "t_game_interface.hpp"
#include "game/session.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"
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
void
TestGameInterfaceCompletionList::testInit()
{
    game::interface::CompletionList testee("x");
    TS_ASSERT_EQUALS(testee.getStem(), "x");
    TS_ASSERT_EQUALS(testee.isEmpty(), true);
    TS_ASSERT_EQUALS(testee.getImmediateCompletion(), "");

    testee.setStem("yy");
    TS_ASSERT_EQUALS(testee.getStem(), "yy");
    TS_ASSERT_EQUALS(testee.isEmpty(), true);
    TS_ASSERT_EQUALS(testee.getImmediateCompletion(), "");

    TS_ASSERT_EQUALS(testee.begin(), testee.end());
}

/** Test addCandidate, normal case. */
void
TestGameInterfaceCompletionList::testAddCandidate()
{
    game::interface::CompletionList testee("se");
    testee.addCandidate("six");
    testee.addCandidate("seven");
    testee.addCandidate("several");

    TS_ASSERT_EQUALS(testee.isEmpty(), false);
    TS_ASSERT_EQUALS(testee.getImmediateCompletion(), "seve");

    game::interface::CompletionList::Iterator_t it = testee.begin();
    TS_ASSERT_DIFFERS(it, testee.end());
    TS_ASSERT_EQUALS(*it, "seven");

    ++it;
    TS_ASSERT_DIFFERS(it, testee.end());
    TS_ASSERT_EQUALS(*it, "several");

    ++it;
    TS_ASSERT_EQUALS(it, testee.end());
}

/** Test addCandidate, handling of '$'. */
void
TestGameInterfaceCompletionList::testAddCandidateDollar()
{
    // Completion does not add $ in the middle
    {
        game::interface::CompletionList testee("a");
        testee.addCandidate("a$b");
        TS_ASSERT_EQUALS(testee.isEmpty(), true);
    }

    // Stem containing a $ is completed normally
    {
        game::interface::CompletionList testee("a$");
        testee.addCandidate("a$b");
        TS_ASSERT_EQUALS(testee.isEmpty(), false);
        TS_ASSERT_EQUALS(*testee.begin(), "a$b");
    }

    // Word ending in $ is completed normally
    {
        game::interface::CompletionList testee("a");
        testee.addCandidate("abc$");
        TS_ASSERT_EQUALS(testee.isEmpty(), false);
        TS_ASSERT_EQUALS(*testee.begin(), "abc$");
    }
}

/** Test addCandidate, handling of mixed-case candidates. */
void
TestGameInterfaceCompletionList::testAddCandidateMixedCase()
{
    game::interface::CompletionList testee("Se");
    testee.addCandidate("Six");
    testee.addCandidate("Seven");
    testee.addCandidate("sEvEral");

    TS_ASSERT_EQUALS(testee.isEmpty(), false);
    TS_ASSERT_EQUALS(testee.getImmediateCompletion(), "Sev");

    game::interface::CompletionList::Iterator_t it = testee.begin();
    TS_ASSERT_DIFFERS(it, testee.end());
    TS_ASSERT_EQUALS(*it, "Seven");

    ++it;
    TS_ASSERT_DIFFERS(it, testee.end());
    TS_ASSERT_EQUALS(*it, "sEvEral");

    ++it;
    TS_ASSERT_EQUALS(it, testee.end());
}

/** Test buildCompletionList() for a session. */
void
TestGameInterfaceCompletionList::testAddBuildCompletionList()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(new game::test::Root(game::HostVersion()));
    afl::container::PtrVector<interpreter::Context> ctx;
    session.world().keymaps().createKeymap("KEYBOARD");
    session.world().keymaps().createKeymap("KEYMAP");

    // Regular command
    game::interface::CompletionList list;
    buildCompletionList(list, "pla", session, false, ctx);
    TS_ASSERT_EQUALS(list.getStem(), "pla");
    TS_ASSERT(hasCompletion(list, "Player"));
    TS_ASSERT(hasCompletion(list, "Planet"));

    // Word in command
    buildCompletionList(list, "if pla", session, false, ctx);
    TS_ASSERT_EQUALS(list.getStem(), "pla");
    TS_ASSERT(hasCompletion(list, "Player"));
    TS_ASSERT(hasCompletion(list, "Planet"));

    // Configuration
    buildCompletionList(list, "cfg(\"allo", session, false, ctx);
    TS_ASSERT_EQUALS(list.getStem(), "allo");
    TS_ASSERT_EQUALS(list.getImmediateCompletion(), "Allow");
    TS_ASSERT(hasCompletion(list, "AllowPlanetAttacks"));
    TS_ASSERT(hasCompletion(list, "AllowChunneling"));

    // Configuration
    buildCompletionList(list, "addconfig 'defen", session, false, ctx);
    TS_ASSERT_EQUALS(list.getStem(), "defen");
    TS_ASSERT_EQUALS(list.getImmediateCompletion(), "Defense");
    TS_ASSERT(hasCompletion(list, "DefenseForUndetectable"));

    // Preferences
    buildCompletionList(list, "pref(\"displ", session, false, ctx);
    TS_ASSERT_EQUALS(list.getStem(), "displ");
    TS_ASSERT_EQUALS(list.getImmediateCompletion(), "Display.");
    TS_ASSERT(hasCompletion(list, "Display.Clans"));

    // Preferences
    buildCompletionList(list, "addpref \"displ", session, false, ctx);
    TS_ASSERT_EQUALS(list.getStem(), "displ");
    TS_ASSERT_EQUALS(list.getImmediateCompletion(), "Display.");
    TS_ASSERT(hasCompletion(list, "Display.Clans"));

    // Keymaps
    buildCompletionList(list, "bind k", session, false, ctx);
    TS_ASSERT_EQUALS(list.getStem(), "k");
    TS_ASSERT_EQUALS(list.getImmediateCompletion(), "Key");
    TS_ASSERT(hasCompletion(list, "Keyboard"));
    TS_ASSERT(hasCompletion(list, "Keymap"));

    // Keymaps
    buildCompletionList(list, "usekeymap Keyb", session, false, ctx);
    TS_ASSERT_EQUALS(list.getStem(), "Keyb");
    TS_ASSERT_EQUALS(list.getImmediateCompletion(), "Keyboard");
    TS_ASSERT(hasCompletion(list, "Keyboard"));

    // Empty
    buildCompletionList(list, "", session, false, ctx);
    TS_ASSERT_EQUALS(list.getStem(), "");
    TS_ASSERT(list.isEmpty());

    // Space after
    buildCompletionList(list, "pla ", session, false, ctx);
    TS_ASSERT_EQUALS(list.getStem(), "");
    TS_ASSERT(list.isEmpty());
}

