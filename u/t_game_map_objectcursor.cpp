/**
  *  \file u/t_game_map_objectcursor.cpp
  *  \brief Test for game::map::ObjectCursor
  */

#include "game/map/objectcursor.hpp"

#include "t_game_map.hpp"

/** Interface test. */
void
TestGameMapObjectCursor::testIt()
{
    class Tester : public game::map::ObjectCursor {
     public:
        virtual game::map::ObjectType* getObjectType() const
            { return 0; }
        virtual void setCurrentIndex(game::Id_t /*index*/)
            { }
        virtual game::Id_t getCurrentIndex() const
            { return 0; }
    };
    Tester t;
}

