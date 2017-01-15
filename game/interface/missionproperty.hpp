/**
  *  \file game/interface/missionproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_MISSIONPROPERTY_HPP
#define C2NG_GAME_INTERFACE_MISSIONPROPERTY_HPP

#include "game/spec/mission.hpp"
#include "afl/data/value.hpp"

namespace game { namespace interface {

    enum MissionProperty {
        impName,                // Name:Str
        impNumber,              // Number:Int
        impRaces,               // Races$:Int
        impFlags,               // Flags:Str
        impShortName,           // Name.Short:Str
        impInterceptType,       // Intercept.Type:Str
        impInterceptFlags,      // Intercept.Flags:Str
        impInterceptName,       // Intercept.Name:Str
        impTowType,             // Tow.Type:Str
        impTowFlags,            // Tow.Flags:Str
        impTowName,             // Tow.Name:Str
        impConditionExpression, // Condition:Str
        impWarningExpression,   // Warning:Str
        impLabelExpression,     // Label:Str
        impSetCommand,          // Command:Str
        impHotkey               // Key:Str
    };

    afl::data::Value* getMissionProperty(const game::spec::Mission& mission, MissionProperty imp);

} }

#endif
