/**
  *  \file server/play/outmessageindexpacker.cpp
  *  \brief Class server::play::OutMessageIndexPacker
  */

#include "server/play/outmessageindexpacker.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/msg/outbox.hpp"
#include "game/turn.hpp"

server::play::OutMessageIndexPacker::OutMessageIndexPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::OutMessageIndexPacker::buildValue() const
{
    game::Game& g = game::actions::mustHaveGame(m_session);
    game::msg::Outbox& outbox = g.currentTurn().outbox();

    afl::base::Ref<afl::data::Vector> vv = afl::data::Vector::create();
    for (size_t index = 0, n = outbox.getNumMessages(); index < n; ++index) {
        vv->pushBackInteger(outbox.getMessageId(index));
    }
    return new afl::data::VectorValue(vv);
}

String_t
server::play::OutMessageIndexPacker::getName() const
{
    return "outidx";
}
