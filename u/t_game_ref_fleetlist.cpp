/**
  *  \file u/t_game_ref_fleetlist.cpp
  *  \brief Test for game::ref::FleetList
  */

#include "game/ref/fleetlist.hpp"

#include "t_game_ref.hpp"
#include "game/ref/sortpredicate.hpp"
#include "afl/string/nulltranslator.hpp"

using game::ref::FleetList;
using game::ref::UserList;
using game::Reference;

namespace {
    FleetList::Item makeItem(String_t name, int shipId, bool here)
    {
        return FleetList::Item(UserList::Item(UserList::ReferenceItem,
                                              name,
                                              Reference(Reference::Ship, shipId),
                                              true,
                                              game::map::Object::ReadOnly,
                                              util::SkinColor::Yellow),
                               here);
    }

    game::map::Ship& addShip(game::map::Universe& univ, game::Id_t id, String_t name, String_t fleetName, int x, int y)
    {
        game::map::Ship& sh = *univ.ships().create(id);
        sh.setName(name);
        sh.setFleetName(fleetName);
        sh.setFleetNumber(id);
        sh.setOwner(1);
        sh.setPlayability(game::map::Object::Playable);
        sh.setPosition(game::map::Point(x, y));
        return sh;
    }
}

/** Basic functionality test. */
void
TestGameRefFleetList::testBasic()
{
    FleetList testee;

    // Comparing tro fresh objects
    TS_ASSERT_EQUALS(testee == FleetList(), true);
    TS_ASSERT_EQUALS(testee != FleetList(), false);

    // Verify initial state
    TS_ASSERT_EQUALS(testee.empty(), true);
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT(testee.get(0) == 0);
    TS_ASSERT_EQUALS(testee.findInitialSelection(), 0U);

    // Add some stuff; verify access
    testee.add(makeItem("i1", 7,  true));
    testee.add(makeItem("i2", 99, false));
    TS_ASSERT_EQUALS(testee == FleetList(), false);
    TS_ASSERT_EQUALS(testee != FleetList(), true);

    TS_ASSERT_EQUALS(testee.empty(), false);
    TS_ASSERT_EQUALS(testee.size(), 2U);
    TS_ASSERT(testee.get(0) != 0);
    TS_ASSERT_EQUALS(testee.findInitialSelection(), 0U);
    TS_ASSERT_EQUALS(testee.get(0)->isAtReferenceLocation, true);
    TS_ASSERT_EQUALS(testee.get(0)->name, "i1");

    TS_ASSERT_EQUALS(*testee.get(0) == *testee.get(0), true);
    TS_ASSERT_EQUALS(*testee.get(0) == *testee.get(1), false);
    TS_ASSERT_EQUALS(*testee.get(0) != *testee.get(0), false);
    TS_ASSERT_EQUALS(*testee.get(0) != *testee.get(1), true);

    // Verify find: unsuccessfully
    {
        size_t pos = 9999;
        TS_ASSERT_EQUALS(testee.find(Reference(Reference::Ship, 66)).get(pos), false);
    }

    // Verify find: successfully
    {
        size_t pos = 9999;
        TS_ASSERT_EQUALS(testee.find(Reference(Reference::Ship, 99)).get(pos), true);
        TS_ASSERT_EQUALS(pos, 1U);
    }

    // Clear; verify state
    testee.clear();
    TS_ASSERT_EQUALS(testee.empty(), true);
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT(testee.get(0) == 0);
}

