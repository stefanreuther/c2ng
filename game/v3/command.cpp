/**
  *  \file game/v3/command.cpp
  *
  *  PCC2 Comment for Command/CommandContainer:
  *
  *  This module provides functions to manage auxiliary commands (PHost
  *  command processor, THost allies) -- basically all this information
  *  that is not represented in a .dat/.dis file pair but must still be
  *  sent to the host.
  *
  *  We parse PHost commands, and send them out in "canonical" format.
  *  Basically, when player says "re c 3" using some message-writing
  *  program and we see it, the spelled-out form will be sent to the
  *  host (which in this case will actually be "remote c 3", because
  *  we only expand secondary words when they make this command belong
  *  to a different "class").
  */

#include "game/v3/command.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/string/string.hpp"
#include "util/string.hpp"

namespace {
    /** Shorten command word until command falls within limits. Older
        PHost versions allow at most 40 characters per command. In case we
        generate a longer command (so far, this happens only for
        'beamup'), this function does some damage reduction. This is not
        always possible. The longest valid and sensible beamup command is
        <tt>be 333 N1000 T1000 D1000 M1000 S1000 $10000 C1000</tt>
        which is 49 characters (assuming ships don't have more than
        10000 cargo).

        Newer PHost versions have no command length limits anymore.

        \param command Command verb
        \param min_len Minimum required letters
        \param arg Command argument */
    static String_t trimCommand(String_t command, size_t min_len, String_t arg)
    {
        if (command.size() + arg.size() > 40) {
            if (arg.size() > 40 - min_len)
                command.erase(min_len);
            else
                command.erase(40 - arg.size());
        }
        return command + arg;
    }
}

game::v3::Command::Command(Type cmd, Id_t id, String_t arg)
    : cmd(cmd), id(id), arg(arg)
{ }

// /** Destructor. */
game::v3::Command::~Command()
{
    // ex GCommand::~GCommand
}

//     /** Get command code. */
game::v3::Command::Type
game::v3::Command::getCommand() const
{
    return cmd;
}
// /** Get associated Id number. */
game::Id_t
game::v3::Command::getId() const
{
    return id;
}

//     /** Get command parameter. */
const String_t&
game::v3::Command::getArg() const
{
    return arg;
}

// /** Set command parameter. */
void
game::v3::Command::setArg(const String_t& s)
{
    arg = s;
}

/** Check whether this command affects a ship. Note that a command can, theoretically,
    affect several objects. Currently this ability is not used (but would make sense
    for 'refit' or 'unload').
    \return ship Id, 0 if no ship affected */
game::Id_t
game::v3::Command::getAffectedShip() const
{
    // ex GCommand::getAffectedShip
    switch (cmd) {
     case phc_GiveShip:
     case phc_RemoteControl:
     case phc_Beamup:
     case phc_Unload:
     case phc_Transfer:
     case phc_ShowShip:
     case phc_Refit:
        return id;
     default:
        return 0;
    }
}

// /** Check whether this command affects a planet.
//     \return planet Id, 0 if no planet affected
//     \see getAffectedShip() */
game::Id_t
game::v3::Command::getAffectedPlanet() const
{
    // ex GCommand::getAffectedPlanet
    switch (cmd) {
     case phc_GivePlanet:
     case phc_ShowPlanet:
        return id;
     default:
        return 0;
    }
}

// /** Check whether this command affects a minefield.
//     \return minefield Id, 0 if no minefields affected
//     \see getAffectedShip() */
game::Id_t
game::v3::Command::getAffectedMinefield() const
{
    // ex GCommand::getAffectedMinefield
    switch (cmd) {
     case phc_ShowMinefield:
        return id;
     default:
        return 0;
    }
}

