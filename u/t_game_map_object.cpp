/**
  *  \file u/t_game_map_object.cpp
  *  \brief Test for game::map::Object
  */

#include "game/map/object.hpp"

#include "t_game_map.hpp"

/** Interface test. */
void
TestGameMapObject::testIt()
{
    class Tester : public game::map::Object {
     public:
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
        virtual game::Id_t getId() const
            { return game::Id_t(); }
        virtual bool getOwner(int& /*result*/) const
            { return false; }
    };
    Tester t;
}
