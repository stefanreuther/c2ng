/**
  *  \file test/game/v3/commandcontainertest.cpp
  *  \brief Test for game::v3::CommandContainer
  */

#include "game/v3/commandcontainer.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/test/testrunner.hpp"

/** Basic container test. */
AFL_TEST("game.v3.CommandContainer:container", a)
{
    // ex GameCommandTestSuite::testContainer
    game::v3::CommandContainer c;
    a.check("01. empty", c.begin() == c.end());
    a.checkNonNull("02. addCommand", c.addCommand(game::v3::Command::ConfigAlly, 9, "+c"));
    a.checkNonNull("03. addNewCommand", c.addNewCommand(game::v3::Command::parseCommand("a a 9", true, false)));
    a.check("04. not empty", c.begin() != c.end());

    game::v3::CommandContainer::Iterator_t i = c.begin();
    a.checkEqual("11. getCommand", (*i)->getCommand(), game::v3::Command::AddDropAlly);
    a.checkEqual("12. getCommand", c.getCommand(game::v3::Command::AddDropAlly, 9), *i);
    a.checkNull ("13. getCommand", c.getCommand(game::v3::Command::AddDropAlly, 7));
    a.checkNull ("14. getCommand", c.getCommand(game::v3::Command::AddDropAlly, 0));

    ++i;
    a.check("21. not end", i != c.end());
    a.checkEqual("22. getCommand", (*i)->getCommand(), game::v3::Command::ConfigAlly);
    a.checkEqual("23. getCommand", c.getCommand(game::v3::Command::ConfigAlly, 9), *i);

    ++i;
    a.check("31. end", i == c.end());

    // remove 'allies config 9', test again
    a.check("41. removeCommand", c.removeCommand(game::v3::Command::ConfigAlly, 9));
    a.checkNull("42. getCommand", c.getCommand(game::v3::Command::ConfigAlly, 9));
    i = c.begin();
    a.check("43. not empty", i != c.end());
    a.checkEqual("44. getCommand", (*i)->getCommand(), game::v3::Command::AddDropAlly);
    ++i;
    a.check("45. end", i == c.end());

    // add 'allies config 9 +c', test again
    a.check("51. addCommand", c.addCommand(game::v3::Command::ConfigAlly, 9, "+c"));
    i = c.begin();
    a.check("52. not empty", i != c.end());
    a.checkEqual("53. getCommand", (*i)->getCommand(), game::v3::Command::AddDropAlly);
    ++i;
    a.check("54. not empty", i != c.end());
    a.checkEqual("55. getCommand", (*i)->getCommand(), game::v3::Command::ConfigAlly);
    ++i;
    a.check("56. end", i == c.end());

    // test inquiry / replacement
    const game::v3::Command* cmd = c.addCommand(game::v3::Command::Language, 0, "en");
    a.checkNonNull("61. addCommand", cmd);
    a.checkEqual("62. getCommand", c.getCommand(game::v3::Command::Language, 0), cmd);
    cmd = c.addCommand(game::v3::Command::Language, 0, "de");
    a.checkNonNull("63. addCommand", cmd);
    a.checkEqual("64. getCommand", c.getCommand(game::v3::Command::Language, 0), cmd);
    for (i = c.begin(); i != c.end(); ++i) {
        if ((*i)->getCommand() == game::v3::Command::Language) {
            a.checkEqual("65. getArg", (*i)->getArg(), "de");
        }
    }

    c.clear();
    a.checkNull("71. getCommand", c.getCommand(game::v3::Command::AddDropAlly, 9));
    a.check("72. empty", c.begin() == c.end());
}

