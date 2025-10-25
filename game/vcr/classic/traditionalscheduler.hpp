/**
  *  \file game/vcr/classic/traditionalscheduler.hpp
  *  \brief Class game::vcr::classic::TraditionalScheduler
  */
#ifndef C2NG_GAME_VCR_CLASSIC_TRADITIONALSCHEDULER_HPP
#define C2NG_GAME_VCR_CLASSIC_TRADITIONALSCHEDULER_HPP

#include "game/vcr/classic/eventlistener.hpp"

namespace game { namespace vcr { namespace classic {

    class ScheduledEventConsumer;

    /** Traditional event scheduler.

        Converts the incoming calls into ScheduledEventConsumer callbacks.

        Implements similar visualisation as vcr.exe/pvcr.exe:
        all event happen strictly in sequence, in the same order they actually happen in the algorithm. */
    class TraditionalScheduler : public EventListener {
     public:
        /** Constructor.
            @param parent Event consumer */
        explicit TraditionalScheduler(ScheduledEventConsumer& parent);

        // EventListener:
        virtual void placeObject(Side side, const UnitInfo& info);
        virtual void updateTime(Time_t time, int32_t distance);
        virtual void startFighter(Side side, int track, int position, int distance, int fighterDiff);
        virtual void landFighter(Side side, int track, int fighterDiff);
        virtual void killFighter(Side side, int track);
        virtual void fireBeam(Side side, int track, int target, int hit, int damage, int kill, const HitEffect& effect);
        virtual void fireTorpedo(Side side, int hit, int launcher, int torpedoDiff, const HitEffect& effect);
        virtual void updateBeam(Side side, int id, int value);
        virtual void updateLauncher(Side side, int id, int value);
        virtual void moveObject(Side side, int position);
        virtual void moveFighter(Side side, int track, int position, int distance, FighterStatus status);
        virtual void killObject(Side side);
        virtual void updateObject(Side side, int damage, int crew, int shield);
        virtual void updateAmmo(Side side, int numTorpedoes, int numFighters);
        virtual void updateFighter(Side side, int track, int position, int distance, FighterStatus status);
        virtual void setResult(BattleResult_t result);
        virtual void removeAnimations();

     private:
        void renderHit(Side side, const HitEffect& effect);
        ScheduledEventConsumer& m_consumer;
    };

} } }

#endif
