/**
  *  \file client/vcr/classic/traditionalscheduler.hpp
  *  \brief Class client::vcr::classic::TraditionalScheduler
  */
#ifndef C2NG_CLIENT_VCR_CLASSIC_TRADITIONALSCHEDULER_HPP
#define C2NG_CLIENT_VCR_CLASSIC_TRADITIONALSCHEDULER_HPP

#include "client/vcr/classic/scheduler.hpp"

namespace client { namespace vcr { namespace classic {

    class EventConsumer;

    /** Traditional event scheduler.
        Implements similar visualisation as vcr.exe/pvcr.exe:
        all event happen strictly in sequence, in the same order they actually happen in the algorithm. */
    class TraditionalScheduler : public Scheduler {
     public:
        TraditionalScheduler(EventConsumer& parent);

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
        void renderHit(Side_t side, const HitEffect& effect);
        EventConsumer& m_consumer;
    };

} } }

#endif
