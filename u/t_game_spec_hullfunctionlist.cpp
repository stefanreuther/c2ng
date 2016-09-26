/**
  *  \file u/t_game_spec_hullfunctionlist.cpp
  */

#include "game/spec/hullfunctionlist.hpp"

#include "u/t_game_spec.hpp"

void
TestGameSpecHullFunctionList::testSimplify()
{
    // Simplify, border case
    game::spec::HullFunction oneR(42);
    game::spec::HullFunction oneH(42);
    oneR.setKind(game::spec::HullFunction::AssignedToRace);
    oneH.setKind(game::spec::HullFunction::AssignedToHull);

    game::spec::HullFunctionList hfl;
    hfl.add(oneR);
    hfl.add(oneH);
    TS_ASSERT_EQUALS(hfl.size(), 2U);
    hfl.simplify();

    TS_ASSERT_EQUALS(hfl.size(), 1U);
    TS_ASSERT_EQUALS(hfl[0].getBasicFunctionId(), 42);
}
