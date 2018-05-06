/**
  *  \file game/vcr/classic/mirroringeventlistener.hpp
  *  \brief Class game::vcr::classic::MirroringEventListener
  */
#ifndef C2NG_GAME_VCR_CLASSIC_MIRRORINGEVENTLISTENER_HPP
#define C2NG_GAME_VCR_CLASSIC_MIRRORINGEVENTLISTENER_HPP

#include "game/vcr/classic/eventlistener.hpp"

namespace game { namespace vcr { namespace classic {

    /** EventListener that swaps sides.
        This is an adaptor to EventListener that reports all events with sides swapped.

        Note that the logical order of callbacks is not adapted, that is,
        if the original battle always reports left weapons firing before right,
        the flipped battle will have right weapons firing before left. */
    class MirroringEventListener : public EventListener {
     public:
        /** Constructor.
            \param listener Target */
        explicit MirroringEventListener(EventListener& listener);

        /** Destructor. */
        ~MirroringEventListener();

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

     private:
        EventListener& m_listener;

        static int flipCoordinate(int x);
    };

} } }

#endif
