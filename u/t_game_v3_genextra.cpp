/**
  *  \file u/t_game_v3_genextra.cpp
  *  \brief Test for game::v3::GenExtra
  */

#include "game/v3/genextra.hpp"

#include "t_game_v3.hpp"
#include "game/turn.hpp"

/** Test access to GenExtra.
    (Derived from TestGameV3GenExtra::testGet) */
void
TestGameV3GenExtra::testAccess()
{
    using game::v3::GenFile;
    using game::v3::GenExtra;

    game::Turn t;
    const game::Turn& ct = t;

    // Initially, no GenExtra present
    TS_ASSERT(GenExtra::get(t) == 0);
    TS_ASSERT(GenExtra::get(ct) == 0);

    // Create one
    GenExtra& e = GenExtra::create(t);

    // Now it's there
    GenExtra* p = GenExtra::get(t);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p, &e);
    TS_ASSERT_EQUALS(GenExtra::get(ct), GenExtra::get(t));

    // Same thing for files
    TS_ASSERT(GenExtra::get(t, 4) == 0);
    TS_ASSERT(GenExtra::get(ct, 4) == 0);

    GenFile& f = p->create(4);
    GenFile* ff = GenExtra::get(t, 4);
    TS_ASSERT(ff != 0);
    TS_ASSERT_EQUALS(ff, &f);
    TS_ASSERT_EQUALS(GenExtra::get(ct, 4), ff);

    // Other player still empty
    TS_ASSERT(GenExtra::get(ct, 5) == 0);
    TS_ASSERT(GenExtra::get(t, 5) == 0);
}

