/**
  *  \file u/t_game_map_objecttype.cpp
  *  \brief Test for game::map::ObjectType
  */

#include "game/map/objecttype.hpp"

#include "t_game_map.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/map/configuration.hpp"
#include "game/map/object.hpp"
#include "game/ref/sortbyid.hpp"

using afl::container::PtrVector;
using game::Id_t;
using game::PlayerSet_t;
using game::map::Configuration;
using game::map::Object;
using game::map::ObjectType;
using game::map::Point;

namespace {
    /*
     *  Test setup: an object with configurable position/id, and an ObjectType which can contain it.
     */
    class TestObject : public Object {
     public:
        TestObject(Id_t id, int owner, Point pos)
            : m_id(id), m_owner(owner), m_pos(pos)
            { }

        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
        virtual game::Id_t getId() const
            { return m_id; }
        virtual afl::base::Optional<int> getOwner() const
            {
                if (m_owner >= 0) {
                    return m_owner;
                } else {
                    return afl::base::Nothing;
                }
            }
        virtual afl::base::Optional<Point> getPosition() const
            {
                if (m_pos.getX() > 0) {
                    return m_pos;
                } else {
                    return afl::base::Nothing;
                }
            }
     private:
        Id_t m_id;
        int m_owner;
        Point m_pos;
    };

    class TestType : public game::map::ObjectType {
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
            {
                if (index < int(m_objects.size())) {
                    return index+1;
                } else {
                    return 0;
                }
            }
        virtual Id_t getPreviousIndex(Id_t index) const
            {
                if (index == 0) {
                    return Id_t(m_objects.size());
                } else {
                    return index-1;
                }
            }

        void addNull()
            { m_objects.pushBackNew(0); }
        TestObject& addObject(Id_t id, int owner, Point pos)
            { return *m_objects.pushBackNew(new TestObject(id, owner, pos)); }

     private:
        PtrVector<TestObject> m_objects;
    };
}



/** Test behaviour on empty list. */
void
TestGameMapObjectType::testEmpty()
{
    TestType t;
    TS_ASSERT(t.isEmpty());
    TS_ASSERT(!t.isUnit());
    TS_ASSERT_EQUALS(t.countObjects(), 0);
    TS_ASSERT_EQUALS(t.findNextIndex(0), 0);

    // Derived objects
    afl::base::Deleter del;
    TS_ASSERT(t.filterPosition(del, game::map::Point()).isEmpty());
    TS_ASSERT(t.filterOwner(del, game::PlayerSet_t()).isEmpty());
    TS_ASSERT(t.filterMarked(del, true).isEmpty());
}

