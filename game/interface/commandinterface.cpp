/**
  *  \file game/interface/commandinterface.cpp
  */

#include "game/interface/commandinterface.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"
#include "game/v3/command.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "interpreter/values.hpp"

using interpreter::checkIntegerArg;
using interpreter::checkStringArg;
using game::v3::Command;
using game::v3::CommandContainer;
using game::v3::CommandExtra;

/* @q AddCommand cmd:Str (Global Command)
   Add a command message.
   These commands are mostly for the PHost command processor.
   PCC knows how commands amend or replace each other, e.g.
   | AddCommand "allies add 3"
   will replace a previous <tt>"allies drop 3"</tt> command.
   @todo document the commands
   @since PCC 1.1.4, PCC2 1.99.9, PCC2 2.40.1 */
void
game::interface::IFAddCommand(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFAddCommand, globint.pas:Global_AddCommand
    args.checkArgumentCount(1);

    // Fetch argument
    String_t cmdStr;
    if (!checkStringArg(cmdStr, args.getNext())) {
        return;
    }

    // Do it
    Game& g = game::actions::mustHaveGame(session);

    // Do we allow commands?
    CommandExtra* ex = CommandExtra::get(g.currentTurn());
    if (ex == 0) {
        throw interpreter::Error("Not allowed on this host");
    }
    CommandContainer& cc = ex->create(g.getViewpointPlayer());

    // Parse command
    Command* cmd = Command::parseCommand(cmdStr, true, false);
    if (cmd == 0) {
        throw interpreter::Error("Invalid command");
    }
    cc.addNewCommand(cmd);
}

/* @q DeleteCommand cmd:Str (Global Command)
   Delete a command.

   @todo document the commands
   @since PCC2 2.40.1 */
void
game::interface::IFDeleteCommand(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(1);

    // Fetch argument
    String_t cmdStr;
    if (!checkStringArg(cmdStr, args.getNext())) {
        return;
    }

    // Do it
    Game& g = game::actions::mustHaveGame(session);
    CommandExtra* ex = CommandExtra::get(g.currentTurn());
    if (ex == 0) {
        return;
    }

    std::auto_ptr<Command> cmd(Command::parseCommand(cmdStr, false, true));
    if (cmd.get() == 0) {
        throw interpreter::Error("Invalid command");
    }

    CommandContainer* cc = ex->get(g.getViewpointPlayer());
    if (cc != 0) {
        cc->removeCommand(cmd->getCommand(), cmd->getId());
    }
}

/* @q GetCommand(cmd:Str):Str (Global Command)
   Get status of a command.

   @todo document the commands
   @since PCC2 2.40.1 */
afl::data::Value*
game::interface::IFGetCommand(game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(1);

    // Fetch argument
    String_t cmdStr;
    if (!checkStringArg(cmdStr, args.getNext())) {
        return 0;
    }

    // Do it
    Game& g = game::actions::mustHaveGame(session);
    CommandExtra* ex = CommandExtra::get(g.currentTurn());
    if (ex == 0) {
        return 0;
    }

    std::auto_ptr<Command> cmd(Command::parseCommand(cmdStr, false, true));
    if (cmd.get() == 0) {
        throw interpreter::Error("Invalid command");
    }

    CommandContainer* cc = ex->get(g.getViewpointPlayer());
    if (cc == 0) {
        return 0;
    }

    const Command* existingCommand = cc->getCommand(cmd->getCommand(), cmd->getId());
    if (existingCommand == 0) {
        return 0;
    } else {
        return interpreter::makeStringValue(existingCommand->getArg());
    }
}
