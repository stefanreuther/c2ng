/**
  *  \file u/t_game_map_objectcursorfactory.cpp
  *  \brief Test for game::map::ObjectCursorFactory
  */

#include "game/map/objectcursorfactory.hpp"

#include "t_game_map.hpp"

/** Interface test. */
void
TestGameMapObjectCursorFactory::testInterface()
{
    class Tester : public game::map::ObjectCursorFactory {
     public:
        virtual game::map::ObjectCursor* getCursor(game::Session& /*session*/)
            { return 0; }
    };
    Tester t;
}

