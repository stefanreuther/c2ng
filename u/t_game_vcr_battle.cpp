/**
  *  \file u/t_game_vcr_battle.cpp
  *  \brief Test for game::vcr::Battle
  */

#include "game/vcr/battle.hpp"

#include "t_game_vcr.hpp"

/** Interface test. */
void
TestGameVcrBattle::testIt()
{
    class Tester : public game::vcr::Battle {
     public:
        virtual size_t getNumObjects() const
            { return 0; }
        virtual game::vcr::Object* getObject(size_t /*slot*/, bool /*after*/)
            { return 0; }
        virtual int getOutcome(const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/, size_t /*slot*/)
            { return 0; }
        virtual Playability getPlayability(const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/)
            { return IsDamaged; }
        virtual void prepareResult(const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/, int /*resultLevel*/)
            { }
        virtual String_t getAlgorithmName(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual bool isESBActive(const game::config::HostConfiguration& /*config*/) const
            { return false; }
    };
    Tester t;
}
