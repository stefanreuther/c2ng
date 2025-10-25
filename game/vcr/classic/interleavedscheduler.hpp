/**
  *  \file game/vcr/classic/interleavedscheduler.hpp
  *  \brief Class game::vcr::classic::InterleavedScheduler
  */
#ifndef C2NG_GAME_VCR_CLASSIC_INTERLEAVEDSCHEDULER_HPP
#define C2NG_GAME_VCR_CLASSIC_INTERLEAVEDSCHEDULER_HPP

#include <vector>
#include "game/vcr/classic/eventlistener.hpp"
#include "game/vcr/classic/scheduledevent.hpp"

namespace game { namespace vcr { namespace classic {

    class ScheduledEventConsumer;

    /** Interleaved event scheduler.
        This attempt to shuffle the events around a bit, so that playback is more fluent.
        Most importantly, torpedoes are fired earlier. */
    class InterleavedScheduler : public EventListener {
     public:
        /** Constructor.
            @param parent Event consumer */
        explicit InterleavedScheduler(ScheduledEventConsumer& parent);

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
        ScheduledEventConsumer& m_consumer;

        enum { NUM_FRAMES = 10 };
        struct Frame {
            std::vector<ScheduledEvent> pre;
            std::vector<ScheduledEvent> post;
        };

        /** Queue of frames, newest (future) at front, current at end.
            Events are added according to desired age, and shifted out of the queue. */
        Frame m_queue[NUM_FRAMES];

        /** Identifier counter for animations. */
        int m_animationCounter;

        /** true if fight ends. */
        bool m_finished;

        void renderHit(Side side, const HitEffect& effect);
        void shift();
    };

} } }

#endif
