/**
  *  \file u/t_server_play_gameaccess.cpp
  *  \brief Test for server::play::GameAccess
  */

#include "server/play/gameaccess.hpp"

#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
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
                r->playerList().create(1);
                r->playerList().create(2);
                r->playerList().create(3);
                session.setRoot(r.asPtr());
            }
    };
}

/** Test getStatus().
    A: write a test message.
    E: first call to getStatus() produces the message (plus possible decoration...), next call produces empty result. */
void
TestServerPlayGameAccess::testGetStatus()
{
    Environment env;
    env.session.log().write(afl::sys::LogListener::Trace, "TestChannel", "TestMessage");

    // Retrieve the message
    String_t result = env.testee.getStatus();
    TS_ASSERT(result.find("TestChannel") != String_t::npos);
    TS_ASSERT(result.find("TestMessage") != String_t::npos);

    // No more messages added since then
    result = env.testee.getStatus();
    TS_ASSERT_EQUALS(result, "");
}

/** Test get(), beams.
    A: 'GET obj/beam'
    E: correct result returned */
void
TestServerPlayGameAccess::testGetBeam()
{
    Environment env;
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/beam"));
    Access a(result.get());
    TS_ASSERT(a("beam").getValue() != 0);
    TS_ASSERT(a("beam")[0].getValue() == 0);

    TS_ASSERT_EQUALS(a("beam")[1]("NAME").toString(), "Laser");
    TS_ASSERT_EQUALS(a("beam")[10]("NAME").toString(), "Heavy Phaser");
}

/** Test get(), torpedoes.
    A: 'GET obj/torp'
    E: correct result returned */
void
TestServerPlayGameAccess::testGetTorp()
{
    Environment env;
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/torp"));
    Access a(result.get());
    TS_ASSERT(a("torp").getValue() != 0);
    TS_ASSERT(a("torp")[0].getValue() == 0);

    TS_ASSERT_EQUALS(a("torp")[1]("NAME").toString(), "Mark 1 Photon");
    TS_ASSERT_EQUALS(a("torp")[10]("NAME").toString(), "Mark 8 Photon");
}

/** Test get(), engines.
    A: 'GET obj/engine'
    E: correct result returned */
void
TestServerPlayGameAccess::testGetEngine()
{
    Environment env;
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/engine"));
    Access a(result.get());
    TS_ASSERT(a("engine").getValue() != 0);
    TS_ASSERT(a("engine")[0].getValue() == 0);

    TS_ASSERT_EQUALS(a("engine")[5]("NAME").toString(), "Nova Drive 5");
    TS_ASSERT_EQUALS(a("engine")[9]("NAME").toString(), "Transwarp Drive");
}

/** Test get(), hull.
    A: 'GET obj/hull1'
    E: correct result returned */
void
TestServerPlayGameAccess::testGetHull()
{
    Environment env;
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/hull1"));
    Access a(result.get());
    TS_ASSERT(a("hull1").getValue() != 0);
    TS_ASSERT_EQUALS(a("hull1")("NAME").toString(), "OUTRIDER CLASS SCOUT");
}

/** Test get(), truehull.
    A: 'GET obj/truehull'
    E: correct result returned */
void
TestServerPlayGameAccess::testGetTruehull()
{
    Environment env;
    env.session.getShipList()->hullAssignments().add(/*player:*/ 2, /*slot:*/ 5, /*hull:*/ 1);
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/truehull"));
    Access a(result.get());
    TS_ASSERT_EQUALS(a("truehull")[/*player:*/2][/*slot-1:*/4].toInteger(), 1);
}

/** Test get(), abilities.
    A: 'GET obj/zab'
    E: correct result returned */
void
TestServerPlayGameAccess::testGetAbilities()
{
    Environment env;
    env.session.getShipList()->basicHullFunctions().addFunction(12, "Twelve");
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/zab"));
    Access a(result.get());
    TS_ASSERT_EQUALS(a("zab")[0]("ID").toInteger(), 12);
    TS_ASSERT_EQUALS(a("zab")[0]("NAME").toString(), "Twelve");
}

/** Test get(), multiple objects.
    A: 'GET obj/x,y,z'
    E: data returned for all objects */
void
TestServerPlayGameAccess::testGetMultiple()
{
    Environment env;
    std::auto_ptr<server::Value_t> result(env.testee.get("obj/beam,hull1,engine"));
    Access a(result.get());
    TS_ASSERT(a("beam").getValue() != 0);
    TS_ASSERT(a("hull1").getValue() != 0);
    TS_ASSERT(a("engine").getValue() != 0);
}

