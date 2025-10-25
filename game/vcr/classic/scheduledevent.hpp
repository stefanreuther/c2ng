/**
  *  \file game/vcr/classic/scheduledevent.hpp
  *  \brief Class game::vcr::classic::ScheduledEvent
  */
#ifndef C2NG_GAME_VCR_CLASSIC_SCHEDULEDEVENT_HPP
#define C2NG_GAME_VCR_CLASSIC_SCHEDULEDEVENT_HPP

#include "afl/base/types.hpp"
#include "game/vcr/classic/types.hpp"

namespace game { namespace vcr { namespace classic {

    /** Encoded event.
        Contains a type and matching parameters. */
    struct ScheduledEvent {
        enum Type {
            UpdateTime,             // -, time
            UpdateDistance,         // -, distance
            MoveObject,             // side, position
            StartFighter,           // side, track, position, distance
            RemoveFighter,          // side, track
            UpdateNumFighters,      // side, delta
            MoveFighter,            // side, track, position, distance, status
            UpdateFighter,          // side, track, position, distance, status
            ExplodeFighter,         // side, track, animId
            FireBeamShipFighter,    // side, track, beamSlot, animId
            FireBeamShipShip,       // side, beamSlot, animId
            FireBeamFighterShip,    // side, track, animId
            FireBeamFighterFighter, // side, track, targetTrack, animId
            BlockBeam,              // side, beamSlot
            UnblockBeam,            // side, beamSlot
            UpdateBeam,             // side, beamSlot, value
            FireTorpedo,            // side, launcher, hit, animId, time
            UpdateNumTorpedoes,     // side, delta
            BlockLauncher,          // side, launcherSlot
            UnblockLauncher,        // side, launcherSlot
            UpdateLauncher,         // side, launcherSlot, value
            UpdateObject,           // side, damage, crew, shield
            UpdateAmmo,             // side, torps, fighters
            HitObject,              // side, damageDone, crewKilled, shieldLost, animId
            SetResult,              // -, result
            WaitTick,
            WaitAnimation           // -, animId
        };

        Type type;
        Side side;
        int32_t a, b, c, d, e;

        ScheduledEvent(Type t, Side side, int32_t a = 0, int32_t b = 0, int32_t c = 0, int32_t d = 0, int32_t e = 0)
            : type(t), side(side), a(a), b(b), c(c), d(d), e(e)
            { }

        /** Get name for a type.
            @param t Type
            @return Name; never null */
        static const char* toString(Type t);
    };

} } }

#endif
