/**
  *  \file game/interface/vcrsideproperty.hpp
  *  \brief Enum game::interface::VcrSideProperty
  */
#ifndef C2NG_GAME_INTERFACE_VCRSIDEPROPERTY_HPP
#define C2NG_GAME_INTERFACE_VCRSIDEPROPERTY_HPP

#include "afl/string/translator.hpp"
#include "game/playerlist.hpp"
#include "game/spec/shiplist.hpp"
#include "game/vcr/battle.hpp"

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
        ivsCrewDefenseRate,
        ivsRole
    };

    /** Get property of a VCR side.
        Note that the outcome is a property of a VCR side.
        Therefore, this function may need to invoke the VCR player.
        For this reason, the Battle cannot be const, because it will be updated with the computed result.

        There is no protection against this calling the (costly) VCR.
        Therefore, the server module must make sure to never call this for ivsStatus / ivsStatusRaw.

        \param battle   The battle
        \param side     Side to query (0=left, 1=right)
        \param ivs      Property to get
        \param tx       Translator (for names)
        \param shipList Ship list (for component names, result computation)
        \param config   Host configuration (for result computation)
        \param players  Player list (for names)
        \return value */
    afl::data::Value* getVcrSideProperty(game::vcr::Battle& battle, size_t side, VcrSideProperty ivs,
                                         afl::string::Translator& tx,
                                         const game::spec::ShipList& shipList,
                                         const game::config::HostConfiguration& config,
                                         const PlayerList& players);

} }

#endif
