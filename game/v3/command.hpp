/**
  *  \file game/v3/command.hpp
  *  \brief Class game::v3::Command
  */
#ifndef C2NG_GAME_V3_COMMAND_HPP
#define C2NG_GAME_V3_COMMAND_HPP

#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/reference.hpp"
#include "game/types.hpp"

namespace game { namespace v3 {

    /** Auxiliary command.

        A command is uniquely identified by its type/id pair.
        A second command with the same pair "overwrites" the first one.
        Some commands require the id to be zero, see the comment in the Type enum definition.
        In addition, each command has a string argument.
        For example, <tt>remote control 33</tt> has type=RemoteControl, id=33, arg="control".

        The Other command is an exception to all these rules:
        it contains a command that is not parsed by PCC and hence not identified in any way.
        Any number of Other commands can coexist although they all have the same id (zero).

        It is not possible to modify the type/id after creating the Command object, to avoid confusing the CommandContainer.
        The argument can be changed, however.

        The actual lookup/replace policy is implemented in CommandContainer.

        This implementation approximately corresponds to PCC 1.1.17
        regarding the command set. */
    class Command {
     public:
        /** Command type. */
        enum Type {
            /** Change language.
                - Id: zero
                - Arg: language name
                ex phc_Language */
            Language,

            /** Request configuration.
                - Id: zero
                - Arg: ignored
                ex phc_SendConfig */
            SendConfig,

            /** Request race name file.
                - Id: zero
                - Arg: ignored
                ex phc_SendRacenames */
            SendRaceNames,

            /** Change race name.
                - Id: LongName/ShortName/AdjectiveName
                - Arg: new name
                ex phc_SetRaceName */
            SetRaceName,

            /** Configure message filter.
                - Id: zero
                - Arg: new status, "yes" or "no"
                ex phc_Filter */
            Filter,

            /** Configure an alliance.
                - Id: player number
                - Arg: alliance status string ("+c ~m -v" etc.)
                ex phc_ConfigAlly */
            ConfigAlly,

            /** Offer/Resign an alliance.
                - Id: player number
                - Arg: "add" or "drop"
                ex phc_AddDropAlly */
            AddDropAlly,

            /** Give away a ship.
                - Id: ship Id
                - Arg: player number as string
                ex phc_GiveShip */
            GiveShip,

            /** Give away a planet.
                - Id: planet Id
                - Arg: player number as string
                ex phc_GivePlanet */
            GivePlanet,

            /** Configure remote control.
                - Id: ship Id
                - Arg: new status, "control", "allow", "forbid", "drop"
                ex phc_RemoteControl */
            RemoteControl,

            /** Configure default remote control status.
                - Id: zero
                - Arg: new status, "allow" or "forbid"
                ex phc_RemoteDefault */
            RemoteDefault,

            /** Beam up multiple.
                - Id: ship Id
                - Arg: cargo string ("M100 Nmax")
                ex phc_Beamup */
            BeamUp,

            /** Configure THost alliances.
                - Id: zero
                - Arg: fcode list (3*N chars, "ffaFF1ee2")
                ex phc_TAlliance */
            TAlliance,

            /** Request friendly code list.
                - Id: zero
                - Arg: ignored
                ex phc_SendFcodes */
            SendFCodes,

            /** File relay (using tcm_SendBack, PHost 4.0+).
                - Id: zero
                - Arg: file name
                ex phc_SendFile */
            SendFile,

            /** Configure permanent enemies (PHost 4.0g+).
                - Id: player number
                - Arg: "add" or "drop"
                ex phc_Enemies */
            Enemies,

            /** Unload stuff from ship to planet/space (PHost 4.0h+).
                - Id: ship Id
                - Arg: cargo string
                ex phc_Unload */
            Unload,

            /** Transfer stuff from ship to other ship (PHost 4.0h+).
                - Id: ship Id
                - Arg: cargo string plus "to TARGET-SID"
                ex phc_Transfer */
            Transfer,

            /** Show ship to ally (PHost 4.0h+).
                - Id: ship Id
                - Arg: race list, "1 2 3"
                ex phc_ShowShip */
            ShowShip,

            /** Show planet to ally (PHost 4.0h+).
                - Id: planet Id
                - Arg: race list, "1 2 3"
                ex phc_ShowPlanet */
            ShowPlanet,

