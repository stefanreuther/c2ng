/**
  *  \file game/interface/baseproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_BASEPROPERTY_HPP
#define C2NG_GAME_INTERFACE_BASEPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/map/planet.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/base/ptr.hpp"
#include "game/game.hpp"

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

    afl::data::Value* getBaseProperty(const game::map::Planet& pl, BaseProperty ibp,
                                      afl::string::Translator& tx,
                                      const game::config::HostConfiguration& config,
                                      const game::spec::ShipList& shipList,
                                      InterpreterInterface& iface,
                                      afl::base::Ptr<Game> game);
    void              setBaseProperty(game::map::Planet& pl, BaseProperty ibp, afl::data::Value* value);

} }

#endif
