/**
  *  \file server/play/shipfriendlycodepacker.cpp
  */

#include <stdexcept>
#include "server/play/shipfriendlycodepacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/values.hpp"
#include "server/errors.hpp"

using game::spec::FriendlyCodeList;

server::play::ShipFriendlyCodePacker::ShipFriendlyCodePacker(game::Session& session, game::Id_t shipId)
    : m_session(session),
      m_shipId(shipId)
{ }

server::Value_t*
server::play::ShipFriendlyCodePacker::buildValue() const
{
    // ex ServerShipFCWriter::write
    game::Root& root = game::actions::mustHaveRoot(m_session);
    game::Game& game = game::actions::mustHaveGame(m_session);
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);

    game::map::Ship* pShip = game.currentTurn().universe().ships().get(m_shipId);
    if (pShip == 0) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    FriendlyCodeList list(sl.friendlyCodes(), game::spec::FriendlyCode::Filter::fromShip(*pShip, game.shipScores(), sl, root.hostConfiguration()), root.registrationKey());
    return buildFriendlyCodeList(list, root.playerList(), m_session.translator());
}

String_t
server::play::ShipFriendlyCodePacker::getName() const
{
    return afl::string::Format("shipfc%d", m_shipId);
}

server::Value_t*
server::play::ShipFriendlyCodePacker::buildFriendlyCodeList(const game::spec::FriendlyCodeList& list,
                                                            const game::PlayerList& players,
                                                            afl::string::Translator& tx)
{
    // ex writeFCodesFor
    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    for (FriendlyCodeList::Iterator_t it = list.begin(); it != list.end(); ++it) {
        if (const game::spec::FriendlyCode* fc = *it) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            hv->setNew("fc", interpreter::makeStringValue(fc->getCode()));
            hv->setNew("desc", interpreter::makeStringValue(fc->getDescription(players, tx)));
            vv->pushBackNew(new afl::data::HashValue(hv));
        }
    }
    return new afl::data::VectorValue(vv);
}
