/**
  *  \file test/game/map/circularobjecttest.cpp
  *  \brief Test for game::map::CircularObject
  */

#include "game/map/circularobject.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.map.CircularObject")
{
    class Tester : public game::map::CircularObject {
     public:
        Tester()
            : CircularObject(0)
            { }
        // Object:
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, const game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
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
