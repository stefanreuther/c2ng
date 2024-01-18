/**
  *  \file test/game/sim/objecttest.hpp
  *  \brief Test for game::sim::Object
  */
#ifndef C2NG_TEST_GAME_SIM_OBJECTTEST_HPP
#define C2NG_TEST_GAME_SIM_OBJECTTEST_HPP

#include "afl/test/assert.hpp"

namespace game { namespace sim {

    class Object;

    /** Common part to verify an object.
        @param a asserter
        @param t object under test */
    void verifyObject(afl::test::Assert a, Object& t);

} }

#endif
