/**
  *  \file game/score/compoundscore.cpp
  *  \brief Class game::score::CompoundScore
  */

#include "game/score/compoundscore.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/limits.hpp"

// Default constructor.
game::score::CompoundScore::CompoundScore()
    : m_valid(true),
      m_numParts(0)
{ }

// Construct single-slot score.
game::score::CompoundScore::CompoundScore(const TurnScoreList& list, ScoreId_t id, int factor)
    : m_valid(true),
      m_numParts(0)
{
    add(list, id, factor);
}

// Construct default score.
game::score::CompoundScore::CompoundScore(const TurnScoreList& list, DefaultScore kind)
    : m_valid(true),
      m_numParts(0)
{
    switch (kind) {
     case TotalShips:
        add(list, ScoreId_Freighters, 1);
        add(list, ScoreId_Capital, 1);
        break;

     case TimScore:
        // ex accessor.pas:Score (sort-of)
        add(list, ScoreId_Freighters, 1);
        add(list, ScoreId_Capital, 10);
        add(list, ScoreId_Planets, 10);
        add(list, ScoreId_Bases, 120);
        break;
    }
}

// Add a score component.
void
game::score::CompoundScore::add(const TurnScoreList& list, ScoreId_t id, int factor)
{
    // WScore::add, sort-of
    if (m_numParts >= MAX) {
        // Cannot represent this; fail
        m_valid = false;
    } else if (!list.getSlot(id, m_slot[m_numParts])) {
        // Slot not present in source data; fail
        m_valid = false;
    } else {
        // OK
        m_factor[m_numParts] = factor;
        ++m_numParts;
    }
}

// Get score from turn, single player.
game::score::CompoundScore::Value_t
game::score::CompoundScore::get(const TurnScore& turn, int player) const
{
    return get(turn, PlayerSet_t(player));
}

// Get score from turn, player list.
game::score::CompoundScore::Value_t
game::score::CompoundScore::get(const TurnScore& turn, PlayerSet_t players) const
{
    // WScore::get, sort-of
    if (!m_valid) {
        return afl::base::Nothing;
    } else if (m_numParts == 0) {
        return 0;
    } else {
        int32_t sum = 0;
        bool did = false;
        for (size_t i = 0, n = m_numParts; i < n; ++i) {
            for (int pl = 1; pl <= MAX_PLAYERS; ++pl) {
                if (players.contains(pl)) {
                    int32_t value;
                    if (turn.get(m_slot[i], pl).get(value)) {
                        sum += m_factor[i] * value;
                        did = true;
                    }
                }
            }
        }
        if (did) {
            return sum;
        } else {
            return afl::base::Nothing;
        }
    }
}

// Get score from turn, single player.
game::score::CompoundScore::Value_t
game::score::CompoundScore::get(const TurnScoreList& list, int turnNr, int player) const
{
    return get(list, turnNr, PlayerSet_t(player));
}

// Get score from turn, player list.
game::score::CompoundScore::Value_t
game::score::CompoundScore::get(const TurnScoreList& list, int turnNr, PlayerSet_t players) const
{
    if (const TurnScore* turn = list.getTurn(turnNr)) {
        return get(*turn, players);
    } else {
        if (m_numParts == 0) {
            return 0;
        } else {
            return afl::base::Nothing;
        }
    }
}
