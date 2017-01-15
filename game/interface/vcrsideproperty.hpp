/**
  *  \file game/interface/vcrsideproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_VCRSIDEPROPERTY_HPP
#define C2NG_GAME_INTERFACE_VCRSIDEPROPERTY_HPP

#include "game/vcr/battle.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/string/translator.hpp"
#include "game/playerlist.hpp"

namespace game { namespace interface {

    /** Property of one side of a VCR. */
    enum VcrSideProperty {
        ivsAuxAmmo,
        ivsAuxCount,
        ivsAuxId,
        ivsAuxName,
        ivsAuxShort,
        ivsFighterBays,
        ivsFighterCount,
        ivsTorpId,
        ivsTorpCount,
        ivsTorpLCount,
        ivsTorpShort,
        ivsTorpName,
        ivsBeamCount,
        ivsBeamId,
        ivsBeamName,
        ivsBeamShort,
        ivsCrew,
        ivsCrewRaw,
        ivsDamage,
        ivsId,
        ivsMass,
        ivsName,
        ivsNameFull,
        ivsOwnerAdj,
        ivsOwnerId,
        ivsOwnerShort,
        ivsShield,
        ivsStatus,
        ivsStatusRaw,
        ivsType,
        ivsTypeShort,
        ivsHullName,
        ivsHullId,
        ivsImage,
        ivsLevel,
        ivsIsPlanet,
        ivsBeamKillRate,
        ivsBeamChargeRate,
        ivsTorpMissRate,
        ivsTorpChargeRate,
        ivsCrewDefenseRate
    };

    afl::data::Value* getVcrSideProperty(game::vcr::Battle& battle, size_t side, VcrSideProperty ivs,
                                         afl::string::Translator& tx,
                                         const game::spec::ShipList& shipList,
                                         const game::config::HostConfiguration& config,
                                         const PlayerList& players);

} }

#endif
