/**
  *  \file test/game/v3/genextratest.cpp
  *  \brief Test for game::v3::GenExtra
  */

#include "game/v3/genextra.hpp"

#include "afl/test/testrunner.hpp"
#include "game/turn.hpp"

/** Test access to GenExtra.
    (Derived from TestGameV3GenExtra::testGet) */
AFL_TEST("game.v3.GenExtra", a)
{
    using game::v3::GenFile;
    using game::v3::GenExtra;

    game::Turn t;
    const game::Turn& ct = t;

    // Initially, no GenExtra present
    a.checkNull("01", GenExtra::get(t));
    a.checkNull("02", GenExtra::get(ct));

    // Create one
    GenExtra& e = GenExtra::create(t);

    // Now it's there
    GenExtra* p = GenExtra::get(t);
    a.checkNonNull("11", p);
    a.checkEqual("12", p, &e);
    a.checkEqual("13", GenExtra::get(ct), GenExtra::get(t));

    // Same thing for files
    a.checkNull("21", GenExtra::get(t, 4));
    a.checkNull("22", GenExtra::get(ct, 4));

    GenFile& f = p->create(4);
    GenFile* ff = GenExtra::get(t, 4);
    a.checkNonNull("31", ff);
    a.checkEqual("32", ff, &f);
    a.checkEqual("33", GenExtra::get(ct, 4), ff);

    // Other player still empty
    a.checkNull("41", GenExtra::get(ct, 5));
    a.checkNull("42", GenExtra::get(t, 5));
}
