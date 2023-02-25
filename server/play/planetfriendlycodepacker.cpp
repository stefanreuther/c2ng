/**
  *  \file server/play/planetfriendlycodepacker.cpp
  */

#include <stdexcept>
#include "server/play/planetfriendlycodepacker.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "server/errors.hpp"
#include "server/play/shipfriendlycodepacker.hpp"

using game::spec::FriendlyCodeList;

server::play::PlanetFriendlyCodePacker::PlanetFriendlyCodePacker(game::Session& session, game::Id_t planetId)
    : m_session(session),
      m_planetId(planetId)
{ }

server::Value_t*
server::play::PlanetFriendlyCodePacker::buildValue() const
{
    // ex ServerPlanetFCWriter::write
    game::Root& root = game::actions::mustHaveRoot(m_session);
    game::Game& game = game::actions::mustHaveGame(m_session);
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);

    game::map::Planet* pPlanet = game.currentTurn().universe().planets().get(m_planetId);
    if (pPlanet == 0) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    FriendlyCodeList list(sl.friendlyCodes(), game::spec::FriendlyCode::Filter::fromPlanet(*pPlanet, root.hostConfiguration()), root.registrationKey());
    return ShipFriendlyCodePacker::buildFriendlyCodeList(list, root.playerList(), m_session.translator());
}

String_t
server::play::PlanetFriendlyCodePacker::getName() const
{
    return afl::string::Format("planetfc%d", m_planetId);
}
