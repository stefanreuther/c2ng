/**
  *  \file u/t_game_commandcontainer.cpp
  *  \brief Test for game::CommandContainer
  */

#include "game/v3/commandcontainer.hpp"

#include "t_game_v3.hpp"
#include "afl/io/constmemorystream.hpp"

/** Basic container test. */
void
TestGameV3CommandContainer::testContainer()
{
    // ex GameCommandTestSuite::testContainer
    game::v3::Command* null_command = 0;
    game::v3::CommandContainer c;
    TS_ASSERT_EQUALS(c.begin(), c.end());
    TS_ASSERT_DIFFERS(c.addCommand(game::v3::Command::ConfigAlly, 9, "+c"), null_command);
    TS_ASSERT_DIFFERS(c.addNewCommand(game::v3::Command::parseCommand("a a 9", true, false)), null_command);
    TS_ASSERT_DIFFERS(c.begin(), c.end());

    game::v3::CommandContainer::Iterator_t i = c.begin();
    TS_ASSERT_EQUALS((*i)->getCommand(), game::v3::Command::AddDropAlly);
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::AddDropAlly, 9), *i);
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::AddDropAlly, 7), null_command);
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::AddDropAlly, 0), null_command);

    ++i;
    TS_ASSERT_DIFFERS(i, c.end());
    TS_ASSERT_EQUALS((*i)->getCommand(), game::v3::Command::ConfigAlly);
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::ConfigAlly, 9), *i);

    ++i;
    TS_ASSERT_EQUALS(i, c.end());

    // remove 'allies config 9', test again
    TS_ASSERT(c.removeCommand(game::v3::Command::ConfigAlly, 9));
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::ConfigAlly, 9), null_command);
    i = c.begin();
    TS_ASSERT_DIFFERS(i, c.end());
    TS_ASSERT_EQUALS((*i)->getCommand(), game::v3::Command::AddDropAlly);
    ++i;
    TS_ASSERT_EQUALS(i, c.end());

    // add 'allies config 9 +c', test again
    TS_ASSERT(c.addCommand(game::v3::Command::ConfigAlly, 9, "+c"));
    i = c.begin();
    TS_ASSERT_DIFFERS(i, c.end());
    TS_ASSERT_EQUALS((*i)->getCommand(), game::v3::Command::AddDropAlly);
    ++i;
    TS_ASSERT_DIFFERS(i, c.end());
    TS_ASSERT_EQUALS((*i)->getCommand(), game::v3::Command::ConfigAlly);
    ++i;
    TS_ASSERT_EQUALS(i, c.end());

    // test inquiry / replacement
    const game::v3::Command* cmd = c.addCommand(game::v3::Command::Language, 0, "en");
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::Language, 0), cmd);
    cmd = c.addCommand(game::v3::Command::Language, 0, "de");
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::Language, 0), cmd);
    for (i = c.begin(); i != c.end(); ++i)
        if ((*i)->getCommand() == game::v3::Command::Language)
            TS_ASSERT_EQUALS((*i)->getArg(), "de");

    c.clear();
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::AddDropAlly, 9), null_command);
    TS_ASSERT_EQUALS(c.begin(), c.end());
}

/** Test sequencing of commands. */
void
TestGameV3CommandContainer::testSequence()
{
    // ex GameCommandTestSuite::testSequence
    game::v3::CommandContainer cmds;
    cmds.addCommand(game::v3::Command::AddDropAlly, 9, "+c");
    cmds.addCommand(game::v3::Command::RemoteControl, 22, "drop");
    cmds.addCommand(game::v3::Command::AddDropAlly, 3, "+m");
    cmds.addCommand(game::v3::Command::RemoteControl, 99, "request");

    // Sequence must be AddDropAlly 9, then 3, then RemoteControl 22, then 99
    game::v3::CommandContainer::Iterator_t it = cmds.begin();
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommand(), game::v3::Command::AddDropAlly);
    TS_ASSERT_EQUALS((*it)->getId(), 9);
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommand(), game::v3::Command::AddDropAlly);
    TS_ASSERT_EQUALS((*it)->getId(), 3);
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommand(), game::v3::Command::RemoteControl);
    TS_ASSERT_EQUALS((*it)->getId(), 22);
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommand(), game::v3::Command::RemoteControl);
    TS_ASSERT_EQUALS((*it)->getId(), 99);
    ++it;
    TS_ASSERT_EQUALS(it, cmds.end());
}

