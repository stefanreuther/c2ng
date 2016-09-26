/**
  *  \file game/historyturn.cpp
  *  \brief Class game::HistoryTurn
  */

#include "game/historyturn.hpp"
#include "game/turn.hpp"

// Constructor.
game::HistoryTurn::HistoryTurn(int nr)
    : m_turnNumber(nr),
      m_timestamp(),
      m_status(Unknown),
      m_turn()
{ }

// Destructor.
game::HistoryTurn::~HistoryTurn()
{ }

// Get turn number.
int
game::HistoryTurn::getTurnNumber() const
{
    return m_turnNumber;
}

// Set timestamp.
void
game::HistoryTurn::setTimestamp(const Timestamp& ts)
{
    m_timestamp = ts;
}

// Get timestamp.
const game::Timestamp&
game::HistoryTurn::getTimestamp() const
{
    return m_timestamp;
}

// Set status.
void
game::HistoryTurn::setStatus(Status st)
{
    m_status = st;
}

// Get status.
game::HistoryTurn::Status
game::HistoryTurn::getStatus() const
{
    return m_status;
}

// Check whether this turn can be loaded.
bool
game::HistoryTurn::isLoadable() const
{
    return m_turn.get() == 0
        && (m_status == StronglyAvailable || m_status == WeaklyAvailable || m_status == Unknown);
}

// Handle successful load.
void
game::HistoryTurn::handleLoadSucceeded(afl::base::Ptr<Turn> turn)
{
    // Do not change the status if we are not actually loadable.
    // This also avoids that we overwrite a previous Turn pointer,
    // which other components might have references to.
    if (isLoadable()) {
        m_turn = turn;
        m_status = Loaded;

        m_timestamp = turn->universe().getTimestamp();
        // FIXME: validate turn? Must be non-null and have correct number
    }
}

// Handle unsuccessful load.
void
game::HistoryTurn::handleLoadFailed()
{
    // Do not change the status if we are not actually loadable
    if (isLoadable()) {
        if (m_status == StronglyAvailable) {
            // They said it would be available, but it isn't. This is an error.
            m_status = Failed;
        } else {
            // We were optimistic but disappointed.
            m_status = Unavailable;
        }
    }
}

// Get turn.
afl::base::Ptr<game::Turn>
game::HistoryTurn::getTurn() const
{
    return m_turn;
}
