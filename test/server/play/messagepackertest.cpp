/**
  *  \file test/server/play/messagepackertest.cpp
  *  \brief Test for server::play::MessagePacker
  */

#include "server/play/messagepacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/hostversion.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"

using afl::base::Ref;
using afl::data::Access;
using game::Game;
using game::HostVersion;
using game::Root;
using game::Session;
using server::play::MessagePacker;

namespace {
    const char*const TEXT =
        "(-r2000)<<< Message >>>\n"
        "FROM: Player 2\n"
        "TO: Player 8\n\n"
        "text";
    const char*const TEXT2 =
        "(-p0363)<<< Planet >>>\n"
        "From a planet...";
    const char*const TEXT3 =
        "(-g0000)<<< Location >>>\n"
        "Contains a place: (1000,2000)";

    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        Session session;

        Environment()
            : tx(), fs(), session(tx, fs)
            {
                Ref<Game> game(*new Game());
                game->currentTurn().inbox().addMessage(TEXT, 10);
                game->currentTurn().inbox().addMessage(TEXT2, 11);
                game->currentTurn().inbox().addMessage(TEXT3, 10);

                Ref<Root> root(game::test::makeRoot(HostVersion()));
                for (int i = 1; i < 10; ++i) {
                    root->playerList().create(i);
                }

                // Session
                session.setGame(game.asPtr());
                session.setRoot(root.asPtr());
            }
    };
}

AFL_TEST("server.play.MessagePacker:basics:1", a)
{
    Environment env;

    MessagePacker testee(env.session, 1);
    a.checkEqual("01. getName", testee.getName(), "msg1");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    Access ap(value.get());
    a.checkEqual("11", ap("GROUP").toString(), "(r) Player 2");
    a.checkEqual("12", ap("TEXT").toString(), TEXT);
    a.checkEqual("13", ap("PARTNER").getArraySize(), 1U);
    a.checkEqual("14", ap("PARTNER")[0].toInteger(), 2);
    a.checkEqual("15", ap("PARTNER.ALL").getArraySize(), 2U);
    a.checkEqual("16", ap("PARTNER.ALL")[0].toInteger(), 2);
    a.checkEqual("17", ap("PARTNER.ALL")[1].toInteger(), 8);
}

AFL_TEST("server.play.MessagePacker:basics:2", a)
{
    Environment env;

    MessagePacker testee(env.session, 2);
    a.checkEqual("01. getName", testee.getName(), "msg2");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    Access ap(value.get());
    a.checkEqual("11", ap("GROUP").toString(), "(p) Planet");
    a.checkEqual("12", ap("TEXT").toString(), TEXT2);
    a.checkEqual("13", ap("LINK").getArraySize(), 2U);
    a.checkEqual("14", ap("LINK")[0].toString(), "planet");
    a.checkEqual("15", ap("LINK")[1].toInteger(), 363);
}

AFL_TEST("server.play.MessagePacker:basics:3", a)
{
    Environment env;

    MessagePacker testee(env.session, 3);
    a.checkEqual("01. getName", testee.getName(), "msg3");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    Access ap(value.get());
    a.checkEqual("11", ap("GROUP").toString(), "(g) HConfig");
    a.checkEqual("12", ap("TEXT").toString(), TEXT3);
    a.checkEqual("13", ap("LINK2").getArraySize(), 3U);
    a.checkEqual("14", ap("LINK2")[0].toString(), "location");
    a.checkEqual("15", ap("LINK2")[1].toInteger(), 1000);
    a.checkEqual("16", ap("LINK2")[2].toInteger(), 2000);

    a.checkEqual("21", ap("PARTNER").getArraySize(), 1U);
    a.checkEqual("22", ap("PARTNER")[0].toInteger(), 0);
    a.checkEqual("23", ap("PARTNER.ALL").getArraySize(), 1U);
    a.checkEqual("24", ap("PARTNER.ALL")[0].toInteger(), 0);
}

AFL_TEST("server.play.MessagePacker:out-of-range", a)
{
    Environment env;

    AFL_CHECK_THROWS(a("zero"), MessagePacker(env.session, 0).buildValue(), std::exception);
    AFL_CHECK_THROWS(a("big"), MessagePacker(env.session, 4).buildValue(), std::exception);
}

