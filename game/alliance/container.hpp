/**
  *  \file game/alliance/container.hpp
  *  \brief Class game::alliance::Container
  */
#ifndef C2NG_GAME_ALLIANCE_CONTAINER_HPP
#define C2NG_GAME_ALLIANCE_CONTAINER_HPP

#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/alliance/handler.hpp"
#include "game/alliance/level.hpp"
#include "game/alliance/offer.hpp"

namespace game { namespace alliance {

    /** Alliances, everything together.
        Contains all available alliance levels and active offers.
        Handler instances implement logic to synchronize to/from the game.

        Levels/offers are referenced by an index (Index_t)
        that is obtained by looking up a level Id (string) using find(). */
    class Container {
     public:
        /** Index into data. */
        typedef size_t Index_t;

        /** Index meaning "not found". */
        static const Index_t nil = Index_t(-1);

        /** Default constructor.
            Makes an empty container. */
        Container();

        /** Copy constructor.
            Makes a new Container object containing the same alliance levels and offers, but no change handlers.
            You can modify the copy and write it back using copyFrom().
            \param other Source object */
        explicit Container(const Container& other);

        /** Destructor. */
        ~Container();

        /** Assignment operator.
            Copies the other container and its structure, but not the handlers.
            \param other Source object
            \return *this */
        Container& operator=(const Container& other);

        /** Postprocess after game load.
            Calls all registered handlers (Handler::postprocess) to import game data into the alliance container. */
        void postprocess();

        /** Add a new alliance level.
            \param level New level (will be copied) */
        void addLevel(const Level& level);

        /** Add a new handler.
            \param handler Newly-allocated alliance handler
            \param tx Translator (passed to Handler::init) */
        void addNewHandler(Handler* handler, afl::string::Translator& tx);

        /** Merge from another alliance object.
            Modifies all offers to the same as in the other object.
            This is an intelligent merge that can deal with different structures on both sides.
            It does not change this object's structure;
            levels not present in other will not be modified, levels not present in this one will not be added.

            This call will trigger Handler::handleChanges().

            \param other Source object*/
        void copyFrom(const Container& other);

        /** Get description of all levels.
            \return levels */
        const Levels_t& getLevels() const;

        /** Get all alliance offers.
            \return offers */
        const Offers_t& getOffers() const;

        /** Find an alliance level by Id.
            The returned index can be used as index into getLevels(), getOffers(),
            and for other functions.
            \param id Level Id (see Level::getId())
            \return index; nil if not found */
        Index_t find(const String_t& id) const;

        /** Get level by index.
            \param index Index, possibly obtained from find()
            \return Level; null if index out of range or nil */
        const Level* getLevel(Index_t index) const;

        /** Get offer by index.
            \param index Index, possibly obtained from find()
            \return Offer; null if index out of range or nil */
        const Offer* getOffer(Index_t index) const;

        /** Get mutable offer by index.
            This method is for use by implementations of process() only.
            Normal manipulation should use the set(), setAll(), and copyFrom() methods;
            manipulations of the Offer will not trigger Handler::handleChanges().
            \param index Index, possibly obtained from find()
            \return Offer, null if index out of range or nil */
        Offer* getMutableOffer(Index_t index);

        /** Check for offer by type.
            Checks whether there is any positive offer to or from the specified player of a level defined by the given flag.
            This can be used to give a quick overview: "there is an alliance".
            \param player Player to check
            \param flag Flag to check (IsOffer, IsEnemy)
            \param fromUs true to check for offers from us, false to check for offers to us
            \retval true there is at least one such offer
            \retval false there is no such offer */
        bool isAny(int player, Level::Flag flag, bool fromUs) const;

        /** Set all offers by type.
            Sets all offers to the specified player for all levels defined by the given flag.
            This can be used to quickly set a set of levels without specifying its identifier.

            This call will trigger Handler::handleChanges().

            \param player Player to modify
            \param flag Flag to check (IsOffer, NeedsOffer, IsEnemy)
            \param set true to set negative offers (Unknown, No) to positive (Yes).
                       false to set positive offers (Yes, Conditional) to negative (No). */
        void setAll(int player, Level::Flag flag, bool set);

        /** Set a single alliance offer.
            This call will trigger Handler::handleChanges().

            \param index Index obtained by find()
            \param player Player to offer
            \param type New setting */
        void set(Index_t index, int player, Offer::Type type);

     private:
        void callHandlers();

        Levels_t m_levels;
        Offers_t m_offers;
        afl::container::PtrVector<Handler> m_handlers;
    };

} }

#endif
