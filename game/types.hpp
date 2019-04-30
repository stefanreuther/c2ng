/**
  *  \file game/types.hpp
  */
#ifndef C2NG_GAME_TYPES_HPP
#define C2NG_GAME_TYPES_HPP

#include "afl/base/inlineoptional.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"

namespace game {

    typedef int32_t Id_t;

    /// Native races.
    enum NativeRace {
        NoNatives,
        HumanoidNatives,
        BovinoidNatives,
        ReptilianNatives,
        AvianNatives,
        AmorphousNatives,
        InsectoidNatives,
        AmphibianNatives,
        GhipsoldalNatives,
        SiliconoidNatives
    };

    /// Shipyard actions. Value of the shipyard_action field.
    enum ShipyardAction {
        NoShipyardAction,
        FixShipyardAction,
        RecycleShipyardAction
    };

    /// Planetary Structures.
    enum PlanetaryBuilding {
        MineBuilding,
        FactoryBuilding,
        DefenseBuilding,
        BaseDefenseBuilding
    };
    const size_t NUM_PLANETARY_BUILDING_TYPES = 4;

    // /** Industrial Activity Ratings. Order matches that used in UTIL.DAT
    //     record 6 (TUtil6SensorSweep::activity), and is also used in
    //     GPlanet::getIndustryLevel(int). Note that older PHost versions
    //     report a sixth level at 150+ structures in UTILx.DAT, this is
    //     considered a bug and fixed in 4.1a/3.5a. */
    enum IndustryLevel {
        MinimalIndustry,                ///< Minimal activity. PHost: 0-29 structures.
        LightIndustry,                  ///< Light activity. PHost: 30-59 structures. Host: 0-99 structures.
        ModerateIndustry,               ///< Moderate activity. PHost: 60-89 structures.
        SubstantialIndustry,            ///< Substantial activity. PHost: 90-119 structures.
        HeavyIndustry                   ///< Heavy activity. PHost: 120+ structures. Host: 100+ structures.
    };

    // /** Parameter identification for a mission. */
    enum MissionParameter {
        InterceptParameter,      ///< Intercept parameter.
        TowParameter             ///< Tow parameter.
    };

    /** A name. Identifies a name that can be queried with getName(). */
    enum ObjectName {
        PlainName,                 /**< Just the unit's name. Example: "USS Dull" */
        LongName,                  /**< Long name, including the Id number. Example: "Ship #1: USS Dull" */
        DetailedName               /**< Name with details. Example: "Ship #1: USS Dull (Fed Outrider)" or "Ship #1: USS Dull: colonizer" */
    };

    enum TechLevel {
        EngineTech,                                             ///< Engine tech level.
        HullTech,                                               ///< Hull tech level.
        BeamTech,                                               ///< Beam weapon tech level.
        TorpedoTech                                             ///< Torpedo tech level.
    };
    const size_t NUM_TECH_AREAS = 4;

    // FIXME: these are internal representation only so we may not have to differentiate between IntegerProperty_t and NegativeProperty_t.
    typedef afl::base::InlineOptional<int16_t,-1,int> IntegerProperty_t;
    typedef afl::base::InlineOptional<int32_t,-1,int32_t> LongProperty_t;
    typedef afl::base::InlineOptional<int16_t,int16_t(0x8000),int> NegativeProperty_t;
    typedef afl::base::Optional<String_t> StringProperty_t;

}

#endif
