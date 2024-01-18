/**
  *  \file test/game/map/objectcursorfactorytest.cpp
  *  \brief Test for game::map::ObjectCursorFactory
  */

#include "game/map/objectcursorfactory.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST_NOARG("game.map.ObjectCursorFactory")
{
    class Tester : public game::map::ObjectCursorFactory {
     public:
        virtual game::map::ObjectCursor* getCursor(game::Session& /*session*/)
            { return 0; }
    };
    Tester t;
}