/** Test behaviour on unit (1-element) list. */
void
TestGameMapObjectType::testUnit()
{
    TestType t;
    t.addObject(100, 1, Point(1000, 2000));

    TS_ASSERT(!t.isEmpty());
    TS_ASSERT(t.isUnit());
    TS_ASSERT_EQUALS(t.countObjects(), 1);
    TS_ASSERT_EQUALS(t.findNextIndex(0), 1);

    TS_ASSERT_EQUALS(t.countObjectsAt(Point(1000, 1000), PlayerSet_t(1)), 0);
    TS_ASSERT_EQUALS(t.countObjectsAt(Point(1000, 2000), PlayerSet_t(1)), 1);
    TS_ASSERT_EQUALS(t.countObjectsAt(Point(1000, 2000), PlayerSet_t(3)), 0);

    TS_ASSERT_EQUALS(t.findPreviousIndexWrap(1, false), 1);
    TS_ASSERT_EQUALS(t.findPreviousIndexWrap(1, true), 0);
    TS_ASSERT_EQUALS(t.findNextIndexWrap(1, false), 1);
    TS_ASSERT_EQUALS(t.findNextIndexWrap(1, true), 0);

    TS_ASSERT_EQUALS(t.findPreviousIndexNoWrap(1), 0);
    TS_ASSERT_EQUALS(t.findNextIndexNoWrap(1), 0);

    TS_ASSERT_EQUALS(t.findPreviousIndexNoWrap(1, false), 0);
    TS_ASSERT_EQUALS(t.findPreviousIndexNoWrap(1, true), 0);
    TS_ASSERT_EQUALS(t.findNextIndexNoWrap(1, false), 0);
    TS_ASSERT_EQUALS(t.findNextIndexNoWrap(1, true), 0);

    TS_ASSERT_EQUALS(t.findPreviousObjectAt(Point(1000, 2000), 1, false), 0);
    TS_ASSERT_EQUALS(t.findPreviousObjectAt(Point(1000, 2000), 1, true), 0);
    TS_ASSERT_EQUALS(t.findNextObjectAt(Point(1000, 2000), 1, false), 0);
    TS_ASSERT_EQUALS(t.findNextObjectAt(Point(1000, 2000), 1, true), 0);

    TS_ASSERT_EQUALS(t.findPreviousObjectAtWrap(Point(1000, 2000), 1, false), 1);
    TS_ASSERT_EQUALS(t.findPreviousObjectAtWrap(Point(1000, 2000), 1, true), 0);
    TS_ASSERT_EQUALS(t.findNextObjectAtWrap(Point(1000, 2000), 1, false), 1);
    TS_ASSERT_EQUALS(t.findNextObjectAtWrap(Point(1000, 2000), 1, true), 0);

    TS_ASSERT_EQUALS(t.findIndexForId(100), 1);
    TS_ASSERT_EQUALS(t.findIndexForId(1), 0);

    // Derived objects
    afl::base::Deleter del;
    {
        game::map::ObjectType& d = t.filterPosition(del, game::map::Point());
        TS_ASSERT(d.isEmpty());
        TS_ASSERT_EQUALS(d.findNextIndexNoWrap(0), 0);
    }
    {
        game::map::ObjectType& d = t.filterPosition(del, game::map::Point(1000, 2000));
        TS_ASSERT(!d.isEmpty());
        TS_ASSERT_EQUALS(d.findNextIndexNoWrap(0), 1);
        TS_ASSERT_EQUALS(d.findNextIndexNoWrap(1), 0);
        TS_ASSERT_EQUALS(d.findPreviousIndexNoWrap(0), 1);
        TS_ASSERT_EQUALS(d.findPreviousIndexNoWrap(1), 0);
    }
    {
        game::map::ObjectType& d = t.filterOwner(del, game::PlayerSet_t());
        TS_ASSERT(d.isEmpty());
    }
    {
        game::map::ObjectType& d = t.filterOwner(del, game::PlayerSet_t(1));
        TS_ASSERT(!d.isEmpty());
        TS_ASSERT_EQUALS(d.findNextIndexNoWrap(0), 1);
    }
    {
        game::map::ObjectType& d = t.filterMarked(del, true);
        TS_ASSERT(d.isEmpty());
    }
}

/** Test list containing several empty slots; needs to behave as empty. */
void
TestGameMapObjectType::testSparseEmpty()
{
    TestType t;
    for (int i = 0; i < 10; ++i) {
        t.addNull();
    }
    TS_ASSERT(t.isEmpty());
    TS_ASSERT(!t.isUnit());
    TS_ASSERT_EQUALS(t.countObjects(), 0);
    TS_ASSERT_EQUALS(t.findNextIndex(0), 0);
}

/** Test sparse unit list (one object between multiple non-objects). */
void
TestGameMapObjectType::testSparseUnit()
{
    TestType t;
    for (int i = 0; i < 5; ++i) {
        t.addNull();
    }
    t.addObject(100, 1, Point(1000, 2000));   // index 6
    for (int i = 0; i < 5; ++i) {
        t.addNull();
    }

    TS_ASSERT(!t.isEmpty());
    TS_ASSERT(t.isUnit());
    TS_ASSERT_EQUALS(t.countObjects(), 1);
    TS_ASSERT_EQUALS(t.findNextIndex(0), 6);

    TS_ASSERT_EQUALS(t.countObjectsAt(Point(1000, 1000), PlayerSet_t(1)), 0);
    TS_ASSERT_EQUALS(t.countObjectsAt(Point(1000, 2000), PlayerSet_t(1)), 1);
    TS_ASSERT_EQUALS(t.countObjectsAt(Point(1000, 2000), PlayerSet_t(3)), 0);

    TS_ASSERT_EQUALS(t.findPreviousIndexWrap(1, false), 6);
    TS_ASSERT_EQUALS(t.findPreviousIndexWrap(1, true), 0);
    TS_ASSERT_EQUALS(t.findNextIndexWrap(1, false), 6);
    TS_ASSERT_EQUALS(t.findNextIndexWrap(1, true), 0);
}