/** Test addNewCommand(), pointer replacement. */
void
TestGameV3CommandContainer::testReplacePointer()
{
    using game::v3::Command;
    game::v3::CommandContainer cmds;
    cmds.addNewCommand(new Command(Command::GiveShip, 17, "3"));
    cmds.addNewCommand(new Command(Command::GiveShip, 32, "4"));
    cmds.addNewCommand(new Command(Command::GiveShip, 17, "5"));

    const Command* a = cmds.getCommand(Command::GiveShip, 17);
    const Command* b = cmds.getCommand(Command::GiveShip, 32);
    TS_ASSERT(a);
    TS_ASSERT(b);
    TS_ASSERT_EQUALS(a->getArg(), "5");
    TS_ASSERT_EQUALS(b->getArg(), "4");

    int n = 0;
    for (game::v3::CommandContainer::Iterator_t i = cmds.begin(); i != cmds.end(); ++i) {
        ++n;
    }
    TS_ASSERT_EQUALS(n, 2);
}

/** Test addNewCommand(), non-replaceables. */
void
TestGameV3CommandContainer::testNonReplaceable()
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
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "$send-file a.txt");
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "give ship 17 to 4");    // note changed arg
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "lol");
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "$send-file b.txt");
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "what");
    ++it;
    TS_ASSERT_EQUALS(it, cmds.end());
}

/** Test loadCommandFile, normal case. */
void
TestGameV3CommandContainer::testLoadNormal()
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
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "buy a vowel");
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "$send-file lol.txt");  // note expansion
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "allies a 3");          // note partial expansion
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "allies config 3 +m");  // note moved to end due to ordering constraint
    ++it;
    TS_ASSERT_EQUALS(it, cmds.end());
}

void
TestGameV3CommandContainer::testLoadTimeMatch()
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("g s 1 5\n"
                                                       "$timestamp 12-31-199912:00:00\n"
                                                       "g s 2 7\n"));
    game::v3::CommandContainer cmds;
    cmds.loadCommandFile(ms, game::Timestamp(1999, 12, 31, 12, 0, 0));

    // Verify sequence: both commands accepted
    game::v3::CommandContainer::Iterator_t it = cmds.begin();
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "give ship 1 to 5");
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "give ship 2 to 7");
    ++it;
    TS_ASSERT_EQUALS(it, cmds.end());
}

void
TestGameV3CommandContainer::testLoadTimeMismatch()
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("g s 1 5\n"
                                                       "$timestamp 01-01-200012:00:00\n"
                                                       "g s 2 7\n"));
    game::v3::CommandContainer cmds;
    cmds.loadCommandFile(ms, game::Timestamp(1999, 12, 31, 12, 0, 0));

    // Verify sequence: only first command accepted, subsequent rejected by timestamp
    game::v3::CommandContainer::Iterator_t it = cmds.begin();
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "give ship 1 to 5");
    ++it;
    TS_ASSERT_EQUALS(it, cmds.end());
}

void
TestGameV3CommandContainer::testRemove()
{
    using game::v3::Command;
    game::v3::CommandContainer cmds;
    const Command* a = cmds.addNewCommand(new Command(Command::GiveShip, 1, "3"));
    const Command* b = cmds.addNewCommand(new Command(Command::GiveShip, 2, "4"));
    const Command* c = cmds.addNewCommand(new Command(Command::GiveShip, 3, "5"));

    (void) a;
    (void) c;

    // Remove b by pointer
    cmds.removeCommand(b);

    // Remove c by identification
    TS_ASSERT_EQUALS(cmds.removeCommand(Command::GiveShip, 3), true);

    // Remove mismatch
    TS_ASSERT_EQUALS(cmds.removeCommand(Command::GiveShip, 2), false);

    // Verify
    game::v3::CommandContainer::Iterator_t it = cmds.begin();
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommandText(), "give ship 1 to 3");
    ++it;
    TS_ASSERT_EQUALS(it, cmds.end());
}

