/**
  *  \file test/game/map/objecttypetest.cpp
  *  \brief Test for game::map::ObjectType
  */

#include "game/map/objecttype.hpp"

#include "afl/container/ptrvector.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/map/object.hpp"
#include "game/ref/sortby.hpp"

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
            : Object(id), m_owner(owner), m_pos(pos)
            { }

        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, const game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
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
AFL_TEST("game.map.ObjectType:empty", a)
{
    TestType t;
    a.check("01. isEmpty", t.isEmpty());
    a.check("02. isUnit", !t.isUnit());
    a.checkEqual("03. countObjects", t.countObjects(), 0);
    a.checkEqual("04. findNextIndex", t.findNextIndex(0), 0);

    // Derived objects
    afl::base::Deleter del;
    a.check("11. filterPosition", t.filterPosition(del, game::map::Point()).isEmpty());
    a.check("12. filterOwner", t.filterOwner(del, game::PlayerSet_t()).isEmpty());
    a.check("13. filterMarked", t.filterMarked(del, true).isEmpty());
}

/** Test behaviour on unit (1-element) list. */
AFL_TEST("game.map.ObjectType:unit", a)
{
    TestType t;
    t.addObject(100, 1, Point(1000, 2000));

    a.check("01. isEmpty", !t.isEmpty());
    a.check("02. isUnit", t.isUnit());
    a.checkEqual("03. countObjects", t.countObjects(), 1);
    a.checkEqual("04. findNextIndex", t.findNextIndex(0), 1);

    a.checkEqual("11. countObjects", t.countObjectsAt(Point(1000, 1000), PlayerSet_t(1)), 0);
    a.checkEqual("12. countObjects", t.countObjectsAt(Point(1000, 2000), PlayerSet_t(1)), 1);
    a.checkEqual("13. countObjects", t.countObjectsAt(Point(1000, 2000), PlayerSet_t(3)), 0);

    a.checkEqual("21. findPreviousIndexWrap", t.findPreviousIndexWrap(1, false), 1);
    a.checkEqual("22. findPreviousIndexWrap", t.findPreviousIndexWrap(1, true), 0);
    a.checkEqual("23. findNextIndexWrap", t.findNextIndexWrap(1, false), 1);
    a.checkEqual("24. findNextIndexWrap", t.findNextIndexWrap(1, true), 0);

    a.checkEqual("31. findPreviousIndexNoWrap", t.findPreviousIndexNoWrap(1), 0);
    a.checkEqual("32. findNextIndexNoWrap", t.findNextIndexNoWrap(1), 0);

    a.checkEqual("41. findPreviousIndexNoWrap", t.findPreviousIndexNoWrap(1, false), 0);
    a.checkEqual("42. findPreviousIndexNoWrap", t.findPreviousIndexNoWrap(1, true), 0);
    a.checkEqual("43. findNextIndexNoWrap", t.findNextIndexNoWrap(1, false), 0);
    a.checkEqual("44. findNextIndexNoWrap", t.findNextIndexNoWrap(1, true), 0);

    a.checkEqual("51. findPreviousObjectAt", t.findPreviousObjectAt(Point(1000, 2000), 1, false), 0);
    a.checkEqual("52. findPreviousObjectAt", t.findPreviousObjectAt(Point(1000, 2000), 1, true), 0);
    a.checkEqual("53. findNextObjectAt", t.findNextObjectAt(Point(1000, 2000), 1, false), 0);
    a.checkEqual("54. findNextObjectAt", t.findNextObjectAt(Point(1000, 2000), 1, true), 0);

    a.checkEqual("61. findPreviousObjectAtWrap", t.findPreviousObjectAtWrap(Point(1000, 2000), 1, false), 1);
    a.checkEqual("62. findPreviousObjectAtWrap", t.findPreviousObjectAtWrap(Point(1000, 2000), 1, true), 0);
    a.checkEqual("63. findNextObjectAtWrap", t.findNextObjectAtWrap(Point(1000, 2000), 1, false), 1);
    a.checkEqual("64. findNextObjectAtWrap", t.findNextObjectAtWrap(Point(1000, 2000), 1, true), 0);

    a.checkEqual("71. findIndexForId", t.findIndexForId(100), 1);
    a.checkEqual("72. findIndexForId", t.findIndexForId(1), 0);

    // Derived objects
    afl::base::Deleter del;
    {
        game::map::ObjectType& d = t.filterPosition(del, game::map::Point());
        a.check("81. isEmpty", d.isEmpty());
        a.checkEqual("82. findNextIndexNoWrap", d.findNextIndexNoWrap(0), 0);
    }
    {
        game::map::ObjectType& d = t.filterPosition(del, game::map::Point(1000, 2000));
        a.check("83. isEmpty", !d.isEmpty());
        a.checkEqual("84. findNextIndexNoWrap", d.findNextIndexNoWrap(0), 1);
        a.checkEqual("85. findNextIndexNoWrap", d.findNextIndexNoWrap(1), 0);
        a.checkEqual("86. findPreviousIndexNoWrap", d.findPreviousIndexNoWrap(0), 1);
        a.checkEqual("87. findPreviousIndexNoWrap", d.findPreviousIndexNoWrap(1), 0);
    }
    {
        game::map::ObjectType& d = t.filterOwner(del, game::PlayerSet_t());
        a.check("88. isEmpty", d.isEmpty());
    }
    {
        game::map::ObjectType& d = t.filterOwner(del, game::PlayerSet_t(1));
        a.check("89. isEmpty", !d.isEmpty());
        a.checkEqual("90. findNextIndexNoWrap", d.findNextIndexNoWrap(0), 1);
    }
    {
        game::map::ObjectType& d = t.filterMarked(del, true);
        a.check("91. isEmpty", d.isEmpty());
    }
}

