/**
  *  \file u/t_game_ref_list.cpp
  *  \brief Test for game::ref::List
  */

#include "game/ref/list.hpp"

#include "t_game_ref.hpp"
#include "util/math.hpp"
#include "game/ref/sortpredicate.hpp"
#include "game/test/simpleturn.hpp"

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
void
TestGameRefList::testEmpty()
{
    game::ref::List testee;
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT_EQUALS(testee.getTypes(), game::ref::List::Types_t());
    TS_ASSERT_EQUALS(testee[0], Reference());
    TS_ASSERT_EQUALS(testee.getIds(Reference::Planet).size(), 0U);

    // Sorting empty succeeds
    testee.sort(Sorter());
    TS_ASSERT_EQUALS(testee.size(), 0U);

    // Set is ignored
    testee.set(1, Reference(Reference::Minefield, 9));
    TS_ASSERT_EQUALS(testee.size(), 0U);
}

/** Test normal behavior on populated list. */
void
TestGameRefList::testNormal()
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
    TS_ASSERT_EQUALS(testee.size(), 5U);
    TS_ASSERT_EQUALS(testee.getTypes(), game::ref::List::Types_t() + Reference::Ship + Reference::Planet);
    TS_ASSERT_EQUALS(testee[0], Reference(Reference::Planet, 3));
    TS_ASSERT_EQUALS(testee[1], Reference(Reference::Ship, 2));
    TS_ASSERT_EQUALS(testee[1000], Reference());

    std::vector<game::Id_t> planetIds = testee.getIds(Reference::Planet);
    TS_ASSERT_EQUALS(planetIds.size(), 2U);
    TS_ASSERT_EQUALS(planetIds[0], 3);
    TS_ASSERT_EQUALS(planetIds[1], 1);

    // Modify
    testee.set(2, Reference(Reference::Minefield, 8));

    // Sort
    testee.sort(Sorter());
    TS_ASSERT_EQUALS(testee.size(), 5U);
    TS_ASSERT_EQUALS(testee[0], Reference(Reference::Planet, 1));
    TS_ASSERT_EQUALS(testee[1], Reference(Reference::Ship, 2));
    TS_ASSERT_EQUALS(testee[2], Reference(Reference::Planet, 3));
    TS_ASSERT_EQUALS(testee[3], Reference(Reference::Ship, 7));
    TS_ASSERT_EQUALS(testee[4], Reference(Reference::Minefield, 8));

    // Clear
    testee.clear();
    TS_ASSERT_EQUALS(testee.size(), 0U);
}

/** Test addObjectsAt(). */
void
TestGameRefList::testAddObjectsAt()
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
        TS_ASSERT_EQUALS(testee.size(), 0U);
    }

    // No options
    {
        game::ref::List testee;
        testee.addObjectsAt(t.universe(), pos, game::ref::List::Options_t(), 0);
        TS_ASSERT_EQUALS(testee.size(), 2U);
        TS_ASSERT_EQUALS(testee[0], Reference(Reference::Ship, 12));
        TS_ASSERT_EQUALS(testee[1], Reference(Reference::Ship, 14));
    }

    // Exclude 14
    {
        game::ref::List testee;
        testee.addObjectsAt(t.universe(), pos, game::ref::List::Options_t(), 14);
        TS_ASSERT_EQUALS(testee.size(), 1U);
        TS_ASSERT_EQUALS(testee[0], Reference(Reference::Ship, 12));
    }

    // With foreign ships
    {
        game::ref::List testee;
        testee.addObjectsAt(t.universe(), pos, game::ref::List::Options_t() + game::ref::List::IncludeForeignShips, 0);
        TS_ASSERT_EQUALS(testee.size(), 4U);
        TS_ASSERT_EQUALS(testee[0], Reference(Reference::Ship, 10));
        TS_ASSERT_EQUALS(testee[1], Reference(Reference::Ship, 12));
        TS_ASSERT_EQUALS(testee[2], Reference(Reference::Ship, 14));
        TS_ASSERT_EQUALS(testee[3], Reference(Reference::Ship, 16));
    }

    // With planet
    {
        game::ref::List testee;
        testee.addObjectsAt(t.universe(), pos, game::ref::List::Options_t() + game::ref::List::IncludePlanet, 0);
        TS_ASSERT_EQUALS(testee.size(), 3U);
        TS_ASSERT_EQUALS(testee[0], Reference(Reference::Planet, 30));
        TS_ASSERT_EQUALS(testee[1], Reference(Reference::Ship, 12));
        TS_ASSERT_EQUALS(testee[2], Reference(Reference::Ship, 14));
    }
}

