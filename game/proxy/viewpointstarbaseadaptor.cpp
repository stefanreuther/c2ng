/**
  *  \file game/proxy/viewpointstarbaseadaptor.cpp
  *  \brief Class game::proxy::ViewpointStarbaseAdaptor
  */

#include "game/proxy/viewpointstarbaseadaptor.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/shiputils.hpp"
#include "game/map/universe.hpp"

game::proxy::ViewpointStarbaseAdaptor::ViewpointStarbaseAdaptor(Session& session, Id_t planetId)
    : StarbaseAdaptor(),
      m_session(session),
      m_turn(game::actions::mustHaveGame(session).viewpointTurn()),
      m_shipList(game::actions::mustHaveShipList(session)),
      m_planet(game::actions::mustExist(m_turn->universe().planets().get(planetId)))
{ }

game::map::Planet&
game::proxy::ViewpointStarbaseAdaptor::planet()
{
    return m_planet;
}

game::Session&
game::proxy::ViewpointStarbaseAdaptor::session()
{
    return m_session;
}

bool
game::proxy::ViewpointStarbaseAdaptor::findShipCloningHere(Id_t& id, String_t& name)
{
    const game::map::Universe& univ = m_turn->universe();
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
game::proxy::ViewpointStarbaseAdaptor::cancelAllCloneOrders()
{
    game::map::cancelAllCloneOrders(m_turn->universe(), m_planet, m_shipList->friendlyCodes(), m_session.rng());
}

void
game::proxy::ViewpointStarbaseAdaptor::notifyListeners()
{
    m_session.notifyListeners();
}

/*
 *  ViewpointStarbaseAdaptorFromSession
 */

game::proxy::ViewpointStarbaseAdaptor*
game::proxy::ViewpointStarbaseAdaptorFromSession::call(Session& session)
{
    return new ViewpointStarbaseAdaptor(session, m_planetId);
}