/** Test list containing several empty slots; needs to behave as empty. */
AFL_TEST("game.map.ObjectType:sparse-empty", a)
{
    TestType t;
    for (int i = 0; i < 10; ++i) {
        t.addNull();
    }
    a.check("01. isEmpty", t.isEmpty());
    a.check("02. isUnit", !t.isUnit());
    a.checkEqual("03. countObjects", t.countObjects(), 0);
    a.checkEqual("04. findNextIndex", t.findNextIndex(0), 0);
}

/** Test sparse unit list (one object between multiple non-objects). */
AFL_TEST("game.map.ObjectType:sparse-unit", a)
{
    TestType t;
    for (int i = 0; i < 5; ++i) {
        t.addNull();
    }
    t.addObject(100, 1, Point(1000, 2000));   // index 6
    for (int i = 0; i < 5; ++i) {
        t.addNull();
    }

    a.check("01. isEmpty", !t.isEmpty());
    a.check("02. isUnit", t.isUnit());
    a.checkEqual("03. countObjects", t.countObjects(), 1);
    a.checkEqual("04. findNextIndex", t.findNextIndex(0), 6);

    a.checkEqual("11. countObjects", t.countObjectsAt(Point(1000, 1000), PlayerSet_t(1)), 0);
    a.checkEqual("12. countObjects", t.countObjectsAt(Point(1000, 2000), PlayerSet_t(1)), 1);
    a.checkEqual("13. countObjects", t.countObjectsAt(Point(1000, 2000), PlayerSet_t(3)), 0);

    a.checkEqual("21. findPreviousIndexWrap", t.findPreviousIndexWrap(1, false), 6);
    a.checkEqual("22. findPreviousIndexWrap", t.findPreviousIndexWrap(1, true), 0);
    a.checkEqual("23. findNextIndexWrap", t.findNextIndexWrap(1, false), 6);
    a.checkEqual("24. findNextIndexWrap", t.findNextIndexWrap(1, true), 0);
}