/** Test sort(), FleetList predicate. */
void
TestGameRefFleetList::testSort1()
{
    // Prepare data
    FleetList testee;
    testee.add(makeItem("i1", 1, false));
    testee.add(makeItem("i2", 7, true));
    testee.add(makeItem("i3", 9, false));
    testee.add(makeItem("i4", 5, false));
    testee.add(makeItem("i5", 3, true));

    // Sort by "is-here"
    class Pred : public FleetList::SortPredicate {
     public:
        virtual int compare(const FleetList::Item& a, const FleetList::Item& b) const
            { return a.isAtReferenceLocation - b.isAtReferenceLocation; }
        virtual String_t getClass(const FleetList::Item& a) const
            { return a.isAtReferenceLocation ? "here" : "elsewhere"; }
    };
    testee.sort(Pred());

    // Verify
    TS_ASSERT_EQUALS(testee.size(), 7U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "elsewhere");
    TS_ASSERT_EQUALS(testee.get(1)->name, "i1");
    TS_ASSERT_EQUALS(testee.get(2)->name, "i4");
    TS_ASSERT_EQUALS(testee.get(3)->name, "i3");
    TS_ASSERT_EQUALS(testee.get(4)->name, "here");
    TS_ASSERT_EQUALS(testee.get(5)->name, "i5");
    TS_ASSERT_EQUALS(testee.get(6)->name, "i2");
    TS_ASSERT_EQUALS(testee.findInitialSelection(), 5U);

    // Sort again, without dividers
    class Pred2 : public FleetList::SortPredicate {
     public:
        virtual int compare(const FleetList::Item& a, const FleetList::Item& b) const
            { return a.isAtReferenceLocation - b.isAtReferenceLocation; }
        virtual String_t getClass(const FleetList::Item& /*a*/) const
            { return String_t(); }
    };
    testee.sort(Pred2());

    // Verify
    TS_ASSERT_EQUALS(testee.size(), 5U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "i1");
    TS_ASSERT_EQUALS(testee.get(1)->name, "i4");
    TS_ASSERT_EQUALS(testee.get(2)->name, "i3");
    TS_ASSERT_EQUALS(testee.get(3)->name, "i5");
    TS_ASSERT_EQUALS(testee.get(4)->name, "i2");
    TS_ASSERT_EQUALS(testee.findInitialSelection(), 3U);
}

/** Test sort(), game::ref::SortPredicate. */
void
TestGameRefFleetList::testSort2()
{
    // Prepare data
    FleetList testee;
    testee.add(makeItem("i1", 1, false));
    testee.add(makeItem("i2", 7, true));
    testee.add(makeItem("i3", 9, false));
    testee.add(makeItem("i4", 5, false));
    testee.add(makeItem("i5", 3, true));

    // Sort by ship Id
    class Pred : public game::ref::SortPredicate {
     public:
        virtual int compare(const Reference& a, const Reference& b) const
            { return a.getId() - b.getId(); }
        virtual String_t getClass(const Reference& /*a*/) const
            { return String_t(); }
    };
    testee.sort(Pred());

    // Verify
    TS_ASSERT_EQUALS(testee.size(), 5U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "i1");
    TS_ASSERT_EQUALS(testee.get(1)->name, "i5");
    TS_ASSERT_EQUALS(testee.get(2)->name, "i4");
    TS_ASSERT_EQUALS(testee.get(3)->name, "i2");
    TS_ASSERT_EQUALS(testee.get(4)->name, "i3");
    TS_ASSERT_EQUALS(testee.findInitialSelection(), 1U);
}

