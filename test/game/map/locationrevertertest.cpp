/**
  *  \file test/game/map/locationrevertertest.cpp
  *  \brief Test for game::map::LocationReverter
  */

#include "game/map/locationreverter.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.map.LocationReverter")
{
    class Tester : public game::map::LocationReverter {
     public:
        virtual game::ref::List getAffectedObjects() const
            { return game::ref::List(); }
        virtual Modes_t getAvailableModes() const
            { return Modes_t(); }
        virtual void commit(Modes_t /*modes*/)
            { }
    };
    Tester t;
}
