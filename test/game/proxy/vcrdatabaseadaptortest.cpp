/**
  *  \file test/game/proxy/vcrdatabaseadaptortest.cpp
  *  \brief Test for game::proxy::VcrDatabaseAdaptor
  */

#include "game/proxy/vcrdatabaseadaptor.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.proxy.VcrDatabaseAdaptor")
{
    class Tester : public game::proxy::VcrDatabaseAdaptor {
     public:
        virtual afl::base::Ref<const game::Root> getRoot() const
            { throw 0; }
        virtual afl::base::Ref<const game::spec::ShipList> getShipList() const
            { throw 0; }
        virtual const game::TeamSettings* getTeamSettings() const
            { return 0; }
        virtual afl::base::Ref<game::vcr::Database> getBattles()
            { throw 0; }
        virtual afl::string::Translator& translator()
            { throw 0; }
        virtual afl::sys::LogListener& log()
            { throw 0; }
        virtual afl::io::FileSystem& fileSystem()
            { throw 0; }
        virtual size_t getCurrentBattle() const
            { return 0; }
        virtual void setCurrentBattle(size_t /*n*/)
            { }
        virtual game::sim::Setup* getSimulationSetup() const
            { return 0; }
        virtual bool isGameObject(const game::vcr::Object&) const
            { return false; }
    };
    Tester t;
}
