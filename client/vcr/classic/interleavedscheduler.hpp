/**
  *  \file client/vcr/classic/interleavedscheduler.hpp
  */
#ifndef C2NG_CLIENT_VCR_CLASSIC_INTERLEAVEDSCHEDULER_HPP
#define C2NG_CLIENT_VCR_CLASSIC_INTERLEAVEDSCHEDULER_HPP

#include "client/vcr/classic/scheduler.hpp"
#include "client/vcr/classic/event.hpp"

namespace client { namespace vcr { namespace classic {

    class EventConsumer;

    /** Interleaved event scheduler.
        This attempt to shuffle the events around a bit, so that playback is more fluent.
        Most importantly, torpedoes are fired earlier. */
    class InterleavedScheduler : public Scheduler {
     public:
        InterleavedScheduler(EventConsumer& parent);

        // EventListener:
        virtual void placeObject(Side_t side, const UnitInfo& info);
        virtual void updateTime(Time_t time, int32_t distance);
        virtual void startFighter(Side_t side, int track, int position, int distance, int fighterDiff);
        virtual void landFighter(Side_t side, int track, int fighterDiff);
        virtual void killFighter(Side_t side, int track);
        virtual void fireBeam(Side_t side, int track, int target, int hit, int damage, int kill, const HitEffect& effect);
        virtual void fireTorpedo(Side_t side, int hit, int launcher, int torpedoDiff, const HitEffect& effect);
        virtual void updateBeam(Side_t side, int id, int value);
        virtual void updateLauncher(Side_t side, int id, int value);
        virtual void moveObject(Side_t side, int position);
        virtual void moveFighter(Side_t side, int track, int position, int distance, FighterStatus_t status);
        virtual void killObject(Side_t side);
        virtual void updateObject(Side_t side, int damage, int crew, int shield);
        virtual void updateAmmo(Side_t side, int numTorpedoes, int numFighters);
        virtual void updateFighter(Side_t side, int track, int position, int distance, FighterStatus_t status);
        virtual void setResult(game::vcr::classic::BattleResult_t result);

        // Scheduler:
        virtual void removeAnimations();
     private:
        EventConsumer& m_consumer;

        enum { NUM_FRAMES = 10 };
        struct Frame {
            std::vector<Event> pre;
            std::vector<Event> post;
        };
        Frame m_queue[NUM_FRAMES];     // indexed by age
        int m_animationCounter;
        bool m_finished;

        void renderHit(Side_t side, const HitEffect& effect);
        void shift();

    };

} } }

#endif
