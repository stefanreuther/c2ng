/**
  *  \file u/t_game_proxy_vcrdatabaseadaptor.cpp
  *  \brief Test for game::proxy::VcrDatabaseAdaptor
  */

#include "game/proxy/vcrdatabaseadaptor.hpp"

#include "t_game_proxy.hpp"

/** Interface test. */
void
TestGameProxyVcrDatabaseAdaptor::testInterface()
{
    class Tester : public game::proxy::VcrDatabaseAdaptor {
     public:
        virtual const game::Root& root() const
            { throw 0; }
        virtual const game::spec::ShipList& shipList() const
            { throw 0; }
        virtual const game::TeamSettings* getTeamSettings() const
            { return 0; }
        virtual game::vcr::Database& battles()
            { throw 0; }
        virtual afl::string::Translator& translator()
            { throw 0; }
        virtual afl::sys::LogListener& log()
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

