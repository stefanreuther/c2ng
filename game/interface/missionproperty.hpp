/**
  *  \file game/interface/missionproperty.hpp
  *  \brief Enum game::interface::MissionProperty
  */
#ifndef C2NG_GAME_INTERFACE_MISSIONPROPERTY_HPP
#define C2NG_GAME_INTERFACE_MISSIONPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/spec/mission.hpp"

namespace game { namespace interface {

    /** Mission definition property. */
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

    /** Get mission definition property.
        @param mission Mission definition
        @param imp     Property to get
        @return Newly-allocated value */
    afl::data::Value* getMissionProperty(const game::spec::Mission& mission, MissionProperty imp);

} }

#endif