/** Test sequencing of commands. */
AFL_TEST("game.v3.CommandContainer:sequence", a)
{
    // ex GameCommandTestSuite::testSequence
    game::v3::CommandContainer cmds;
    cmds.addCommand(game::v3::Command::AddDropAlly, 9, "+c");
    cmds.addCommand(game::v3::Command::RemoteControl, 22, "drop");
    cmds.addCommand(game::v3::Command::AddDropAlly, 3, "+m");
    cmds.addCommand(game::v3::Command::RemoteControl, 99, "request");

    // Sequence must be AddDropAlly 9, then 3, then RemoteControl 22, then 99
    game::v3::CommandContainer::Iterator_t it = cmds.begin();
    a.check("01. not end", it != cmds.end());
    a.checkEqual("02. getCommand", (*it)->getCommand(), game::v3::Command::AddDropAlly);
    a.checkEqual("03. getId", (*it)->getId(), 9);
    ++it;
    a.check("04. not end", it != cmds.end());
    a.checkEqual("05. getCommand", (*it)->getCommand(), game::v3::Command::AddDropAlly);
    a.checkEqual("06. getId", (*it)->getId(), 3);
    ++it;
    a.check("07. not end", it != cmds.end());
    a.checkEqual("08. getCommand", (*it)->getCommand(), game::v3::Command::RemoteControl);
    a.checkEqual("09. getId", (*it)->getId(), 22);
    ++it;
    a.check("10. not end", it != cmds.end());
    a.checkEqual("11. getCommand", (*it)->getCommand(), game::v3::Command::RemoteControl);
    a.checkEqual("12. getId", (*it)->getId(), 99);
    ++it;
    a.check("13. end", it == cmds.end());
}

/** Test addNewCommand(), pointer replacement. */
AFL_TEST("game.v3.CommandContainer:addNewCommand:replace", a)
{
    using game::v3::Command;
    game::v3::CommandContainer cmds;
    cmds.addNewCommand(new Command(Command::GiveShip, 17, "3"));
    cmds.addNewCommand(new Command(Command::GiveShip, 32, "4"));
    cmds.addNewCommand(new Command(Command::GiveShip, 17, "5"));

    const Command* ca = cmds.getCommand(Command::GiveShip, 17);
    const Command* cb = cmds.getCommand(Command::GiveShip, 32);
    a.checkNonNull("01. getCommand", ca);
    a.checkNonNull("02. getCommand", cb);
    a.checkEqual("03. getArg", ca->getArg(), "5");
    a.checkEqual("04. getArg", cb->getArg(), "4");

    int n = 0;
    for (game::v3::CommandContainer::Iterator_t i = cmds.begin(); i != cmds.end(); ++i) {
        ++n;
    }
    a.checkEqual("11. count", n, 2);
}

/** Test addNewCommand(), non-replaceables. */
AFL_TEST("game.v3.CommandContainer:addNewCommand:non-replaceable", a)
{
    // Add commands of replaceable and non-replaceable type
    using game::v3::Command;
    game::v3::CommandContainer cmds;
    cmds.addNewCommand(new Command(Command::SendFile, 0, "a.txt"));
    cmds.addNewCommand(new Command(Command::GiveShip, 17, "3"));
    cmds.addNewCommand(new Command(Command::Other,    0, "lol"));
    cmds.addNewCommand(new Command(Command::SendFile, 0, "b.txt"));
    cmds.addNewCommand(new Command(Command::GiveShip, 17, "4"));      // replaces previous GiveShip
    cmds.addNewCommand(new Command(Command::Other,    0, "what"));

    // Verify sequence
    game::v3::CommandContainer::Iterator_t it = cmds.begin();
    a.check("01. not end", it != cmds.end());
    a.checkEqual("02. getCommand", (*it)->getCommandText(), "$send-file a.txt");
    ++it;
    a.check("03. not end", it != cmds.end());
    a.checkEqual("04. getCommand", (*it)->getCommandText(), "give ship 17 to 4");    // note changed arg
    ++it;
    a.check("05. not end", it != cmds.end());
    a.checkEqual("06. getCommand", (*it)->getCommandText(), "lol");
    ++it;
    a.check("07. not end", it != cmds.end());
    a.checkEqual("08. getCommand", (*it)->getCommandText(), "$send-file b.txt");
    ++it;
    a.check("09. not end", it != cmds.end());
    a.checkEqual("10. getCommand", (*it)->getCommandText(), "what");
    ++it;
    a.check("11. end", it == cmds.end());
}

