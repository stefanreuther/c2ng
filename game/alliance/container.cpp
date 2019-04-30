/**
  *  \file game/alliance/container.cpp
  */

#include "game/alliance/container.hpp"

// /** Constructor. */
game::alliance::Container::Container()
    : m_levels(),
      m_offers(),
      m_handlers()
{
    // ex GAlliances::GAlliances
}

// /** Copy constructor.
//     Makes a new GAlliances object containing the same alliance levels and offers, but no change handlers.
//     You can modify the copy and write it back using copyFrom(). */
game::alliance::Container::Container(const Container& other)
    : m_levels(other.m_levels),
      m_offers(other.m_offers),
      m_handlers()
{
    // ex GAlliances::GAlliances
}

// /** Destructor. */
game::alliance::Container::~Container()
{
    // ex GAlliances::~GAlliances
}

game::alliance::Container&
game::alliance::Container::operator=(const Container& other)
{
    if (&other != this) {
        m_levels = other.m_levels;
        m_offers = other.m_offers;
        callHandlers();
    }
    return *this;
}

// /** Postprocess after game load.
//     Calls all registered handlers. */
void
game::alliance::Container::postprocess()
{
    // ex GAlliances::postprocess
    for (size_t i = 0; i < m_handlers.size(); ++i) {
        m_handlers[i]->postprocess(*this);
    }
}

// /** Add a new alliance level.
//     \param level Level description */
void
game::alliance::Container::addLevel(const Level& level)
{
    // ex GAlliances::addLevel
    m_levels.push_back(level);
    m_offers.push_back(Offer());
}

// /** Add a new handler.
//     \param handler Newly-allocated alliance handler */
void
game::alliance::Container::addNewHandler(Handler* handler, afl::string::Translator& tx)
{
    // ex GAlliances::addNewHandler
    if (handler != 0) {
        m_handlers.pushBackNew(handler);
        handler->init(*this, tx);
    }
}

// /** Merge from another alliance object.
//     Modifies all offers to the same as in the other object.
//     This is an intelligent merge that can deal with different structures on both sides. */
void
game::alliance::Container::copyFrom(const Container& other)
{
    // ex GAlliances::copyFrom
    // FIXME: const?
    for (Index_t i = 0; i < other.m_levels.size(); ++i) {
        const Index_t index = find(other.m_levels[i].getId());
        if (index != nil) {
            m_offers[index] = other.m_offers[i];
        }
    }
    callHandlers();
}

// /** Get description of all levels. */
const game::alliance::Levels_t&
game::alliance::Container::getLevels() const
{
    // ex GAlliances::getLevels
    return m_levels;
}

// /** Get description of all offers. */
const game::alliance::Offers_t&
game::alliance::Container::getOffers() const
{
    // ex GAlliances::getOffers
    return m_offers;
}

// /** Find an alliance level, by Id.
//     \param id Id to look for
//     \return index, nil if not found */
game::alliance::Container::Index_t
game::alliance::Container::find(const String_t& id) const
{
    // ex GAlliances::find
    for (Index_t i = 0, e = m_levels.size(); i < e; ++i) {
        if (m_levels[i].getId() == id) {
            return i;
        }
    }
    return nil;
}

// /** Get level by index.
//     \param index index, possibly obtained from find()
//     \return level description, null if index is out of range */
const game::alliance::Level*
game::alliance::Container::getLevel(Index_t index) const
{
    // ex GAlliances::getLevel
    return (index < m_levels.size()
            ? &m_levels[index]
            : 0);
}

// /** Get offer by index.
//     \param index index, possibly obtained from find()
//     \return offer, null if index is out of range */
const game::alliance::Offer*
game::alliance::Container::getOffer(Index_t index) const
{
    // ex GAlliances::getOffer
    return (index < m_offers.size()
            ? &m_offers[index]
            : 0);
}

// /** Get mutable offer by index.
//     This method is for use by implementations of process() only.
//     Normal manipulation should use the set(), setAll(), and copyFrom() methods.
//     \param index index, possibly obtained from find()
//     \return offer, null if index is out of range */
game::alliance::Offer*
game::alliance::Container::getMutableOffer(Index_t index)
{
    // ex GAlliances::getMutableOffer
    return (index < m_offers.size()
            ? &m_offers[index]
            : 0);
}

// /** Check for offer by type.
//     Checks whether there is any positive offer to or from the specified player of a level defined by the given flag.
//     This can be used to give a quick overview: "there is an alliance".
//     \param player Player to check
//     \param flag Flag to check (IsOffer, IsEnemy)
//     \param fromUs true to check for offers from us, false to check for offers to us
//     \retval true there is at least one such offer
//     \retval false there is no such offer */
bool
game::alliance::Container::isAny(int player, Level::Flag flag, bool fromUs) const
{
    // ex GAlliances::isAny
    for (Index_t i = 0, e = m_offers.size(); i < e; ++i) {
        if (m_levels[i].hasFlag(flag)
            && Offer::isOffer(fromUs ? m_offers[i].newOffer.get(player) : m_offers[i].theirOffer.get(player)))
        {
            return true;
        }
    }
    return false;
}

// /** Set all offers by type.
//     Sets all offers to the specified player for all levels defined by the given flag.
//     This can be used to quickly set a set of levels without specifying its identifier.

//     \param player Player to modify
//     \param flag Flag to check (IsOffer, NeedsOffer, IsEnemy)
//     \param set true to set negative offers (Unknown, No) to positive (Yes).
//                false to set positive offers (Yes, Conditional) to negative (No). */
void
game::alliance::Container::setAll(int player, Level::Flag flag, bool set)
{
    // ex GAlliances::setAll
    bool did = false;
    for (Index_t i = 0, e = m_offers.size(); i < e; ++i) {
        if (m_levels[i].hasFlag(flag) && Offer::isOffer(m_offers[i].newOffer.get(player)) != set) {
            m_offers[i].newOffer.set(player, set ? Offer::Yes : Offer::No);
            did = true;
        }
    }
    if (did) {
        callHandlers();
    }
}

// /** Set a single alliance offer.
//     Unlike direct access to getMutableOffer(), this will call listeners.
//     \param index Index obtained by find()
//     \param player Player to offer
//     \param type New setting */
void
game::alliance::Container::set(Index_t index, int player, Offer::Type type)
{
    // ex GAlliances::set
    if (index < m_offers.size()) {
        if (Offer::Type* p = m_offers[index].newOffer.at(player)) {
            if (*p != type) {
                // We are changing a valid offer.
                *p = type;

                // Notify listeners to update downstream data.
                callHandlers();
            }
        }
    }
}

void
game::alliance::Container::callHandlers()
{
    // ex GAlliances::callHandlers
    for (size_t i = 0; i < m_handlers.size(); ++i) {
        m_handlers[i]->handleChanges(*this);
    }
}
