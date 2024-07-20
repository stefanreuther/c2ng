/**
  *  \file test/server/play/imperialstatspackertest.cpp
  *  \brief Test for server::play::ImperialStatsPacker
  */

#include "server/play/imperialstatspacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/json/writer.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/hostversion.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"

using game::Player;

namespace {
    struct TestHarness {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        TestHarness()
            : tx(), fs(), session(tx, fs)
            {
            }
    };

    void createTurn(TestHarness& h)
    {
        h.session.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))).asPtr());
        h.session.setShipList(new game::spec::ShipList());
        h.session.setGame(new game::Game());
    }
}

/** Success case.
    This produces roughly the same data as "game.map.info.Browser:ColonyPage:empty". */
AFL_TEST("server.play.ImperialStatsPacker:success", a)
{
    TestHarness h;
    createTurn(h);

    server::play::ImperialStatsPacker testee(h.session, 3, 0);
    a.checkEqual("01. getName", testee.getName(), "istat3.0");    // ColonyPage, no options

    std::auto_ptr<afl::data::Value> p(testee.buildValue());

    afl::io::InternalSink sink;
    afl::io::json::Writer(sink).visit(p.get());
    a.checkEqual("11. buildValue", afl::string::fromBytes(sink.getContent()),
                 "{\"content\":[[\"h1\",{},\"Colony\"],"
                 "[\"table\",{\"align\":\"left\"},[\"tr\",{},[\"td\",{\"width\":\"16\"},[\"font\",{\"color\":\"white\"},\"Top 5 Colonists Planets\"]],[\"td\",{\"align\":\"right\",\"width\":\"8\"},\"(clans)\"]]],"
                 "[\"table\",{\"align\":\"left\"},[\"tr\",{},[\"td\",{\"width\":\"16\"},[\"font\",{\"color\":\"white\"},\"Top 5 Supplies Planets\"]],[\"td\",{\"align\":\"right\",\"width\":\"8\"},\"(kt)\"]]],"
                 "[\"table\",{\"align\":\"left\"},[\"tr\",{},[\"td\",{\"width\":\"16\"},[\"font\",{\"color\":\"white\"},\"Top 5 Money Planets\"]],[\"td\",{\"align\":\"right\",\"width\":\"8\"},\"(mc)\"]]]],"
                 "\"options\":[{\"text\":\"Show all info\",\"value\":0},{\"text\":\"Show only Colonists\",\"value\":16},{\"text\":\"Show only Supplies\",\"value\":32},{\"text\":\"Show only Money\",\"value\":48}]}");
}

/** Error case: no turn present.
    Fails on ImperialStatsPacker level because no NumberFormatter can be acquired. */
AFL_TEST("server.play.ImperialStatsPacker:error:no-turn", a)
{
    TestHarness h;

    server::play::ImperialStatsPacker testee(h.session, 3, 0);
    a.checkEqual("01. getName", testee.getName(), "istat3.0");    // ColonyPage, no options

    std::auto_ptr<afl::data::Value> p(testee.buildValue());
    a.checkNull("11. buildValue", p.get());
}

/** Error case: index out of range. */
AFL_TEST("server.play.ImperialStatsPacker:error:range", a)
{
    TestHarness h;
    createTurn(h);

    server::play::ImperialStatsPacker testee(h.session, 999, 0);
    a.checkEqual("01. getName", testee.getName(), "istat999.0");

    std::auto_ptr<afl::data::Value> p(testee.buildValue());
    a.checkNull("11. buildValue", p.get());
}
