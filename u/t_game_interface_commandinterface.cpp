/**
  *  \file u/t_game_interface_commandinterface.cpp
  *  \brief Test for game::interface::CommandInterface
  */

#include "game/interface/commandinterface.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"
#include "interpreter/test/valueverifier.hpp"

using game::v3::Command;
using game::v3::CommandContainer;
using game::v3::CommandExtra;

/*
 *  For these test, it's unspecified whether they throw game::Exception or interpreter::Error.
 *  We therefore check for std::exception.
 */

namespace {
    const int PLAYER_NR = 5;

    struct Environment {
        afl::io::NullFileSystem fs;
        afl::string::NullTranslator tx;
        game::Session session;
        interpreter::Process proc;

        Environment()
            : fs(), tx(), session(tx, fs),
              proc(session.world(), "dummy", 1)
            { }
    };

    game::Game& addGame(Environment& env)
    {
        if (env.session.getGame().get() == 0) {
            env.session.setGame(new game::Game());
            env.session.getGame()->setViewpointPlayer(PLAYER_NR);
        }
        return *env.session.getGame();
    }

    CommandExtra& addCommandExtra(Environment& env)
    {
        return CommandExtra::create(addGame(env).currentTurn());
    }
}

/** Test IFAddCommand, success case.
    A: prepare complete turn. Execute "AddCommand 'a a 3'"
    E: AddDropAlly ally command must be created */
void
TestGameInterfaceCommandInterface::testAdd()
{
    Environment env;
    CommandExtra& ex = addCommandExtra(env);

    // Execute command
    afl::data::Segment seg;
    seg.pushBackString("a a 3");
    interpreter::Arguments args(seg, 0, 1);
    game::interface::IFAddCommand(env.session, env.proc, args);

    // Verify that command exists
    CommandContainer* cc = ex.get(PLAYER_NR);
    TS_ASSERT(cc != 0);

    const Command* cmd = cc->getCommand(Command::AddDropAlly, 3);
    TS_ASSERT(cmd != 0);
    TS_ASSERT_EQUALS(cmd->getArg(), "a");
}

/** Test IFAddCommand, null parameter.
    A: prepare complete turn. Execute "AddCommand Z(0)"
    E: no command must be created */
void
TestGameInterfaceCommandInterface::testAddNull()
{
    Environment env;
    CommandExtra& ex = addCommandExtra(env);

    // Execute command
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    game::interface::IFAddCommand(env.session, env.proc, args);

    // If CommandContainer exists, it must still be empty
    if (CommandContainer* cc = ex.get(PLAYER_NR)) {
        TS_ASSERT_EQUALS(cc->begin(), cc->end());
    }
}

/** Test IFAddCommand, bad command.
    A: prepare complete turn. Execute "AddCommand 'buy a vowel'"
    E: error */
void
TestGameInterfaceCommandInterface::testAddBadCommand()
{
    Environment env;
    CommandExtra& ex = addCommandExtra(env);

    afl::data::Segment seg;
    seg.pushBackString("buy a vowel");
    interpreter::Arguments args(seg, 0, 1);
    TS_ASSERT_THROWS(game::interface::IFAddCommand(env.session, env.proc, args), std::exception);
}

/** Test IFAddCommand, no game loaded.
    A: prepare empty session. Execute "AddCommand 'a a 3'"
    E: error */
void
TestGameInterfaceCommandInterface::testAddNoGame()
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("a a 3");
    interpreter::Arguments args(seg, 0, 1);
    TS_ASSERT_THROWS(game::interface::IFAddCommand(env.session, env.proc, args), std::exception);
}

/** Test IFAddCommand, commands not supported.
    A: prepare session with game but no CommandContainer. Execute "AddCommand 'a a 3'"
    E: error */
void
TestGameInterfaceCommandInterface::testAddNoCommand()
{
    Environment env;
    addGame(env);

    afl::data::Segment seg;
    seg.pushBackString("a a 3");
    interpreter::Arguments args(seg, 0, 1);
    TS_ASSERT_THROWS(game::interface::IFAddCommand(env.session, env.proc, args), std::exception);
}

/** Test IFDeleteCommand, success case.
    A: prepare complete turn with commands. Execute "DeleteCommand 'a a 3'"
    E: appropriate command is removed */
void
TestGameInterfaceCommandInterface::testDelete()
{
    Environment env;
    CommandExtra& ex = addCommandExtra(env);
    ex.create(PLAYER_NR).addCommand(Command::AddDropAlly, 3, "drop");
    ex.create(PLAYER_NR).addCommand(Command::AddDropAlly, 7, "add");

    // Execute command
    afl::data::Segment seg;
    seg.pushBackString("a a 3");
    interpreter::Arguments args(seg, 0, 1);
    game::interface::IFDeleteCommand(env.session, env.proc, args);

    // Verify that command no longer exists (and other command is untouched)
    TS_ASSERT(ex.create(PLAYER_NR).getCommand(Command::AddDropAlly, 3) == 0);
    TS_ASSERT(ex.create(PLAYER_NR).getCommand(Command::AddDropAlly, 7) != 0);
}

