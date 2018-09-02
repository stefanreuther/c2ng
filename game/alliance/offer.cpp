/**
  *  \file game/alliance/offer.cpp
  */

#include "game/alliance/offer.hpp"

namespace {
    void mergeSet(game::PlayerArray<game::alliance::Offer::Type>& dst, const game::PlayerArray<game::alliance::Offer::Type>& src)
    {
        for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
            if (src.get(i) != game::alliance::Offer::Unknown) {
                dst.set(i, src.get(i));
            }
        }
    }
}


// /** Constructor.
//     Make blank (unknown) alliance offer. */
game::alliance::Offer::Offer()
{
    // ex GAllianceOffer::GAllianceOffer
    theirOffer.setAll(Unknown);
    oldOffer.setAll(Unknown);
    newOffer.setAll(Unknown);
}

// /** Merge two alliance offer sets.
//     Values from the other set are copied into this one, excluding Unknown values.. */
void
game::alliance::Offer::merge(const Offer& other)
{
    // ex GAllianceOffer::merge
    mergeSet(theirOffer, other.theirOffer);
    mergeSet(oldOffer, other.oldOffer);
    mergeSet(newOffer, other.newOffer);
}

// /** Check whether the given OfferType means a positive offer. */
bool
game::alliance::Offer::isOffer(Type type)
{
    // ex GAllianceOffer::isOffer
    return type == Yes || type == Conditional;
}
