/**
  *  \file u/t_game_ref_fleetmemberlist.cpp
  *  \brief Test for game::ref::FleetMemberList
  */

#include "game/ref/fleetmemberlist.hpp"

#include "t_game_ref.hpp"
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
        sh.addShipXYData(game::map::Point(x, y), 1, 1000, game::PlayerSet_t(1));
        sh.setName(name);
        sh.setFleetNumber(fleetNumber);
        sh.setPlayability(game::map::Object::Playable);
        sh.setFriendlyCode(friendlyCode);
        sh.internalCheck();
        return sh;
    }
}

/** Basic functionality test. */
void
TestGameRefFleetMemberList::testBasic()
{
    FleetMemberList testee;

    // Comparing tro fresh objects
    TS_ASSERT_EQUALS(testee == FleetMemberList(), true);
    TS_ASSERT_EQUALS(testee != FleetMemberList(), false);

    // Verify initial state
    TS_ASSERT_EQUALS(testee.empty(), true);
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT(testee.get(0) == 0);

    // Add some stuff; verify access
    testee.add(makeItem("i1", 7,  "abc", FleetMemberList::Flags_t(FleetMemberList::Leader), 1000, 2000));
    testee.add(makeItem("i2", 99, "xyz", FleetMemberList::Flags_t(),                        2200, 2400));
    TS_ASSERT_EQUALS(testee == FleetMemberList(), false);
    TS_ASSERT_EQUALS(testee != FleetMemberList(), true);

    TS_ASSERT_EQUALS(testee.empty(), false);
    TS_ASSERT_EQUALS(testee.size(), 2U);
    TS_ASSERT(testee.get(0) != 0);
    TS_ASSERT_EQUALS(testee.get(0)->friendlyCode, "abc");
    TS_ASSERT_EQUALS(testee.get(0)->flags, FleetMemberList::Flags_t(FleetMemberList::Leader));
    TS_ASSERT_EQUALS(testee.get(0)->name, "i1");
    TS_ASSERT_EQUALS(testee.get(0)->position.getX(), 1000);
    TS_ASSERT_EQUALS(testee.get(0)->position.getY(), 2000);

    TS_ASSERT_EQUALS(*testee.get(0) == *testee.get(0), true);
    TS_ASSERT_EQUALS(*testee.get(0) == *testee.get(1), false);
    TS_ASSERT_EQUALS(*testee.get(0) != *testee.get(0), false);
    TS_ASSERT_EQUALS(*testee.get(0) != *testee.get(1), true);

    // Verify find: unsuccessfully
    {
        size_t pos = 9999;
        TS_ASSERT_EQUALS(testee.find(Reference(Reference::Ship, 66), pos), false);
    }

    // Verify find: successfully
    {
        size_t pos = 9999;
        TS_ASSERT_EQUALS(testee.find(Reference(Reference::Ship, 99), pos), true);
        TS_ASSERT_EQUALS(pos, 1U);
    }

    // Clear; verify state
    testee.clear();
    TS_ASSERT_EQUALS(testee.empty(), true);
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT(testee.get(0) == 0);
}

/** Test sort(), FleetMemberList predicate. */
void
TestGameRefFleetMemberList::testSort1()
{
    // Prepare data
    FleetMemberList testee;
    testee.add(makeItem("i1", 1, "abc", FleetMemberList::Flags_t(), 1200, 1100));
    testee.add(makeItem("i2", 7, "xxx", FleetMemberList::Flags_t(), 1200, 1100));
    testee.add(makeItem("i3", 9, "abc", FleetMemberList::Flags_t(), 1200, 1100));
    testee.add(makeItem("i4", 5, "yyy", FleetMemberList::Flags_t(), 1200, 1100));
    testee.add(makeItem("i5", 3, "abc", FleetMemberList::Flags_t(), 1200, 1100));

    // Sort by "is-here"
    class Pred : public FleetMemberList::SortPredicate {
     public:
        virtual int compare(const FleetMemberList::Item& a, const FleetMemberList::Item& b) const
            { return afl::string::strCaseCompare(a.friendlyCode, b.friendlyCode); }
        virtual String_t getClass(const FleetMemberList::Item& a) const
            { return a.friendlyCode; }
    };
    testee.sort(Pred());

    // Verify
    TS_ASSERT_EQUALS(testee.size(), 8U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "abc");
    TS_ASSERT_EQUALS(testee.get(1)->name, "i1");
    TS_ASSERT_EQUALS(testee.get(2)->name, "i5");  // note sort by Id!
    TS_ASSERT_EQUALS(testee.get(3)->name, "i3");
    TS_ASSERT_EQUALS(testee.get(4)->name, "xxx");
    TS_ASSERT_EQUALS(testee.get(5)->name, "i2");
    TS_ASSERT_EQUALS(testee.get(6)->name, "yyy");
    TS_ASSERT_EQUALS(testee.get(7)->name, "i4");

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
    TS_ASSERT_EQUALS(testee.size(), 5U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "i1");
    TS_ASSERT_EQUALS(testee.get(1)->name, "i5");
    TS_ASSERT_EQUALS(testee.get(2)->name, "i3");
    TS_ASSERT_EQUALS(testee.get(3)->name, "i2");
    TS_ASSERT_EQUALS(testee.get(4)->name, "i4");
}

