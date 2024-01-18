/**
  *  \file test/game/alliance/offertest.cpp
  *  \brief Test for game::alliance::Offer
  */

#include "game/alliance/offer.hpp"
#include "afl/test/testrunner.hpp"

using game::alliance::Offer;

AFL_TEST("game.alliance.Offer", a)
{
    // Test initial values
    Offer t;
    a.checkEqual("01", t.theirOffer.get(1), Offer::Unknown);
    a.checkEqual("02", t.newOffer.get(1), Offer::Unknown);
    a.checkEqual("03", t.oldOffer.get(1), Offer::Unknown);

    // Set some non-zero values
    t.theirOffer.set(2, Offer::Yes);
    t.newOffer.set(7, Offer::No);
    t.oldOffer.set(4, Offer::Conditional);

    // Merge
    {
        Offer t2;
        t2.theirOffer.set(2, Offer::No);
        t2.newOffer.set(5, Offer::Yes);
        t.merge(t2);
    }

    // Verify merge result
    // - their(2) has been overwritten
    a.checkEqual("11", t.theirOffer.get(2), Offer::No);

    // - new(7) unchanged, new(5) newly-set
    a.checkEqual("21", t.newOffer.get(7), Offer::No);
    a.checkEqual("22", t.newOffer.get(5), Offer::Yes);

    // - old(4) unchanged
    a.checkEqual("31", t.oldOffer.get(4), Offer::Conditional);
}

/** Test Offer::isOffer(). */
AFL_TEST("game.alliance.Offer:isOffer", a)
{
    a.checkEqual("01", Offer::isOffer(Offer::No),          false);
    a.checkEqual("02", Offer::isOffer(Offer::Unknown),     false);
    a.checkEqual("03", Offer::isOffer(Offer::Yes),         true);
    a.checkEqual("04", Offer::isOffer(Offer::Conditional), true);
}
