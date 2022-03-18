/**
  *  \file game/spec/mission.hpp
  *  \brief Class game::spec::Mission
  */
#ifndef C2NG_GAME_SPEC_MISSION_HPP
#define C2NG_GAME_SPEC_MISSION_HPP

#include "afl/bits/smallset.hpp"
#include "afl/string/string.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/playerset.hpp"
#include "game/types.hpp"

namespace game {
    class RegistrationKey;
    class HostVersion;
}

namespace game { namespace map {
    class Ship;
} }

namespace game { namespace spec {

    /** Mission description.
        Describes a ship mission and its arguments.
        A mission is identified by its number and race mask.
        It has up to two parameters (InterceptParameter, TowParameter). */
    class Mission {
     public:
        /** Mission flag.
            Describes an overall property of the mission. */
        enum Flag {
            WaypointMission,      ///< Mission affects waypoint ("Intercept").
            RegisteredMission     ///< Mission is registered-only.
        };
        typedef afl::bits::SmallSet<Flag> FlagSet_t;

        /** Parameter flag.
            Describes a restriction (sub-type) of the parameter. */
        enum ParameterFlag {
            NotThisParameter,     ///< Not this unit as parameter (ex ma_NotThis)
            OwnParameter          ///< Own units only (ex ma_Own)
        };
        typedef afl::bits::SmallSet<ParameterFlag> ParameterFlagSet_t;

        /** Parameter type.
            Describes the overall type of a parameter. */
        enum ParameterType {
            NoParameter,          ///< No parameter
            IntegerParameter,     ///< Integer value (ex mat_Integer)
            PlanetParameter,      ///< Planet Id (ex mat_PID)
            ShipParameter,        ///< Ship Id (ex mat_SID)
            HereParameter,        ///< Ship Id of ship here (ex mat_SID_here)
            BaseParameter,        ///< Base Id (ex mat_BID)
            PlayerParameter       ///< Player number (ex mat_Player)
        };

        /** Default constructor.
            Construct blank mission.
            This constructor is only intended to make Mission usable with containers. */
        Mission();

        /** Construct mission definition from mission.cc line.
            \param number           Mission number
            \param descriptionLine  Description line (flags, ",", name) */
        Mission(int number, String_t descriptionLine);

        /** Destructor. */
        ~Mission();

        /*
         *  Mission accessors
         */

        /** Get mission number.
            \return number */
        int getNumber() const;

        /** Get races which can do this mission.
            (This is indeed races, not players!).
            \return set of race numbers */
        PlayerSet_t getRaceMask() const;

        /** Set races which can do this mission.
            \param mask set of race numbers */
        void setRaceMask(PlayerSet_t mask);

        /** Get flags.
            \return flags */
        FlagSet_t getFlags() const;

        /** Check for flag.
            \param flag Flag to check
            \return true if flag is set */
        bool hasFlag(Flag flag) const;

        /** Set flags.
            \param flags new flags */
        void setFlags(FlagSet_t flags);

        /** Get mission name.
            This name should be displayed in selection lists.
            \return name */
        String_t getName() const;

        /** Set mission name.
            \param name Name */
        void setName(String_t name);

        /** Get short mission name.
            This name is used when space is tight.
            \return name */
        String_t getShortName() const;

        /** Set short mission name.
            \param shortName Name */
        void setShortName(String_t shortName);

        /** Get hot-key.
            The hot-key is a US-ASCII character for a key to select this mission.
            \return hot-key; can be 0 */
        char getHotkey() const;

        /** Set hot-key.
            \param c Hot-key */
        void setHotkey(char c);

        /*
         *  Parameter accessors)
         */

        /** Get parameter type.
            \param p Which parameter to query
            \return type */
        ParameterType getParameterType(MissionParameter p) const;

        /** Set parameter type.
            \param p Parameter
            \param type Type */
        void setParameterType(MissionParameter p, ParameterType type);

        /** Get parameter flags (sub-type).
            \param p Which parameter to query
            \return flags */
        ParameterFlagSet_t getParameterFlags(MissionParameter p) const;

        /** Set parameter flags (sub-type).
            \param p Which parameter to query
            \param flags Flags */
        void setParameterFlags(MissionParameter p, ParameterFlagSet_t flags);

        /** Get parameter name.
            \param p Which parameter to query
            \return name; never empty */
        String_t getParameterName(MissionParameter p) const;

