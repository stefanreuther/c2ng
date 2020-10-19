/**
  *  \file u/t_server_play_outmessagepacker.cpp
  *  \brief Test for server::play::OutMessagePacker
  */

#include "server/play/outmessagepacker.hpp"

#include <memory>
#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/msg/outbox.hpp"
#include "game/turn.hpp"

void
TestServerPlayOutMessagePacker::testIt()
{
    afl::base::Ref<game::Game> game(*new game::Game());
    game::Id_t id = game->currentTurn().outbox().addMessage(1, "hi there", game::PlayerSet_t(7) + 9);

    // Check message Id. This is not contractual, but used for getName() later on.
    TS_ASSERT_EQUALS(id, 1);

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(game.asPtr());

    // Testee
    server::play::OutMessagePacker testee(session, id);
    TS_ASSERT_EQUALS(testee.getName(), "outmsg1");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    // Verify
    TS_ASSERT_EQUALS(a("TEXT").toString(), "hi there");
    TS_ASSERT_EQUALS(a("TO").getArraySize(), 2U);
    TS_ASSERT_EQUALS(a("TO")[0].toInteger(), 7);
    TS_ASSERT_EQUALS(a("TO")[1].toInteger(), 9);
}

