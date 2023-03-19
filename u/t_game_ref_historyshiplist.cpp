/**
  *  \file u/t_game_ref_historyshiplist.cpp
  *  \brief Test for game::ref::HistoryShipList
  */

#include "game/ref/historyshiplist.hpp"

#include "t_game_ref.hpp"
#include "afl/string/format.hpp"
#include "game/ref/sortpredicate.hpp"

using game::ref::HistoryShipList;
using game::ref::UserList;
using game::Reference;

namespace {
    HistoryShipList::Item makeItem(String_t name, int shipId, int turnNumber)
    {
        return HistoryShipList::Item(UserList::Item(UserList::ReferenceItem,
                                                    name,
                                                    Reference(Reference::Ship, shipId),
                                                    true,
                                                    game::map::Object::ReadOnly,
                                                    util::SkinColor::Yellow),
                                     turnNumber);
    }
}


/** Basic functionality test. */
void
TestGameRefHistoryShipList::testIt()
{
    HistoryShipList testee;

    // Comparing tro fresh objects
    TS_ASSERT_EQUALS(testee == HistoryShipList(), true);
    TS_ASSERT_EQUALS(testee != HistoryShipList(), false);

    // Verify reference turn attribute
    testee.setReferenceTurn(7);
    TS_ASSERT_EQUALS(testee.getReferenceTurn(), 7);

    // This makes the comparison fail!
    TS_ASSERT_EQUALS(testee == HistoryShipList(), false);
    TS_ASSERT_EQUALS(testee != HistoryShipList(), true);

    // Verify initial state
    TS_ASSERT_EQUALS(testee.empty(), true);
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT(testee.get(0) == 0);

    // Add some stuff; verify access
    testee.add(makeItem("i1", 7, 99));
    testee.add(makeItem("i2", 99, 77));
    TS_ASSERT_EQUALS(testee == HistoryShipList(), false);
    TS_ASSERT_EQUALS(testee != HistoryShipList(), true);

    TS_ASSERT_EQUALS(testee.empty(), false);
    TS_ASSERT_EQUALS(testee.size(), 2U);
    TS_ASSERT(testee.get(0) != 0);
    TS_ASSERT_EQUALS(testee.get(0)->turnNumber, 99);
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

/** Test sort(), HistoryShipList predicate. */
void
TestGameRefHistoryShipList::testSort1()
{
    // Prepare data
    HistoryShipList testee;
    testee.add(makeItem("i1", 1, 11));
    testee.add(makeItem("i2", 7, 33));
    testee.add(makeItem("i3", 9, 11));
    testee.add(makeItem("i4", 5, 11));
    testee.add(makeItem("i5", 3, 33));

    // Sort by turn number, with dividers
    class Pred : public HistoryShipList::SortPredicate {
     public:
        virtual int compare(const HistoryShipList::Item& a, const HistoryShipList::Item& b) const
            { return a.turnNumber - b.turnNumber; }
        virtual String_t getClass(const HistoryShipList::Item& a) const
            { return afl::string::Format("Turn %d", a.turnNumber); }
    };
    testee.sort(Pred());

    // Verify
    TS_ASSERT_EQUALS(testee.size(), 7U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "Turn 11");
    TS_ASSERT_EQUALS(testee.get(1)->name, "i1");
    TS_ASSERT_EQUALS(testee.get(2)->name, "i4");
    TS_ASSERT_EQUALS(testee.get(3)->name, "i3");
    TS_ASSERT_EQUALS(testee.get(4)->name, "Turn 33");
    TS_ASSERT_EQUALS(testee.get(5)->name, "i5");
    TS_ASSERT_EQUALS(testee.get(6)->name, "i2");

    // Sort again, without dividers
    class Pred2 : public HistoryShipList::SortPredicate {
     public:
        virtual int compare(const HistoryShipList::Item& a, const HistoryShipList::Item& b) const
            { return a.turnNumber - b.turnNumber; }
        virtual String_t getClass(const HistoryShipList::Item& /*a*/) const
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
}

/** Test sort(), game::ref::SortPredicate. */
void
TestGameRefHistoryShipList::testSort2()
{
    // Prepare data
    HistoryShipList testee;
    testee.add(makeItem("i1", 1, 11));
    testee.add(makeItem("i2", 7, 33));
    testee.add(makeItem("i3", 9, 11));
    testee.add(makeItem("i4", 5, 11));
    testee.add(makeItem("i5", 3, 33));

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