/** Test normal behaviour with a diverse situation. */
AFL_TEST("game.map.ObjectType:normal", a)
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

    a.check("01. isEmpty", !t.isEmpty());
    a.check("02. isUnit", !t.isUnit());
    a.checkEqual("03. countObjects", t.countObjects(), 7);
    a.checkEqual("04. findNextIndex", t.findNextIndex(0), 1);

    // countObjectsAt
    a.checkEqual("11. countObjects", t.countObjectsAt(A, PlayerSet_t(1)), 2);
    a.checkEqual("12. countObjects", t.countObjectsAt(A, PlayerSet_t() + 1 + 2), 4);
    a.checkEqual("13. countObjects", t.countObjectsAt(A, PlayerSet_t::allUpTo(10)), 4);
    a.checkEqual("14. countObjects", t.countObjectsAt(Point(1,1), PlayerSet_t::allUpTo(10)), 0);

    // findPreviousIndexWrap
    a.checkEqual("21. findPreviousIndexWrap", t.findPreviousIndexWrap(0, false), 7);
    a.checkEqual("22. findPreviousIndexWrap", t.findPreviousIndexWrap(5, false), 4);
    a.checkEqual("23. findPreviousIndexWrap", t.findPreviousIndexWrap(1, false), 7);

    a.checkEqual("31. findPreviousIndexWrap", t.findPreviousIndexWrap(0, true), 7);
    a.checkEqual("32. findPreviousIndexWrap", t.findPreviousIndexWrap(5, true), 2);
    a.checkEqual("33. findPreviousIndexWrap", t.findPreviousIndexWrap(1, true), 7);

    // findNextIndexWrap
    a.checkEqual("41. findNextIndexWrap", t.findNextIndexWrap(0, false), 1);
    a.checkEqual("42. findNextIndexWrap", t.findNextIndexWrap(2, false), 3);
    a.checkEqual("43. findNextIndexWrap", t.findNextIndexWrap(7, false), 1);

    a.checkEqual("51. findNextIndexWrap", t.findNextIndexWrap(0, true), 2);
    a.checkEqual("52. findNextIndexWrap", t.findNextIndexWrap(2, true), 5);
    a.checkEqual("53. findNextIndexWrap", t.findNextIndexWrap(7, true), 2);

    // findPreviousIndexNoWrap
    a.checkEqual("61. findPreviousIndexNoWrap", t.findPreviousIndexNoWrap(0, false), 7);
    a.checkEqual("62. findPreviousIndexNoWrap", t.findPreviousIndexNoWrap(5, false), 4);
    a.checkEqual("63. findPreviousIndexNoWrap", t.findPreviousIndexNoWrap(1, false), 0);

    a.checkEqual("71. findPreviousIndexNoWrap", t.findPreviousIndexNoWrap(0, true), 7);
    a.checkEqual("72. findPreviousIndexNoWrap", t.findPreviousIndexNoWrap(5, true), 2);
    a.checkEqual("73. findPreviousIndexNoWrap", t.findPreviousIndexNoWrap(1, true), 0);

    // findNextIndexNoWrap
    a.checkEqual("81. findNextIndexNoWrap", t.findNextIndexNoWrap(0, false), 1);
    a.checkEqual("82. findNextIndexNoWrap", t.findNextIndexNoWrap(2, false), 3);
    a.checkEqual("83. findNextIndexNoWrap", t.findNextIndexNoWrap(7, false), 0);

    a.checkEqual("91. findNextIndexNoWrap", t.findNextIndexNoWrap(0, true), 2);
    a.checkEqual("92. findNextIndexNoWrap", t.findNextIndexNoWrap(2, true), 5);
    a.checkEqual("93. findNextIndexNoWrap", t.findNextIndexNoWrap(7, true), 0);

    // findNextObjectAt
    a.checkEqual("101. findNextObjectAt", t.findNextObjectAt(B, 0, false), 3);
    a.checkEqual("102. findNextObjectAt", t.findNextObjectAt(B, 1, false), 3);
    a.checkEqual("103. findNextObjectAt", t.findNextObjectAt(B, 3, false), 4);
    a.checkEqual("104. findNextObjectAt", t.findNextObjectAt(B, 5, false), 0);

    a.checkEqual("111. findNextObjectAt", t.findNextObjectAt(B, 0, true), 5);
    a.checkEqual("112. findNextObjectAt", t.findNextObjectAt(B, 1, true), 5);
    a.checkEqual("113. findNextObjectAt", t.findNextObjectAt(B, 3, true), 5);
    a.checkEqual("114. findNextObjectAt", t.findNextObjectAt(B, 5, true), 0);

    // findPreviousObjectAt
    a.checkEqual("121. findPreviousObjectAt", t.findPreviousObjectAt(B, 0, false), 5);
    a.checkEqual("122. findPreviousObjectAt", t.findPreviousObjectAt(B, 1, false), 0);
    a.checkEqual("123. findPreviousObjectAt", t.findPreviousObjectAt(B, 3, false), 0);
    a.checkEqual("124. findPreviousObjectAt", t.findPreviousObjectAt(B, 4, false), 3);

    a.checkEqual("131. findPreviousObjectAt", t.findPreviousObjectAt(B, 0, true), 5);
    a.checkEqual("132. findPreviousObjectAt", t.findPreviousObjectAt(B, 3, true), 0);
    a.checkEqual("133. findPreviousObjectAt", t.findPreviousObjectAt(B, 5, true), 0);
    a.checkEqual("134. findPreviousObjectAt", t.findPreviousObjectAt(B, 7, true), 5);

    // findNextObjectAtWrap
    a.checkEqual("141. findNextObjectAtWrap", t.findNextObjectAtWrap(B, 0, false), 3);
    a.checkEqual("142. findNextObjectAtWrap", t.findNextObjectAtWrap(B, 1, false), 3);
    a.checkEqual("143. findNextObjectAtWrap", t.findNextObjectAtWrap(B, 3, false), 4);
    a.checkEqual("144. findNextObjectAtWrap", t.findNextObjectAtWrap(B, 5, false), 3);

    a.checkEqual("151. findNextObjectAtWrap", t.findNextObjectAtWrap(B, 0, true), 5);
    a.checkEqual("152. findNextObjectAtWrap", t.findNextObjectAtWrap(B, 1, true), 5);
    a.checkEqual("153. findNextObjectAtWrap", t.findNextObjectAtWrap(B, 3, true), 5);
    a.checkEqual("154. findNextObjectAtWrap", t.findNextObjectAtWrap(B, 5, true), 5);

    // findPreviousObjectAtWrap
    a.checkEqual("161. findPreviousObjectAtWrap", t.findPreviousObjectAtWrap(B, 0, false), 5);
    a.checkEqual("162. findPreviousObjectAtWrap", t.findPreviousObjectAtWrap(B, 1, false), 5);
    a.checkEqual("163. findPreviousObjectAtWrap", t.findPreviousObjectAtWrap(B, 3, false), 5);
    a.checkEqual("164. findPreviousObjectAtWrap", t.findPreviousObjectAtWrap(B, 4, false), 3);

    a.checkEqual("171. findPreviousObjectAtWrap", t.findPreviousObjectAtWrap(B, 0, true), 5);
    a.checkEqual("172. findPreviousObjectAtWrap", t.findPreviousObjectAtWrap(B, 3, true), 5);
    a.checkEqual("173. findPreviousObjectAtWrap", t.findPreviousObjectAtWrap(B, 5, true), 5);
    a.checkEqual("174. findPreviousObjectAtWrap", t.findPreviousObjectAtWrap(B, 7, true), 5);

    // findIndexForId
    a.checkEqual("181. findIndexForId", t.findIndexForId(500), 5);
    a.checkEqual("182. findIndexForId", t.findIndexForId(499), 0);

    // findIndexForObject
    a.checkNonNull("191. getObjectByIndex", t.getObjectByIndex(3));
    a.checkEqual("192. getObjectByIndex", t.getObjectByIndex(3)->getId(), 300);
    a.checkEqual("193. findIndexForObject", t.findIndexForObject(t.getObjectByIndex(3)), 3);
    a.checkEqual("194. findIndexForObject", t.findIndexForObject(0), 0);

    TestObject alien(88, 8, Point());
    a.checkEqual("201. findIndexForObject", t.findIndexForObject(&alien), 0);

    // Filters
    afl::base::Deleter del;
    a.checkEqual("211. filterPosition", t.filterPosition(del, A).countObjects(), 4);
    a.checkEqual("212. filterPosition", t.filterPosition(del, B).countObjects(), 3);
    a.checkEqual("213. filterMarked", t.filterMarked(del, true).countObjects(), 4);
    a.checkEqual("214. filterMarked", t.filterMarked(del, false).countObjects(), 7);
    a.checkEqual("215. filterOwner", t.filterOwner(del, game::PlayerSet_t(1)).countObjects(), 3);
    a.checkEqual("216. filterOwner", t.filterOwner(del, game::PlayerSet_t(2)).countObjects(), 4);
    a.checkEqual("217. filterOwner", t.filterOwner(del, game::PlayerSet_t(3)).countObjects(), 0);
    a.checkEqual("218. filterOwner", t.filterOwner(del, game::PlayerSet_t() + 1 + 2).countObjects(), 7);
}

