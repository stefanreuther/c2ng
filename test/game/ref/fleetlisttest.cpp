/**
  *  \file test/game/ref/fleetlisttest.cpp
  *  \brief Test for game::ref::FleetList
  */

#include "game/ref/fleetlist.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/ref/sortpredicate.hpp"

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
AFL_TEST("game.ref.FleetList:basics", a)
{
    FleetList testee;

    // Comparing tro fresh objects
    a.checkEqual("01. eq", testee == FleetList(), true);
    a.checkEqual("02. ne", testee != FleetList(), false);

    // Verify initial state
    a.checkEqual("11. empty", testee.empty(), true);
    a.checkEqual("12. size", testee.size(), 0U);
    a.checkNull("13. get", testee.get(0));
    a.checkEqual("14. findInitialSelection", testee.findInitialSelection(), 0U);

    // Add some stuff; verify access
    testee.add(makeItem("i1", 7,  true));
    testee.add(makeItem("i2", 99, false));
    a.checkEqual("21. eq", testee == FleetList(), false);
    a.checkEqual("22. ne", testee != FleetList(), true);

    a.checkEqual("31. empty", testee.empty(), false);
    a.checkEqual("32. size", testee.size(), 2U);
    a.checkNonNull("33. get", testee.get(0));
    a.checkEqual("34. findInitialSelection", testee.findInitialSelection(), 0U);
    a.checkEqual("35. isAtReferenceLocation", testee.get(0)->isAtReferenceLocation, true);
    a.checkEqual("36. name", testee.get(0)->name, "i1");

    a.checkEqual("41. eq", *testee.get(0) == *testee.get(0), true);
    a.checkEqual("42. eq", *testee.get(0) == *testee.get(1), false);
    a.checkEqual("43. ne", *testee.get(0) != *testee.get(0), false);
    a.checkEqual("44. ne", *testee.get(0) != *testee.get(1), true);

    // Verify find: unsuccessfully
    {
        size_t pos = 9999;
        a.checkEqual("51. find", testee.find(Reference(Reference::Ship, 66)).get(pos), false);
    }

    // Verify find: successfully
    {
        size_t pos = 9999;
        a.checkEqual("61. find", testee.find(Reference(Reference::Ship, 99)).get(pos), true);
        a.checkEqual("62. pos", pos, 1U);
    }

    // Clear; verify state
    testee.clear();
    a.checkEqual("71. empty", testee.empty(), true);
    a.checkEqual("72. size", testee.size(), 0U);
    a.checkNull("73. get", testee.get(0));
}

/** Test sort(), FleetList predicate. */
AFL_TEST("game.ref.FleetList:sort:fleet-predicate", a)
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
    a.checkEqual("01. size", testee.size(), 7U);
    a.checkEqual("02. name", testee.get(0)->name, "elsewhere");
    a.checkEqual("03. name", testee.get(1)->name, "i1");
    a.checkEqual("04. name", testee.get(2)->name, "i4");
    a.checkEqual("05. name", testee.get(3)->name, "i3");
    a.checkEqual("06. name", testee.get(4)->name, "here");
    a.checkEqual("07. name", testee.get(5)->name, "i5");
    a.checkEqual("08. name", testee.get(6)->name, "i2");
    a.checkEqual("09. findInitialSelection", testee.findInitialSelection(), 5U);

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
    a.checkEqual("11. size", testee.size(), 5U);
    a.checkEqual("12. name", testee.get(0)->name, "i1");
    a.checkEqual("13. name", testee.get(1)->name, "i4");
    a.checkEqual("14. name", testee.get(2)->name, "i3");
    a.checkEqual("15. name", testee.get(3)->name, "i5");
    a.checkEqual("16. name", testee.get(4)->name, "i2");
    a.checkEqual("17. findInitialSelection", testee.findInitialSelection(), 3U);
}

/** Test sort(), game::ref::SortPredicate. */
AFL_TEST("game.ref.FleetList:sort:plain-predicate", a)
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
    a.checkEqual("01. size", testee.size(), 5U);
    a.checkEqual("02. name", testee.get(0)->name, "i1");
    a.checkEqual("03. name", testee.get(1)->name, "i5");
    a.checkEqual("04. name", testee.get(2)->name, "i4");
    a.checkEqual("05. name", testee.get(3)->name, "i2");
    a.checkEqual("06. name", testee.get(4)->name, "i3");
    a.checkEqual("07. findInitialSelection", testee.findInitialSelection(), 1U);
}

