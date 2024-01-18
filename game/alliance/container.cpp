/**
  *  \file game/alliance/container.cpp
  *  \brief Class game::alliance::Container
  */

#include "game/alliance/container.hpp"

const game::alliance::Container::Index_t game::alliance::Container::nil;

// Default constructor.
game::alliance::Container::Container()
    : m_levels(),
      m_offers(),
      m_handlers()
{
    // ex GAlliances::GAlliances
}

// Copy constructor.
game::alliance::Container::Container(const Container& other)
    : m_levels(other.m_levels),
      m_offers(other.m_offers),
      m_handlers()
{
    // ex GAlliances::GAlliances
}

// Destructor.
game::alliance::Container::~Container()
{
    // ex GAlliances::~GAlliances
}

// Assignment operator.
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

// Postprocess after game load.
void
game::alliance::Container::postprocess()
{
    // ex GAlliances::postprocess
    for (size_t i = 0; i < m_handlers.size(); ++i) {
        m_handlers[i]->postprocess(*this);
    }
}

// Add a new alliance level.
void
game::alliance::Container::addLevel(const Level& level)
{
    // ex GAlliances::addLevel
    m_levels.push_back(level);
    m_offers.push_back(Offer());
}

// Add a new handler.
void
game::alliance::Container::addNewHandler(Handler* handler, afl::string::Translator& tx)
{
    // ex GAlliances::addNewHandler
    if (handler != 0) {
        m_handlers.pushBackNew(handler);
        handler->init(*this, tx);
    }
}

// Merge from another alliance object.
void
game::alliance::Container::copyFrom(const Container& other)
{
    // ex GAlliances::copyFrom
    for (Index_t i = 0; i < other.m_levels.size(); ++i) {
        const Index_t index = find(other.m_levels[i].getId());
        if (index != nil) {
            m_offers[index] = other.m_offers[i];
        }
    }
    callHandlers();
}

// Get description of all levels.
const game::alliance::Levels_t&
game::alliance::Container::getLevels() const
{
    // ex GAlliances::getLevels
    return m_levels;
}

// Get all alliance offers.
const game::alliance::Offers_t&
game::alliance::Container::getOffers() const
{
    // ex GAlliances::getOffers
    return m_offers;
}

// Find an alliance level, by Id.
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

// Get level by index.
const game::alliance::Level*
game::alliance::Container::getLevel(Index_t index) const
{
    // ex GAlliances::getLevel
    return (index < m_levels.size()
            ? &m_levels[index]
            : 0);
}

// Get offer by index.
const game::alliance::Offer*
game::alliance::Container::getOffer(Index_t index) const
{
    // ex GAlliances::getOffer
    return (index < m_offers.size()
            ? &m_offers[index]
            : 0);
}

// Get mutable offer by index.
game::alliance::Offer*
game::alliance::Container::getMutableOffer(Index_t index)
{
    // ex GAlliances::getMutableOffer
    return (index < m_offers.size()
            ? &m_offers[index]
            : 0);
}

// Check for offer by type.
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

// Set all offers by type.
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

// Set a single alliance offer.
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
