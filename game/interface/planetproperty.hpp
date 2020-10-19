/**
  *  \file game/interface/planetproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_PLANETPROPERTY_HPP
#define C2NG_GAME_INTERFACE_PLANETPROPERTY_HPP

#include "afl/base/ptr.hpp"
#include "afl/data/value.hpp"
#include "game/game.hpp"
#include "game/interpreterinterface.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"

namespace game { namespace interface {

    enum PlanetProperty {
        ippBaseBuildFlag,
        ippBaseDefenseWanted,
        ippBaseFlag,
        ippBaseStr,
        ippCashTime,
        ippColonistChange,
        ippColonistChangeStr,
        ippColonistHappy,
        ippColonistHappyStr,
        ippColonistSupported,
        ippColonistTax,
        ippColonistTaxIncome,
        ippColonistTime,
        ippColonists,
        ippDefense,
        ippDefenseMax,
        ippDefenseWanted,
        ippDensityD,
        ippDensityM,
        ippDensityN,
        ippDensityT,
        ippFCode,
        ippFactories,
        ippFactoriesMax,
        ippFactoriesWanted,
        ippGroundD,
        ippGroundM,
        ippGroundN,
        ippGroundT,
        ippId,
        ippIndustry,
        ippIndustryCode,
        ippLevel,
        ippLocX,
        ippLocY,
        ippMarked,
        ippMinedD,
        ippMinedM,
        ippMinedN,
        ippMinedStr,
        ippMinedT,
        ippMineralTime,
        ippMines,
        ippMinesMax,
        ippMinesWanted,
        ippMoney,
        ippName,
        ippNativeChange,
        ippNativeChangeStr,
        ippNativeGov,
        ippNativeGovCode,
        ippNativeHappy,
        ippNativeHappyStr,
        ippNativeRace,
        ippNativeRaceCode,
        ippNativeTax,
        ippNativeTaxBase,
        ippNativeTaxIncome,
        ippNativeTaxMax,
        ippNativeTime,
        ippNatives,
        ippOrbitingEnemies,
        ippOrbitingOwn,
        ippOrbitingShips,
        ippPlayed,
        ippScore,
        ippSupplies,
        ippTask,
        ippTaskBase,
        ippTemp,
        ippTempStr,
        ippTypeChar,
        ippTypeStr
    };

    afl::data::Value* getPlanetProperty(const game::map::Planet& pl, PlanetProperty ipp,
                                        afl::string::Translator& tx,
                                        const game::HostVersion& host,
                                        const game::config::HostConfiguration& config,
                                        InterpreterInterface& iface,
                                        afl::base::Ref<Game> game);
    void setPlanetProperty(game::map::Planet& pl, PlanetProperty ipp, afl::data::Value* value, Root& root);

} }

#endif