        /** Set parameter name.
            \param p Which parameter
            \param name Name; can be blank to invoke default */
        void setParameterName(MissionParameter p, String_t name);

        /*
         *  Script accessors
         */

        /** Get condition expression.
            \return expression */
        String_t getConditionExpression() const;

        /** Set condition expression.
            This condition verifies whether the mission is allowed to be set (hard condition).
            \param cond Condition */
        void setConditionExpression(String_t cond);

        /** Get warning expression.
            \return expression */
        String_t getWarningExpression() const;

        /** Set warning expression.
            This condition verifies whether the mission will work (soft condition) or a warning shall be shown. */
        void setWarningExpression(String_t warning);

        /** Get label expression.
            \return expression */
        String_t getLabelExpression() const;

        /** Set label expression.
            This expression produces a string to display when the mission is set on a ship.
            \return expression */
        void setLabelExpression(String_t label);

        /** Get "on-set" command.
            \return command */
        String_t getSetCommand() const;

        /** Set "on-set" command.
            This command is invoked after the mission is set via the user-interface.
            \param cmd Command */
        void setSetCommand(String_t cmd);

        /*
         *  Inquiry
         */

        /** Check whether mission works on a ship.
            Determines whether the user shall be offered this mission.
            Note that this is implemented as a script in c2ng; this is a partial implementation only.

            \param ship Ship
            \param config Host configuration
            \param host   Host version
            \param key    Registration key
            \return true if mission will probably work */
        bool worksOn(const game::map::Ship& ship, const game::config::HostConfiguration& config, const HostVersion& host, const RegistrationKey& key) const;

        /** Standard Mission Numbers. */
        enum {
            // Standard missions
            msn_Explore     = 1,
            msn_MineSweep   = 2,
            msn_LayMines    = 3,
            msn_Kill        = 4,
            // msn_SensorSweep = 5,
            // msn_LnD         = 6,    // Colonize, Land&Disassemble, Decommission
            msn_Tow         = 7,
            msn_Intercept   = 8,
            msn_Special     = 9,
            msn_Cloak       = 10,
            // msn_BUFuel      = 11,
            // msn_BUDur       = 12,
            // msn_BUTri       = 13,
            // msn_BUMol       = 14,
            // msn_BUSup       = 15,

            // PHost extended missions. These numbers are relative to ExtMissionsStartAt.
            pmsn_BuildTorpsFromCargo = 0,
            pmsn_LayMines            = 1,
            pmsn_LayWeb              = 2,
            pmsn_ScoopTorps          = 3,
            // pmsn_GatherBuildTorps    = 4,
            // pmsn_BDmc                = 5,
            // pmsn_XferTorp            = 6,
            // pmsn_XferFtr             = 7,
            // pmsn_XferMc              = 8,
            pmsn_StandardSuperSpy    = 9,
            pmsn_Cloak               = 10,
            pmsn_Special             = 11,
            // pmsn_GatherBuildFtr      = 12,
            // pmsn_BUmc                = 13,
            // pmsn_BUclans             = 14,
            pmsn_BeamUpMultiple      = 15,
            pmsn_LayMinesIn          = 16,
            pmsn_LayWebIn            = 17,
            pmsn_Training            = 18
        };

     private:
        int         m_number;                    ///< Mission.
        PlayerSet_t m_raceMask;                  ///< Race mask.
        ParameterFlagSet_t m_parameterFlags[2];  ///< Parameter flags. Indexed by Parameter.
        ParameterType m_parameterTypes[2];       ///< Parameter types. Indexed by Parameter.
        FlagSet_t   m_flags;                     ///< Other flags.
        String_t    m_name;                      ///< Mission name used in selection lists.
        String_t    m_shortName;                 ///< Short name. Used for scripts.
        String_t    m_parameterNames[2];         ///< Argument names. Indexed by Parameter.
        String_t    m_conditionExpression;       ///< Condition for selectability.
        String_t    m_warningExpression;         ///< Condition to check whether it will work.
        String_t    m_labelExpression;           ///< Expression for label on ship screen.
        String_t    m_setCommand;                ///< "on set" expression.
        char        m_hotkey;                    ///< Mission hot-key.

        void        parseDescription(const String_t& desc_line);
    };

} }

#endif