/** Test handling partial information (no position, no owner). */
AFL_TEST("game.map.ObjectType:partial-information", a)
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

    a.checkEqual("01. countObjects", t.countObjects(), 7);

    // Counting will skip 2 object
    a.checkEqual("11. countObjects", t.countObjectsAt(A, PlayerSet_t::allUpTo(20)), 5);

    // Finding at position will report ownerless object, but skip positionless
    a.checkEqual("21. findNextObjectAt", t.findNextObjectAt(A, 2, false), 3);
    a.checkEqual("22. findNextObjectAt", t.findNextObjectAt(A, 4, false), 6);
}

/** Test findNearestIndex(). */
AFL_TEST("game.map.ObjectType:findNearestIndex", a)
{
    TestType t;
    t.addObject(100, 1, Point(1000, 1000));
    t.addObject(200, 1, Point(1000, 2000));
    t.addObject(300, 1, Point(1000, 1500));
    t.addObject(400, 1, Point(1500, 1500));

    Configuration config;
    a.checkEqual("01. findNearestIndex", t.findNearestIndex(Point(1010, 1010), config), 1);
    a.checkEqual("02. findNearestIndex", t.findNearestIndex(Point(1400, 1400), config), 4);
    a.checkEqual("03. findNearestIndex", t.findNearestIndex(Point(500, 1500), config), 3);
}

