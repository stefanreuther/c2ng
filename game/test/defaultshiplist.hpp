/**
  *  \file game/test/defaultshiplist.hpp
  *  \brief Default ship list
  */
#ifndef C2NG_GAME_TEST_DEFAULTSHIPLIST_HPP
#define C2NG_GAME_TEST_DEFAULTSHIPLIST_HPP

#include "game/spec/shiplist.hpp"

namespace game { namespace test {

    /** Load default ship list from compiled-in values.
        For now, this is for testing use only.
        The production binaries expose original data files.

        Provides hulls, engines, beams, torpedoes and hull assignments.

        @param list Ship list */
    void initDefaultShipList(game::spec::ShipList& list);

} }

#endif
