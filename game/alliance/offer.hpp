/**
  *  \file game/alliance/offer.hpp
  */
#ifndef C2NG_GAME_ALLIANCE_OFFER_HPP
#define C2NG_GAME_ALLIANCE_OFFER_HPP

#include <vector>
#include "game/playerarray.hpp"

namespace game { namespace alliance {

    // /** Alliance offer.
    //     This structure contains the status of an alliance offer. */
    // FIXME: polish it?
    struct Offer {
        enum Type {      // ex OfferType
            No,
            Unknown,
            Conditional,
            Yes
        };
        PlayerArray<Type> theirOffer;
        PlayerArray<Type> oldOffer;
        PlayerArray<Type> newOffer;

        Offer();

        void merge(const Offer& other);

        static bool isOffer(Type type);
    };

    // /** Vector of alliance offers. */
    // ex GAllianceOffers
    typedef std::vector<Offer> Offers_t;

} }

#endif
