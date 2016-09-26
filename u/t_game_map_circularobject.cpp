/**
  *  \file u/t_game_map_circularobject.cpp
  *  \brief Test for game::map::CircularObject
  */

#include "game/map/circularobject.hpp"

#include "t_game_map.hpp"

/** Interface test. */
void
TestGameMapCircularObject::testIt()
{
    class Tester : public game::map::CircularObject {
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

        // CircularObject:
        virtual bool getRadius(int& /*result*/) const
            { return false; }
        virtual bool getRadiusSquared(int32_t& /*result*/) const
            { return false; }
    };
    Tester t;
}

