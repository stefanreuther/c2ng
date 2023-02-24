/**
  *  \file u/t_game_map_objectvectortype.cpp
  *  \brief Test for game::map::ObjectVectorType
  */

#include "game/map/objectvectortype.hpp"

#include "t_game_map.hpp"
#include "game/map/object.hpp"

namespace {
    class TestObj : public game::map::Object {
     public:
        TestObj(game::Id_t id)
            : Object(id)
            { }
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
        virtual afl::base::Optional<int> getOwner() const
            { return 0; }
        virtual afl::base::Optional<game::map::Point> getPosition() const
            { return afl::base::Nothing; }
    };
    class Tester : public game::map::ObjectVectorType<TestObj> {
     public:
        Tester(game::map::ObjectVector<TestObj>& vec)
            : ObjectVectorType(vec)
            { }
        virtual bool isValid(const TestObj& obj) const
            { return obj.getId() % 2 == 0; }
    };
}


/** Simple test. */
void
TestGameMapObjectVectorType::testIt()
{
    game::map::ObjectVector<TestObj> vec;
    vec.create(1);
    vec.create(2);
    vec.create(5);
    vec.create(6);
    vec.create(7);

    Tester t(vec);
    TS_ASSERT(t.getObjectByIndex(0) == 0);
    TS_ASSERT(t.getObjectByIndex(1) == 0);  // present, but odd
    TS_ASSERT(t.getObjectByIndex(2) != 0);  // valid
    TS_ASSERT(t.getObjectByIndex(3) == 0);
    TS_ASSERT(t.getObjectByIndex(4) == 0);
    TS_ASSERT(t.getObjectByIndex(5) == 0);  // present, but odd
    TS_ASSERT(t.getObjectByIndex(6) != 0);  // valid
    TS_ASSERT(t.getObjectByIndex(7) == 0);  // present, but odd
    TS_ASSERT(t.getObjectByIndex(8) == 0);
}

