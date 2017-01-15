/**
  *  \file game/interface/baseproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_BASEPROPERTY_HPP
#define C2NG_GAME_INTERFACE_BASEPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/map/planet.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/base/ptr.hpp"
#include "game/turn.hpp"

namespace game { namespace interface {

    enum BaseProperty {
        ibpBaseDamage,
        ibpBaseDefense,
        ibpBaseDefenseMax,
        ibpBaseFighters,
        ibpBaseFightersMax,
        ibpBeamTech,
        ibpBuildBeam,
        ibpBuildBeamCount,
        ibpBuildEngine,
        ibpBuildFlag,
        ibpBuildHull,
        ibpBuildHullName,
        ibpBuildHullShort,
        ibpBuildQueuePos,
        ibpBuildTorp,
        ibpBuildTorpCount,
        ibpEngineTech,
        ibpHullTech,
        ibpMission,
        ibpMissionName,
        ibpShipyardAction,
        ibpShipyardId,
        ibpShipyardName,
        ibpShipyardStr,
        ibpTorpedoTech,

        ibpEngineStorage,
        ibpHullStorage,
        ibpBeamStorage,
        ibpLauncherStorage,
        ibpAmmoStorage
    };

    /** Get starbase property.
        \param pl        Planet to inquire
        \param ibp       Starbase property
        \param tx        Translator.
        \param config    Host configuration (needed for limits, build orders)
        \param shipList  Ship list (needed for build orders)
        \param iface     Interface to other properties.
        \param turn      Turn (needed for related units, namely: ships being repaired)
        \return property value */
    afl::data::Value* getBaseProperty(const game::map::Planet& pl, BaseProperty ibp,
                                      afl::string::Translator& tx,
                                      const game::config::HostConfiguration& config,
                                      afl::base::Ptr<const game::spec::ShipList> shipList,
                                      InterpreterInterface& iface,
                                      afl::base::Ptr<Turn> turn);
    void              setBaseProperty(game::map::Planet& pl, BaseProperty ibp, afl::data::Value* value);

} }

#endif
