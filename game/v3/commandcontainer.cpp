/**
  *  \file game/v3/commandcontainer.cpp
  */

#include <algorithm>
#include "game/v3/commandcontainer.hpp"
#include "afl/io/textfile.hpp"
#include "util/string.hpp"

// /** Construct empty command container. */
game::v3::CommandContainer::CommandContainer()
    : sig_commandChange(),
      cmds()
{
    // ex GCommandContainer::GCommandContainer
}

// /** Destructor. Destroys all contained commands. */
game::v3::CommandContainer::~CommandContainer()
{
    // ex GCommandContainer::~GCommandContainer
}

// /** Clear this container. Discards all the commands. */
void
game::v3::CommandContainer::clear()
{
    // ex GCommandContainer::clear
    // safeClear(cmds);
    // command_list().swap(cmds);
    for (ConstIterator_t it = cmds.begin(); it != cmds.end(); ++it) {
        sig_commandChange.raise(**it, false);
    }
    cmds.clear();
}

// /** Fetch command typ/id. \returns command, or 0 if no such command
//     exists. */
const game::v3::Command*
game::v3::CommandContainer::getCommand(Command::Type typ, Id_t id)
{
    // ex GCommandContainer::getCommand
    CommandList_t::iterator i = findCommand(typ, id);
    if (i != cmds.end())
        return *i;
    else
        return 0;

}

// /** Submit a command. When a command with the same typ/id pair exists,
//     it is overwritten; otherwise a new command is created.
//     \returns the GCommand created/reused by this operation. */
const game::v3::Command*
game::v3::CommandContainer::addCommand(Command::Type typ, Id_t id, String_t arg)
{
    // ex GCommandContainer::setCommand
    CommandList_t::iterator i = findCommand(typ, id);
    if (i != cmds.end()) {
        (*i)->setArg(arg);
        sig_commandChange.raise(**i, true);
        return *i;
    } else {
        Command* cmd = new Command(typ, id, arg);
        insertNewCommand(cmd);
        return cmd;
    }
}

/** Submit a command. Like addCommand(GCommand::Type,int,string_t),
    but this one uses an existing GCommand object. The GCommand*
    pointer gets owned by this GCommandContainer.
    \returns cmd */
const game::v3::Command*
game::v3::CommandContainer::addNewCommand(Command* cmd)
{
    // ex GCommandContainer::setCommand
    if (cmd != 0) {
        CommandList_t::iterator i = findCommand(cmd->getCommand(), cmd->getId());
        if (i != cmds.end()) {
            // FIXME: original code:
            // delete *i;
            // *i = cmd;
            **i = *cmd;
            sig_commandChange.raise(**i, true);

            delete cmd;
            cmd = *i;
        } else {
            insertNewCommand(cmd);
        }
    }
    return cmd;
}

// /** Remove a command. \returns true iff command found, false if not. */
bool
game::v3::CommandContainer::removeCommand(Command::Type typ, Id_t id)
{
    // ex GCommandContainer::removeCommand
    CommandList_t::iterator i = findCommand(typ, id);
    if (i != cmds.end()) {
        sig_commandChange.raise(**i, false);
        cmds.erase(i);
        return true;
    } else {
        return false;
    }
}

// /** Remove a command. */
void
game::v3::CommandContainer::removeCommand(Command* cmd)
{
    // ex GCommandContainer::removeCommand
    // FIXME: this fails because PtrMultiList::iterator does not have required types (difference_type, iterator_category, etc.)
    // CommandList_t::iterator i = std::find(cmds.begin(), cmds.end(), cmd);
    CommandList_t::iterator i = cmds.begin();
    while (i != cmds.end() && *i != cmd) {
        ++i;
    }
    if (i != cmds.end()) {
        sig_commandChange.raise(**i, false);
        cmds.erase(i);
        delete *i;
    }
}

// /** Load command file (cmdX.txt). */
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


// /** Find command by type/id. \return iterator pointing to command, or cmds.end() */
game::v3::CommandContainer::Iterator_t
game::v3::CommandContainer::findCommand(Command::Type typ, Id_t id)
{
    // ex GCommandContainer::findCommand
    /* any number of phc_Other can coexist, so we can not find a
       particular one by just giving a typ/id pair */
    if (typ == Command::phc_Other) {
        return cmds.end();
    }

    CommandList_t::iterator i = cmds.begin();
    while (i != cmds.end() && ((*i)->getId() != id || (*i)->getCommand() != typ)) {
        ++i;
    }
    return i;
}

// /** Get value for sorting. Commands are sorted into some sensible
//     order to increase the chance that they're processed correctly.
//     Older PHost versions process commands in whatever order they come
//     in, so a 'remote control' which precedes the enabling 'allies add'
//     will fail. Newer PHost versions do no longer have this problem.

//     \return sort key. Low values sort before higher values. */
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

// /** Insert newly-created command at proper position. This is the
//     backend of the addCommand() functions.

//     This function assumes responsibility for /cmd/; if anything in it
//     throws, the command will be deleted. */
void
game::v3::CommandContainer::insertNewCommand(Command* cmd)
{
    // ex GCommandContainer::insertNewCommand
    /* find place to insert new command */
    const int this_val = getValue(cmd->getCommand());
    CommandList_t::iterator i = cmds.begin();

    while (i != cmds.end() && getValue((*i)->getCommand()) <= this_val) {
        ++i;
    }

    /* insert new command */
    cmds.insertNew(i, cmd);
    sig_commandChange.raise(*cmd, true);
}
