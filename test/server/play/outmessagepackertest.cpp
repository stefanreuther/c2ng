/**
  *  \file test/server/play/outmessagepackertest.cpp
  *  \brief Test for server::play::OutMessagePacker
  */

#include "server/play/outmessagepacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/msg/outbox.hpp"
#include "game/turn.hpp"
#include <memory>

AFL_TEST("server.play.OutMessagePacker", a)
{
    afl::base::Ref<game::Game> game(*new game::Game());
    game::Id_t id = game->currentTurn().outbox().addMessage(1, "hi there", game::PlayerSet_t(7) + 9);

    // Check message Id. This is not contractual, but used for getName() later on.
    a.checkEqual("01. id", id, 1);

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(game.asPtr());

    // Testee
    server::play::OutMessagePacker testee(session, id);
    a.checkEqual("11. getName", testee.getName(), "outmsg1");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    // Verify
    a.checkEqual("21", ap("TEXT").toString(), "hi there");
    a.checkEqual("22", ap("TO").getArraySize(), 2U);
    a.checkEqual("23", ap("TO")[0].toInteger(), 7);
    a.checkEqual("24", ap("TO")[1].toInteger(), 9);
}
