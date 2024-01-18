/**
  *  \file test/game/map/objectobservertest.cpp
  *  \brief Test for game::map::ObjectObserver
  */

#include "game/map/objectobserver.hpp"

#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.map.ObjectObserver:null", a)
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
    a.checkNull("01. getCurrentObject", testee.getCurrentObject());
    a.checkNull("02. getObjectType", testee.getObjectType());
    a.check("03. cursor", &testee.cursor() == &null);
}

AFL_TEST("game.map.ObjectObserver:normal", a)
{
    // A minimum implementation of Object
    class SimpleObject : public game::map::Object {
     public:
        SimpleObject(Id_t id)
            : Object(id)
            { }
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, const game::InterpreterInterface& /*iface*/) const
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
    SimpleObject oa(55), ob(66);
    SimpleObjectType ty(oa, ob);
    SimpleObjectCursor cursor;
    Counter ctr;

    // Test observer
    game::map::ObjectObserver testee(cursor);
    testee.sig_objectChange.add(&ctr, &Counter::increment);
    a.checkEqual("01. counter", ctr.get(), 0);

    // Connecting cursor and type will produce first signal; selects a
    cursor.setObjectType(&ty);
    a.checkEqual("11. counter", ctr.get(), 1);
    a.checkEqual("12. getCurrentIndex", cursor.getCurrentIndex(), 1);
    a.check("13. getCurrentObject", testee.getCurrentObject() == &oa);

    // Modifying the object will produce a signal
    oa.notify();
    a.checkEqual("21. counter", ctr.get(), 2);

    // Selecting another object will produce a signal
    cursor.setCurrentIndex(2);
    a.checkEqual("31. counter", ctr.get(), 3);
    a.checkEqual("32. getCurrentIndex", cursor.getCurrentIndex(), 2);
    a.check("33. getCurrentObject", testee.getCurrentObject() == &ob);

    // Modifying a will no longer produce a signal
    oa.notify();
    a.checkEqual("41. counter", ctr.get(), 3);

    // Disconnecting the cursor will produce a signal
    cursor.setObjectType(0);
    a.checkEqual("51. counter", ctr.get(), 4);
}
