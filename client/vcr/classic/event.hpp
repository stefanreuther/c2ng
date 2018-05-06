/**
  *  \file client/vcr/classic/event.hpp
  */
#ifndef C2NG_CLIENT_VCR_CLASSIC_EVENT_HPP
#define C2NG_CLIENT_VCR_CLASSIC_EVENT_HPP

#include "game/vcr/classic/types.hpp"
#include "afl/base/types.hpp"

namespace client { namespace vcr { namespace classic {

    struct Event {
        typedef game::vcr::classic::Side Side_t;

        enum Type {
            UpdateTime,             // time
            UpdateDistance,         // distance
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
            WaitAnimation           // animId
        };

        Type type;
        Side_t side;
        int32_t a, b, c, d, e;

        Event(Type t, Side_t side, int32_t a = 0, int32_t b = 0, int32_t c = 0, int32_t d = 0, int32_t e = 0)
            : type(t), side(side), a(a), b(b), c(c), d(d), e(e)
            { }

        static const char* toString(Type t);
    };

} } }

#endif
