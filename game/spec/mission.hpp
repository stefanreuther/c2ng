/**
  *  \file game/spec/mission.hpp
  */
#ifndef C2NG_GAME_SPEC_MISSION_HPP
#define C2NG_GAME_SPEC_MISSION_HPP

#include "afl/string/string.hpp"
#include "game/playerset.hpp"
#include "afl/bits/smallset.hpp"
#include "game/types.hpp"

namespace game { namespace spec {

    /** Mission description.
        Describes a ship mission and its arguments.
        A mission is identified by its number and race mask. */
    class Mission {
     public:
        enum Flag {
            WaypointMission,      ///< Mission affects waypoint ("Intercept").
            RegisteredMission     ///< Mission is registered-only.
        };
        typedef afl::bits::SmallSet<Flag> FlagSet_t;

        enum ParameterFlag {
            NotThisParameter,   // Not this unit as parameter (ex ma_NotThis)
            OwnParameter        // Own units only (ex ma_Own)
        };
        typedef afl::bits::SmallSet<ParameterFlag> ParameterFlagSet_t;

        enum ParameterType {
            NoParameter,        // No parameter
            IntegerParameter,   // Integer value (ex mat_Integer)
            PlanetParameter,    // Planet Id (ex mat_PID)
            ShipParameter,      // Ship Id (ex mat_SID)
            HereParameter,      // Ship Id of ship here (ex mat_SID_here)
            BaseParameter,      // Base Id (ex mat_BID)
            PlayerParameter     // Player number (ex mat_Player)
        };

        /** Default constructor.
            This constructor is only intended to make Mission usable with containers. */
        Mission();
        Mission(int number, String_t descriptionLine);
        ~Mission();

        // Mission accessors
        int        getNumber() const;
        PlayerSet_t getRaceMask() const;
        void       setRaceMask(PlayerSet_t mask);
        FlagSet_t  getFlags() const;
        bool       hasFlag(Flag flag) const;
        void       setFlags(FlagSet_t flags);
        String_t   getName() const;
        void       setName(String_t name);
        String_t   getShortName() const;
        void       setShortName(String_t short_name);
        char       getHotkey() const;
        void       setHotkey(char c);

        // Parameter accessors
        ParameterType getParameterType(MissionParameter p) const;
        void          setParameterType(MissionParameter p, ParameterType type);
        ParameterFlagSet_t getParameterFlags(MissionParameter p) const;
        void          setParameterFlags(MissionParameter p, ParameterFlagSet_t flags);
        String_t      getParameterName(MissionParameter id) const;
        void          setParameterName(MissionParameter id, String_t name);

        // Script accessors
        String_t   getConditionExpression() const;
        void       setConditionExpression(String_t cond);
        String_t   getWarningExpression() const;
        void       setWarningExpression(String_t warning);
        String_t   getLabelExpression() const;
        void       setLabelExpression(String_t label);
        String_t   getSetCommand() const;
        void       setSetCommand(String_t cmd);

        // Script execution
        // bool       worksOn(GShip& ship) const;
        // string_t   getLabel(GShip& ship) const;
        // bool       isWarning(GShip& ship) const;


        // /** Standard Mission Numbers. */
        // FIXME!
        enum {
        //     // Standard missions
            msn_Explore     = 1,
        //     msn_MineSweep   = 2,
        //     msn_LayMines    = 3,
        //     msn_Kill        = 4,
        //     msn_SensorSweep = 5,
        //     msn_LnD         = 6,    // Colonize, Land&Disassemble, Decommission
            msn_Tow         = 7
        //     msn_Intercept   = 8,
        //     msn_Special     = 9,
        //     msn_Cloak       = 10,
        //     msn_BUFuel      = 11,
        //     msn_BUDur       = 12,
        //     msn_BUTri       = 13,
        //     msn_BUMol       = 14,
        //     msn_BUSup       = 15,

        //     // PHost extended missions. These numbers are relative to ExtMissionsStartAt.
        //     pmsn_BuildTorpsFromCargo = 0,
        //     pmsn_LayMines            = 1,
        //     pmsn_LayWeb              = 2,
        //     pmsn_ScoopTorps          = 3,
        //     pmsn_GatherBuildTorps    = 4,
        //     pmsn_BDmc                = 5,
        //     pmsn_XferTorp            = 6,
        //     pmsn_XferFtr             = 7,
        //     pmsn_XferMc              = 8,
        //     pmsn_StandardSuperSpy    = 9,
        //     pmsn_Cloak               = 10,
        //     pmsn_Special             = 11,
        //     pmsn_GatherBuildFtr      = 12,
        //     pmsn_BUmc                = 13,
        //     pmsn_BUclans             = 14,
        //     pmsn_BeamUpMultiple      = 15,
        //     pmsn_LayMinesIn          = 16,
        //     pmsn_LayWebIn            = 17,
        //     pmsn_Training            = 18
        };

     private:
        int         m_number;         ///< Mission.
        PlayerSet_t m_raceMask;       ///< Race mask.
        ParameterFlagSet_t m_parameterFlags[2];
        ParameterType m_parameterTypes[2];
        FlagSet_t   m_flags;          ///< Other flags.
        String_t    name;             ///< Mission name used in selection lists.
        String_t    short_name;       ///< Short name. Used for scripts.
        String_t    arg_names[2];     ///< Argument names. Indexed by Parameter.
        String_t    exp_condition;    ///< Condition for selectability.
        String_t    exp_warning;      ///< Condition to check whether it will work.
        String_t    exp_label;        ///< Expression for label on ship screen.
        String_t    cmd_onset;        ///< "on set" expression.
        char        hotkey;           ///< Mission hot-key.

        void        parseDescription(const String_t& desc_line);
    };

} }

#endif
