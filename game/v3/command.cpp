/**
  *  \file game/v3/command.cpp
  *  \brief Class game::v3::Command
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
#include "util/translation.hpp"

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


    /*
     *  Command Classification
     *
     *  - 3 bits for getCommandOrder()  [currently 2 needed]
     *  - 1 bit for getAffectedShip() etc.
     *  - 1 bit for isReplaceableCommand()
     */

    const int ORDER_MASK = 7;
    const int AffectsShip = 8;
    const int AffectsPlanet = 16;
    const int AffectsMinefield = 32;
    const int NotReplaceable = 64;
}


// Create new command.
game::v3::Command::Command(Type cmd, Id_t id, String_t arg)
    : cmd(cmd), id(id), arg(arg)
{ }

// Destructor.
game::v3::Command::~Command()
{
    // ex GCommand::~GCommand
}

// Get command type.
game::v3::Command::Type
game::v3::Command::getCommand() const
{
    return cmd;
}

// Get associated Id number.
game::Id_t
game::v3::Command::getId() const
{
    return id;
}

// Get command parameter.
const String_t&
game::v3::Command::getArg() const
{
    return arg;
}

// Set command parameter.
void
game::v3::Command::setArg(const String_t& s)
{
    arg = s;
}

// Get affected ship Id.
game::Id_t
game::v3::Command::getAffectedShip() const
{
    // ex GCommand::getAffectedShip
    // ex phost.pas:SidCommands
    return (classifyCommand(cmd) & AffectsShip) ? id : 0;
}

// Get affected planet Id.
game::Id_t
game::v3::Command::getAffectedPlanet() const
{
    // ex GCommand::getAffectedPlanet
    // ex phost.pas:PidCommands
    return (classifyCommand(cmd) & AffectsPlanet) ? id : 0;
}

// Get affected minefield Id.
game::Id_t
game::v3::Command::getAffectedMinefield() const
{
    // ex GCommand::getAffectedMinefield
    return (classifyCommand(cmd) & AffectsMinefield) ? id : 0;
}

// Get affected unit reference.
game::Reference
game::v3::Command::getAffectedUnit() const
{
    if (Id_t shipId = getAffectedShip()) {
        return Reference(Reference::Ship, shipId);
    } else if (Id_t planetId = getAffectedPlanet()) {
        return Reference(Reference::Planet, planetId);
    } else if (Id_t minefieldId = getAffectedMinefield()) {
        return Reference(Reference::Minefield, minefieldId);
    } else {
        return Reference();
    }
}

// Get complete command text.
String_t
game::v3::Command::getCommandText() const
{
    // ex GCommand::getCommandText
    // ex phost.pas::PHostCommandString
    using afl::string::Format;
    switch (cmd) {
     case Other:
        return arg;
     case Language:
        return "language " + arg;
     case SendConfig:
        return "send config";
     case SendRaceNames:
        return "send racenames";
     case SendFCodes:
        return "send fcodes";
     case SetRaceName:
        switch (id) {
         case LongName:
            // PCC1 would apply trimCommand here, but this command cannot legally overflow
            return "race long " + arg;
         case ShortName:
            return "race short " + arg;
         case AdjectiveName:
            return "race adj " + arg;
         default:
            return String_t();
        }
     case Filter:
        return "filter " + arg;
     case ConfigAlly:
        return Format("allies config %d %s", id, arg);
     case AddDropAlly:
        return Format("allies %s %d", arg, id);
     case RemoteControl:
        return Format("remote %s %d", arg, id);
     case RemoteDefault:
        return "remote " + arg + " default";
     case BeamUp:
        return trimCommand("beamup", 2, Format(" %d %s", id, arg));
     case GiveShip:
        return Format("give ship %d to %s", id, arg);
     case GivePlanet:
        return Format("give planet %d to %s", id, arg);
     case TAlliance:
        return "$thost-allies " + arg;
     case SendFile:
        return "$send-file " + arg;
     case Enemies:
        return Format("enemies %s %d", arg, id);
     case Unload:
        return trimCommand("unload", 3, Format(" %d %s", id, arg));
     case Transfer:
        return trimCommand("transfer", 3, Format(" %d %s", id, arg));
     case ShowShip:
        return trimCommand("show ship", 6, Format(" %d %s", id, arg));
     case ShowPlanet:
        return trimCommand("show planet", 6, Format(" %d %s", id, arg));
     case ShowMinefield:
        return trimCommand("show minefield", 6, Format(" %d %s", id, arg));
     case Refit:
        return Format("refit %d %s", id, arg);
    }
    return String_t();
}

