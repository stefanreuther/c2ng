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

// FIXME: missing
// /** Add alliance value. For MessageAlliance only. */
// GMessageInformation::addAllianceValue(string_t id, const GAllianceOffer& offer)
// {
//     ASSERT(obj == MessageAlliance);
//     values.push_back(new GMessageAllianceValue(id, offer));
// }