/** Test addAll(). */
void
TestGameRefFleetList::testAdd()
{
    game::map::Universe univ;
    addShip(univ, 1, "s1", "",     1000, 1200);
    addShip(univ, 3, "s3", "",     1000, 1200);
    addShip(univ, 5, "s5", "five", 1000, 1200);
    addShip(univ, 7, "s7", "",     2000, 1200);
    addShip(univ, 9, "s9", "",     1000, 1200);

    afl::string::NullTranslator tx;

    // Nothing
    {
        FleetList t;
        t.addAll(univ, afl::base::Nothing, 0, false, tx);
        TS_ASSERT_EQUALS(t.size(), 0U);
    }

    // Everything
    {
        FleetList t;
        t.addAll(univ, afl::base::Nothing, 0, true, tx);
        TS_ASSERT_EQUALS(t.size(), 5U);
        TS_ASSERT_EQUALS(t.get(0)->isAtReferenceLocation, false);
        TS_ASSERT_EQUALS(t.get(0)->reference.getId(), 1);
        TS_ASSERT_EQUALS(t.get(0)->name, "Fleet 1: led by s1");
        TS_ASSERT_EQUALS(t.get(2)->isAtReferenceLocation, false);
        TS_ASSERT_EQUALS(t.get(2)->reference.getId(), 5);
        TS_ASSERT_EQUALS(t.get(2)->name, "Fleet 5: five");
        TS_ASSERT_EQUALS(t.get(4)->isAtReferenceLocation, false);
        TS_ASSERT_EQUALS(t.get(4)->reference.getId(), 9);
        TS_ASSERT_EQUALS(t.get(4)->name, "Fleet 9: led by s9");
        TS_ASSERT_EQUALS(t.findInitialSelection(), 0U);
    }

    // Location filter
    {
        FleetList t;
        t.addAll(univ, game::map::Point(1000, 1200), 0, false, tx);
        TS_ASSERT_EQUALS(t.size(), 4U);
        TS_ASSERT_EQUALS(t.get(0)->isAtReferenceLocation, true);
        TS_ASSERT_EQUALS(t.get(0)->reference.getId(), 1);
        TS_ASSERT_EQUALS(t.get(0)->name, "Fleet 1: led by s1");
        TS_ASSERT_EQUALS(t.get(3)->isAtReferenceLocation, true);
        TS_ASSERT_EQUALS(t.get(3)->reference.getId(), 9);
        TS_ASSERT_EQUALS(t.get(3)->name, "Fleet 9: led by s9");
        TS_ASSERT_EQUALS(t.findInitialSelection(), 0U);
    }

    // Everything, with reference location
    {
        FleetList t;
        t.addAll(univ, game::map::Point(1000, 1200), 0, true, tx);
        TS_ASSERT_EQUALS(t.size(), 5U);
        TS_ASSERT_EQUALS(t.get(0)->isAtReferenceLocation, true);
        TS_ASSERT_EQUALS(t.get(0)->reference.getId(), 1);
        TS_ASSERT_EQUALS(t.get(0)->name, "Fleet 1: led by s1");
        TS_ASSERT_EQUALS(t.get(2)->isAtReferenceLocation, true);
        TS_ASSERT_EQUALS(t.get(2)->reference.getId(), 5);
        TS_ASSERT_EQUALS(t.get(2)->name, "Fleet 5: five");
        TS_ASSERT_EQUALS(t.get(3)->isAtReferenceLocation, false);
        TS_ASSERT_EQUALS(t.get(3)->reference.getId(), 7);
        TS_ASSERT_EQUALS(t.get(3)->name, "Fleet 7: led by s7");
        TS_ASSERT_EQUALS(t.get(4)->isAtReferenceLocation, true);
        TS_ASSERT_EQUALS(t.get(4)->reference.getId(), 9);
        TS_ASSERT_EQUALS(t.get(4)->name, "Fleet 9: led by s9");
        TS_ASSERT_EQUALS(t.findInitialSelection(), 0U);
    }

    // Location filter, except
    {
        FleetList t;
        t.addAll(univ, game::map::Point(1000, 1200), 1, false, tx);
        TS_ASSERT_EQUALS(t.size(), 3U);
        TS_ASSERT_EQUALS(t.get(0)->isAtReferenceLocation, true);
        TS_ASSERT_EQUALS(t.get(0)->reference.getId(), 3);
        TS_ASSERT_EQUALS(t.get(0)->name, "Fleet 3: led by s3");
        TS_ASSERT_EQUALS(t.get(2)->isAtReferenceLocation, true);
        TS_ASSERT_EQUALS(t.get(2)->reference.getId(), 9);
        TS_ASSERT_EQUALS(t.get(2)->name, "Fleet 9: led by s9");
        TS_ASSERT_EQUALS(t.findInitialSelection(), 0U);
    }
}