// Parse a command.
game::v3::Command*
game::v3::Command::parseCommand(String_t text, bool fromFile, bool acceptProto)
{
    // ex GCommand::parseCommand
    // ex phost.pas:ReverseEngineerPHostCommand
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
    if (fromFile && strCaseCompare(verb, "$THOST-ALLIES") == 0)
        return new Command(TAlliance, 0, strNthWordRest(text, 1));
    if (fromFile && stringMatch("$SEND-File", verb))
        return new Command(SendFile, 0, strNthWordRest(text, 1));

    /* PHost 4 Command Syntax */
    if (strCaseCompare(verb, "phost") == 0 || strCaseCompare(verb, "phost:") == 0) {
        ++wordoffs;
        verb = strNthWord(text, wordoffs);
    }

    String_t arg = strNthWord(text, wordoffs + 1);

    if (stringMatch("Send", verb)) {
        /* send config|racename|fcodes */
        if (stringMatch("Config", arg))
            return new Command(SendConfig, 0, String_t());
        else if (stringMatch("Racenames", arg))
            return new Command(SendRaceNames, 0, String_t());
        else if (stringMatch("Fcodes", arg))
            return new Command(SendFCodes, 0, String_t());
    } else if (stringMatch("Language", verb) && (acceptProto || !arg.empty())) {
        /* language <language> */
        return new Command(Language, 0, arg);
    } else if (stringMatch("Filter", verb) && (acceptProto || !arg.empty())) {
        /* filter yes|no */
        return new Command(Filter, 0, arg);
    } else if (stringMatch("Give", verb)) {
        /* give ship|planet <id> [to] <race> */
        Id_t id;
        if (strToInteger(strNthWord(text, wordoffs + 2), id)) {
            int k = stringMatch("To", strNthWord(text, wordoffs + 3)) ? 4 : 3;
            String_t who = strNthWord(text, wordoffs + k);
            if (acceptProto || who.size()) {
                if (stringMatch("Ship", arg))
                    return new Command(GiveShip, id, who);
                else if (stringMatch("Planet", arg))
                    return new Command(GivePlanet, id, who);
            }
        }
    } else if (stringMatch("Allies", verb)) {
        Id_t player;
        if (strToInteger(strNthWord(text, wordoffs + 2), player)) {
            if (stringMatch("Config", arg)) {
                /* allies config <race> <flags> */
                String_t rest = strNthWordRest(text, wordoffs + 3);
                if (!rest.empty() || acceptProto)
                    return new Command(ConfigAlly, player, rest);
            } else if (stringMatch("Add", arg) || stringMatch("Drop", arg)) {
                /* allies add|drop <race> */
                return new Command(AddDropAlly, player, arg);
            }
        }
    } else if (stringMatch("REmote", verb)) {
        /* remote <verb> <id>|default */
        Id_t sid;
        String_t id = strNthWord(text, wordoffs + 2);
        if (stringMatch("Default", id)) {
            return new Command(RemoteDefault, 0, arg);
        } else if (strToInteger(id, sid)
                   && (acceptProto
                       || stringMatch("Allow", arg) || stringMatch("Forbid", arg)
                       || stringMatch("Control", arg) || stringMatch("Drop", arg)))
        {
            return new Command(RemoteControl, sid, arg);
        }
    } else if (stringMatch("BEamup", verb)) {
        /* beamup <pid> <stuff> */
        Id_t pid;
        if (strToInteger(arg, pid))
            return new Command(BeamUp, pid, strNthWordRest(text, wordoffs + 2));
    } else if (stringMatch("UNLoad", verb)) {
        /* unload <sid> <stuff> */
        Id_t sid;
        if (strToInteger(arg, sid))
            return new Command(Unload, sid, strNthWordRest(text, wordoffs + 2));
    } else if (stringMatch("TRAnsfer", verb)) {
        /* transfer <sid> <stuff_TO_sid> */
        Id_t sid;
        if (strToInteger(arg, sid))
            return new Command(Transfer, sid, strNthWordRest(text, wordoffs + 2));
    } else if (stringMatch("RAcename", verb)) {
        /* racename long|short|adj <name> */
        Id_t what =
            stringMatch("Long", arg) ? LongName :
            stringMatch("Short", arg) ? ShortName :
            stringMatch("Adjective", arg) ? AdjectiveName : -1;
        if (what >= 0)
            return new Command(SetRaceName, what, strNthWordRest(text, wordoffs + 2));
    } else if (stringMatch("ENEmies", verb)) {
        /* enemies add|drop <race> */
        if (acceptProto || stringMatch("Add", arg) || stringMatch("Drop", arg)) {
            Id_t player;
            if (strToInteger(strNthWord(text, wordoffs + 2), player))
                return new Command(Enemies, player, arg);
        }
    } else if (stringMatch("SHow", verb)) {
        /* show type <id> [to] <races...> */
        Type t =
            stringMatch("Ship", arg) ? ShowShip :
            stringMatch("Planet", arg) ? ShowPlanet :
            stringMatch("Minefield", arg) ? ShowMinefield : Other;
        if (t != Other) {
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
            return new Command(Refit, sid, strNthWordRest(text, wordoffs + 2));
    }

    return 0;
}

// Check for message introducer commands.
bool
game::v3::Command::isMessageIntroducer(String_t text)
{
    // ex GCommand::isMessageIntroducer
    // ex phost.pas:IsPHostMessageIntroducer
    using afl::string::strNthWord;
    using util::stringMatch;

    String_t cmd = strNthWord(text, 0);
    return (stringMatch("RUmor", cmd) || stringMatch("RUmour", cmd) || stringMatch("Message", cmd));
}

// Check for replaceable command.
bool
game::v3::Command::isReplaceableCommand(Type cmd)
{
    // ex phost.pas:IsUniqId
    return (classifyCommand(cmd) & NotReplaceable) == 0;
}

// Get ordering value for sorting.
int
game::v3::Command::getCommandOrder(Type cmd)
{
    // ex GCommandContainer::getValue
    return (classifyCommand(cmd) & ORDER_MASK);
}

String_t
game::v3::Command::getCommandInfo(Type cmd, afl::string::Translator& tx)
{
    // ex WCommandInfo::drawContent (sort-of), CCommandHelper.Draw
    const char* text = N_("(command not understood by PCC2)");
    switch (cmd) {
     case Language:       text = N_("Change message language");                                       break;
     case SendConfig:     text = N_("Request configuration");                                         break;
     case SendRaceNames:  text = N_("Request race names file");                                       break;
     case SetRaceName:    text = N_("Change our race name");                                          break;
     case Filter:         text = N_("Change message detail (host-side filter)");                      break;
     case ConfigAlly:     text = N_("Change ally privileges");                                        break;
     case AddDropAlly:    text = N_("Add/drop ally (PHost version)");                                 break;
     case GiveShip:       text = N_("Give away a ship");                                              break;
     case GivePlanet:     text = N_("Give away a planet");                                            break;
     case RemoteControl:  text = N_("Configure or request remote control over a ship");               break;
     case RemoteDefault:  text = N_("Configure remote control setting for newly-built ships");        break;
     case BeamUp:         text = N_("Gather resources from a planet");                                break;
     case TAlliance:      text = N_("Add/drop ally (Host 3.22.007+ version)");                        break;
     case SendFCodes:     text = N_("Request friendly code list");                                    break;
     case SendFile:       text = N_("Send file with TRN (will echo back with next RST, PHost 4.0+)"); break;
     case Enemies:        text = N_("Add/drop permanent enemy");                                      break;
     case Unload:         text = N_("Unload cash/ammo onto foreign planet");                          break;
     case Transfer:       text = N_("Transfer cash/ammo to foreign ship");                            break;
     case ShowShip:       text = N_("Show ship to an ally");                                          break;
     case ShowPlanet:     text = N_("Show planet to an ally");                                        break;
     case ShowMinefield:  text = N_("Show minefield to an ally");                                     break;
     case Refit:          text = N_("Configure parts for Super-Refit mission");                       break;
     case Other:                                                                                      break;
    }
    return tx(text);
}

uint8_t
game::v3::Command::classifyCommand(Type cmd)
{
    switch (cmd) {
     case Language:       return 0;
     case SendConfig:     return 1;    // after Filter, Language
     case SendRaceNames:  return 2;    // after SetRaceName
     case SetRaceName:    return 0;
     case Filter:         return 0;
     case ConfigAlly:     return 1;    // after AddDropAlly
     case AddDropAlly:    return 0;
     case GiveShip:       return 0 + AffectsShip;
     case GivePlanet:     return 0 + AffectsPlanet;
     case RemoteControl:  return 2 + AffectsShip;   // after ConfigAlly
     case RemoteDefault:  return 0;
     case BeamUp:         return 0 + AffectsShip;
     case TAlliance:      return 0;
     case SendFCodes:     return 0;
     case SendFile:       return 0 + NotReplaceable;
     case Enemies:        return 0;
     case Unload:         return 0 + AffectsShip;
     case Transfer:       return 0 + AffectsShip;
     case ShowShip:       return 0 + AffectsShip;
     case ShowPlanet:     return 0 + AffectsPlanet;
     case ShowMinefield:  return 0 + AffectsMinefield;
     case Refit:          return 0 + AffectsShip;
     case Other:          return 0 + NotReplaceable;
    }
    return 0;
}
