/**
  *  \file test/server/play/outmessageindexpackertest.cpp
  *  \brief Test for server::play::OutMessageIndexPacker
  */

#include "server/play/outmessageindexpacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/msg/outbox.hpp"
#include "game/turn.hpp"
#include <memory>

AFL_TEST("server.play.OutMessageIndexPacker", a)
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
    a.checkEqual("01. getName", testee.getName(), "outidx");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    // Verify
    a.checkEqual("11. getArraySize", ap.getArraySize(), 3U);
    a.checkEqual("12", ap[0].toInteger(), i);
    a.checkEqual("13", ap[1].toInteger(), j);
    a.checkEqual("14", ap[2].toInteger(), k);
}
