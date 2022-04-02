/**
  *  \file game/proxy/currentstarbaseadaptor.cpp
  *  \brief Class game::proxy::CurrentStarbaseAdaptor
  */

#include "game/proxy/currentstarbaseadaptor.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/shiputils.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"

game::proxy::CurrentStarbaseAdaptor::CurrentStarbaseAdaptor(Session& session, Id_t planetId)
    : StarbaseAdaptor(),
      m_session(session),
      m_pGame(session.getGame()),
      m_pShipList(session.getShipList()),
      m_game(game::actions::mustHaveGame(session)),
      m_shipList(game::actions::mustHaveShipList(session)),
      m_planet(game::actions::mustExist(m_game.currentTurn().universe().planets().get(planetId)))
{ }

game::map::Planet&
game::proxy::CurrentStarbaseAdaptor::planet()
{
    return m_planet;
}

game::Session&
game::proxy::CurrentStarbaseAdaptor::session()
{
    return m_session;
}

bool
game::proxy::CurrentStarbaseAdaptor::findShipCloningHere(Id_t& id, String_t& name)
{
    const game::map::Universe& univ = m_game.currentTurn().universe();
    if (Id_t shipId = univ.findShipCloningAt(m_planet.getId())) {
        const game::map::Ship* sh = univ.ships().get(shipId);
        id = shipId;
        name = (sh != 0 ? sh->getName() : String_t());
        return true;
    } else {
        return false;
    }
}

void
game::proxy::CurrentStarbaseAdaptor::cancelAllCloneOrders()
{
    game::map::cancelAllCloneOrders(m_game.currentTurn().universe(), m_planet, m_shipList.friendlyCodes(), m_session.rng());
}

void
game::proxy::CurrentStarbaseAdaptor::notifyListeners()
{
    m_session.notifyListeners();
}

/*
 *  CurrentStarbaseAdaptorFromSession
 */

game::proxy::CurrentStarbaseAdaptor*
game::proxy::CurrentStarbaseAdaptorFromSession::call(Session& session)
{
    return new CurrentStarbaseAdaptor(session, m_planetId);
}
