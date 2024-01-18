/**
  *  \file test/game/ref/historyshiplisttest.cpp
  *  \brief Test for game::ref::HistoryShipList
  */

#include "game/ref/historyshiplist.hpp"

#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.ref.HistoryShipList:basics", a)
{
    HistoryShipList testee;

    // Comparing tro fresh objects
    a.checkEqual("01. eq", testee == HistoryShipList(), true);
    a.checkEqual("02. ne", testee != HistoryShipList(), false);

    // Verify reference turn attribute
    testee.setReferenceTurn(7);
    a.checkEqual("11. getReferenceTurn", testee.getReferenceTurn(), 7);

    // This makes the comparison fail!
    a.checkEqual("21. eq", testee == HistoryShipList(), false);
    a.checkEqual("22. ne", testee != HistoryShipList(), true);

    // Verify initial state
    a.checkEqual("31. empty", testee.empty(), true);
    a.checkEqual("32. size", testee.size(), 0U);
    a.checkNull("33. get", testee.get(0));

    // Add some stuff; verify access
    testee.add(makeItem("i1", 7, 99));
    testee.add(makeItem("i2", 99, 77));
    a.checkEqual("41. eq", testee == HistoryShipList(), false);
    a.checkEqual("42. ne", testee != HistoryShipList(), true);

    a.checkEqual("51. empty", testee.empty(), false);
    a.checkEqual("52. size", testee.size(), 2U);
    a.checkNonNull("53. get", testee.get(0));
    a.checkEqual("54. turnNumber", testee.get(0)->turnNumber, 99);
    a.checkEqual("55. name", testee.get(0)->name, "i1");

    a.checkEqual("61. eq", *testee.get(0) == *testee.get(0), true);
    a.checkEqual("62. eq", *testee.get(0) == *testee.get(1), false);
    a.checkEqual("63. ne", *testee.get(0) != *testee.get(0), false);
    a.checkEqual("64. ne", *testee.get(0) != *testee.get(1), true);

    // Verify find: unsuccessfully
    {
        size_t pos = 9999;
        a.checkEqual("71. find", testee.find(Reference(Reference::Ship, 66)).get(pos), false);
    }

    // Verify find: successfully
    {
        size_t pos = 9999;
        a.checkEqual("81. find", testee.find(Reference(Reference::Ship, 99)).get(pos), true);
        a.checkEqual("82. result", pos, 1U);
    }

    // Clear; verify state
    testee.clear();
    a.checkEqual("91. empty", testee.empty(), true);
    a.checkEqual("92. size", testee.size(), 0U);
    a.checkNull("93. get", testee.get(0));
}

/** Test sort(), HistoryShipList predicate. */
AFL_TEST("game.ref.HistoryShipList:sort:history-predicate", a)
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
    a.checkEqual("01. size", testee.size(), 7U);
    a.checkEqual("02. name", testee.get(0)->name, "Turn 11");
    a.checkEqual("03. name", testee.get(1)->name, "i1");
    a.checkEqual("04. name", testee.get(2)->name, "i4");
    a.checkEqual("05. name", testee.get(3)->name, "i3");
    a.checkEqual("06. name", testee.get(4)->name, "Turn 33");
    a.checkEqual("07. name", testee.get(5)->name, "i5");
    a.checkEqual("08. name", testee.get(6)->name, "i2");

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
    a.checkEqual("11. size", testee.size(), 5U);
    a.checkEqual("12. name", testee.get(0)->name, "i1");
    a.checkEqual("13. name", testee.get(1)->name, "i4");
    a.checkEqual("14. name", testee.get(2)->name, "i3");
    a.checkEqual("15. name", testee.get(3)->name, "i5");
    a.checkEqual("16. name", testee.get(4)->name, "i2");
}

/** Test sort(), game::ref::SortPredicate. */
AFL_TEST("game.ref.HistoryShipList:sort:plain-predicate", a)
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
    a.checkEqual("01. size", testee.size(), 5U);
    a.checkEqual("02. name", testee.get(0)->name, "i1");
    a.checkEqual("03. name", testee.get(1)->name, "i5");
    a.checkEqual("04. name", testee.get(2)->name, "i4");
    a.checkEqual("05. name", testee.get(3)->name, "i2");
    a.checkEqual("06. name", testee.get(4)->name, "i3");
}