/** Test normal behaviour with a diverse situation. */
void
TestGameMapObjectType::testNormal()
{
    const Point A(1000, 2000);
    const Point B(1000, 4000);
    TestType t;
    t.addObject(100, 1, A);                      // 1
    t.addObject(200, 1, A).setIsMarked(true);    // 2
    t.addObject(300, 2, B);                      // 3
    t.addObject(400, 1, B);                      // 4
    t.addObject(500, 2, B).setIsMarked(true);    // 5
    t.addObject(600, 2, A).setIsMarked(true);    // 6
    t.addObject(700, 2, A).setIsMarked(true);    // 7

    TS_ASSERT(!t.isEmpty());
    TS_ASSERT(!t.isUnit());
    TS_ASSERT_EQUALS(t.countObjects(), 7);
    TS_ASSERT_EQUALS(t.findNextIndex(0), 1);;

    // countObjectsAt
    TS_ASSERT_EQUALS(t.countObjectsAt(A, PlayerSet_t(1)), 2);
    TS_ASSERT_EQUALS(t.countObjectsAt(A, PlayerSet_t() + 1 + 2), 4);
    TS_ASSERT_EQUALS(t.countObjectsAt(A, PlayerSet_t::allUpTo(10)), 4);
    TS_ASSERT_EQUALS(t.countObjectsAt(Point(1,1), PlayerSet_t::allUpTo(10)), 0);

    // findPreviousIndexWrap
    TS_ASSERT_EQUALS(t.findPreviousIndexWrap(0, false), 7);
    TS_ASSERT_EQUALS(t.findPreviousIndexWrap(5, false), 4);
    TS_ASSERT_EQUALS(t.findPreviousIndexWrap(1, false), 7);

    TS_ASSERT_EQUALS(t.findPreviousIndexWrap(0, true), 7);
    TS_ASSERT_EQUALS(t.findPreviousIndexWrap(5, true), 2);
    TS_ASSERT_EQUALS(t.findPreviousIndexWrap(1, true), 7);

    // findNextIndexWrap
    TS_ASSERT_EQUALS(t.findNextIndexWrap(0, false), 1);
    TS_ASSERT_EQUALS(t.findNextIndexWrap(2, false), 3);
    TS_ASSERT_EQUALS(t.findNextIndexWrap(7, false), 1);

    TS_ASSERT_EQUALS(t.findNextIndexWrap(0, true), 2);
    TS_ASSERT_EQUALS(t.findNextIndexWrap(2, true), 5);
    TS_ASSERT_EQUALS(t.findNextIndexWrap(7, true), 2);

    // findPreviousIndexNoWrap
    TS_ASSERT_EQUALS(t.findPreviousIndexNoWrap(0, false), 7);
    TS_ASSERT_EQUALS(t.findPreviousIndexNoWrap(5, false), 4);
    TS_ASSERT_EQUALS(t.findPreviousIndexNoWrap(1, false), 0);

    TS_ASSERT_EQUALS(t.findPreviousIndexNoWrap(0, true), 7);
    TS_ASSERT_EQUALS(t.findPreviousIndexNoWrap(5, true), 2);
    TS_ASSERT_EQUALS(t.findPreviousIndexNoWrap(1, true), 0);

    // findNextIndexNoWrap
    TS_ASSERT_EQUALS(t.findNextIndexNoWrap(0, false), 1);
    TS_ASSERT_EQUALS(t.findNextIndexNoWrap(2, false), 3);
    TS_ASSERT_EQUALS(t.findNextIndexNoWrap(7, false), 0);

    TS_ASSERT_EQUALS(t.findNextIndexNoWrap(0, true), 2);
    TS_ASSERT_EQUALS(t.findNextIndexNoWrap(2, true), 5);
    TS_ASSERT_EQUALS(t.findNextIndexNoWrap(7, true), 0);

    // findNextObjectAt
    TS_ASSERT_EQUALS(t.findNextObjectAt(B, 0, false), 3);
    TS_ASSERT_EQUALS(t.findNextObjectAt(B, 1, false), 3);
    TS_ASSERT_EQUALS(t.findNextObjectAt(B, 3, false), 4);
    TS_ASSERT_EQUALS(t.findNextObjectAt(B, 5, false), 0);

    TS_ASSERT_EQUALS(t.findNextObjectAt(B, 0, true), 5);
    TS_ASSERT_EQUALS(t.findNextObjectAt(B, 1, true), 5);
    TS_ASSERT_EQUALS(t.findNextObjectAt(B, 3, true), 5);
    TS_ASSERT_EQUALS(t.findNextObjectAt(B, 5, true), 0);

    // findPreviousObjectAt
    TS_ASSERT_EQUALS(t.findPreviousObjectAt(B, 0, false), 5);
    TS_ASSERT_EQUALS(t.findPreviousObjectAt(B, 1, false), 0);
    TS_ASSERT_EQUALS(t.findPreviousObjectAt(B, 3, false), 0);
    TS_ASSERT_EQUALS(t.findPreviousObjectAt(B, 4, false), 3);

    TS_ASSERT_EQUALS(t.findPreviousObjectAt(B, 0, true), 5);
    TS_ASSERT_EQUALS(t.findPreviousObjectAt(B, 3, true), 0);
    TS_ASSERT_EQUALS(t.findPreviousObjectAt(B, 5, true), 0);
    TS_ASSERT_EQUALS(t.findPreviousObjectAt(B, 7, true), 5);

    // findNextObjectAtWrap
    TS_ASSERT_EQUALS(t.findNextObjectAtWrap(B, 0, false), 3);
    TS_ASSERT_EQUALS(t.findNextObjectAtWrap(B, 1, false), 3);
    TS_ASSERT_EQUALS(t.findNextObjectAtWrap(B, 3, false), 4);
    TS_ASSERT_EQUALS(t.findNextObjectAtWrap(B, 5, false), 3);

    TS_ASSERT_EQUALS(t.findNextObjectAtWrap(B, 0, true), 5);
    TS_ASSERT_EQUALS(t.findNextObjectAtWrap(B, 1, true), 5);
    TS_ASSERT_EQUALS(t.findNextObjectAtWrap(B, 3, true), 5);
    TS_ASSERT_EQUALS(t.findNextObjectAtWrap(B, 5, true), 5);

    // findPreviousObjectAtWrap
    TS_ASSERT_EQUALS(t.findPreviousObjectAtWrap(B, 0, false), 5);
    TS_ASSERT_EQUALS(t.findPreviousObjectAtWrap(B, 1, false), 5);
    TS_ASSERT_EQUALS(t.findPreviousObjectAtWrap(B, 3, false), 5);
    TS_ASSERT_EQUALS(t.findPreviousObjectAtWrap(B, 4, false), 3);

    TS_ASSERT_EQUALS(t.findPreviousObjectAtWrap(B, 0, true), 5);
    TS_ASSERT_EQUALS(t.findPreviousObjectAtWrap(B, 3, true), 5);
    TS_ASSERT_EQUALS(t.findPreviousObjectAtWrap(B, 5, true), 5);
    TS_ASSERT_EQUALS(t.findPreviousObjectAtWrap(B, 7, true), 5);

    // findIndexForId
    TS_ASSERT_EQUALS(t.findIndexForId(500), 5);
    TS_ASSERT_EQUALS(t.findIndexForId(499), 0);

    // findIndexForObject
    TS_ASSERT(t.getObjectByIndex(3) != 0);
    TS_ASSERT_EQUALS(t.getObjectByIndex(3)->getId(), 300);
    TS_ASSERT_EQUALS(t.findIndexForObject(t.getObjectByIndex(3)), 3);
    TS_ASSERT_EQUALS(t.findIndexForObject(0), 0);

    TestObject alien(88, 8, Point());
    TS_ASSERT_EQUALS(t.findIndexForObject(&alien), 0);

    // Filters
    afl::base::Deleter del;
    TS_ASSERT_EQUALS(t.filterPosition(del, A).countObjects(), 4);
    TS_ASSERT_EQUALS(t.filterPosition(del, B).countObjects(), 3);
    TS_ASSERT_EQUALS(t.filterMarked(del, true).countObjects(), 4);
    TS_ASSERT_EQUALS(t.filterMarked(del, false).countObjects(), 7);
    TS_ASSERT_EQUALS(t.filterOwner(del, game::PlayerSet_t(1)).countObjects(), 3);
    TS_ASSERT_EQUALS(t.filterOwner(del, game::PlayerSet_t(2)).countObjects(), 4);
    TS_ASSERT_EQUALS(t.filterOwner(del, game::PlayerSet_t(3)).countObjects(), 0);
    TS_ASSERT_EQUALS(t.filterOwner(del, game::PlayerSet_t() + 1 + 2).countObjects(), 7);
}

