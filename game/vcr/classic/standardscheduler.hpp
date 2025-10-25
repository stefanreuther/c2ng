/**
  *  \file game/vcr/classic/standardscheduler.hpp
  *  \brief Class game::vcr::classic::StandardScheduler
  */
#ifndef C2NG_GAME_VCR_CLASSIC_STANDARDSCHEDULER_HPP
#define C2NG_GAME_VCR_CLASSIC_STANDARDSCHEDULER_HPP

#include <vector>
#include "game/vcr/classic/eventlistener.hpp"
#include "game/vcr/classic/scheduledevent.hpp"

namespace game { namespace vcr { namespace classic {

    class ScheduledEventConsumer;

    /** Standard event scheduler.
        Implements similar visualisation as PCC2:
        - all weapons fire simultaneously
        - all explosions occur simultaneously
        - everything that happens within a tick, stays in that tick */
    class StandardScheduler : public EventListener {
     public:
        /** Constructor.
            @param parent Event consumer */
        explicit StandardScheduler(ScheduledEventConsumer& parent);

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
        std::vector<ScheduledEvent> m_pre;
        std::vector<ScheduledEvent> m_post;
    };

} } }

#endif
