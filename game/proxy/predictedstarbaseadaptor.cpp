/**
  *  \file game/proxy/predictedstarbaseadaptor.cpp
  *  \brief Class game::proxy::PredictedStarbaseAdaptor
  */

#include "game/proxy/predictedstarbaseadaptor.hpp"
#include "game/actions/preconditions.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"

game::proxy::PredictedStarbaseAdaptor::PredictedStarbaseAdaptor(Session& session, Id_t planetId, bool waitClear)
    : m_session(session),
      m_pGame(session.getGame()),
      m_pShipList(session.getShipList()),
      m_pRoot(session.getRoot()),
      m_game(game::actions::mustHaveGame(session)),
      m_pred(game::actions::mustExist(m_game.currentTurn().universe().planets().get(planetId)),
             m_game.currentTurn().universe(),
             game::actions::mustHaveShipList(session),
             game::actions::mustHaveRoot(session).hostConfiguration())
{
    // Predict
    // Because this will usually be called from the auto-task screen,
    // this will re-use the existing editor and not run the task on release.
    afl::base::Ptr<interpreter::TaskEditor> ed = m_session.getAutoTaskEditor(planetId, interpreter::Process::pkBaseTask, false);
    if (ed.get() != 0) {
        m_pred.predictTask(*ed, ed->getCursor());
        m_session.releaseAutoTaskEditor(ed);
    }

    // We may making an EnqueueShip command, which will wait until its
    // preceding command is performed. So do that now.
    if (waitClear && m_pred.planet().getBaseBuildOrderHullIndex().orElse(0) > 0) {
        m_pred.advanceTurn();
    }
}

game::proxy::PredictedStarbaseAdaptor::~PredictedStarbaseAdaptor()
{ }

game::map::Planet&
game::proxy::PredictedStarbaseAdaptor::planet()
{
    return m_pred.planet();
}

game::Session&
game::proxy::PredictedStarbaseAdaptor::session()
{
    return m_session;
}
bool
game::proxy::PredictedStarbaseAdaptor::findShipCloningHere(Id_t& /*id*/, String_t& /*name*/)
{
    return false;
}

void
game::proxy::PredictedStarbaseAdaptor::cancelAllCloneOrders()
{ }

void
game::proxy::PredictedStarbaseAdaptor::notifyListeners()
{ }


/*
 *  PredictedStarbaseAdaptorFromSession
 */

game::proxy::PredictedStarbaseAdaptor*
game::proxy::PredictedStarbaseAdaptorFromSession::call(Session& session)
{
    return new PredictedStarbaseAdaptor(session, m_planetId, m_waitClear);
}
