/**
  *  \file game/v3/commandcontainer.cpp
  *  \brief Class game::v3::CommandContainer
  */

#include <algorithm>
#include "game/v3/commandcontainer.hpp"
#include "afl/io/textfile.hpp"
#include "util/string.hpp"

// Constructor.
game::v3::CommandContainer::CommandContainer()
    : sig_commandChange(),
      cmds()
{
    // ex GCommandContainer::GCommandContainer
}

// Destructor.
game::v3::CommandContainer::~CommandContainer()
{
    // ex GCommandContainer::~GCommandContainer
}

// Clear this container.
void
game::v3::CommandContainer::clear()
{
    // ex GCommandContainer::clear
    for (ConstIterator_t it = cmds.begin(); it != cmds.end(); ++it) {
        sig_commandChange.raise(**it, false);
    }
    cmds.clear();
}

// Get command by type and Id.
const game::v3::Command*
game::v3::CommandContainer::getCommand(Command::Type typ, Id_t id) const
{
    // ex GCommandContainer::getCommand
    // ex phost.pas:GetPHostCmd
    Iterator_t i = const_cast<CommandContainer*>(this)->findCommand(typ, id);
    if (i != cmds.end()) {
        return *i;
    } else {
        return 0;
    }
}

// Add a command.
const game::v3::Command*
game::v3::CommandContainer::addCommand(Command::Type typ, Id_t id, String_t arg)
{
    // ex GCommandContainer::setCommand
    // ex phost.pas:SetPHostCmd
    Iterator_t i = findCommand(typ, id);
    if (i != cmds.end()) {
        if (arg != (*i)->getArg()) {
            (*i)->setArg(arg);
            sig_commandChange.raise(**i, true);
        }
        return *i;
    } else {
        Command* cmd = new Command(typ, id, arg);
        insertNewCommand(cmd);
        return cmd;
    }
}

// Add a command.
const game::v3::Command*
game::v3::CommandContainer::addNewCommand(Command* cmd)
{
    // ex GCommandContainer::setCommand
    // @change PCC2 would always 'recycle' the command object; this one may not.
    if (cmd != 0) {
        Iterator_t i = findCommand(cmd->getCommand(), cmd->getId());
        if (i != cmds.end()) {
            if ((*i)->getArg() != cmd->getArg()) {
                (*i)->setArg(cmd->getArg());
                sig_commandChange.raise(**i, true);
            }
            delete cmd;
            cmd = *i;
        } else {
            insertNewCommand(cmd);
        }
    }
    return cmd;
}

// Remove a command.
bool
game::v3::CommandContainer::removeCommand(Command::Type typ, Id_t id)
{
    // ex GCommandContainer::removeCommand, phost.pas:RemovePHostCmd
    Iterator_t i = findCommand(typ, id);
    if (i != cmds.end()) {
        sig_commandChange.raise(**i, false);
        cmds.erase(i);
        return true;
    } else {
        return false;
    }
}

// Remove a command.
void
game::v3::CommandContainer::removeCommand(const Command* cmd)
{
    // ex GCommandContainer::removeCommand, phost.pas:RemovePHostCmdByPtr
    // FIXME: this fails because PtrMultiList::iterator does not have required types (difference_type, iterator_category, etc.)
    // Iterator_t i = std::find(cmds.begin(), cmds.end(), cmd);
    Iterator_t i = cmds.begin();
    while (i != cmds.end() && *i != cmd) {
        ++i;
    }
    if (i != cmds.end()) {
        sig_commandChange.raise(**i, false);
        cmds.erase(i);
    }
}

// Remove commands by affected unit.
void
game::v3::CommandContainer::removeCommandsByReference(Reference ref)
{
    // ex GReset::removeCommands (sort-of)
    for (Iterator_t i = cmds.begin(); i != cmds.end(); ++i) {
        if ((*i)->getAffectedUnit() == ref) {
            sig_commandChange.raise(**i, false);
            cmds.erase(i);
        }
    }
}

// Get player set from a command that takes one as its parameter.
game::PlayerSet_t
game::v3::CommandContainer::getCommandPlayerSet(Command::Type typ, Id_t id) const
{
    if (const Command* c = getCommand(typ, id)) {
        return parsePlayerListAsSet(c->getArg());
    } else {
        return PlayerSet_t();
    }
}

// Create command with player set parameter.
void
game::v3::CommandContainer::setCommandPlayerSet(Command::Type typ, Id_t id, PlayerSet_t set)
{
    String_t arg = formatPlayerSetAsList(set);
    if (arg.empty()) {
        removeCommand(typ, id);
    } else {
        addCommand(typ, id, arg);
    }
}

// Load command file (cmdX.txt).
void
game::v3::CommandContainer::loadCommandFile(afl::io::Stream& file, const Timestamp& time)
{
    // ex GCommandContainer::loadCommandFile
    // ex phost.pas:ParseCmdFileLine (sort-of)
    afl::io::TextFile tf(file);
    // FIXME: tf.setCharacterSet(getGameCharacterSet());
    String_t line;
    // FIXME: use FileParser
    while (tf.readLine(line)) {
        line = afl::string::strTrim(line);
        if (line.size() == 0 || line[0] == '#') {
            // comment or blank line
        } else {
            // regular line
            if (util::stringMatch("$TIMEstamp", afl::string::strNthWord(line, 0))) {
                // timestamp command. Argument must be same as our timestamp,
                // otherwise the file is stale and shall be ignored.
                if (afl::string::strNthWord(line, 1) != time.getTimestampAsString()) {
                    break;
                }
            } else {
                // regular command
                Command* cmd = Command::parseCommand(line, true, false);
                if (!cmd) {
                    cmd = new Command(Command::Other, 0, line);
                }
                addNewCommand(cmd);
            }
        }
    }
}

// Find command by type/id.
game::v3::CommandContainer::Iterator_t
game::v3::CommandContainer::findCommand(Command::Type typ, Id_t id)
{
    // ex GCommandContainer::findCommand
    // ex phost.pas:GetPHostCmd
    if (!Command::isReplaceableCommand(typ)) {
        return cmds.end();
    }

    Iterator_t i = cmds.begin();
    while (i != cmds.end() && ((*i)->getId() != id || (*i)->getCommand() != typ)) {
        ++i;
    }
    return i;
}

// Insert newly-created command at proper position.
void
game::v3::CommandContainer::insertNewCommand(Command* cmd)
{
    // ex GCommandContainer::insertNewCommand
    /* find place to insert new command */
    const int this_val = Command::getCommandOrder(cmd->getCommand());
    Iterator_t i = cmds.begin();

    while (i != cmds.end() && Command::getCommandOrder((*i)->getCommand()) <= this_val) {
        ++i;
    }

    /* insert new command */
    cmds.insertNew(i, cmd);
    sig_commandChange.raise(*cmd, true);
}
