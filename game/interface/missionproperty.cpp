/**
  *  \file game/interface/missionproperty.cpp
  *  \brief Enum game::interface::MissionProperty
  */

#include "game/interface/missionproperty.hpp"
#include "interpreter/values.hpp"

using interpreter::makeStringValue;
using interpreter::makeIntegerValue;
using game::spec::Mission;

namespace {
    String_t convertFlags(Mission::FlagSet_t flags)
    {
        String_t result;
        if (flags.contains(Mission::WaypointMission)) {
            result += "i";
        }
        if (flags.contains(Mission::RegisteredMission)) {
            result += "r";
        }
        return result;
    }

    String_t convertFlags(Mission::ParameterFlagSet_t flags)
    {
        String_t result;
        if (flags.contains(Mission::NotThisParameter)) {
            result += "!";
        }
        if (flags.contains(Mission::OwnParameter)) {
            result += "o";
        }
        return result;
    }

    afl::data::Value* convertType(Mission::ParameterType type)
    {
        switch (type) {
         case Mission::NoParameter:
            return 0;
         case Mission::IntegerParameter:
            return makeStringValue("n");
         case Mission::PlanetParameter:
            return makeStringValue("p");
         case Mission::ShipParameter:
            return makeStringValue("s");
         case Mission::HereParameter:
            return makeStringValue("h");
         case Mission::BaseParameter:
            return makeStringValue("b");
         case Mission::PlayerParameter:
            return makeStringValue("y");
        }
        return 0;
    }
}

afl::data::Value*
game::interface::getMissionProperty(const game::spec::Mission& mission, MissionProperty imp)
{
    switch (imp) {
     case impName:                // Name:Str
        /* @q Name:Str (Mission Property)
           Name of the mission.
           @since PCC2 2.40.1 */
        return makeStringValue(mission.getName());
     case impNumber:              // Number:Int
        /* @q Number:Int (Mission Property)
           Mission number.
           @since PCC2 2.40.1 */
        return makeIntegerValue(mission.getNumber());
     case impRaces:               // Races$:Int
        /* @q Races$:Int (Mission Property)
           Race mask, binary.
           Combination of all races that can use this mission.
           For example, if this mission is usable by Borg (=a player who has PlayerSpecialMission=6) only,
           this field has value 2^6 = 64.
           @since PCC2 2.40.1 */
        return makeIntegerValue(mission.getRaceMask().toInteger());
     case impFlags:               // Flags:Str
        /* @q Flags:Str (Mission Property)
           Mission flags.
           Lists properties of this mission:
           - "i": this mission affects the ship's waypoint (Intercept)
           - "r": this mission is available to registered players only

           These are parts of the flags given for the mission in mission.cc.
           @since PCC2 2.40.1 */
        return makeStringValue(convertFlags(mission.getFlags()));
     case impShortName:           // Name.Short:Str
        /* @q Name.Short:Str (Mission Property)
           Short name of the mission.

           This is the value of the "S=" ("Shortname=") assignment in mission.cc.
           @since PCC2 2.40.1 */
        return makeStringValue(mission.getShortName());
     case impInterceptType:       // Intercept.Type:Str
        /* @q Intercept.Type:Str (Mission Property), Tow.Type:Str (Mission Property)
           Type of mission parameter.
           - EMPTY: this mission does not take this parameter
           - "n": number (e.g. number of torpedoes to make)
           - "p": planet Id (not used in HOST/PHost missions)
           - "s": ship Id (e.g. Id of ship to intercept)
           - "h": ship here (e.g. Id of ship to transfer stuff to)
           - "b": starbase Id (not used in HOST/PHost missions)
           - "y": player number (e.g. player to lay minefields as)

           These are parts of the flags given for the mission in mission.cc.
           @since PCC2 2.40.1 */
        return convertType(mission.getParameterType(InterceptParameter));
     case impInterceptFlags:      // Intercept.Flags:Str
        /* @q Intercept.Flags:Str (Mission Property), Tow.Flags:Str (Mission Property)
           Additional restrictions for type of mission parameter.
           - "!": value cannot be this ship/player
           - "o": value needs to refer to an own ship/planet/base

           These are parts of the flags given for the mission in mission.cc.
           @since PCC2 2.40.1 */
        return makeStringValue(convertFlags(mission.getParameterFlags(InterceptParameter)));
     case impInterceptName:       // Intercept.Name:Str
        /* @q Intercept.Name:Str (Mission Property), Tow.Name:Str (Mission Property)
           Name of mission paramter, if any.

           This is the value of the "I=", "J=" assignments in mission.cc.
           @since PCC2 2.40.1 */
        if (mission.getParameterType(InterceptParameter) != Mission::NoParameter) {
            return makeStringValue(mission.getParameterName(InterceptParameter));
        } else {
            return 0;
        }
     case impTowType:             // Tow.Type:Str
        return convertType(mission.getParameterType(TowParameter));
     case impTowFlags:            // Tow.Flags:Str
        return makeStringValue(convertFlags(mission.getParameterFlags(TowParameter)));
     case impTowName:             // Tow.Name:Str
        if (mission.getParameterType(TowParameter) != Mission::NoParameter) {
            return makeStringValue(mission.getParameterName(TowParameter));
        } else {
            return 0;
        }
     case impConditionExpression: // Condition:Str
        /* @q Condition:Str (Mission Property)
           Condition expression.
           Mission is offered if this expression yields nonzero.
           Can be blank if there is no condition.

           This is the value of the "C=", ("Condition=") assignment in mission.cc.
           @since PCC2 2.40.1 */
        return makeStringValue(mission.getConditionExpression());
     case impWarningExpression:   // Warning:Str
        /* @q Warning:Str (Mission Property)
           Warning expression.
           Mission warning will be shown if this expression yields zero.
           Can be blank if there is no condition.

           This is the value of the "W=", ("WillWork=") assignment in mission.cc.
           @since PCC2 2.40.1 */
        return makeStringValue(mission.getWarningExpression());
     case impLabelExpression:     // Label:Str
        /* @q Label:Str (Mission Property)
           Label expression.
           This text will be shown if the mission is set; it can format mission parameters nicely.
           If not given, the name is shown instead.

           This is the value of the "T=", ("Text=") assignment in mission.cc.
           @since PCC2 2.40.1 */
        return makeStringValue(mission.getLabelExpression());
     case impSetCommand:          // Command:Str
        /* @q Command:Str (Mission Property)
           Command.
           This command will be executed when the mission is set.

           This is the value of the "O=", ("OnSet=") assignment in mission.cc.
           @since PCC2 2.40.1 */
        return makeStringValue(mission.getSetCommand());
     case impHotkey:              // Key:Str
        /* @q Key:Str (Mission Property)
           Hotkey.
           This hotkey can be used in the mission selection screen.
           @since PCC2 2.40.1 */
        if (char hk = mission.getHotkey()) {
            return makeStringValue(String_t(1, hk));
        } else {
            return 0;
        }
     case impGroup:
        /* @q Group:Str (Mission Property)
           Group name.
           A possibly empty, comma-delimited list of groups this mission belongs to. */
        return makeStringValue(mission.getGroup());
    }
    return 0;
}