// /** Return complete text of command. This text can be written
//     cmdX.txt. Not all commands are suitable for command messages. */
String_t
game::v3::Command::getCommandText() const
{
    // ex GCommand::getCommandText
    /* PCC 1.x: phost.pas::PHostCommandString */
    switch (cmd) {
     case phc_Other:
        return arg;
     case phc_Language:
        return "language " + arg;
     case phc_SendConfig:
        return "send config";
     case phc_SendRacenames:
        return "send racenames";
     case phc_SendFcodes:
        return "send fcodes";
     case phc_SetRaceName:
        switch (id) {
         case phcrn_Long:
            return "race long " + arg;
         case phcrn_Short:
            return "race short " + arg;
         case phcrn_Adjective:
            return "race adj " + arg;
         default:
            return String_t();
        }
     case phc_Filter:
        return "filter " + arg;
     case phc_ConfigAlly:
        return afl::string::Format("allies config %d %s", id, arg);
     case phc_AddDropAlly:
        return afl::string::Format("allies %s %d", arg, id);
     case phc_RemoteControl:
        return afl::string::Format("remote %s %d", arg, id);
     case phc_RemoteDefault:
        return "remote " + arg + " default";
     case phc_Beamup:
        return trimCommand("beamup", 2, afl::string::Format(" %d %s", id, arg));
     case phc_GiveShip:
        return afl::string::Format("give ship %d to %s", id, arg);
     case phc_GivePlanet:
        return afl::string::Format("give planet %d to %s", id, arg);
     case phc_TAlliance:
        return "$thost-allies " + arg;
     case phc_SendFile:
        return "$send-file " + arg;
     case phc_Enemies:
        return afl::string::Format("enemies %s %d", arg, id);
     case phc_Unload:
        return trimCommand("unload", 3, afl::string::Format(" %d %s", id, arg));
     case phc_Transfer:
        return trimCommand("transfer", 3, afl::string::Format(" %d %s", id, arg));
     case phc_ShowShip:
        return trimCommand("show ship", 6, afl::string::Format(" %d %s", id, arg));
     case phc_ShowPlanet:
        return trimCommand("show planet", 6, afl::string::Format(" %d %s", id, arg));
     case phc_ShowMinefield:
        return trimCommand("show minefield", 6, afl::string::Format(" %d %s", id, arg));
     case phc_Refit:
        return afl::string::Format("refit %d %s", id, arg);
    }
    return String_t();
}

