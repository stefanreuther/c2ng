/**
  *  \file u/t_game_commandcontainer.cpp
  *  \brief Test for game::CommandContainer
  */

#include "game/v3/commandcontainer.hpp"

#include "t_game_v3.hpp"

/** Basic container test. */
void
TestGameV3CommandContainer::testContainer()
{
    // ex GameCommandTestSuite::testContainer
    game::v3::Command* null_command = 0;
    game::v3::CommandContainer c;
    TS_ASSERT_EQUALS(c.begin(), c.end());
    TS_ASSERT_DIFFERS(c.addCommand(game::v3::Command::phc_ConfigAlly, 9, "+c"), null_command);
    TS_ASSERT_DIFFERS(c.addNewCommand(game::v3::Command::parseCommand("a a 9", true)), null_command);
    TS_ASSERT_DIFFERS(c.begin(), c.end());

    game::v3::CommandContainer::Iterator_t i = c.begin();
    TS_ASSERT_EQUALS((*i)->getCommand(), game::v3::Command::phc_AddDropAlly);
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::phc_AddDropAlly, 9), *i);
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::phc_AddDropAlly, 7), null_command);
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::phc_AddDropAlly, 0), null_command);

    ++i;
    TS_ASSERT_DIFFERS(i, c.end());
    TS_ASSERT_EQUALS((*i)->getCommand(), game::v3::Command::phc_ConfigAlly);
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::phc_ConfigAlly, 9), *i);

    ++i;
    TS_ASSERT_EQUALS(i, c.end());

    // remove 'allies config 9', test again
    TS_ASSERT(c.removeCommand(game::v3::Command::phc_ConfigAlly, 9));
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::phc_ConfigAlly, 9), null_command);
    i = c.begin();
    TS_ASSERT_DIFFERS(i, c.end());
    TS_ASSERT_EQUALS((*i)->getCommand(), game::v3::Command::phc_AddDropAlly);
    ++i;
    TS_ASSERT_EQUALS(i, c.end());

    // add 'allies config 9 +c', test again
    TS_ASSERT(c.addCommand(game::v3::Command::phc_ConfigAlly, 9, "+c"));
    i = c.begin();
    TS_ASSERT_DIFFERS(i, c.end());
    TS_ASSERT_EQUALS((*i)->getCommand(), game::v3::Command::phc_AddDropAlly);
    ++i;
    TS_ASSERT_DIFFERS(i, c.end());
    TS_ASSERT_EQUALS((*i)->getCommand(), game::v3::Command::phc_ConfigAlly);
    ++i;
    TS_ASSERT_EQUALS(i, c.end());

    // test inquiry / replacement
    const game::v3::Command* cmd = c.addCommand(game::v3::Command::phc_Language, 0, "en");
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::phc_Language, 0), cmd);
    cmd = c.addCommand(game::v3::Command::phc_Language, 0, "de");
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::phc_Language, 0), cmd);
    for (i = c.begin(); i != c.end(); ++i)
        if ((*i)->getCommand() == game::v3::Command::phc_Language)
            TS_ASSERT_EQUALS((*i)->getArg(), "de");

    c.clear();
    TS_ASSERT_EQUALS(c.getCommand(game::v3::Command::phc_AddDropAlly, 9), null_command);
    TS_ASSERT_EQUALS(c.begin(), c.end());
}

/** Test sequencing of commands. */
void
TestGameV3CommandContainer::testSequence()
{
    // ex GameCommandTestSuite::testSequence
    game::v3::CommandContainer cmds;
    cmds.addCommand(game::v3::Command::phc_AddDropAlly, 9, "+c");
    cmds.addCommand(game::v3::Command::phc_RemoteControl, 22, "drop");
    cmds.addCommand(game::v3::Command::phc_AddDropAlly, 3, "+m");
    cmds.addCommand(game::v3::Command::phc_RemoteControl, 99, "request");

    // Sequence must be AddDropAlly 9, then 3, then RemoteControl 22, then 99
    game::v3::CommandContainer::Iterator_t it = cmds.begin();
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommand(), game::v3::Command::phc_AddDropAlly);
    TS_ASSERT_EQUALS((*it)->getId(), 9);
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommand(), game::v3::Command::phc_AddDropAlly);
    TS_ASSERT_EQUALS((*it)->getId(), 3);
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommand(), game::v3::Command::phc_RemoteControl);
    TS_ASSERT_EQUALS((*it)->getId(), 22);
    ++it;
    TS_ASSERT_DIFFERS(it, cmds.end());
    TS_ASSERT_EQUALS((*it)->getCommand(), game::v3::Command::phc_RemoteControl);
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
    cmds.addNewCommand(new Command(Command::phc_GiveShip, 17, "3"));
    cmds.addNewCommand(new Command(Command::phc_GiveShip, 32, "4"));
    cmds.addNewCommand(new Command(Command::phc_GiveShip, 17, "5"));

    const Command* a = cmds.getCommand(Command::phc_GiveShip, 17);
    const Command* b = cmds.getCommand(Command::phc_GiveShip, 32);
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
