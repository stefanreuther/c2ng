/**
  *  \file test/game/spec/costsummarytest.cpp
  *  \brief Test for game::spec::CostSummary
  */

#include "game/spec/costsummary.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.spec.CostSummary", a)
{
    using game::spec::CostSummary;
    using game::spec::Cost;

    // Verify initial state
    CostSummary t;
    a.checkEqual("01. getNumItems", t.getNumItems(), 0U);
    a.checkNull("02. get", t.get(0));
    a.check("03. getTotalCost", t.getTotalCost().isZero());

    // Add something
    t.add(CostSummary::Item(1, 2, "one",   Cost::fromString("1T")));
    t.add(CostSummary::Item(2, 5, "two",   Cost::fromString("1D")));
    t.add(CostSummary::Item(1, 7, "three", Cost::fromString("1M")));
    t.add(CostSummary::Item(4, 8, "four",  Cost::fromString("3T")));

    // Verify populated state
    a.checkEqual("11. getNumItems", t.getNumItems(), 4U);

    a.checkNonNull("21. get",      t.get(0));
    a.checkEqual("22. id",         t.get(0)->id, 1);
    a.checkEqual("23. multiplier", t.get(0)->multiplier, 2);

    a.checkNonNull("31. get",      t.get(1));
    a.checkEqual("32. id",         t.get(1)->id, 2);
    a.checkEqual("33. multiplier", t.get(1)->multiplier, 5);

    // Check find
    a.checkNonNull("41. find", t.find(1, 0));

    size_t index = 99;
    a.checkNonNull("51. find", t.find(4, &index));
    a.checkEqual("52. index", index, 3U);

    a.checkNull("61. find", t.find(99, &index));

    // Check getTotalCost()
    Cost total = t.getTotalCost();
    a.checkEqual("71. getTotalCost", total.get(Cost::Tritanium), 4);

    // Check clear()
    t.clear();
    a.checkEqual("81. getNumItems", t.getNumItems(), 0U);
    a.check("82. getTotalCost", t.getTotalCost().isZero());
}
