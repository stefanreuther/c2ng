/**
  *  \file game/vcr/classic/types.hpp
  */
#ifndef C2NG_GAME_VCR_CLASSIC_TYPES_HPP
#define C2NG_GAME_VCR_CLASSIC_TYPES_HPP

#include "afl/bits/smallset.hpp"
#include "afl/base/types.hpp"

namespace game { namespace vcr { namespace classic {

    enum Type {
        // xref game::vcr::classic::Battle::getAlgorithmName
        Unknown,            ///< Unknown.
        Host,               ///< Host.
        UnknownPHost,       ///< Unknown brand of PHost.
        PHost2,             ///< PHost 2.
        PHost3,             ///< PHost 3 or PHost 4 without extensions.
        PHost4,             ///< PHost 4 with extensions.
        NuHost              ///< NuHost.
    };

    /** Status values (battle outcome). */
    enum BattleResult {
        LeftDestroyed,      ///< Left object has been destroyed
        RightDestroyed,     ///< Right object has been destroyed
        LeftCaptured,       ///< Left object has been captured
        RightCaptured,      ///< Right object has been captured
        Timeout,            ///< Battle timed out
        Stalemate,          ///< Stalemate (neither has ammo).
        Invalid             ///< Battle cannot be played.
    };
    typedef afl::bits::SmallSet<BattleResult> BattleResult_t;

    /** Fighter statuses.
        FIXME: Those are hardcoded at many places, don't change! */
    enum FighterStatus {
        FighterIdle,
        FighterAttacks,
        FighterReturns
    };


    enum Side {
        LeftSide,
        RightSide
    };

    typedef int32_t Time_t;

} } }

#endif
