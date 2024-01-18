/**
  *  \file test/game/map/typedobjecttypetest.cpp
  *  \brief Test for game::map::TypedObjectType
  */

#include "game/map/typedobjecttype.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/object.hpp"

namespace {
    /* Object descendant for testing. Need not be constructible. */
    class MyObject : public game::map::Object {
     public:
        MyObject()
            : Object(0)
            { }
    };
}

/** Interface test. */
AFL_TEST_NOARG("game.map.TypedObjectType")
{
    class Tester : public game::map::TypedObjectType<MyObject> {
     public:
        virtual MyObject* getObjectByIndex(game::Id_t /*index*/)
            { return 0; }
        virtual game::Id_t getNextIndex(game::Id_t /*index*/) const
            { return 0; }
        virtual game::Id_t getPreviousIndex(game::Id_t /*index*/) const
            { return 0; }
    };
    Tester t;
}
