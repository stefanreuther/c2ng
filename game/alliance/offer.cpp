/**
  *  \file game/alliance/offer.cpp
  *  \brief Structure game::alliance::Offer
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


// Constructor.
game::alliance::Offer::Offer()
{
    // ex GAllianceOffer::GAllianceOffer
    theirOffer.setAll(Unknown);
    oldOffer.setAll(Unknown);
    newOffer.setAll(Unknown);
}

// Merge.
void
game::alliance::Offer::merge(const Offer& other)
{
    // ex GAllianceOffer::merge
    mergeSet(theirOffer, other.theirOffer);
    mergeSet(oldOffer, other.oldOffer);
    mergeSet(newOffer, other.newOffer);
}

// Check for positive offer.
bool
game::alliance::Offer::isOffer(Type type)
{
    // ex GAllianceOffer::isOffer
    return type == Yes || type == Conditional;
}