void
TestGameV3CommandContainer::testRemoveByReference()
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
        TS_ASSERT_DIFFERS(it, cmds.end());
        TS_ASSERT_EQUALS((*it)->getCommandText(), "give ship 42 to 3");
        ++it;
        TS_ASSERT_DIFFERS(it, cmds.end());
        TS_ASSERT_EQUALS((*it)->getCommandText(), "give ship 7 to 4");
        ++it;
        TS_ASSERT_DIFFERS(it, cmds.end());
        TS_ASSERT_EQUALS((*it)->getCommandText(), "unload 42 n10");
        ++it;
        TS_ASSERT_DIFFERS(it, cmds.end());
        TS_ASSERT_EQUALS((*it)->getCommandText(), "transfer 42 n10 to 2");
        ++it;
        TS_ASSERT_DIFFERS(it, cmds.end());
        TS_ASSERT_EQUALS((*it)->getCommandText(), "give ship 8 to 4");
        ++it;
        TS_ASSERT_DIFFERS(it, cmds.end());
        TS_ASSERT_EQUALS((*it)->getCommandText(), "show ship 42 7");
        ++it;
        TS_ASSERT_EQUALS(it, cmds.end());
    }

    // Action
    cmds.removeCommandsByReference(game::Reference(game::Reference::Ship, 42));

    // Verify
    {
        CommandContainer::Iterator_t it = cmds.begin();
        TS_ASSERT_DIFFERS(it, cmds.end());
        TS_ASSERT_EQUALS((*it)->getCommandText(), "give ship 7 to 4");
        ++it;
        TS_ASSERT_DIFFERS(it, cmds.end());
        TS_ASSERT_EQUALS((*it)->getCommandText(), "give ship 8 to 4");
        ++it;
        TS_ASSERT_EQUALS(it, cmds.end());
    }
}

/** Test PlayerSet operations. */
void
TestGameV3CommandContainer::testPlayerSet()
{
    game::v3::CommandContainer testee;
    const game::v3::Command* p;

    // Initially empty
    TS_ASSERT_EQUALS(testee.getCommandPlayerSet(game::v3::Command::ShowShip, 10), game::PlayerSet_t());

    // Set to create
    testee.setCommandPlayerSet(game::v3::Command::ShowShip, 10, game::PlayerSet_t() + 3 + 5);
    TS_ASSERT_EQUALS(testee.getCommandPlayerSet(game::v3::Command::ShowShip, 10), game::PlayerSet_t() + 3 + 5);
    p = testee.getCommand(game::v3::Command::ShowShip, 10);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->getArg(), "3 5");
    TS_ASSERT_EQUALS(p->getCommandText(), "show ship 10 3 5");

    // Set to update
    testee.setCommandPlayerSet(game::v3::Command::ShowShip, 10, game::PlayerSet_t() + 9);
    TS_ASSERT_EQUALS(testee.getCommandPlayerSet(game::v3::Command::ShowShip, 10), game::PlayerSet_t() + 9);
    p = testee.getCommand(game::v3::Command::ShowShip, 10);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->getArg(), "9");
    TS_ASSERT_EQUALS(p->getCommandText(), "show ship 10 9");

    // Set to delete
    testee.setCommandPlayerSet(game::v3::Command::ShowShip, 10, game::PlayerSet_t());
    TS_ASSERT_EQUALS(testee.getCommandPlayerSet(game::v3::Command::ShowShip, 10), game::PlayerSet_t());
    p = testee.getCommand(game::v3::Command::ShowShip, 10);
    TS_ASSERT(p == 0);
    TS_ASSERT_EQUALS(testee.begin(), testee.end());
}

