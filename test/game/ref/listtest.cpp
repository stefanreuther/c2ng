/**
  *  \file test/game/ref/listtest.cpp
  *  \brief Test for game::ref::List
  */

#include "game/ref/list.hpp"

#include "afl/test/testrunner.hpp"
#include "game/ref/sortpredicate.hpp"
#include "game/test/simpleturn.hpp"
#include "util/math.hpp"

using game::Reference;

namespace {
    class Sorter : public game::ref::SortPredicate {
     public:
        virtual int compare(const Reference& a, const Reference& b) const
            { return util::compare3(a.getId(), b.getId()); }
        virtual String_t getClass(const Reference& /*a*/) const
            { return String_t(); }
    };
}

/** Test behaviour on empty list. */
AFL_TEST("game.ref.List:empty", a)
{
    game::ref::List testee;
    a.checkEqual("01. size", testee.size(), 0U);
    a.checkEqual("02. getTypes", testee.getTypes(), game::ref::List::Types_t());
    a.checkEqual("03. item", testee[0], Reference());
    a.checkEqual("04. getIds", testee.getIds(Reference::Planet).size(), 0U);

    // Sorting empty succeeds
    testee.sort(Sorter());
    a.checkEqual("11. size", testee.size(), 0U);

    // Set is ignored
    testee.set(1, Reference(Reference::Minefield, 9));
    a.checkEqual("21. size", testee.size(), 0U);
}

/** Test normal behavior on populated list. */
AFL_TEST("game.ref.List:normal", a)
{
    // Some Ids
    std::vector<game::Id_t> ids;
    ids.push_back(2);
    ids.push_back(9);
    ids.push_back(7);

    // Build a list
    //  p3  s2  s7  s9  p1
    game::ref::List testee;
    testee.add(Reference(Reference::Planet, 3));
    testee.add(Reference::Ship, ids);
    testee.add(Reference(Reference::Planet, 1));

    // Verify
    a.checkEqual("01. size", testee.size(), 5U);
    a.checkEqual("02. getTypes", testee.getTypes(), game::ref::List::Types_t() + Reference::Ship + Reference::Planet);
    a.checkEqual("03. item", testee[0], Reference(Reference::Planet, 3));
    a.checkEqual("04. item", testee[1], Reference(Reference::Ship, 2));
    a.checkEqual("05. item", testee[1000], Reference());

    std::vector<game::Id_t> planetIds = testee.getIds(Reference::Planet);
    a.checkEqual("11. size", planetIds.size(), 2U);
    a.checkEqual("12. getIds", planetIds[0], 3);
    a.checkEqual("13. getIds", planetIds[1], 1);

    // Modify
    testee.set(2, Reference(Reference::Minefield, 8));

    // Sort
    testee.sort(Sorter());
    a.checkEqual("21. size", testee.size(), 5U);
    a.checkEqual("22. item", testee[0], Reference(Reference::Planet, 1));
    a.checkEqual("23. item", testee[1], Reference(Reference::Ship, 2));
    a.checkEqual("24. item", testee[2], Reference(Reference::Planet, 3));
    a.checkEqual("25. item", testee[3], Reference(Reference::Ship, 7));
    a.checkEqual("26. item", testee[4], Reference(Reference::Minefield, 8));

    // Clear
    testee.clear();
    a.checkEqual("31. size", testee.size(), 0U);
}

/** Test add(List). */
AFL_TEST("game.ref.List:add:list", a)
{
    // Build a list
    game::ref::List testee;
    testee.add(Reference(Reference::Planet, 3));
    testee.add(Reference(Reference::Planet, 1));
    testee.add(Reference(Reference::Planet, 5));

    // Another list
    game::ref::List b;
    b.add(Reference(Reference::Ship, 10));
    b.add(Reference(Reference::Ship, 30));

    // Add
    testee.add(b);

    // Verify
    a.checkEqual("01. size", testee.size(), 5U);
    a.checkEqual("02. item", testee[0], Reference(Reference::Planet, 3));
    a.checkEqual("03. item", testee[1], Reference(Reference::Planet, 1));
    a.checkEqual("04. item", testee[2], Reference(Reference::Planet, 5));
    a.checkEqual("05. item", testee[3], Reference(Reference::Ship, 10));
    a.checkEqual("06. item", testee[4], Reference(Reference::Ship, 30));
}

/** Test add(List), self-addition. */
AFL_TEST("game.ref.List:add:list:self", a)
{
    // Build a list
    game::ref::List testee;
    testee.add(Reference(Reference::Planet, 3));
    testee.add(Reference(Reference::Planet, 1));

    // Add
    testee.add(testee);

    // Verify
    a.checkEqual("01. size", testee.size(), 4U);
    a.checkEqual("02. item", testee[0], Reference(Reference::Planet, 3));
    a.checkEqual("03. item", testee[1], Reference(Reference::Planet, 1));
    a.checkEqual("04. item", testee[2], Reference(Reference::Planet, 3));
    a.checkEqual("05. item", testee[3], Reference(Reference::Planet, 1));
}

/** Test addObjectsAt(). */
AFL_TEST("game.ref.List:addObjectsAt", a)
{
    game::test::SimpleTurn t;
    game::map::Point pos(1100, 1200);
    t.setPosition(pos);
    t.addPlanet(30, 4, game::map::Object::NotPlayable);
    t.addShip(10, 4, game::map::Object::NotPlayable);
    t.addShip(12, 5, game::map::Object::ReadOnly);;
    t.addShip(14, 6, game::map::Object::Playable);
    t.addShip(16, 4, game::map::Object::NotPlayable);

    // Wrong position
    {
        game::ref::List testee;
        testee.addObjectsAt(t.universe(), pos + game::map::Point(1, 0), game::ref::List::Options_t(), 0);
        a.checkEqual("01. size", testee.size(), 0U);
    }

    // No options
    {
        game::ref::List testee;
        testee.addObjectsAt(t.universe(), pos, game::ref::List::Options_t(), 0);
        a.checkEqual("11. size", testee.size(), 2U);
        a.checkEqual("12. item", testee[0], Reference(Reference::Ship, 12));
        a.checkEqual("13. item", testee[1], Reference(Reference::Ship, 14));
    }

    // Exclude 14
    {
        game::ref::List testee;
        testee.addObjectsAt(t.universe(), pos, game::ref::List::Options_t(), 14);
        a.checkEqual("21. size", testee.size(), 1U);
        a.checkEqual("22. item", testee[0], Reference(Reference::Ship, 12));
    }

    // With foreign ships
    {
        game::ref::List testee;
        testee.addObjectsAt(t.universe(), pos, game::ref::List::Options_t() + game::ref::List::IncludeForeignShips, 0);
        a.checkEqual("31. size", testee.size(), 4U);
        a.checkEqual("32. item", testee[0], Reference(Reference::Ship, 10));
        a.checkEqual("33. item", testee[1], Reference(Reference::Ship, 12));
        a.checkEqual("34. item", testee[2], Reference(Reference::Ship, 14));
        a.checkEqual("35. item", testee[3], Reference(Reference::Ship, 16));
    }

    // With planet
    {
        game::ref::List testee;
        testee.addObjectsAt(t.universe(), pos, game::ref::List::Options_t() + game::ref::List::IncludePlanet, 0);
        a.checkEqual("41. size", testee.size(), 3U);
        a.checkEqual("42. item", testee[0], Reference(Reference::Planet, 30));
        a.checkEqual("43. item", testee[1], Reference(Reference::Ship, 12));
        a.checkEqual("44. item", testee[2], Reference(Reference::Ship, 14));
    }
}