// /** Parse a command.
//     \return newly-allocated command, 0 if not recognized.
//     \param text the command to process
//     \param fromfile true iff this command comes from a cmdX.txt file, false
//     if from a message. */
game::v3::Command*
game::v3::Command::parseCommand(String_t text, bool fromfile, bool acceptProto)
{
    // ex GCommand::parseCommand
    using afl::string::strLTrim;
    using afl::string::strNthWord;
    using afl::string::strNthWordRest;
    using afl::string::strCaseCompare;
    using afl::string::strToInteger;
    using util::stringMatch;
    
    text = strLTrim(text);

    String_t verb = strNthWord(text, 0);
    int wordoffs  = 0;

    /* Private Commands */
    if (fromfile && strCaseCompare(verb, "$THOST-ALLIES") == 0)
        return new Command(phc_TAlliance, 0, strNthWordRest(text, 1));
    if (fromfile && stringMatch("$SEND-File", verb))
        return new Command(phc_SendFile, 0, strNthWordRest(text, 1));

    /* PHost 4 Command Syntax */
    if (strCaseCompare(verb, "phost") == 0 || strCaseCompare(verb, "phost:") == 0) {
        ++wordoffs;
        verb = strNthWord(text, wordoffs);
    }

    String_t arg = strNthWord(text, wordoffs + 1);

    if (stringMatch("Send", verb)) {
        /* send config|racename|fcodes */
        if (stringMatch("Config", arg))
            return new Command(phc_SendConfig, 0, String_t());
        else if (stringMatch("Racenames", arg))
            return new Command(phc_SendRacenames, 0, String_t());
        else if (stringMatch("Fcodes", arg))
            return new Command(phc_SendFcodes, 0, String_t());
    } else if (stringMatch("Language", verb) && (acceptProto || !arg.empty())) {
        /* language <language> */
        return new Command(phc_Language, 0, arg);
    } else if (stringMatch("Filter", verb) && (acceptProto || !arg.empty())) {
        /* filter yes|no */
        return new Command(phc_Filter, 0, arg);
    } else if (stringMatch("Give", verb)) {
        /* give ship|planet <id> [to] <race> */
        Id_t id;
        if (strToInteger(strNthWord(text, wordoffs + 2), id)) {
            int k = stringMatch("To", strNthWord(text, wordoffs + 3)) ? 4 : 3;
            String_t who = strNthWord(text, wordoffs + k);
            if (acceptProto || who.size()) {
                if (stringMatch("Ship", arg))
                    return new Command(phc_GiveShip, id, who);
                else if (stringMatch("Planet", arg))
                    return new Command(phc_GivePlanet, id, who);
            }
        }
    } else if (stringMatch("Allies", verb)) {
        Id_t player;
        if (strToInteger(strNthWord(text, wordoffs + 2), player)) {
            if (stringMatch("Config", arg)) {
                /* allies config <race> <flags> */
                String_t rest = strNthWordRest(text, wordoffs + 3);
                if (!rest.empty() || acceptProto)
                    return new Command(phc_ConfigAlly, player, rest);
            } else if (stringMatch("Add", arg) || stringMatch("Drop", arg)) {
                /* allies add|drop <race> */
                return new Command(phc_AddDropAlly, player, arg);
            }
        }
    } else if (stringMatch("REmote", verb)) {
        /* remote <verb> <id>|default */
        Id_t sid;
        String_t id = strNthWord(text, wordoffs + 2);
        if (stringMatch("Default", id)) {
            return new Command(phc_RemoteDefault, 0, arg);
        } else if (strToInteger(id, sid)
                   && (acceptProto
                       || stringMatch("Allow", arg) || stringMatch("Forbid", arg)
                       || stringMatch("Control", arg) || stringMatch("Drop", arg)))
        {
            return new Command(phc_RemoteControl, sid, arg);
        }
    } else if (stringMatch("BEamup", verb)) {
        /* beamup <pid> <stuff> */
        Id_t pid;
        if (strToInteger(arg, pid))
            return new Command(phc_Beamup, pid, strNthWordRest(text, wordoffs + 2));
    } else if (stringMatch("UNLoad", verb)) {
        /* unload <sid> <stuff> */
        Id_t sid;
        if (strToInteger(arg, sid))
            return new Command(phc_Unload, sid, strNthWordRest(text, wordoffs + 2));
    } else if (stringMatch("TRAnsfer", verb)) {
        /* transfer <sid> <stuff_TO_sid> */
        Id_t sid;
        if (strToInteger(arg, sid))
            return new Command(phc_Transfer, sid, strNthWordRest(text, wordoffs + 2));
    } else if (stringMatch("RAcename", verb)) {
        /* racename long|short|adj <name> */
        Id_t what =
            stringMatch("Long", arg) ? phcrn_Long :
            stringMatch("Short", arg) ? phcrn_Short :
            stringMatch("Adjective", arg) ? phcrn_Adjective : -1;
        if (what >= 0)
            return new Command(phc_SetRaceName, what, strNthWordRest(text, wordoffs + 2));
    } else if (stringMatch("ENEmies", verb)) {
        /* enemies add|drop <race> */
        if (acceptProto || stringMatch("Add", arg) || stringMatch("Drop", arg)) {
            Id_t player;
            if (strToInteger(strNthWord(text, wordoffs + 2), player))
                return new Command(phc_Enemies, player, arg);
        }
    } else if (stringMatch("SHow", verb)) {
        /* show type <id> [to] <races...> */
        Type t =
            stringMatch("Ship", arg) ? phc_ShowShip :
            stringMatch("Planet", arg) ? phc_ShowPlanet :
            stringMatch("Minefield", arg) ? phc_ShowMinefield : phc_Other;
        if (t != phc_Other) {
            /* FIXME? PCC 1.x 20080301 would here merge this command with an existing
               one of the same type, e.g. "show ship 1 to 2" and "show ship 1 to 5"
               would put "show ship 1 to 2 5" into our list. This matches PHost
               behaviour more closely, but makes an ugly back-dependency of
               Command to CommandContainer. Effectively, the same problem appears
               with 'allies config' as well, but there PCC 1.x and this implementation
               agree in using replace semantics. */
            Id_t id;
            int index = wordoffs + 2;
            if (strToInteger(strNthWord(text, index), id)) {
                ++index;
                if (stringMatch("To", strNthWord(text, index)))
                    ++index;
                return new Command(t, id, strNthWordRest(text, index));
            }
        }
    } else if (stringMatch("REFit", verb)) {
        /* refit <id> <specs...> */
        Id_t sid;
        if (strToInteger(arg, sid))
            return new Command(phc_Refit, sid, strNthWordRest(text, wordoffs + 2));
    }

    return 0;
}

// /** Check for message introducer commands. \returns true iff text is a
//     command that starts a message (i.e. the rest of the message is
//     the message, not further commands). */
bool
game::v3::Command::isMessageIntroducer(String_t text)
{
    // ex GCommand::isMessageIntroducer
    using afl::string::strNthWord;
    using util::stringMatch;

    String_t cmd = strNthWord(text, 0);
    return (stringMatch("RUmor", cmd) || stringMatch("RUmour", cmd) || stringMatch("Message", cmd));
}
