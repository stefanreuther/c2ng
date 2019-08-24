/**
  *  \file u/t_game_alliance_offer.cpp
  *  \brief Test for game::alliance::Offer
  */

#include "game/alliance/offer.hpp"

#include "t_game_alliance.hpp"

using game::alliance::Offer;

/** Simple test. */
void
TestGameAllianceOffer::testIt()
{
    // Test initial values
    Offer t;
    TS_ASSERT_EQUALS(t.theirOffer.get(1), Offer::Unknown);
    TS_ASSERT_EQUALS(t.newOffer.get(1), Offer::Unknown);
    TS_ASSERT_EQUALS(t.oldOffer.get(1), Offer::Unknown);

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
    TS_ASSERT_EQUALS(t.theirOffer.get(2), Offer::No);

    // - new(7) unchanged, new(5) newly-set
    TS_ASSERT_EQUALS(t.newOffer.get(7), Offer::No);
    TS_ASSERT_EQUALS(t.newOffer.get(5), Offer::Yes);

    // - old(4) unchanged
    TS_ASSERT_EQUALS(t.oldOffer.get(4), Offer::Conditional);
}

/** Test Offer::isOffer(). */
void
TestGameAllianceOffer::testIsOffer()
{
    TS_ASSERT_EQUALS(Offer::isOffer(Offer::No),          false);
    TS_ASSERT_EQUALS(Offer::isOffer(Offer::Unknown),     false);
    TS_ASSERT_EQUALS(Offer::isOffer(Offer::Yes),         true);
    TS_ASSERT_EQUALS(Offer::isOffer(Offer::Conditional), true);
}

