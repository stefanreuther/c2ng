/**
  *  \file game/score/turnscore.cpp
  */

#include "game/score/turnscore.hpp"
#include "game/limits.hpp"


// /** Constructor.
//     \param turn        Turn number
//     \param time        Timestamp
//     \param num_fields  Number of fields to allocate */
game::score::TurnScore::TurnScore(int turn, Timestamp time)
    : m_turnNumber(turn),
      m_timestamp(time)
{
    // ex GStatRecord::GStatRecord
}

// /** Destructor. */
game::score::TurnScore::~TurnScore()
{ }

// /** Get turn number. */
int
game::score::TurnScore::getTurnNumber() const
{
    // ex GStatRecord::getTurnNumber
    return m_turnNumber;
}

// /** Get timestamp of this turn. */
const game::Timestamp&
game::score::TurnScore::getTimestamp() const
{
    // ex GStatRecord::getTimestamp
    return m_timestamp;
}


void
game::score::TurnScore::set(Slot_t slot, int player, Value_t value)
{
    // ex GStatRecord::operator(), sort-of
    if (player > 0 && player <= MAX_PLAYERS) {
        size_t index = slot * MAX_PLAYERS + (player-1);
        if (index >= m_values.size() && value.isValid()) {
            m_values.resize(index+1);
        }
        if (index < m_values.size()) {
            m_values[index] = value;
        }
    }
}


game::score::TurnScore::Value_t
game::score::TurnScore::get(Slot_t slot, int player) const
{
    if (player > 0 && player <= MAX_PLAYERS) {
        size_t index = slot * MAX_PLAYERS + (player-1);
        if (index < m_values.size()) {
            return m_values[index];
        } else {
            return afl::base::Nothing;
        }
    } else {
        return afl::base::Nothing;
    }
}
