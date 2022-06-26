/**
  *  \file game/interface/planetproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_PLANETPROPERTY_HPP
#define C2NG_GAME_INTERFACE_PLANETPROPERTY_HPP

#include "afl/base/ptr.hpp"
#include "afl/data/value.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    enum PlanetProperty {
        ippBaseBuildFlag,
        ippBaseDefenseSpeed,
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
        ippDefenseSpeed,
        ippDefenseWanted,
        ippDensityD,
        ippDensityM,
        ippDensityN,
        ippDensityT,
        ippFCode,
        ippFactories,
        ippFactoriesMax,
        ippFactoriesSpeed,
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
        ippMessages,
        ippMinedD,
        ippMinedM,
        ippMinedN,
        ippMinedStr,
        ippMinedT,
        ippMineralTime,
        ippMines,
        ippMinesMax,
        ippMinesSpeed,
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
        ippReference,
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
                                        Session& session,
                                        afl::base::Ref<Root> root,
                                        afl::base::Ref<Game> game);
    void setPlanetProperty(game::map::Planet& pl, PlanetProperty ipp, const afl::data::Value* value, Root& root);

} }

#endif
