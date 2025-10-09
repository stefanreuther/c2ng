/**
  *  \file test/game/proxy/simulationadaptortest.cpp
  *  \brief Test for game::proxy::SimulationAdaptor
  */

#include "game/proxy/simulationadaptor.hpp"

#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.proxy.SimulationAdaptor")
{
    class Tester : public game::proxy::SimulationAdaptor {
     public:
        virtual game::sim::Session& simSession()
            { throw std::string("no ref"); }
        virtual afl::base::Ptr<const game::Root> getRoot() const
            { return 0; }
        virtual afl::base::Ptr<const game::spec::ShipList> getShipList() const
            { return 0; }
        virtual const game::TeamSettings* getTeamSettings() const
            { return 0; }
        virtual afl::string::Translator& translator()
            { throw std::string("no ref"); }
        virtual afl::sys::LogListener& log()
            { throw std::string("no ref"); }
        virtual afl::io::FileSystem& fileSystem()
            { throw std::string("no ref"); }
        virtual util::RandomNumberGenerator& rng()
            { throw std::string("no ref"); }
        virtual bool isGameObject(const game::vcr::Object& /*obj*/) const
            { return false; }
        virtual size_t getNumProcessors() const
            { return 0; }
    };
    Tester t;
}