/** Test handling partial information (no position, no owner). */
void
TestGameMapObjectType::testPartial()
{
    const Point A(1000, 2000);
    TestType t;
    t.addObject(100, 1,  A);         // #1
    t.addObject(200, 1,  A);         // #2
    t.addObject(300, -1, A);         // #3 - no owner
    t.addObject(400, 1,  A);         // #4
    t.addObject(500, 2,  Point());   // #5 - no position
    t.addObject(600, 2,  A);         // #6
    t.addObject(700, 2,  A);         // #7

    TS_ASSERT_EQUALS(t.countObjects(), 7);

    // Counting will skip 2 object
    TS_ASSERT_EQUALS(t.countObjectsAt(A, PlayerSet_t::allUpTo(20)), 5);

    // Finding at position will report ownerless object, but skip positionless
    TS_ASSERT_EQUALS(t.findNextObjectAt(A, 2, false), 3);
    TS_ASSERT_EQUALS(t.findNextObjectAt(A, 4, false), 6);
}

/** Test findNearestIndex(). */
void
TestGameMapObjectType::testFindNearest()
{
    TestType t;
    t.addObject(100, 1, Point(1000, 1000));
    t.addObject(200, 1, Point(1000, 2000));
    t.addObject(300, 1, Point(1000, 1500));
    t.addObject(400, 1, Point(1500, 1500));

    Configuration config;
    TS_ASSERT_EQUALS(t.findNearestIndex(Point(1010, 1010), config), 1);
    TS_ASSERT_EQUALS(t.findNearestIndex(Point(1400, 1400), config), 4);
    TS_ASSERT_EQUALS(t.findNearestIndex(Point(500, 1500), config), 3);
}

