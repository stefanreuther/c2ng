/**
  *  \file u/t_game_map_mapobject.cpp
  *  \brief Test for game::map::MapObject
  */

#include "game/map/mapobject.hpp"

#include "t_game_map.hpp"

/** Interface test. */
void
TestGameMapMapObject::testIt()
{
    class Tester : public game::map::MapObject {
     public:
        // Object:
        virtual String_t getName(Name /*which*/, afl::string::Translator& /*tx*/, game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
        virtual game::Id_t getId() const
            { return game::Id_t(); }
        virtual bool getOwner(int& /*result*/) const
            { return false; }

        // MapObject:
        virtual bool getPosition(game::map::Point& /*result*/) const
            { return false; }
    };
    Tester t;
}