/** Test loadCommandFile, normal case. */
AFL_TEST("game.v3.CommandContainer:loadCommandFile:normal", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("# test file\n"
                                                       "a c 3 +m\n"
                                                       "buy a vowel\n"
                                                       "$send-f lol.txt\n"
                                                       "a a 3\n"));
    game::v3::CommandContainer cmds;
    cmds.loadCommandFile(ms, game::Timestamp(1999, 12, 31, 12, 0, 0));

    // Verify sequence
    game::v3::CommandContainer::Iterator_t it = cmds.begin();
    a.check("01. not end", it != cmds.end());
    a.checkEqual("02. getCommand", (*it)->getCommandText(), "buy a vowel");
    ++it;
    a.check("03. not end", it != cmds.end());
    a.checkEqual("04. getCommand", (*it)->getCommandText(), "$send-file lol.txt");  // note expansion
    ++it;
    a.check("05. not end", it != cmds.end());
    a.checkEqual("06. getCommand", (*it)->getCommandText(), "allies a 3");          // note partial expansion
    ++it;
    a.check("07. not end", it != cmds.end());
    a.checkEqual("08. getCommand", (*it)->getCommandText(), "allies config 3 +m");  // note moved to end due to ordering constraint
    ++it;
    a.check("09. end", it == cmds.end());
}

AFL_TEST("game.v3.CommandContainer:loadCommandFile:time:match", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("g s 1 5\n"
                                                       "$timestamp 12-31-199912:00:00\n"
                                                       "g s 2 7\n"));
    game::v3::CommandContainer cmds;
    cmds.loadCommandFile(ms, game::Timestamp(1999, 12, 31, 12, 0, 0));

    // Verify sequence: both commands accepted
    game::v3::CommandContainer::Iterator_t it = cmds.begin();
    a.check("01. not end", it != cmds.end());
    a.checkEqual("02. getCommand", (*it)->getCommandText(), "give ship 1 to 5");
    ++it;
    a.check("03. not end", it != cmds.end());
    a.checkEqual("04. getCommand", (*it)->getCommandText(), "give ship 2 to 7");
    ++it;
    a.check("05. end", it == cmds.end());
}

AFL_TEST("game.v3.CommandContainer:loadCommandFile:time:mismatch", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("g s 1 5\n"
                                                       "$timestamp 01-01-200012:00:00\n"
                                                       "g s 2 7\n"));
    game::v3::CommandContainer cmds;
    cmds.loadCommandFile(ms, game::Timestamp(1999, 12, 31, 12, 0, 0));

    // Verify sequence: only first command accepted, subsequent rejected by timestamp
    game::v3::CommandContainer::Iterator_t it = cmds.begin();
    a.check("01. not end", it != cmds.end());
    a.checkEqual("02. getCommand", (*it)->getCommandText(), "give ship 1 to 5");
    ++it;
    a.check("03. end", it == cmds.end());
}

AFL_TEST("game.v3.CommandContainer:removeCommand:by-pointer", a)
{
    using game::v3::Command;
    game::v3::CommandContainer cmds;
    const Command* ca = cmds.addNewCommand(new Command(Command::GiveShip, 1, "3"));
    const Command* cb = cmds.addNewCommand(new Command(Command::GiveShip, 2, "4"));
    const Command* cc = cmds.addNewCommand(new Command(Command::GiveShip, 3, "5"));

    (void) ca;
    (void) cc;

    // Remove b by pointer
    cmds.removeCommand(cb);

    // Remove c by identification
    a.checkEqual("01", cmds.removeCommand(Command::GiveShip, 3), true);

    // Remove mismatch
    a.checkEqual("11", cmds.removeCommand(Command::GiveShip, 2), false);

    // Verify
    game::v3::CommandContainer::Iterator_t it = cmds.begin();
    a.check("21. not end", it != cmds.end());
    a.checkEqual("22. getCommand", (*it)->getCommandText(), "give ship 1 to 3");
    ++it;
    a.check("23. end", it == cmds.end());
}

