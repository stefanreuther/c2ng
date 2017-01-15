/**
  *  \file game/v3/command.hpp
  */
#ifndef C2NG_GAME_V3_COMMAND_HPP
#define C2NG_GAME_V3_COMMAND_HPP

#include "afl/string/string.hpp"
#include "game/types.hpp"

namespace game { namespace v3 {

    /** Auxiliary command.

        A command is uniquely identified by its type/id pair. A second
        command with the same pair "overwrites" the first one. Some
        commands require the id to be zero, see the comment in the Type
        enum definition. In addition, each command has a string argument.
        For example, <tt>remote control 33</tt> has type=phc_RemoteControl,
        id=33, arg="control".

        The phc_Other command is an exception to all these rules: it
        contains a command that is not parsed by PCC and hence not
        identified in any way. Any number of phc_Other commands can
        coexist although they all have the same id.

        It is not possible to modify the type/id after creating the
        GCommand object, to avoid confusing the GCommandContainer. The
        argument can be changed, however.

        The actual lookup/replace policy is implemented in
        GCommandContainer.

        This implementation approximately corresponds to PCC 1.1.17
        regarding the command set. */
    class Command {
     public:
        /** Command type. */
        enum Type {
            phc_Language,           /**< Change language.
                                       - Id: zero
                                       - Arg: language name */
            phc_SendConfig,         /**< Request configuration.
                                       - Id: zero
                                       - Arg: ignored */
            phc_SendRacenames,      /**< Request race name file.
                                       - Id: zero
                                       - Arg: ignored */
            phc_SetRaceName,        /**< Change race name.
                                       - Id: phcrn_XXX
                                       - Arg: new name */
            phc_Filter,             /**< Configure message filter.
                                       - Id: zero
                                       - Arg: new status, "yes" or "no" */
            phc_ConfigAlly,         /**< Configure an alliance.
                                       - Id: player number
                                       - Arg: alliance status string ("+c ~m -v" etc.) */
            phc_AddDropAlly,        /**< Offer/Resign an alliance.
                                       - Id: player number
                                       - Arg: "add" or "drop" */
            phc_GiveShip,           /**< Give away a ship.
                                       - Id: ship Id
                                       - Arg: player number as string */
            phc_GivePlanet,         /**< Give away a planet.
                                       - Id: planet Id
                                       - Arg: player number as string */
            phc_RemoteControl,      /**< Configure remote control.
                                       - Id: ship Id
                                       - Arg: new status, "control", "allow", "forbid", "drop" */
            phc_RemoteDefault,      /**< Configure default remote control status.
                                       - Id: zero
                                       - Arg: new status, "allow" or "forbid" */
            phc_Beamup,             /**< Beam up multiple.
                                       - Id: ship Id
                                       - Arg: cargo string ("M100 Nmax") */
            phc_TAlliance,          /**< Configure THost alliances.
                                       - Id: zero
                                       - Arg: fcode list (3*N chars, "ffaFF1ee2") */
            phc_SendFcodes,         /**< Request friendly code list.
                                       - Id: zero
                                       - Arg: ignored */
            phc_SendFile,           /**< File relay (using tcm_SendBack, PHost 4.0+).
                                       - Id: zero
                                       - Arg: file name */
            phc_Enemies,            /**< Configure permanent enemies (PHost 4.0g+).
                                       - Id: player number
                                       - Arg: "add" or "drop" */
            phc_Unload,             /**< Unload stuff from ship to planet/space (PHost 4.0h+).
                                       - Id: ship Id
                                       - Arg: cargo string */
            phc_Transfer,           /**< Transfer stuff from ship to other ship (PHost 4.0h+).
                                       - Id: ship Id
                                       - Arg: cargo string plus "to TARGET-SID" */
            phc_ShowShip,           /**< Show ship to ally (PHost 4.0h+).
                                       - Id: ship Id
                                       - Arg: race list, "1 2 3" */
            phc_ShowPlanet,         /**< Show planet to ally (PHost 4.0h+).
                                       - Id: planet Id
                                       - Arg: race list, "1 2 3" */
            phc_ShowMinefield,      /**< Show minefield to ally (PHost 4.0h+).
                                       - Id: minefield Id
                                       - Arg: race list, "1 2 3" */
            phc_Refit,              /**< Extended refit order (PHost 4.0h+).
                                       - Id: ship Id
                                       - Arg: equipment */
            phc_Other,              /**< Other command.
                                       - Id: zero
                                       - Arg: the command */
            phc_LAST = phc_Other    /**< Identifies the last phc_XXX value. */
        };

        /** "id" parameters for phc_SetRaceName. */
        enum {
            phcrn_Long,
            phcrn_Short,
            phcrn_Adjective
        };

        //     /** Create new command. */
        Command(Type cmd, Id_t id, String_t arg);

        ~Command();

        //     /** Get command code. */
        Type getCommand() const;
        // /** Get associated Id number. */
        Id_t getId() const;
        //     /** Get command parameter. */
        const String_t& getArg() const;
        // /** Set command parameter. */
        void setArg(const String_t& s);

        Id_t getAffectedShip() const;
        Id_t getAffectedPlanet() const;
        Id_t getAffectedMinefield() const;

        String_t getCommandText() const;

        static Command* parseCommand(String_t text, bool fromfile, bool acceptProto = false);
        static bool isMessageIntroducer(String_t text);

     private:
        Type      cmd;              ///< Command code.
        Id_t      id;               ///< Associated Id.
        String_t  arg;              ///< Parameter.
    };


} }

#endif
