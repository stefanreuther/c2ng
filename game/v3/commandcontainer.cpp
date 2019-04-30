/**
  *  \file game/v3/commandcontainer.cpp
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
    // FIXME: can this be const?
    Iterator_t i = const_cast<CommandContainer*>(this)->findCommand(typ, id);
    if (i != cmds.end())
        return *i;
    else
        return 0;

}

// Add a command.
const game::v3::Command*
game::v3::CommandContainer::addCommand(Command::Type typ, Id_t id, String_t arg)
{
    // ex GCommandContainer::setCommand
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
    // ex GCommandContainer::removeCommand
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
game::v3::CommandContainer::removeCommand(Command* cmd)
{
    // ex GCommandContainer::removeCommand
    // FIXME: this fails because PtrMultiList::iterator does not have required types (difference_type, iterator_category, etc.)
    // Iterator_t i = std::find(cmds.begin(), cmds.end(), cmd);
    Iterator_t i = cmds.begin();
    while (i != cmds.end() && *i != cmd) {
        ++i;
    }
    if (i != cmds.end()) {
        sig_commandChange.raise(**i, false);
        cmds.erase(i);
        delete *i;
    }
}

// Load command file (cmdX.txt).
void
game::v3::CommandContainer::loadCommandFile(afl::io::Stream& file, const Timestamp& time)
{
    // ex GCommandContainer::loadCommandFile
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
                Command* cmd = Command::parseCommand(line, true);
                if (!cmd) {
                    cmd = new Command(Command::phc_Other, 0, line);
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
    /* any number of phc_Other can coexist, so we can not find a
       particular one by just giving a typ/id pair */
    if (typ == Command::phc_Other) {
        return cmds.end();
    }

    Iterator_t i = cmds.begin();
    while (i != cmds.end() && ((*i)->getId() != id || (*i)->getCommand() != typ)) {
        ++i;
    }
    return i;
}

// Get value for sorting.
int
game::v3::CommandContainer::getValue(Command::Type type)
{
    // ex GCommandContainer::getValue
    /* PCC 1.x has '1' for phc_SendFcodes, although I cannot come up
       with a reason. */
    switch (type) {
     case Command::phc_SendRacenames:
        // should be after phc_SetRaceName
        return 2;
     case Command::phc_SendConfig:
        // should be after phc_Filter, phc_Language
        return 1;
     case Command::phc_ConfigAlly:
        // should be after phc_AddDropAlly
        return 1;
     case Command::phc_RemoteControl:
        // should be after phc_ConfigAlly
        return 2;
     default:
        return 0;
    }
}

// Insert newly-created command at proper position.
void
game::v3::CommandContainer::insertNewCommand(Command* cmd)
{
    // ex GCommandContainer::insertNewCommand
    /* find place to insert new command */
    const int this_val = getValue(cmd->getCommand());
    Iterator_t i = cmds.begin();

    while (i != cmds.end() && getValue((*i)->getCommand()) <= this_val) {
        ++i;
    }

    /* insert new command */
    cmds.insertNew(i, cmd);
    sig_commandChange.raise(*cmd, true);
}
