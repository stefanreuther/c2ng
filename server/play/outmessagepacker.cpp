/**
  *  \file server/play/outmessagepacker.cpp
  *  \brief Class server::play::OutMessagePacker
  */

#include "server/play/outmessagepacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/msg/outbox.hpp"
#include "game/turn.hpp"

namespace {
    server::Value_t* packPlayerSet(game::PlayerSet_t s)
    {
        afl::base::Ref<afl::data::Vector> vv = afl::data::Vector::create();
        for (int i = 0; i <= game::MAX_PLAYERS; ++i) {
            if (s.contains(i)) {
                vv->pushBackInteger(i);
            }
        }
        return new afl::data::VectorValue(vv);
    }
}

server::play::OutMessagePacker::OutMessagePacker(game::Session& session, game::Id_t id)
    : m_session(session),
      m_id(id)
{ }

server::Value_t*
server::play::OutMessagePacker::buildValue() const
{
    game::Game& g = game::actions::mustHaveGame(m_session);
    game::msg::Outbox& outbox = g.currentTurn().outbox();
    size_t index = 0;
    if (outbox.findMessageById(m_id, index)) {
        afl::base::Ref<afl::data::Hash> hv = afl::data::Hash::create();
        addValueNew(*hv, makeStringValue(outbox.getMessageRawText(index)), "TEXT");
        addValueNew(*hv, packPlayerSet(outbox.getMessageReceivers(index)), "TO");
        return new afl::data::HashValue(hv);
    } else {
        return 0;
    }
}

String_t
server::play::OutMessagePacker::getName() const
{
    return afl::string::Format("outmsg%d", m_id);
}

