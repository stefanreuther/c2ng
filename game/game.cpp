/**
  *  \file game/game.cpp
  */

#include "game/game.hpp"
#include "game/turn.hpp"

game::Game::Game()
    : sig_viewpointTurnChange(),
      m_currentTurn(new Turn()),
      m_planetScores(),
      m_shipScores(),
      m_teamSettings(),
      m_viewpointTurnNumber(0),
      m_scores(),
      m_cursors(),
      m_markings()
{
    m_cursors.setUniverse(&m_currentTurn->universe());
}

game::Game::~Game()
{ }

game::Turn&
game::Game::currentTurn()
{
    return *m_currentTurn;
}

const game::Turn&
game::Game::currentTurn() const
{
    return *m_currentTurn;
}

game::HistoryTurnList&
game::Game::previousTurns()
{
    return m_previousTurns;
}

const game::HistoryTurnList&
game::Game::previousTurns() const
{
    return m_previousTurns;
}

game::UnitScoreDefinitionList&
game::Game::planetScores()
{
    return m_planetScores;
}

const game::UnitScoreDefinitionList&
game::Game::planetScores() const
{
    return m_planetScores;
}

game::UnitScoreDefinitionList&
game::Game::shipScores()
{
    return m_shipScores;
}

const game::UnitScoreDefinitionList&
game::Game::shipScores() const
{
    return m_shipScores;
}

int
game::Game::getViewpointPlayer() const
{
    return m_teamSettings.getViewpointPlayer();
}

void
game::Game::setViewpointPlayer(int pid)
{
    m_teamSettings.setViewpointPlayer(pid);
}

afl::base::Ptr<game::Turn>
game::Game::getViewpointTurn() const
{
    if (m_viewpointTurnNumber == 0 || m_viewpointTurnNumber == currentTurn().getTurnNumber()) {
        return m_currentTurn;
    } else if (HistoryTurn* ht = m_previousTurns.get(m_viewpointTurnNumber)) {
        return ht->getTurn();
    } else {
        return 0;
    }
}

int
game::Game::getViewpointTurnNumber() const
{
    if (m_viewpointTurnNumber == 0) {
        return currentTurn().getTurnNumber();
    } else {
        return m_viewpointTurnNumber;
    }
}

void
game::Game::setViewpointTurnNumber(int nr)
{
    // Change turn number
    Turn* oldTurn = getViewpointTurn().get();
    m_viewpointTurnNumber = nr;
    Turn* newTurn = getViewpointTurn().get();

    // Update
    if (oldTurn != newTurn) {
        // Transfer selection to new turn
        // FIXME: the limitToExistingObjects() will unmark objects that don't exist in the new turn.
        // It would be nice if we could avoid that.
        // However, the copyFrom() will already unmark nonexistant objects,
        // effectively doing the equivalent of limitToExistingObjects().
        // Until we can somehow avoid that, keep the limitToExistingObjects().
        if (oldTurn != 0) {
            m_markings.copyFrom(oldTurn->universe(), m_markings.getCurrentLayer());
        }
        if (newTurn != 0) {
            m_markings.copyTo(newTurn->universe(), m_markings.getCurrentLayer());
            m_markings.limitToExistingObjects(newTurn->universe(), m_markings.getCurrentLayer());
        }

        // Change cursor
        m_cursors.setUniverse(newTurn != 0 ? &newTurn->universe() : 0);
        sig_viewpointTurnChange.raise();
    }
}

game::TeamSettings&
game::Game::teamSettings()
{
    return m_teamSettings;
}

const game::TeamSettings&
game::Game::teamSettings() const
{
    return m_teamSettings;
}

game::score::TurnScoreList&
game::Game::scores()
{
    return m_scores;
}

const game::score::TurnScoreList&
game::Game::scores() const
{
    return m_scores;
}

game::map::Cursors&
game::Game::cursors()
{
    return m_cursors;
}

game::map::Markings&
game::Game::markings()
{
    return m_markings;
}

game::msg::Configuration&
game::Game::messageConfiguration()
{
    return m_messageConfiguration;
}

const game::msg::Configuration&
game::Game::messageConfiguration() const
{
    return m_messageConfiguration;
}

void
game::Game::notifyListeners()
{
    // FIXME: as of 20180101, the script side only operates on "current", but the C++ GUI side partially displays "viewpoint".
    // Thus, notify both.

    // Current turn
    Turn* t1 = m_currentTurn.get();
    if (t1 != 0) {
        t1->notifyListeners();
    }

    // Viewpoint turn
    Turn* t2 = getViewpointTurn().get();
    if (t2 != 0 && t2 != t1) {
        t2->notifyListeners();
    }
}