/** Test addAll(). */
AFL_TEST("game.ref.FleetList:addAll", a)
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
        a.checkEqual("01. size", t.size(), 0U);
    }

    // Everything
    {
        FleetList t;
        t.addAll(univ, afl::base::Nothing, 0, true, tx);
        a.checkEqual("11. size",                  t.size(), 5U);
        a.checkEqual("12. isAtReferenceLocation", t.get(0)->isAtReferenceLocation, false);
        a.checkEqual("13. reference",             t.get(0)->reference.getId(), 1);
        a.checkEqual("14. name",                  t.get(0)->name, "Fleet 1: led by s1");
        a.checkEqual("15. isAtReferenceLocation", t.get(2)->isAtReferenceLocation, false);
        a.checkEqual("16. reference",             t.get(2)->reference.getId(), 5);
        a.checkEqual("17. name",                  t.get(2)->name, "Fleet 5: five");
        a.checkEqual("18. isAtReferenceLocation", t.get(4)->isAtReferenceLocation, false);
        a.checkEqual("19. reference",             t.get(4)->reference.getId(), 9);
        a.checkEqual("20. name",                  t.get(4)->name, "Fleet 9: led by s9");
        a.checkEqual("21. findInitialSelection",  t.findInitialSelection(), 0U);
    }

    // Location filter
    {
        FleetList t;
        t.addAll(univ, game::map::Point(1000, 1200), 0, false, tx);
        a.checkEqual("31. size",                  t.size(), 4U);
        a.checkEqual("32. isAtReferenceLocation", t.get(0)->isAtReferenceLocation, true);
        a.checkEqual("33. reference",             t.get(0)->reference.getId(), 1);
        a.checkEqual("34. name",                  t.get(0)->name, "Fleet 1: led by s1");
        a.checkEqual("35. isAtReferenceLocation", t.get(3)->isAtReferenceLocation, true);
        a.checkEqual("36. reference",             t.get(3)->reference.getId(), 9);
        a.checkEqual("37. name",                  t.get(3)->name, "Fleet 9: led by s9");
        a.checkEqual("38. findInitialSelection",  t.findInitialSelection(), 0U);
    }

    // Everything, with reference location
    {
        FleetList t;
        t.addAll(univ, game::map::Point(1000, 1200), 0, true, tx);
        a.checkEqual("41. size", t.size(), 5U);
        a.checkEqual("42. isAtReferenceLocation", t.get(0)->isAtReferenceLocation, true);
        a.checkEqual("43. reference",             t.get(0)->reference.getId(), 1);
        a.checkEqual("44. name",                  t.get(0)->name, "Fleet 1: led by s1");
        a.checkEqual("45. isAtReferenceLocation", t.get(2)->isAtReferenceLocation, true);
        a.checkEqual("46. reference",             t.get(2)->reference.getId(), 5);
        a.checkEqual("47. name",                  t.get(2)->name, "Fleet 5: five");
        a.checkEqual("48. isAtReferenceLocation", t.get(3)->isAtReferenceLocation, false);
        a.checkEqual("49. reference",             t.get(3)->reference.getId(), 7);
        a.checkEqual("50. name",                  t.get(3)->name, "Fleet 7: led by s7");
        a.checkEqual("51. isAtReferenceLocation", t.get(4)->isAtReferenceLocation, true);
        a.checkEqual("52. reference",             t.get(4)->reference.getId(), 9);
        a.checkEqual("53. name",                  t.get(4)->name, "Fleet 9: led by s9");
        a.checkEqual("54. findInitialSelection",  t.findInitialSelection(), 0U);
    }

    // Location filter, except
    {
        FleetList t;
        t.addAll(univ, game::map::Point(1000, 1200), 1, false, tx);
        a.checkEqual("61. size",                  t.size(), 3U);
        a.checkEqual("62. isAtReferenceLocation", t.get(0)->isAtReferenceLocation, true);
        a.checkEqual("63. reference",             t.get(0)->reference.getId(), 3);
        a.checkEqual("64. name",                  t.get(0)->name, "Fleet 3: led by s3");
        a.checkEqual("65. isAtReferenceLocation", t.get(2)->isAtReferenceLocation, true);
        a.checkEqual("66. reference",             t.get(2)->reference.getId(), 9);
        a.checkEqual("67. name",                  t.get(2)->name, "Fleet 9: led by s9");
        a.checkEqual("68. findInitialSelection",  t.findInitialSelection(), 0U);
    }
}
