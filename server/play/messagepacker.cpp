/**
  *  \file server/play/messagepacker.cpp
  */

#include <stdexcept>
#include "server/play/messagepacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
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
    game::Root& r = game::actions::mustHaveRoot(m_session);
    game::msg::Inbox& inbox = g.currentTurn().inbox();
    afl::string::Translator& tx = m_session.translator();
    game::PlayerList& pl = r.playerList();

    // Validate number
    if (m_index <= 0 || size_t(m_index) > inbox.getNumMessages()) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }
    size_t realIndex = size_t(m_index - 1);
    
    afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
    addValueNew(*hv, makeStringValue(inbox.getMessageHeading(realIndex, tx, pl)), "GROUP");
    addValueNew(*hv, makeStringValue(inbox.getMessageText(realIndex, tx, pl)), "TEXT");

    return new afl::data::HashValue(hv);
}

String_t
server::play::MessagePacker::getName() const
{
    return afl::string::Format("msg%d", m_index);
}
