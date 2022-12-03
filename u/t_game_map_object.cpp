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
    using game::map::Object;

    class Tester : public Object {
     public:
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
        virtual game::Id_t getId() const
            { return game::Id_t(); }
        virtual afl::base::Optional<int> getOwner() const
            { return 0; }
        virtual afl::base::Optional<game::map::Point> getPosition() const
            { return afl::base::Nothing; }
    };
    Tester t;

    TS_ASSERT(!t.isDirty());
    TS_ASSERT(!t.isMarked());
    TS_ASSERT(!t.isPlayable(Object::Playable));

    t.setIsMarked(true);
    t.setPlayability(Object::Playable);

    TS_ASSERT(t.isDirty());
    TS_ASSERT(t.isMarked());
    TS_ASSERT(t.isPlayable(Object::Playable));
    TS_ASSERT(t.isPlayable(Object::ReadOnly));
}
