/**
  *  \file test/game/interface/commandinterfacetest.cpp
  *  \brief Test for game::interface::CommandInterface
  */

#include "game/interface/commandinterface.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.interface.CommandInterface:IFAddCommand:normal", a)
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
    a.checkNonNull("01. cc", cc);

    const Command* cmd = cc->getCommand(Command::AddDropAlly, 3);
    a.checkNonNull("11. cmd", cmd);
    a.checkEqual("12. getArg", cmd->getArg(), "a");
}

/** Test IFAddCommand, null parameter.
    A: prepare complete turn. Execute "AddCommand Z(0)"
    E: no command must be created */
AFL_TEST("game.interface.CommandInterface:IFAddCommand:null", a)
{
    Environment env;
    CommandExtra& ex = addCommandExtra(env);

    // Execute command
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    game::interface::IFAddCommand(env.session, env.proc, args);

    // If CommandContainer exists, it must still be empty
    if (CommandContainer* cc = ex.get(PLAYER_NR)) {
        a.check("CommandContainer empty", cc->begin() == cc->end());
    }
}

/** Test IFAddCommand, bad command.
    A: prepare complete turn. Execute "AddCommand 'buy a vowel'"
    E: error */
AFL_TEST("game.interface.CommandInterface:IFAddCommand:bad-command", a)
{
    Environment env;
    CommandExtra& ex = addCommandExtra(env);

    afl::data::Segment seg;
    seg.pushBackString("buy a vowel");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFAddCommand(env.session, env.proc, args), std::exception);
}

/** Test IFAddCommand, no game loaded.
    A: prepare empty session. Execute "AddCommand 'a a 3'"
    E: error */
AFL_TEST("game.interface.CommandInterface:IFAddCommand:error:no-game", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("a a 3");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFAddCommand(env.session, env.proc, args), std::exception);
}

/** Test IFAddCommand, commands not supported.
    A: prepare session with game but no CommandContainer. Execute "AddCommand 'a a 3'"
    E: error */
AFL_TEST("game.interface.CommandInterface:IFAddCommand:error:no-cc", a)
{
    Environment env;
    addGame(env);

    afl::data::Segment seg;
    seg.pushBackString("a a 3");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFAddCommand(env.session, env.proc, args), std::exception);
}

/** Test IFDeleteCommand, success case.
    A: prepare complete turn with commands. Execute "DeleteCommand 'a a 3'"
    E: appropriate command is removed */
AFL_TEST("game.interface.CommandInterface:IFDeleteCommand:normal", a)
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
    a.check("01", ex.create(PLAYER_NR).getCommand(Command::AddDropAlly, 3) == 0);
    a.check("02", ex.create(PLAYER_NR).getCommand(Command::AddDropAlly, 7) != 0);
}

/** Test IFDeleteCommand, null parameter.
    A: prepare complete turn with commands. Execute "DeleteCommand Z(0)"
    E: command list unchanged */
AFL_TEST("game.interface.CommandInterface:IFDeleteCommand:null", a)
{
    Environment env;
    CommandExtra& ex = addCommandExtra(env);
    ex.create(PLAYER_NR).addCommand(Command::AddDropAlly, 3, "drop");

    // Execute command
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    game::interface::IFDeleteCommand(env.session, env.proc, args);

    // Verify that command still exists
    a.checkNonNull("01", ex.create(PLAYER_NR).getCommand(Command::AddDropAlly, 3));
}

/** Test IFDeleteCommand, bad command.
    A: prepare complete turn. Execute "DeleteCommand" with bad command.
    E: error */
AFL_TEST("game.interface.CommandInterface:IFDeleteCommand:error:bad-command", a)
{
    Environment env;
    CommandExtra& ex = addCommandExtra(env);

    // Execute command
    afl::data::Segment seg;
    seg.pushBackString("do a barrel roll");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFDeleteCommand(env.session, env.proc, args), std::exception);
}

/** Test IFDeleteCommand, no game.
    A: prepare empty session. Execute "DeleteCommand".
    E: error */
AFL_TEST("game.interface.CommandInterface:IFDeleteCommand:error:no-game", a)
{
    Environment env;

    // Execute command
    afl::data::Segment seg;
    seg.pushBackString("a a 3");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFDeleteCommand(env.session, env.proc, args), std::exception);
}

/** Test IFDeleteCommand, commands not supported.
    A: prepare session with game but no CommandContainer. Execute "DeleteCommand".
    E: command ignored. Postcondition (command not present) is trivially fulfilled. */
AFL_TEST("game.interface.CommandInterface:IFDeleteCommand:error:no-cc", a)
{
    Environment env;
    addGame(env);

    // Execute command
    afl::data::Segment seg;
    seg.pushBackString("a a 3");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFDeleteCommand(env.session, env.proc, args));
}

/** Test IFGetCommand, standard cases. */
    // Normal, found
AFL_TEST("game.interface.CommandInterface:IFGetCommand:found", a)
{
    Environment env;
    addCommandExtra(env).create(PLAYER_NR).addCommand(Command::AddDropAlly, 3, "drop");

    afl::data::Segment seg;
    seg.pushBackString("a a 3");
    interpreter::Arguments args(seg, 0, 1);
    interpreter::test::verifyNewString(a, game::interface::IFGetCommand(env.session, args), "drop");
}

// Normal, not found
AFL_TEST("game.interface.CommandInterface:IFGetCommand:not-found", a)
{
    Environment env;
    addCommandExtra(env).create(PLAYER_NR).addCommand(Command::AddDropAlly, 3, "drop");

    afl::data::Segment seg;
    seg.pushBackString("a a 7");
    interpreter::Arguments args(seg, 0, 1);
    interpreter::test::verifyNewNull(a, game::interface::IFGetCommand(env.session, args));
}

// Null arg
AFL_TEST("game.interface.CommandInterface:IFGetCommand:null", a)
{
    Environment env;
    addCommandExtra(env).create(PLAYER_NR).addCommand(Command::AddDropAlly, 3, "drop");

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    interpreter::test::verifyNewNull(a, game::interface::IFGetCommand(env.session, args));
}

// Bad arg
AFL_TEST("game.interface.CommandInterface:IFGetCommand:error:bad-command", a)
{
    Environment env;
    addCommandExtra(env).create(PLAYER_NR).addCommand(Command::AddDropAlly, 3, "drop");

    afl::data::Segment seg;
    seg.pushBackString("buy a vowel");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFGetCommand(env.session, args), std::exception);
}

/** Test IFGetCommand, no game.
    This is a hard error (bad program state). */
AFL_TEST("game.interface.CommandInterface:IFGetCommand:error:no-game", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("a a 7");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFGetCommand(env.session, args), std::exception);
}

/** Test IFGetCommand, no CommandExtra (game does not support commands).
    This means we report null, command does not exist. */
AFL_TEST("game.interface.CommandInterface:IFGetCommand:error:no-extra", a)
{
    Environment env;
    addGame(env);

    afl::data::Segment seg;
    seg.pushBackString("a a 7");
    interpreter::Arguments args(seg, 0, 1);
    interpreter::test::verifyNewNull(a, game::interface::IFGetCommand(env.session, args));
}

/** Test IFGetCommand, no CommandContainer (no command created yet).
    This means we report null, command does not exist. */
AFL_TEST("game.interface.CommandInterface:IFGetCommand:error:no-cc", a)
{
    Environment env;
    addCommandExtra(env);

    afl::data::Segment seg;
    seg.pushBackString("a a 7");
    interpreter::Arguments args(seg, 0, 1);
    interpreter::test::verifyNewNull(a, game::interface::IFGetCommand(env.session, args));
}
