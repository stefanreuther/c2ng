/**
  *  \file u/t_game_vcr_classic_eventlistener.cpp
  *  \brief Test for game::vcr::classic::EventListener
  */

#include "game/vcr/classic/eventlistener.hpp"

#include "t_game_vcr_classic.hpp"

/** Interface test. */
void
TestGameVcrClassicEventListener::testInterface()
{
    using game::vcr::classic::Side;
    using game::vcr::classic::Time_t;
    using game::vcr::classic::FighterStatus;
    class Tester : public game::vcr::classic::EventListener {
     public:
        virtual void placeObject(Side /*side*/, const UnitInfo& /*info*/)
            { }
        virtual void updateTime(Time_t /*time*/, int32_t /*distance*/)
            { }
        virtual void startFighter(Side /*side*/, int /*track*/, int /*position*/, int /*distance*/, int /*fighterDiff*/)
            { }
        virtual void landFighter(Side /*side*/, int /*track*/, int /*fighterDiff*/)
            { }
        virtual void killFighter(Side /*side*/, int /*track*/)
            { }
        virtual void fireBeam(Side /*side*/, int /*track*/, int /*target*/, int /*hit*/, int /*damage*/, int /*kill*/, const HitEffect& /*effect*/)
            { }
        virtual void fireTorpedo(Side /*side*/, int /*hit*/, int /*launcher*/, int /*torpedoDiff*/, const HitEffect& /*effect*/)
            { }
        virtual void updateBeam(Side /*side*/, int /*id*/, int /*value*/)
            { }
        virtual void updateLauncher(Side /*side*/, int /*id*/, int /*value*/)
            { }
        virtual void moveObject(Side /*side*/, int /*position*/)
            { }
        virtual void moveFighter(Side /*side*/, int /*track*/, int /*position*/, int /*distance*/, FighterStatus /*status*/)
            { }
        virtual void killObject(Side /*side*/)
            { }
        virtual void updateObject(Side /*side*/, int /*damage*/, int /*crew*/, int /*shield*/)
            { }
        virtual void updateAmmo(Side /*side*/, int /*numTorpedoes*/, int /*numFighters*/)
            { }
        virtual void updateFighter(Side /*side*/, int /*track*/, int /*position*/, int /*distance*/, FighterStatus /*status*/)
            { }
        virtual void setResult(game::vcr::classic::BattleResult_t /*result*/)
            { }
    };
    Tester t;
}
