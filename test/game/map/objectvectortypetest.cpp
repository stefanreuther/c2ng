/**
  *  \file test/game/map/objectvectortypetest.cpp
  *  \brief Test for game::map::ObjectVectorType
  */

#include "game/map/objectvectortype.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/object.hpp"

namespace {
    class TestObj : public game::map::Object {
     public:
        TestObj(game::Id_t id)
            : Object(id)
            { }
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, const game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
        virtual afl::base::Optional<int> getOwner() const
            { return 0; }
        virtual afl::base::Optional<game::map::Point> getPosition() const
            { return afl::base::Nothing; }
    };
    class Tester : public game::map::ObjectVectorType<TestObj> {
     public:
        Tester(game::map::ObjectVector<TestObj>& vec)
            : game::map::ObjectVectorType<TestObj>(vec)
            { }
        virtual bool isValid(const TestObj& obj) const
            { return obj.getId() % 2 == 0; }
    };
}


/** Simple test. */
AFL_TEST("game.map.ObjectVectorType", a)
{
    game::map::ObjectVector<TestObj> vec;
    vec.create(1);
    vec.create(2);
    vec.create(5);
    vec.create(6);
    vec.create(7);

    Tester t(vec);
    a.checkNull("01", t.getObjectByIndex(0));
    a.checkNull("02", t.getObjectByIndex(1));  // present, but odd
    a.checkNonNull("03", t.getObjectByIndex(2));  // valid
    a.checkNull("04", t.getObjectByIndex(3));
    a.checkNull("05", t.getObjectByIndex(4));
    a.checkNull("06", t.getObjectByIndex(5));  // present, but odd
    a.checkNonNull("07", t.getObjectByIndex(6));  // valid
    a.checkNull("08", t.getObjectByIndex(7));  // present, but odd
    a.checkNull("09", t.getObjectByIndex(8));
}
