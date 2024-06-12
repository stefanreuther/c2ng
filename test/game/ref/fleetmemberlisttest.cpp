/**
  *  \file test/game/ref/fleetmemberlisttest.cpp
  *  \brief Test for game::ref::FleetMemberList
  */

#include "game/ref/fleetmemberlist.hpp"

#include "afl/test/testrunner.hpp"
#include "game/ref/sortpredicate.hpp"

using game::ref::FleetMemberList;
using game::ref::UserList;
using game::Reference;

namespace {
    FleetMemberList::Item makeItem(String_t name, int shipId, String_t friendlyCode, FleetMemberList::Flags_t flags, int x, int y)
    {
        return FleetMemberList::Item(UserList::Item(UserList::ReferenceItem,
                                                    name,
                                                    Reference(Reference::Ship, shipId),
                                                    true,
                                                    game::map::Object::ReadOnly,
                                                    util::SkinColor::Yellow),
                                     flags,
                                     friendlyCode,
                                     game::map::Point(x, y));
    }

    game::map::Ship& addShip(game::map::Universe& univ, game::Id_t id, String_t name, String_t friendlyCode, int x, int y, int fleetNumber)
    {
        game::map::Ship& sh = *univ.ships().create(id);
        game::map::ShipData sd;
        sd.x = x;
        sd.y = y;
        sd.owner = 1;
        sd.name = name;
        sh.addCurrentShipData(sd, game::PlayerSet_t(1));
        sh.setName(name);
        sh.setFleetNumber(fleetNumber);
        sh.setPlayability(game::map::Object::Playable);
        sh.setFriendlyCode(friendlyCode);
        sh.internalCheck(game::PlayerSet_t(1), 15);
        return sh;
    }
}