            /** Show minefield to ally (PHost 4.0h+).
                - Id: minefield Id
                - Arg: race list, "1 2 3"
                ex phc_ShowMinefield */
            ShowMinefield,

            /** Extended refit order (PHost 4.0h+).
                - Id: ship Id
                - Arg: equipment
                ex phc_Refit */
            Refit,

            /** Other command.
                - Id: zero
                - Arg: the command
                ex phc_Other */
            Other           
        };

        /** "id" parameters for SetRaceName. */
        enum Name {
            LongName,           ///< Long name; compare Player::LongName. ex phcrn_Long.
            ShortName,          ///< Short name; compare Player::ShortName. ex phcrn_Short.
            AdjectiveName       ///< Adjective; compare Player::AdjectiveName. ex phcrn_Adjective.
        };

        /** Create new command.
            \param cmd Command
            \param id Id (ship, minefield, etc.; depending on command)
            \param arg Argument (depending on command) */
        Command(Type cmd, Id_t id, String_t arg);

        /** Destructor. */
        ~Command();

        /** Get command type.
            \return type */
        Type getCommand() const;

        /** Get associated Id number.
            \return Id */
        Id_t getId() const;

        /** Get command parameter.
            \return parameter. */
        const String_t& getArg() const;

        /** Set command parameter.
            \return parameter. */
        void setArg(const String_t& s);

        /** Get affected ship Id.
            In theory, a command can affect multiple objects (e.g. Unload or Refit).
            For now, we only associate the command with the single object it is directly addressed at.
            \return Id of affected ship, 0 if none. */
        Id_t getAffectedShip() const;

        /** Get affected planet Id.
            \return Id of affected planet, 0 if none.
            \see getAffectedShip */
        Id_t getAffectedPlanet() const;

        /** Get affected minefield Id.
            \return Id of affected minefield, 0 if none.
            \see getAffectedShip */
        Id_t getAffectedMinefield() const;

        /** Get affected unit reference.
            \return Reference to affected unit; can be empty (!isSet()). */
        Reference getAffectedUnit() const;

        /** Get complete command text.
            This text can be written to cmdX.txt.
            For messages that are actual PHost command messages, this is the same as the command message text.
            \return text */
        String_t getCommandText() const;

        /** Parse command.

            With fromFile=true, all commands are accepted, unrecognized commands produce Other objects.
            With fromFile=false, only proper command messages are accepted, everything else shall be left in the message file.

            With acceptProto=true, partial commands ("proto-commands") are accepted.
            Those are commands missing their "arg" part.
            Proto-commands are used for matching existing commands.
            For example, <tt>parseCommand("give ship 100", false, true)</tt> will be identified as a GiveShip command for ship 100,
            allowing CommandContainer to look up that command.

            \param text Text to parse
            \param fromFile    true if this command comes from cmdX.txt; false if it comes from a command message
            \param acceptProto true to accept proto-commands
            \return newly-allocated Command object on success; otherwise null. */
        static Command* parseCommand(String_t text, bool fromFile, bool acceptProto);

        /** Check for message introducer commands.
            \param text Message line
            \return true iff text is a command that starts a message
            (i.e. the rest of the command message is the message content, not further commands). */
        static bool isMessageIntroducer(String_t text);

        /** Check for replaceable command.
            \param cmd Command type
            \retval true A command with the same type and Id will replace another
            \retval false There can be any number of commands with that type and Id, so we cannot detect replacement. */
        static bool isReplaceableCommand(Type cmd);

        /** Get ordering value for sorting.
            Commands are sorted into some sensible order to increase the chance that they're processed correctly.
            Older PHost versions process commands in whatever order they come in, so a 'remote control'
            which precedes the enabling 'allies add' will fail.
            Newer PHost versions do no longer have this problem.

            \param cmd Command type

            \return sort key. Low values sort before higher values. */
        static int getCommandOrder(Type cmd);

        /** Get information for a command.
            \param cmd Command type
            \param tx  Translator
            \return command description */
        static String_t getCommandInfo(Type cmd, afl::string::Translator& tx);

     private:
        Type      cmd;              ///< Command code.
        Id_t      id;               ///< Associated Id.
        String_t  arg;              ///< Parameter.

        static uint8_t classifyCommand(Type cmd);
    };


} }

#endif
