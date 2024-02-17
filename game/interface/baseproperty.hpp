/**
  *  \file game/interface/baseproperty.hpp
  *  \brief Enum game::interface::BaseProperty
  */
#ifndef C2NG_GAME_INTERFACE_BASEPROPERTY_HPP
#define C2NG_GAME_INTERFACE_BASEPROPERTY_HPP

#include "afl/base/ptr.hpp"
#include "afl/base/ref.hpp"
#include "afl/data/value.hpp"
#include "game/map/planet.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"

namespace game { namespace interface {

    /** Starbase property identifier. */
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
        \param pl        Planet to inquire. This Planet object must be kept alive as least as long as the given Turn.
        \param ibp       Starbase property
        \param tx        Translator.
        \param root      Root (for host configuration, needed for limits, build orders)
        \param shipList  Ship list (needed for build orders)
        \param turn      Turn (needed for related units, namely: ships being repaired)
        \return newly-allocated property value */
    afl::data::Value* getBaseProperty(const game::map::Planet& pl, BaseProperty ibp,
                                      afl::string::Translator& tx,
                                      const afl::base::Ref<const Root>& root,
                                      const afl::base::Ptr<const game::spec::ShipList>& shipList,
                                      const afl::base::Ref<const Turn>& turn);

    /** Set starbase property.
        \param pl    Planet
        \param ibp   Starbase property
        \param value Value, owned by caller */
    void setBaseProperty(game::map::Planet& pl, BaseProperty ibp, const afl::data::Value* value);

} }

#endif
