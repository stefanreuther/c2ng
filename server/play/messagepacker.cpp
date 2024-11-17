/**
  *  \file server/play/messagepacker.cpp
  *  \brief Class server::play::MessagePacker
  */

#include <stdexcept>
#include "server/play/messagepacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/inboxcontext.hpp"
#include "game/msg/inbox.hpp"
#include "game/playerlist.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "server/errors.hpp"

server::play::MessagePacker::MessagePacker(game::Session& session, int index)
    : m_session(session),
      m_index(index)
{ }

server::Value_t*
server::play::MessagePacker::buildValue() const
{
    // ex ServerMessageWriter::write
    // Preconditions
    game::Game& g = game::actions::mustHaveGame(m_session);
    game::msg::Inbox& inbox = g.currentTurn().inbox();

    // Validate number
    if (m_index <= 0 || size_t(m_index) > inbox.getNumMessages()) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }
    size_t realIndex = size_t(m_index - 1);

    // Build result
    afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
    game::interface::InboxContext ctx(realIndex, m_session, g.currentTurn());

    addValue(*hv, ctx, "GROUP", "GROUP");
    addValue(*hv, ctx, "FULLTEXT", "TEXT");
    addValue(*hv, ctx, "LINK", "LINK");
    addValue(*hv, ctx, "LINK2", "LINK2");
    addValue(*hv, ctx, "PARTNER", "PARTNER");
    addValue(*hv, ctx, "PARTNER.ALL", "PARTNER.ALL");
    addValue(*hv, ctx, "DATASTATUS", "DATASTATUS");

    return new afl::data::HashValue(hv);
}

String_t
server::play::MessagePacker::getName() const
{
    return afl::string::Format("msg%d", m_index);
}