/** Test sort(). */
void
TestGameMapObjectType::testSort()
{
    // Similar situation as in testNormal.
    // Use duplicate Ids to exercise tie-breaking, because SortById has no further dependencies.
    const Point A(1000, 2000);
    const Point B(1000, 4000);
    TestType t;
    t.addObject(1, 7, A);                      // 1
    t.addObject(1, 7, A).setIsMarked(true);    // 2
    t.addObject(2, 7, B);                      // 3
    t.addObject(1, 7, B);                      // 4
    t.addObject(2, 7, B).setIsMarked(true);    // 5
    t.addObject(2, 7, A).setIsMarked(true);    // 6
    t.addObject(2, 7, A).setIsMarked(true);    // 7

    // Test sorting
    afl::base::Deleter del;
    game::ref::SortById pred;
    game::map::ObjectType& sorted = t.sort(del, pred, game::Reference::Ship);
    TS_ASSERT_EQUALS(sorted.findNextIndexNoWrap(0), 1);
    TS_ASSERT_EQUALS(sorted.findNextIndexNoWrap(1), 2);
    TS_ASSERT_EQUALS(sorted.findNextIndexNoWrap(2), 4);
    TS_ASSERT_EQUALS(sorted.findNextIndexNoWrap(4), 3);
    TS_ASSERT_EQUALS(sorted.findNextIndexNoWrap(3), 5);
    TS_ASSERT_EQUALS(sorted.findNextIndexNoWrap(5), 6);
    TS_ASSERT_EQUALS(sorted.findNextIndexNoWrap(6), 7);
    TS_ASSERT_EQUALS(sorted.findNextIndexNoWrap(7), 0);

    TS_ASSERT_EQUALS(sorted.findPreviousIndexNoWrap(0), 7);
    TS_ASSERT_EQUALS(sorted.findPreviousIndexNoWrap(7), 6);
    TS_ASSERT_EQUALS(sorted.findPreviousIndexNoWrap(6), 5);
    TS_ASSERT_EQUALS(sorted.findPreviousIndexNoWrap(5), 3);
    TS_ASSERT_EQUALS(sorted.findPreviousIndexNoWrap(3), 4);
    TS_ASSERT_EQUALS(sorted.findPreviousIndexNoWrap(4), 2);
    TS_ASSERT_EQUALS(sorted.findPreviousIndexNoWrap(2), 1);
    TS_ASSERT_EQUALS(sorted.findPreviousIndexNoWrap(1), 0);

    // Test further processing the sorted result (not recommended but possible)
    TS_ASSERT_EQUALS(sorted.countObjects(), 7);
    TS_ASSERT_EQUALS(sorted.filterMarked(del, true).countObjects(), 4);
}