/** Test IFDeleteCommand, null parameter.
    A: prepare complete turn with commands. Execute "DeleteCommand Z(0)"
    E: command list unchanged */
void
TestGameInterfaceCommandInterface::testDeleteNull()
{
    Environment env;
    CommandExtra& ex = addCommandExtra(env);
    ex.create(PLAYER_NR).addCommand(Command::AddDropAlly, 3, "drop");

    // Execute command
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    game::interface::IFDeleteCommand(env.session, env.proc, args);

    // Verify that command still exists
    TS_ASSERT(ex.create(PLAYER_NR).getCommand(Command::AddDropAlly, 3) != 0);
}

/** Test IFDeleteCommand, bad command.
    A: prepare complete turn. Execute "DeleteCommand" with bad command.
    E: error */
void
TestGameInterfaceCommandInterface::testDeleteBadCommand()
{
    Environment env;
    CommandExtra& ex = addCommandExtra(env);

    // Execute command
    afl::data::Segment seg;
    seg.pushBackString("do a barrel roll");
    interpreter::Arguments args(seg, 0, 1);
    TS_ASSERT_THROWS(game::interface::IFDeleteCommand(env.session, env.proc, args), std::exception);
}

/** Test IFDeleteCommand, no game.
    A: prepare empty session. Execute "DeleteCommand".
    E: error */
void
TestGameInterfaceCommandInterface::testDeleteNoGame()
{
    Environment env;

    // Execute command
    afl::data::Segment seg;
    seg.pushBackString("a a 3");
    interpreter::Arguments args(seg, 0, 1);
    TS_ASSERT_THROWS(game::interface::IFDeleteCommand(env.session, env.proc, args), std::exception);
}

/** Test IFDeleteCommand, commands not supported.
    A: prepare session with game but no CommandContainer. Execute "DeleteCommand".
    E: command ignored. Postcondition (command not present) is trivially fulfilled. */
void
TestGameInterfaceCommandInterface::testDeleteNoCommand()
{
    Environment env;
    addGame(env);

    // Execute command
    afl::data::Segment seg;
    seg.pushBackString("a a 3");
    interpreter::Arguments args(seg, 0, 1);
    TS_ASSERT_THROWS_NOTHING(game::interface::IFDeleteCommand(env.session, env.proc, args));
}

/** Test IFGetCommand, standard cases. */
void
TestGameInterfaceCommandInterface::testGet()
{
    Environment env;
    CommandExtra& ex = addCommandExtra(env);
    ex.create(PLAYER_NR).addCommand(Command::AddDropAlly, 3, "drop");

    // Normal, found
    {
        afl::data::Segment seg;
        seg.pushBackString("a a 3");
        interpreter::Arguments args(seg, 0, 1);
        interpreter::test::verifyNewString("get found", game::interface::IFGetCommand(env.session, args), "drop");
    }

    // Normal, not found
    {
        afl::data::Segment seg;
        seg.pushBackString("a a 7");
        interpreter::Arguments args(seg, 0, 1);
        interpreter::test::verifyNewNull("get not-found", game::interface::IFGetCommand(env.session, args));
    }

    // Null arg
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        interpreter::test::verifyNewNull("get null", game::interface::IFGetCommand(env.session, args));
    }

    // Bad arg
    {
        afl::data::Segment seg;
        seg.pushBackString("buy a vowel");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFGetCommand(env.session, args), std::exception);
    }
}

/** Test IFGetCommand, no game.
    This is a hard error (bad program state). */
void
TestGameInterfaceCommandInterface::testGetNoGame()
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("a a 7");
    interpreter::Arguments args(seg, 0, 1);
    TS_ASSERT_THROWS(game::interface::IFGetCommand(env.session, args), std::exception);
}

/** Test IFGetCommand, no CommandExtra (game does not support commands).
    This means we report null, command does not exist. */
void
TestGameInterfaceCommandInterface::testGetNoCommandExtra()
{
    Environment env;
    addGame(env);

    afl::data::Segment seg;
    seg.pushBackString("a a 7");
    interpreter::Arguments args(seg, 0, 1);
    interpreter::test::verifyNewNull("testGetNoCommandExtra", game::interface::IFGetCommand(env.session, args));
}

/** Test IFGetCommand, no CommandContainer (no command created yet).
    This means we report null, command does not exist. */
void
TestGameInterfaceCommandInterface::testGetNoCommandContainer()
{
    Environment env;
    addCommandExtra(env);

    afl::data::Segment seg;
    seg.pushBackString("a a 7");
    interpreter::Arguments args(seg, 0, 1);
    interpreter::test::verifyNewNull("testGetNoCommandContainer", game::interface::IFGetCommand(env.session, args));
}

