/**
  *  \file test/server/play/gameaccesstest.cpp
  *  \brief Test for server::play::GameAccess
  */

#include "server/play/gameaccess.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "util/messagecollector.hpp"

using afl::data::Access;

namespace {
    struct Environment {
        afl::io::NullFileSystem fs;
        afl::string::NullTranslator tx;
        game::Session session;

        util::MessageCollector log;

        server::play::GameAccess testee;

        Environment()
            : fs(), tx(), session(tx, fs), log(), testee(session, log)
            {
                session.log().addListener(log);

                // Add shiplist
                afl::base::Ptr<game::spec::ShipList> sl(new game::spec::ShipList());
                game::test::initStandardBeams(*sl);
                game::test::initStandardTorpedoes(*sl);
                game::test::addOutrider(*sl);
                game::test::addNovaDrive(*sl);
                game::test::addTranswarp(*sl);
                session.setShipList(sl);

                // Add root
                afl::base::Ref<game::Root> r(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));
                r->playerList().create(1)->setName(game::Player::AdjectiveName, "Fed");
                r->playerList().create(2)->setName(game::Player::AdjectiveName, "Lizard");
                r->playerList().create(3)->setName(game::Player::AdjectiveName, "Bird");
                session.setRoot(r.asPtr());
            }
    };
}

/** Test getStatus().
    A: write a test message.
    E: first call to getStatus() produces the message (plus possible decoration...), next call produces empty result. */
AFL_TEST("server.play.GameAccess:getStatus", a)
{
    Environment env;
    env.session.log().write(afl::sys::LogListener::Trace, "TestChannel", "TestMessage");

    // Retrieve the message
    String_t result = env.testee.getStatus();
    a.check("01", result.find("TestChannel") != String_t::npos);
    a.check("02", result.find("TestMessage") != String_t::npos);

    // No more messages added since then
    result = env.testee.getStatus();
    a.checkEqual("11. getStatus", result, "");
}

/** Test get(), beams.
    A: 'GET obj/beam'
    E: correct result returned */
AFL_TEST("server.play.GameAccess:get:obj/beam", a)
{
    Environment env;
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/beam"));
    Access ap(result.get());
    a.checkNonNull("01", ap("beam").getValue());
    a.checkNull("02", ap("beam")[0].getValue());

    a.checkEqual("11", ap("beam")[1]("NAME").toString(), "Laser");
    a.checkEqual("12", ap("beam")[10]("NAME").toString(), "Heavy Phaser");
}

/** Test get(), torpedoes.
    A: 'GET obj/torp'
    E: correct result returned */
AFL_TEST("server.play.GameAccess:get:obj/torp", a)
{
    Environment env;
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/torp"));
    Access ap(result.get());
    a.checkNonNull("01", ap("torp").getValue());
    a.checkNull("02", ap("torp")[0].getValue());

    a.checkEqual("11", ap("torp")[1]("NAME").toString(), "Mark 1 Photon");
    a.checkEqual("12", ap("torp")[10]("NAME").toString(), "Mark 8 Photon");
}

/** Test get(), engines.
    A: 'GET obj/engine'
    E: correct result returned */
AFL_TEST("server.play.GameAccess:get:obj/engine", a)
{
    Environment env;
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/engine"));
    Access ap(result.get());
    a.checkNonNull("01", ap("engine").getValue());
    a.checkNull("02", ap("engine")[0].getValue());

    a.checkEqual("11", ap("engine")[5]("NAME").toString(), "Nova Drive 5");
    a.checkEqual("12", ap("engine")[9]("NAME").toString(), "Transwarp Drive");
}

/** Test get(), hull.
    A: 'GET obj/hull1'
    E: correct result returned */
AFL_TEST("server.play.GameAccess:get:obj/hull1", a)
{
    Environment env;
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/hull1"));
    Access ap(result.get());
    a.checkNonNull("01", ap("hull1").getValue());
    a.checkEqual("02", ap("hull1")("NAME").toString(), "OUTRIDER CLASS SCOUT");
}

/** Test get(), truehull.
    A: 'GET obj/truehull'
    E: correct result returned */
AFL_TEST("server.play.GameAccess:get:obj/truehull", a)
{
    Environment env;
    env.session.getShipList()->hullAssignments().add(/*player:*/ 2, /*slot:*/ 5, /*hull:*/ 1);
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/truehull"));
    Access ap(result.get());
    a.checkEqual("01", ap("truehull")[/*player:*/2][/*slot-1:*/4].toInteger(), 1);
}

/** Test get(), racename.
    A: 'GET obj/racename'
    E: correct result returned */
AFL_TEST("server.play.GameAccess:get:obj/racename", a)
{
    Environment env;
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/racename"));
    Access ap(result.get());
    a.checkEqual("01", ap("racename")[/*player:*/2]("RACE.ADJ").toString(), "Lizard");
}

/** Test get(), abilities.
    A: 'GET obj/zab'
    E: correct result returned */
AFL_TEST("server.play.GameAccess:get:obj/zab", a)
{
    Environment env;
    env.session.getShipList()->basicHullFunctions().addFunction(12, "Twelve");
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/zab"));
    Access ap(result.get());
    a.checkEqual("01", ap("zab")[0]("ID").toInteger(), 12);
    a.checkEqual("02", ap("zab")[0]("NAME").toString(), "Twelve");
}

/** Test get(), multiple objects.
    A: 'GET obj/x,y,z'
    E: data returned for all objects */
AFL_TEST("server.play.GameAccess:get:multiple", a)
{
    Environment env;
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/beam,hull1,engine"));
    Access ap(result.get());
    a.checkNonNull("01", ap("beam").getValue());
    a.checkNonNull("02", ap("hull1").getValue());
    a.checkNonNull("03", ap("engine").getValue());
}