AFL_TEST("game.v3.CommandContainer:removeCommand:by-reference", a)
{
    using game::v3::Command;
    using game::v3::CommandContainer;
    CommandContainer cmds;

    // Build a command container
    cmds.addNewCommand(new Command(Command::GiveShip, 42, "3"));
    cmds.addNewCommand(new Command(Command::GiveShip,  7, "4"));
    cmds.addNewCommand(new Command(Command::Unload,   42, "n10"));
    cmds.addNewCommand(new Command(Command::Transfer, 42, "n10 to 2"));
    cmds.addNewCommand(new Command(Command::GiveShip,  8, "4"));
    cmds.addNewCommand(new Command(Command::ShowShip, 42, "7"));

    // Verify initial state
    {
        CommandContainer::Iterator_t it = cmds.begin();
        a.check("01. not end", it != cmds.end());
        a.checkEqual("02. getCommand", (*it)->getCommandText(), "give ship 42 to 3");
        ++it;
        a.check("03. not end", it != cmds.end());
        a.checkEqual("04. getCommand", (*it)->getCommandText(), "give ship 7 to 4");
        ++it;
        a.check("05. not end", it != cmds.end());
        a.checkEqual("06. getCommand", (*it)->getCommandText(), "unload 42 n10");
        ++it;
        a.check("07. not end", it != cmds.end());
        a.checkEqual("08. getCommand", (*it)->getCommandText(), "transfer 42 n10 to 2");
        ++it;
        a.check("09. not end", it != cmds.end());
        a.checkEqual("10. getCommand", (*it)->getCommandText(), "give ship 8 to 4");
        ++it;
        a.check("11. not end", it != cmds.end());
        a.checkEqual("12. getCommand", (*it)->getCommandText(), "show ship 42 7");
        ++it;
        a.check("13. end", it == cmds.end());
    }

    // Action
    cmds.removeCommandsByReference(game::Reference(game::Reference::Ship, 42));

    // Verify
    {
        CommandContainer::Iterator_t it = cmds.begin();
        a.check("21. not end", it != cmds.end());
        a.checkEqual("22. getCommand", (*it)->getCommandText(), "give ship 7 to 4");
        ++it;
        a.check("23. not end", it != cmds.end());
        a.checkEqual("24. getCommand", (*it)->getCommandText(), "give ship 8 to 4");
        ++it;
        a.check("25. end", it == cmds.end());
    }
}

/** Test PlayerSet operations. */
AFL_TEST("game.v3.CommandContainer:player-set", a)
{
    game::v3::CommandContainer testee;
    const game::v3::Command* p;

    // Initially empty
    a.checkEqual("01. getCommandPlayerSet", testee.getCommandPlayerSet(game::v3::Command::ShowShip, 10), game::PlayerSet_t());

    // Set to create
    testee.setCommandPlayerSet(game::v3::Command::ShowShip, 10, game::PlayerSet_t() + 3 + 5);
    a.checkEqual("11. getCommandPlayerSet", testee.getCommandPlayerSet(game::v3::Command::ShowShip, 10), game::PlayerSet_t() + 3 + 5);
    p = testee.getCommand(game::v3::Command::ShowShip, 10);
    a.checkNonNull("12. getCommand", p);
    a.checkEqual("13. getArg", p->getArg(), "3 5");
    a.checkEqual("14. getCommandText", p->getCommandText(), "show ship 10 3 5");

    // Set to update
    testee.setCommandPlayerSet(game::v3::Command::ShowShip, 10, game::PlayerSet_t() + 9);
    a.checkEqual("21. getCommandPlayerSet", testee.getCommandPlayerSet(game::v3::Command::ShowShip, 10), game::PlayerSet_t() + 9);
    p = testee.getCommand(game::v3::Command::ShowShip, 10);
    a.checkNonNull("22. getCommand", p);
    a.checkEqual("23. getArg", p->getArg(), "9");
    a.checkEqual("24. getCommandText", p->getCommandText(), "show ship 10 9");

    // Set to delete
    testee.setCommandPlayerSet(game::v3::Command::ShowShip, 10, game::PlayerSet_t());
    a.checkEqual("31. getCommandPlayerSet", testee.getCommandPlayerSet(game::v3::Command::ShowShip, 10), game::PlayerSet_t());
    p = testee.getCommand(game::v3::Command::ShowShip, 10);
    a.checkNull("32. getCommand", p);
    a.check("33. empty", testee.begin() == testee.end());
}
