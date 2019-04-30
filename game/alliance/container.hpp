/**
  *  \file game/alliance/container.hpp
  */
#ifndef C2NG_GAME_ALLIANCE_CONTAINER_HPP
#define C2NG_GAME_ALLIANCE_CONTAINER_HPP

#include "game/alliance/level.hpp"
#include "game/alliance/offer.hpp"
#include "game/alliance/handler.hpp"
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace alliance {

    // /** Alliances, everything together. */
    class Container {
     public:
        typedef size_t Index_t;
        static const Index_t nil = Index_t(-1);

        Container();
        explicit Container(const Container& other);
        ~Container();

        Container& operator=(const Container& other);

        void postprocess();
        void addLevel(const Level& level);
        void addNewHandler(Handler* handler, afl::string::Translator& tx);

        void copyFrom(const Container& other);

        const Levels_t& getLevels() const;
        const Offers_t& getOffers() const;

        // FIXME: names?
        Index_t find(const String_t& id) const;
        const Level* getLevel(Index_t index) const;
        const Offer* getOffer(Index_t index) const;
        Offer* getMutableOffer(Index_t index);

        bool isAny(int player, Level::Flag flag, bool fromUs) const;
        void setAll(int player, Level::Flag flag, bool set);

        void set(Index_t index, int player, Offer::Type type);

     private:
        void callHandlers();

        Levels_t m_levels;
        Offers_t m_offers;
        afl::container::PtrVector<Handler> m_handlers;
    };

} }

#endif
