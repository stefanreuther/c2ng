/**
  *  \file game/sim/sort.cpp
  *  \brief Simulator-Related Sort Predicates
  */

#include "game/sim/sort.hpp"
#include "afl/string/string.hpp"
#include "game/battleorderrule.hpp"
#include "game/hostversion.hpp"
#include "game/sim/ship.hpp"
#include "util/math.hpp"

using util::compare3;

int
game::sim::compareId(const Ship& a, const Ship& b)
{
    // ex sortById, ccsim.pas:SortId
    return compare3(a.getId(), b.getId());
}

int
game::sim::compareOwner(const Ship& a, const Ship& b)
{
    // ex sortByOwner, ccsim.pas:SortOwner
    return compare3(a.getOwner(), b.getOwner());
}

int
game::sim::compareHull(const Ship& a, const Ship& b)
{
    // ex sortByHull, ccsim.pas:SortHull
    return compare3(a.getHullType(), b.getHullType());
}

int
game::sim::compareBattleOrderHost(const Ship& a, const Ship& b)
{
    // ex sortByBattleOrder (part), ccsim.pas:SortFCBO (part)
    BattleOrderRule rule(HostVersion(HostVersion::Host, MKVERSION(3,22,48)));
    return compare3(rule.get(a), rule.get(b));
}

int
game::sim::compareBattleOrderPHost(const Ship& a, const Ship& b)
{
    // ex sortByBattleOrder (part), ccsim.pas:SortFCBO (part)
    BattleOrderRule rule(HostVersion(HostVersion::PHost, MKVERSION(4,0,0)));
    return compare3(rule.get(a), rule.get(b));
}

int
game::sim::compareName(const Ship& a, const Ship& b)
{
    // ex sortByName
    return compare3(afl::string::strUCase(a.getName()), afl::string::strUCase(b.getName()));
}