/** Basic functionality test. */
AFL_TEST("game.ref.FleetMemberList:basics", a)
{
    FleetMemberList testee;

    // Comparing tro fresh objects
    a.checkEqual("01. eq", testee == FleetMemberList(), true);
    a.checkEqual("02. ne", testee != FleetMemberList(), false);

    // Verify initial state
    a.checkEqual("11. empty", testee.empty(), true);
    a.checkEqual("12. size", testee.size(), 0U);
    a.checkNull("13. get", testee.get(0));

    // Add some stuff; verify access
    testee.add(makeItem("i1", 7,  "abc", FleetMemberList::Flags_t(FleetMemberList::Leader), 1000, 2000));
    testee.add(makeItem("i2", 99, "xyz", FleetMemberList::Flags_t(),                        2200, 2400));
    a.checkEqual("21. eq", testee == FleetMemberList(), false);
    a.checkEqual("22. ne", testee != FleetMemberList(), true);

    a.checkEqual("31. empty", testee.empty(), false);
    a.checkEqual("32. size", testee.size(), 2U);
    a.checkNonNull("33. get", testee.get(0));
    a.checkEqual("34. friendlyCode", testee.get(0)->friendlyCode, "abc");
    a.checkEqual("35. flags", testee.get(0)->flags, FleetMemberList::Flags_t(FleetMemberList::Leader));
    a.checkEqual("36. name", testee.get(0)->name, "i1");
    a.checkEqual("37. X", testee.get(0)->position.getX(), 1000);
    a.checkEqual("38. Y", testee.get(0)->position.getY(), 2000);

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

/** Test sort(), FleetMemberList predicate. */
AFL_TEST("game.ref.FleetMemberList:sort:fleet-member-predicate", a)
{
    // Prepare data
    FleetMemberList testee;
    testee.add(makeItem("i1", 1, "abc", FleetMemberList::Flags_t(), 1200, 1100));
    testee.add(makeItem("i2", 7, "xxx", FleetMemberList::Flags_t(), 1200, 1100));
    testee.add(makeItem("i3", 9, "abc", FleetMemberList::Flags_t(), 1200, 1100));
    testee.add(makeItem("i4", 5, "yyy", FleetMemberList::Flags_t(), 1200, 1100));
    testee.add(makeItem("i5", 3, "abc", FleetMemberList::Flags_t(), 1200, 1100));

    // Sort by friendly-code
    class Pred : public FleetMemberList::SortPredicate {
     public:
        virtual int compare(const FleetMemberList::Item& a, const FleetMemberList::Item& b) const
            { return afl::string::strCaseCompare(a.friendlyCode, b.friendlyCode); }
        virtual String_t getClass(const FleetMemberList::Item& a) const
            { return a.friendlyCode; }
    };
    testee.sort(Pred());

    // Verify
    a.checkEqual("01. size", testee.size(), 8U);
    a.checkEqual("02. name", testee.get(0)->name, "abc");
    a.checkEqual("03. name", testee.get(1)->name, "i1");
    a.checkEqual("04. name", testee.get(2)->name, "i5");  // note sort by Id!
    a.checkEqual("05. name", testee.get(3)->name, "i3");
    a.checkEqual("06. name", testee.get(4)->name, "xxx");
    a.checkEqual("07. name", testee.get(5)->name, "i2");
    a.checkEqual("08. name", testee.get(6)->name, "yyy");
    a.checkEqual("09. name", testee.get(7)->name, "i4");

    // Sort again, without dividers
    class Pred2 : public FleetMemberList::SortPredicate {
     public:
        virtual int compare(const FleetMemberList::Item& a, const FleetMemberList::Item& b) const
            { return afl::string::strCaseCompare(a.friendlyCode, b.friendlyCode); }
        virtual String_t getClass(const FleetMemberList::Item& /*a*/) const
            { return String_t(); }
    };
    testee.sort(Pred2());

    // Verify
    a.checkEqual("11. size", testee.size(), 5U);
    a.checkEqual("12. name", testee.get(0)->name, "i1");
    a.checkEqual("13. name", testee.get(1)->name, "i5");
    a.checkEqual("14. name", testee.get(2)->name, "i3");
    a.checkEqual("15. name", testee.get(3)->name, "i2");
    a.checkEqual("16. name", testee.get(4)->name, "i4");
}

/** Test sort(), game::ref::SortPredicate. */
AFL_TEST("game.ref.FleetMemberList:sort:plain-predicate", a)
{
    // Prepare data
    FleetMemberList testee;
    testee.add(makeItem("i1", 1, "xyz", FleetMemberList::Flags_t(), 1200, 1100));
    testee.add(makeItem("i2", 7, "xyz", FleetMemberList::Flags_t(), 1200, 1100));
    testee.add(makeItem("i3", 9, "xyz", FleetMemberList::Flags_t(), 1200, 1100));
    testee.add(makeItem("i4", 5, "xyz", FleetMemberList::Flags_t(), 1200, 1100));
    testee.add(makeItem("i5", 3, "xyz", FleetMemberList::Flags_t(), 1200, 1100));

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
}

/** Test setFleet(). */
AFL_TEST("game.ref.FleetMemberList:setFleet", a)
{
    game::map::Universe univ;
    addShip(univ, 1,  "s1", "one",  1000, 1200, 0);
    addShip(univ, 3,  "s3", "thr",  1000, 1200, 3);
    addShip(univ, 5,  "s5", "fiv",  1000, 1200, 9);
    addShip(univ, 7,  "s7", "sev",  2000, 1200, 3);
    addShip(univ, 9,  "s9", "nin",  1000, 1200, 9);
    addShip(univ, 11, "s11", "ele", 1000, 1200, 9);

    // Load fleet #3 (ships 3+7)
    FleetMemberList testee;
    testee.setFleet(univ, 3);
    a.checkEqual("01. size",         testee.size(), 2U);
    a.checkEqual("02. name",         testee.get(0)->name, "s3");
    a.checkEqual("03. friendlyCode", testee.get(0)->friendlyCode, "thr");
    a.checkEqual("04. flags",        testee.get(0)->flags, FleetMemberList::Flags_t(FleetMemberList::Leader));
    a.checkEqual("05. name",         testee.get(1)->name, "s7");
    a.checkEqual("06. friendlyCode", testee.get(1)->friendlyCode, "sev");
    a.checkEqual("07. flags",        testee.get(1)->flags, FleetMemberList::Flags_t(FleetMemberList::Away));

    // Load fleet 9 (ships 9+5+11)
    testee.setFleet(univ, 9);
    a.checkEqual("11. size",         testee.size(), 3U);
    a.checkEqual("12. name",         testee.get(0)->name, "s9");
    a.checkEqual("13. friendlyCode", testee.get(0)->friendlyCode, "nin");
    a.checkEqual("14. flags",        testee.get(0)->flags, FleetMemberList::Flags_t(FleetMemberList::Leader));
    a.checkEqual("15. name",         testee.get(1)->name, "s5");
    a.checkEqual("16. friendlyCode", testee.get(1)->friendlyCode, "fiv");
    a.checkEqual("17. flags",        testee.get(1)->flags, FleetMemberList::Flags_t());
    a.checkEqual("18. name",         testee.get(2)->name, "s11");
    a.checkEqual("19. friendlyCode", testee.get(2)->friendlyCode, "ele");
    a.checkEqual("20. flags",        testee.get(2)->flags, FleetMemberList::Flags_t());

    // Load single ship [border usecase]
    testee.setFleet(univ, 1);
    a.checkEqual("21. size",         testee.size(), 1U);
    a.checkEqual("22. name",         testee.get(0)->name, "s1");
    a.checkEqual("23. friendlyCode", testee.get(0)->friendlyCode, "one");
    a.checkEqual("24. flags",        testee.get(0)->flags, FleetMemberList::Flags_t(FleetMemberList::Leader));

    // Load nonexistant ship [border usecase]
    testee.setFleet(univ, 0);
    a.checkEqual("31. size", testee.size(), 0U);
}

/** Test setFleet(), with towing. */
AFL_TEST("game.ref.FleetMemberList:setFleet:tow", a)
{
    game::map::Universe univ;
    addShip(univ, 1,  "s1", "one",  1000, 1200, 5).setMission(game::spec::Mission::msn_Tow, 0, 5);
    addShip(univ, 3,  "s3", "thr",  1000, 1200, 5).setMission(game::spec::Mission::msn_Tow, 0, 2); // tow non-member
    addShip(univ, 5,  "s5", "fiv",  1000, 1200, 5);
    addShip(univ, 7,  "s7", "sev",  2000, 1200, 5).setMission(game::spec::Mission::msn_Tow, 0, 9);
    addShip(univ, 9,  "s9", "nin",  1000, 1200, 5);

    // Load fleet 5 (ships 5+1+3+7+9)
    FleetMemberList testee;
    testee.setFleet(univ, 5);
    a.checkEqual("01. size",  testee.size(), 5U);
    a.checkEqual("02. name",  testee.get(0)->name, "s5");
    a.checkEqual("03. flags", testee.get(0)->flags, FleetMemberList::Flags_t() + FleetMemberList::Leader + FleetMemberList::Towed);
    a.checkEqual("04. name",  testee.get(1)->name, "s1");
    a.checkEqual("05. flags", testee.get(1)->flags, FleetMemberList::Flags_t() + FleetMemberList::Towing);
    a.checkEqual("06. name",  testee.get(2)->name, "s3");
    a.checkEqual("07. flags", testee.get(2)->flags, FleetMemberList::Flags_t());
    a.checkEqual("08. name",  testee.get(3)->name, "s7");
    a.checkEqual("09. flags", testee.get(3)->flags, FleetMemberList::Flags_t() + FleetMemberList::Towing + FleetMemberList::Away);
    a.checkEqual("10. name",  testee.get(4)->name, "s9");
    a.checkEqual("11. flags", testee.get(4)->flags, FleetMemberList::Flags_t() + FleetMemberList::Towed);
}
