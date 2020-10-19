/**
  *  \file u/t_server_play_outmessageindexpacker.cpp
  *  \brief Test for server::play::OutMessageIndexPacker
  */

#include "server/play/outmessageindexpacker.hpp"

#include <memory>
#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/msg/outbox.hpp"
#include "game/turn.hpp"

void
TestServerPlayOutMessageIndexPacker::testIt()
{
    afl::base::Ref<game::Game> game(*new game::Game());
    game::msg::Outbox& outbox = game->currentTurn().outbox();

    // Create some messages (their content does not matter)
    game::Id_t i = outbox.addMessage(1, "a", game::PlayerSet_t(7) + 9);
    game::Id_t j = outbox.addMessage(3, "b", game::PlayerSet_t(2) + 4);
    game::Id_t k = outbox.addMessage(1, "c", game::PlayerSet_t(1) + 9);

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(game.asPtr());

    // Testee
    server::play::OutMessageIndexPacker testee(session);
    TS_ASSERT_EQUALS(testee.getName(), "outidx");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    // Verify
    TS_ASSERT_EQUALS(a.getArraySize(), 3U);
    TS_ASSERT_EQUALS(a[0].toInteger(), i);
    TS_ASSERT_EQUALS(a[1].toInteger(), j);
    TS_ASSERT_EQUALS(a[2].toInteger(), k);
}

