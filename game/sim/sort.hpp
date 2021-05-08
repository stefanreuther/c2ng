/**
  *  \file game/sim/sort.hpp
  *  \brief Simulator-Related Sort Predicates
  */
#ifndef C2NG_GAME_SIM_SORT_HPP
#define C2NG_GAME_SIM_SORT_HPP

namespace game { namespace sim {

    class Ship;

    int compareId(const Ship& a, const Ship& b);
    int compareOwner(const Ship& a, const Ship& b);
    int compareHull(const Ship& a, const Ship& b);
    int compareBattleOrderHost(const Ship& a, const Ship& b);
    int compareBattleOrderPHost(const Ship& a, const Ship& b);
    int compareName(const Ship& a, const Ship& b);

} }

#endif
