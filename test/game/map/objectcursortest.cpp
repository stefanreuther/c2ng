/**
  *  \file test/game/map/objectcursortest.cpp
  *  \brief Test for game::map::ObjectCursor
  */

#include "game/map/objectcursor.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/object.hpp"
#include "game/map/objecttype.hpp"

using game::Id_t;
using game::map::Object;
using game::map::ObjectType;
using game::map::ObjectCursor;

namespace {
    /* Object for testing; totally passive. */
    class TestObject : public Object {
     public:
        TestObject()
            : Object(0)
            { }
        virtual afl::base::Optional<int> getOwner() const
            { return 0; }
        virtual afl::base::Optional<game::map::Point> getPosition() const
            { return afl::base::Nothing; }
        virtual String_t getName(game::ObjectName, afl::string::Translator&, const game::InterpreterInterface&) const
            { return String_t(); }
    };

    /* Object for testing, with position. */
    class TestObjectWithPosition : public Object {
     public:
        TestObjectWithPosition(int x, int y)
            : Object(0), m_pos(x, y)
            { }
        virtual afl::base::Optional<int> getOwner() const
            { return 0; }
        virtual afl::base::Optional<game::map::Point> getPosition() const
            { return m_pos; }
        virtual String_t getName(game::ObjectName, afl::string::Translator&, const game::InterpreterInterface&) const
            { return String_t(); }
     private:
        game::map::Point m_pos;
    };

    /* Object type for testing; contains a vector of objects (but does not manage them). */
    class TestObjectType : public ObjectType {
     public:
        virtual Object* getObjectByIndex(Id_t index)
            {
                if (index > 0 && index <= int(m_objects.size())) {
                    return m_objects[index-1];
                } else {
                    return 0;
                }
            }
        virtual Id_t getNextIndex(Id_t index) const
            { return index < int(m_objects.size()) ? index+1 : 0; }
        virtual Id_t getPreviousIndex(Id_t index) const
            { return index > 0 ? index-1 : int(m_objects.size()); }
        void addObject(Object& obj)
            { m_objects.push_back(&obj); }
     private:
        std::vector<Object*> m_objects;
    };

    /* Cursor for testing; minimum-possible implementation. */
    class TestObjectCursor : public ObjectCursor {
     public:
        TestObjectCursor(ObjectType& type, Id_t index)
            : m_type(type), m_index(index)
            { }
        virtual ObjectType* getObjectType() const
            { return &m_type; }
        virtual void setCurrentIndex(Id_t index)
            { m_index = index; }
        virtual Id_t getCurrentIndex() const
            { return m_index; }
     private:
        ObjectType& m_type;
        Id_t m_index;
    };
}

/** Interface test. */
AFL_TEST_NOARG("game.map.ObjectCursor")
{
    class Tester : public ObjectCursor {
     public:
        virtual ObjectType* getObjectType() const
            { return 0; }
        virtual void setCurrentIndex(Id_t /*index*/)
            { }
        virtual Id_t getCurrentIndex() const
            { return 0; }
    };
    Tester t;
}

/** Test browse(). */
AFL_TEST("game.map.ObjectCursor:browse", a)
{
    TestObject unmarked;
    TestObject marked;
    marked.setIsMarked(true);

    TestObjectType ty;
    ty.addObject(unmarked);     // 1
    ty.addObject(unmarked);     // 2
    ty.addObject(marked);       // 3
    ty.addObject(unmarked);     // 4
    ty.addObject(unmarked);     // 5
    ty.addObject(marked);       // 6
    ty.addObject(unmarked);     // 7
    ty.addObject(unmarked);     // 8
    ty.addObject(marked);       // 9
    ty.addObject(unmarked);     // 10

    TestObjectCursor c(ty, 3);

    // Next
    c.browse(ObjectCursor::Next, false);
    a.checkEqual("01", c.getCurrentIndex(), 4);
    c.browse(ObjectCursor::Next, true);
    a.checkEqual("02", c.getCurrentIndex(), 6);

    // Previous
    c.browse(ObjectCursor::Previous, false);
    a.checkEqual("11", c.getCurrentIndex(), 5);
    c.browse(ObjectCursor::Previous, true);
    a.checkEqual("12", c.getCurrentIndex(), 3);

    // Last
    c.browse(ObjectCursor::Last, false);
    a.checkEqual("21", c.getCurrentIndex(), 10);
    c.browse(ObjectCursor::Last, true);
    a.checkEqual("22", c.getCurrentIndex(), 9);

    // First
    c.browse(ObjectCursor::First, false);
    a.checkEqual("31", c.getCurrentIndex(), 1);
    c.browse(ObjectCursor::First, true);
    a.checkEqual("32", c.getCurrentIndex(), 3);

    // Wrap
    c.browse(ObjectCursor::Previous, true);
    a.checkEqual("41", c.getCurrentIndex(), 9);
    c.browse(ObjectCursor::Next, true);
    a.checkEqual("42", c.getCurrentIndex(), 3);

    // Here. Neither of those changes the cursor as our objects have no position.
    c.browse(ObjectCursor::NextHere, false);
    a.checkEqual("51", c.getCurrentIndex(), 3);
    c.browse(ObjectCursor::PreviousHere, false);
    a.checkEqual("52", c.getCurrentIndex(), 3);
}

/** Test browse(marked=true) when there are no marked units.
    In this case, selection does not change. */
AFL_TEST("game.map.ObjectCursor:browse:no-marked-objects", a)
{
    TestObject unmarked;

    TestObjectType ty;
    ty.addObject(unmarked);     // 1
    ty.addObject(unmarked);     // 2
    ty.addObject(unmarked);     // 3

    TestObjectCursor c(ty, 2);

    // Next
    c.browse(ObjectCursor::Next, true);
    a.checkEqual("01", c.getCurrentIndex(), 2);

    // Previous
    c.browse(ObjectCursor::Previous, true);
    a.checkEqual("11", c.getCurrentIndex(), 2);

    // Last
    c.browse(ObjectCursor::Last, true);
    a.checkEqual("21", c.getCurrentIndex(), 2);

    // First
    c.browse(ObjectCursor::First, true);
    a.checkEqual("31", c.getCurrentIndex(), 2);
}

AFL_TEST("game.map.ObjectCursor:browse:here", a)
{
    TestObjectWithPosition oa(1000, 1000);
    TestObjectWithPosition ob(1000, 1001);

    TestObjectType ty;
    ty.addObject(oa);            // 1
    ty.addObject(oa);            // 2
    ty.addObject(ob);            // 3
    ty.addObject(oa);            // 4
    ty.addObject(oa);            // 5
    ty.addObject(ob);            // 6
    ty.addObject(ob);            // 7
    ty.addObject(oa);            // 8

    TestObjectCursor c(ty, 3);

    // NextHere
    c.browse(ObjectCursor::NextHere, false);
    a.checkEqual("01", c.getCurrentIndex(), 6);

    // PreviousHere
    c.browse(ObjectCursor::PreviousHere, false);
    a.checkEqual("11", c.getCurrentIndex(), 3);
    c.browse(ObjectCursor::PreviousHere, false);
    a.checkEqual("12", c.getCurrentIndex(), 7);
}