/** Test sort(), game::ref::SortPredicate. */
void
TestGameRefFleetMemberList::testSort2()
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
    TS_ASSERT_EQUALS(testee.size(), 5U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "i1");
    TS_ASSERT_EQUALS(testee.get(1)->name, "i5");
    TS_ASSERT_EQUALS(testee.get(2)->name, "i4");
    TS_ASSERT_EQUALS(testee.get(3)->name, "i2");
    TS_ASSERT_EQUALS(testee.get(4)->name, "i3");
}

/** Test setFleet(). */
void
TestGameRefFleetMemberList::testSet()
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
    TS_ASSERT_EQUALS(testee.size(), 2U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "s3");
    TS_ASSERT_EQUALS(testee.get(0)->friendlyCode, "thr");
    TS_ASSERT_EQUALS(testee.get(0)->flags, FleetMemberList::Flags_t(FleetMemberList::Leader));
    TS_ASSERT_EQUALS(testee.get(1)->name, "s7");
    TS_ASSERT_EQUALS(testee.get(1)->friendlyCode, "sev");
    TS_ASSERT_EQUALS(testee.get(1)->flags, FleetMemberList::Flags_t(FleetMemberList::Away));

    // Load fleet 9 (ships 9+5+11)
    testee.setFleet(univ, 9);
    TS_ASSERT_EQUALS(testee.size(), 3U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "s9");
    TS_ASSERT_EQUALS(testee.get(0)->friendlyCode, "nin");
    TS_ASSERT_EQUALS(testee.get(0)->flags, FleetMemberList::Flags_t(FleetMemberList::Leader));
    TS_ASSERT_EQUALS(testee.get(1)->name, "s5");
    TS_ASSERT_EQUALS(testee.get(1)->friendlyCode, "fiv");
    TS_ASSERT_EQUALS(testee.get(1)->flags, FleetMemberList::Flags_t());
    TS_ASSERT_EQUALS(testee.get(2)->name, "s11");
    TS_ASSERT_EQUALS(testee.get(2)->friendlyCode, "ele");
    TS_ASSERT_EQUALS(testee.get(2)->flags, FleetMemberList::Flags_t());

    // Load single ship [border usecase]
    testee.setFleet(univ, 1);
    TS_ASSERT_EQUALS(testee.size(), 1U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "s1");
    TS_ASSERT_EQUALS(testee.get(0)->friendlyCode, "one");
    TS_ASSERT_EQUALS(testee.get(0)->flags, FleetMemberList::Flags_t(FleetMemberList::Leader));

    // Load nonexistant ship [border usecase]
    testee.setFleet(univ, 0);
    TS_ASSERT_EQUALS(testee.size(), 0U);
}

/** Test setFleet(), with towing. */
void
TestGameRefFleetMemberList::testSet2()
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
    TS_ASSERT_EQUALS(testee.size(), 5U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "s5");
    TS_ASSERT_EQUALS(testee.get(0)->flags, FleetMemberList::Flags_t() + FleetMemberList::Leader + FleetMemberList::Towed);
    TS_ASSERT_EQUALS(testee.get(1)->name, "s1");
    TS_ASSERT_EQUALS(testee.get(1)->flags, FleetMemberList::Flags_t() + FleetMemberList::Towing);
    TS_ASSERT_EQUALS(testee.get(2)->name, "s3");
    TS_ASSERT_EQUALS(testee.get(2)->flags, FleetMemberList::Flags_t());
    TS_ASSERT_EQUALS(testee.get(3)->name, "s7");
    TS_ASSERT_EQUALS(testee.get(3)->flags, FleetMemberList::Flags_t() + FleetMemberList::Towing + FleetMemberList::Away);
    TS_ASSERT_EQUALS(testee.get(4)->name, "s9");
    TS_ASSERT_EQUALS(testee.get(4)->flags, FleetMemberList::Flags_t() + FleetMemberList::Towed);
}

