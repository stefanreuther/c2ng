/**
  *  \file game/parser/messageinformation.cpp
  *  \brief Class game::parser::MessageInformation
  */

#include <cassert>
#include "game/parser/messageinformation.hpp"


// Constructor.
game::parser::MessageInformation::MessageInformation(Type type, int32_t id, int turn)
    : m_type(type),
      m_id(id),
      m_turnNumber(turn),
      m_values()
{
    // ex GMessageInformation::GMessageInformation
}

// Destructor.
game::parser::MessageInformation::~MessageInformation()
{ }

// Add string value.
void
game::parser::MessageInformation::addValue(MessageStringIndex si, const String_t& s)
{
    // ex GMessageInformation::addValue
    m_values.pushBackNew(new MessageStringValue_t(si, s));
}

// Add integer value.
void
game::parser::MessageInformation::addValue(MessageIntegerIndex ii, int32_t i)
{
    // ex GMessageInformation::addValue
    m_values.pushBackNew(new MessageIntegerValue_t(ii, i));
}

// Add configuration value.
void
game::parser::MessageInformation::addConfigurationValue(String_t key, String_t value)
{
    // ex GMessageInformation::addConfigValue
    assert(m_type == Configuration);
    m_values.pushBackNew(new MessageConfigurationValue_t(key, value));
}

// Add score value.
void
game::parser::MessageInformation::addScoreValue(int player, int32_t value)
{
    // ex GMessageInformation::addScoreValue
    assert(m_type == PlayerScore);
    m_values.pushBackNew(new MessageScoreValue_t(player, value));
}

// Add alliance value.
void
game::parser::MessageInformation::addAllianceValue(String_t id, const game::alliance::Offer& offer)
{
    // ex GMessageInformation::addAllianceValue
    assert(m_type == Alliance);
    m_values.pushBackNew(new MessageAllianceValue_t(id, offer));
}

// Get string value.
bool
game::parser::MessageInformation::getValue(MessageStringIndex si, String_t& out) const
{
    for (ConstIterator_t it = begin(), e = end(); it != e; ++it) {
        if (MessageStringValue_t* sv = dynamic_cast<MessageStringValue_t*>(*it)) {
            if (sv->getIndex() == si) {
                out = sv->getValue();
                return true;
            }
        }
    }
    return false;
}

// Get integer value.
bool
game::parser::MessageInformation::getValue(MessageIntegerIndex ii, int32_t& out) const
{
    for (ConstIterator_t it = begin(), e = end(); it != e; ++it) {
        if (MessageIntegerValue_t* sv = dynamic_cast<MessageIntegerValue_t*>(*it)) {
            if (sv->getIndex() == ii) {
                out = sv->getValue();
                return true;
            }
        }
    }
    return false;
}

// Get integer value, with range checking.
bool
game::parser::MessageInformation::getValue(MessageIntegerIndex ii, int32_t& out, int32_t min, int32_t max) const
{
    int32_t tmp;
    if (getValue(ii, tmp) && tmp >= min && tmp <= max) {
        out = tmp;
        return true;
    } else {
        return false;
    }
}

// Get object type/Id in Reference format.
game::Reference
game::parser::MessageInformation::getObjectReference() const
{
    switch (m_type) {
     case Ship:
        return Reference(Reference::Ship, m_id);
     case Planet:
        return Reference(Reference::Planet, m_id);
     case Starbase:
        return Reference(Reference::Starbase, m_id);
     case Minefield:
        return Reference(Reference::Minefield, m_id);
     case IonStorm:
        return Reference(Reference::IonStorm, m_id);
     case Ufo:
        return Reference(Reference::Ufo, m_id);
     case Wormhole:
     case Explosion:
     case Configuration:
     case PlayerScore:
     case Alliance:
     case MarkerDrawing:
     case CircleDrawing:
     case LineDrawing:
     case RectangleDrawing:
     case ExtraShip:
     case ExtraPlanet:
     case ExtraMinefield:
     case NoObject:
        break;
    }
    return Reference();
}
