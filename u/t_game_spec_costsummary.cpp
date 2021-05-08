/**
  *  \file u/t_game_spec_costsummary.cpp
  *  \brief Test for game::spec::CostSummary
  */

#include "game/spec/costsummary.hpp"

#include "t_game_spec.hpp"

void
TestGameSpecCostSummary::testIt()
{
    using game::spec::CostSummary;
    using game::spec::Cost;

    // Verify initial state
    CostSummary t;
    TS_ASSERT_EQUALS(t.getNumItems(), 0U);
    TS_ASSERT(t.get(0) == 0);
    TS_ASSERT(t.getTotalCost().isZero());

    // Add something
    t.add(CostSummary::Item(1, 2, "one",   Cost::fromString("1T")));
    t.add(CostSummary::Item(2, 5, "two",   Cost::fromString("1D")));
    t.add(CostSummary::Item(1, 7, "three", Cost::fromString("1M")));
    t.add(CostSummary::Item(4, 8, "four",  Cost::fromString("3T")));

    // Verify populated state
    TS_ASSERT_EQUALS(t.getNumItems(), 4U);

    TS_ASSERT(t.get(0) != 0);
    TS_ASSERT_EQUALS(t.get(0)->id, 1);
    TS_ASSERT_EQUALS(t.get(0)->multiplier, 2);

    TS_ASSERT(t.get(1) != 0);
    TS_ASSERT_EQUALS(t.get(1)->id, 2);
    TS_ASSERT_EQUALS(t.get(1)->multiplier, 5);

    // Check find
    TS_ASSERT(t.find(1, 0) != 0);
    TS_ASSERT_EQUALS(t.get(0)->multiplier, 2);

    size_t index = 99;
    TS_ASSERT(t.find(4, &index) != 0);
    TS_ASSERT_EQUALS(index, 3U);

    TS_ASSERT(t.find(99, &index) == 0);

    // Check getTotalCost()
    Cost total = t.getTotalCost();
    TS_ASSERT_EQUALS(total.get(Cost::Tritanium), 4);

    // Check clear()
    t.clear();
    TS_ASSERT_EQUALS(t.getNumItems(), 0U);
    TS_ASSERT(t.getTotalCost().isZero());
}

