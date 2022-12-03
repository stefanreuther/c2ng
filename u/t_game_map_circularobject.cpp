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
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
        virtual game::Id_t getId() const
            { return game::Id_t(); }
        virtual afl::base::Optional<int> getOwner() const
            { return 0; }
        virtual afl::base::Optional<game::map::Point> getPosition() const
            { return afl::base::Nothing; }

        // CircularObject:
        virtual afl::base::Optional<int> getRadius() const
            { return afl::base::Nothing; }
        virtual afl::base::Optional<int32_t> getRadiusSquared() const
            { return afl::base::Nothing; }
    };
    Tester t;
}

