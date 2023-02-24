/**
  *  \file u/t_game_map_objectobserver.cpp
  *  \brief Test for game::map::ObjectObserver
  */

#include "game/map/objectobserver.hpp"

#include "t_game_map.hpp"
#include "game/map/object.hpp"
#include "game/map/objectcursor.hpp"
#include "game/map/objecttype.hpp"
#include "game/map/simpleobjectcursor.hpp"
#include "game/test/counter.hpp"

using game::Id_t;
using game::map::SimpleObjectCursor;
using game::test::Counter;

/** Test behaviour with a null object type.
    A: create ObjectCursor that has no ObjectType.
    E: all methods of ObjectObserver return null. */
void
TestGameMapObjectObserver::testNull()
{
    class NullObjectCursor : public game::map::ObjectCursor {
     public:
        virtual game::map::ObjectType* getObjectType() const
            { return 0; }
        virtual void setCurrentIndex(Id_t /*index*/)
            { }
        virtual Id_t getCurrentIndex() const
            { return 0; }
    };
    NullObjectCursor null;

    game::map::ObjectObserver testee(null);
    TS_ASSERT(testee.getCurrentObject() == 0);
    TS_ASSERT(testee.getObjectType() == 0);
    TS_ASSERT(&testee.cursor() == &null);
}

void
TestGameMapObjectObserver::testNormal()
{
    // A minimum implementation of Object
    class SimpleObject : public game::map::Object {
     public:
        SimpleObject(Id_t id)
            : Object(id)
            { }
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
        virtual afl::base::Optional<int> getOwner() const
            { return afl::base::Nothing; }
        virtual afl::base::Optional<game::map::Point> getPosition() const
            { return afl::base::Nothing; }
        void notify()
            { sig_change.raise(getId()); }
    };

    // A minimum implementation of ObjectType with two objects
    class SimpleObjectType : public game::map::ObjectType {
     public:
        SimpleObjectType(SimpleObject& a, SimpleObject& b)
            : m_a(a), m_b(b)
            { }
        virtual game::map::Object* getObjectByIndex(Id_t index)
            { return index == 1 ? &m_a : index == 2 ? &m_b : 0; }
        virtual Id_t getNextIndex(Id_t index) const
            { return index < 2 ? index+1 : 0; }
        virtual Id_t getPreviousIndex(Id_t index) const
            { return index > 0 ? index-1 : 2; }
     private:
        SimpleObject& m_a;
        SimpleObject& m_b;
    };

    // Using SimpleObjectCursor as simple implementation of ObjectCursor
    SimpleObject a(55), b(66);
    SimpleObjectType ty(a, b);
    SimpleObjectCursor cursor;
    Counter ctr;

    // Test observer
    game::map::ObjectObserver testee(cursor);
    testee.sig_objectChange.add(&ctr, &Counter::increment);
    TS_ASSERT_EQUALS(ctr.get(), 0);

    // Connecting cursor and type will produce first signal; selects a
    cursor.setObjectType(&ty);
    TS_ASSERT_EQUALS(ctr.get(), 1);
    TS_ASSERT_EQUALS(cursor.getCurrentIndex(), 1);
    TS_ASSERT(testee.getCurrentObject() == &a);

    // Modifying the object will produce a signal
    a.notify();
    TS_ASSERT_EQUALS(ctr.get(), 2);

    // Selecting another object will produce a signal
    cursor.setCurrentIndex(2);
    TS_ASSERT_EQUALS(ctr.get(), 3);
    TS_ASSERT_EQUALS(cursor.getCurrentIndex(), 2);
    TS_ASSERT(testee.getCurrentObject() == &b);

    // Modifying a will no longer produce a signal
    a.notify();
    TS_ASSERT_EQUALS(ctr.get(), 3);

    // Disconnecting the cursor will produce a signal
    cursor.setObjectType(0);
    TS_ASSERT_EQUALS(ctr.get(), 4);
}

