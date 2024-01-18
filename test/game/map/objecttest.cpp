/**
  *  \file test/game/map/objecttest.cpp
  *  \brief Test for game::map::Object
  */

#include "game/map/object.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST("game.map.Object", a)
{
    using game::map::Object;

    class Tester : public Object {
     public:
        Tester()
            : Object(42)
            { }
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, const game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
        virtual afl::base::Optional<int> getOwner() const
            { return 0; }
        virtual afl::base::Optional<game::map::Point> getPosition() const
            { return afl::base::Nothing; }

        using Object::setId;
    };
    Tester t;

    a.check("01. isDirty", !t.isDirty());
    a.check("02. isMarked", !t.isMarked());
    a.check("03. isPlayable", !t.isPlayable(Object::Playable));

    t.setIsMarked(true);
    t.setPlayability(Object::Playable);

    a.check("11. isDirty", t.isDirty());
    a.check("12. isMarked", t.isMarked());
    a.check("13. isPlayable", t.isPlayable(Object::Playable));
    a.check("14. isPlayable", t.isPlayable(Object::ReadOnly));

    a.checkEqual("21. getId", t.getId(), 42);
    t.setId(43);
    a.checkEqual("22. getId", t.getId(), 43);
}
