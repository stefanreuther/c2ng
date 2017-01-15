/**
  *  \file game/interface/missionproperty.cpp
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
        return makeStringValue(mission.getName());
     case impNumber:              // Number:Int
        return makeIntegerValue(mission.getNumber());
     case impRaces:               // Races$:Int
        return makeIntegerValue(mission.getRaceMask().toInteger());
     case impFlags:               // Flags:Str
        return makeStringValue(convertFlags(mission.getFlags()));
     case impShortName:           // Name.Short:Str
        return makeStringValue(mission.getShortName());
     case impInterceptType:       // Intercept.Type:Str
        return convertType(mission.getParameterType(InterceptParameter));
     case impInterceptFlags:      // Intercept.Flags:Str
        return makeStringValue(convertFlags(mission.getParameterFlags(InterceptParameter)));
     case impInterceptName:       // Intercept.Name:Str
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
        return makeStringValue(mission.getConditionExpression());
     case impWarningExpression:   // Warning:Str
        return makeStringValue(mission.getWarningExpression());
     case impLabelExpression:     // Label:Str
        return makeStringValue(mission.getLabelExpression());
     case impSetCommand:          // Command:Str
        return makeStringValue(mission.getSetCommand());
     case impHotkey:              // Key:Str
        if (char hk = mission.getHotkey()) {
            return makeStringValue(String_t(1, hk));
        } else {
            return 0;
        }
    }
    return 0;
}

