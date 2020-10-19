/**
  *  \file u/t_game_map_locationreverter.cpp
  *  \brief Test for game::map::LocationReverter
  */

#include "game/map/locationreverter.hpp"

#include "t_game_map.hpp"

/** Interface test. */
void
TestGameMapLocationReverter::testInterface()
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

