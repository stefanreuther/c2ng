/**
  *  \file game/alliance/offer.hpp
  *  \brief Structure game::alliance::Offer
  */
#ifndef C2NG_GAME_ALLIANCE_OFFER_HPP
#define C2NG_GAME_ALLIANCE_OFFER_HPP

#include <vector>
#include "game/playerarray.hpp"

namespace game { namespace alliance {

    /** Alliance offer.
        This structure contains the status of an alliance offer.
        An object of this type represents the alliances for one level. */
    struct Offer {
        /** Type of offer. */
        enum Type {             // ex OfferType
            No,                 ///< Alliance not offered.
            Unknown,            ///< Status unknown.
            Conditional,        ///< Alliance offered conditionally.
            Yes                 ///< Alliance offered unconditionally.
        };

        /** Other races' offers to us. */
        PlayerArray<Type> theirOffer;

        /** Our offers to other races, at beginning of turn. */
        PlayerArray<Type> oldOffer;

        /** Our offers to other races, new. */
        PlayerArray<Type> newOffer;

        /** Constructur.
            Make blank (unknown) alliance offer. */
        Offer();

        /** Merge.
            Any value that is not Unknown in \c other overrides the corresponding value in this object.
            \param other Other offer set */
        void merge(const Offer& other);

        /** Check for positive offer.
            \param type Type
            \return true if type is Conditional or Yes */
        static bool isOffer(Type type);
    };

    /** Vector of alliance offers. */
    typedef std::vector<Offer> Offers_t;    // ex GAllianceOffers

} }

#endif
