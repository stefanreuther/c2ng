/**
  *  \file game/interface/planetproperty.hpp
  *  \brief Enum game::interface::PlanetProperty
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
#include "game/turn.hpp"

namespace game { namespace interface {

    /** Planet property identifier. */
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
        ippEncodedMessage,
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

    /** Get planet property.
        @param pl      Planet
        @param ipp     Property to retrieve
        @param session Session (for translator, ReferenceContext, interface)
        @param root    Root (for host configuration, host version, charset)
        @param game    Game (for score definitions)
        @param turn    Turn (for messages, location info)
        @return newly-allocated value */
    afl::data::Value* getPlanetProperty(const game::map::Planet& pl, PlanetProperty ipp,
                                        Session& session,
                                        const afl::base::Ref<Root>& root,
                                        const afl::base::Ref<Game>& game,
                                        const afl::base::Ref<Turn>& turn);

    /** Set planet property.
        @param pl      Planet
        @param ipp     Property to set
        @param value   Value to set
        @param root    Root (for StringVerifier)
        @throw interpreter::Error if value cannot be assigned */
    void setPlanetProperty(game::map::Planet& pl, PlanetProperty ipp, const afl::data::Value* value, Root& root);

} }

#endif