/** Test sort(). */
AFL_TEST("game.map.ObjectType:sort", a)
{
    // Similar situation as in testNormal.
    // Use duplicate Ids to exercise tie-breaking, because SortBy::Id has no further dependencies.
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
    game::ref::SortBy::Id pred;
    game::map::ObjectType& sorted = t.sort(del, pred, game::Reference::Ship);
    a.checkEqual("01. findNextIndexNoWrap", sorted.findNextIndexNoWrap(0), 1);
    a.checkEqual("02. findNextIndexNoWrap", sorted.findNextIndexNoWrap(1), 2);
    a.checkEqual("03. findNextIndexNoWrap", sorted.findNextIndexNoWrap(2), 4);
    a.checkEqual("04. findNextIndexNoWrap", sorted.findNextIndexNoWrap(4), 3);
    a.checkEqual("05. findNextIndexNoWrap", sorted.findNextIndexNoWrap(3), 5);
    a.checkEqual("06. findNextIndexNoWrap", sorted.findNextIndexNoWrap(5), 6);
    a.checkEqual("07. findNextIndexNoWrap", sorted.findNextIndexNoWrap(6), 7);
    a.checkEqual("08. findNextIndexNoWrap", sorted.findNextIndexNoWrap(7), 0);

    a.checkEqual("11. findPreviousIndexNoWrap", sorted.findPreviousIndexNoWrap(0), 7);
    a.checkEqual("12. findPreviousIndexNoWrap", sorted.findPreviousIndexNoWrap(7), 6);
    a.checkEqual("13. findPreviousIndexNoWrap", sorted.findPreviousIndexNoWrap(6), 5);
    a.checkEqual("14. findPreviousIndexNoWrap", sorted.findPreviousIndexNoWrap(5), 3);
    a.checkEqual("15. findPreviousIndexNoWrap", sorted.findPreviousIndexNoWrap(3), 4);
    a.checkEqual("16. findPreviousIndexNoWrap", sorted.findPreviousIndexNoWrap(4), 2);
    a.checkEqual("17. findPreviousIndexNoWrap", sorted.findPreviousIndexNoWrap(2), 1);
    a.checkEqual("18. findPreviousIndexNoWrap", sorted.findPreviousIndexNoWrap(1), 0);

    // Test further processing the sorted result (not recommended but possible)
    a.checkEqual("21. countObjects", sorted.countObjects(), 7);
    a.checkEqual("22. filterMarked", sorted.filterMarked(del, true).countObjects(), 4);
}
