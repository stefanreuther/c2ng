/**
  *  \file test/game/proxy/classicvcrplayerproxytest.cpp
  *  \brief Test for game::proxy::ClassicVcrPlayerProxy
  */

#include "game/proxy/classicvcrplayerproxy.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/vcr/classic/battle.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/classic/eventlistener.hpp"
#include "game/vcr/classic/eventrecorder.hpp"
#include "util/simplerequestdispatcher.hpp"

namespace {
    using game::vcr::classic::Side;
    using game::vcr::classic::LeftSide;
    using game::vcr::classic::RightSide;
    using game::vcr::classic::FighterStatus;

    struct Environment {
        afl::base::Ref<game::Root> root;
        afl::base::Ref<game::spec::ShipList> shipList;
        game::TeamSettings* pTeamSettings;
        afl::base::Ref<game::vcr::classic::Database> battles;
        afl::string::NullTranslator translator;
        afl::sys::Log log;
        size_t currentBattle;

        Environment()
            : root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)))),
              shipList(*new game::spec::ShipList()), pTeamSettings(0), battles(*new game::vcr::classic::Database()), translator(), currentBattle(0)
            {
                game::test::initStandardBeams(*shipList);
                game::test::initStandardTorpedoes(*shipList);
            }
    };

    class TestAdaptor : public game::proxy::VcrDatabaseAdaptor {
     public:
        TestAdaptor(Environment& env)
            : m_env(env)
            { }
        virtual afl::base::Ref<const game::Root> getRoot() const
            { return m_env.root; }
        virtual afl::base::Ref<const game::spec::ShipList> getShipList() const
            { return m_env.shipList; }
        virtual const game::TeamSettings* getTeamSettings() const
            { return m_env.pTeamSettings; }
        virtual afl::base::Ref<game::vcr::Database> getBattles()
            { return m_env.battles; }
        virtual afl::string::Translator& translator()
            { return m_env.translator; }
        virtual afl::sys::LogListener& log()
            { return m_env.log; }
        virtual afl::io::FileSystem& fileSystem()
            { return m_fileSystem; }
        virtual size_t getCurrentBattle() const
            { return m_env.currentBattle; }
        virtual void setCurrentBattle(size_t n)
            { m_env.currentBattle = n; }
        virtual game::sim::Setup* getSimulationSetup() const
            { return 0; }
        virtual bool isGameObject(const game::vcr::Object&) const
            { return false; }
     private:
        Environment& m_env;
        afl::io::NullFileSystem m_fileSystem;
    };

    game::vcr::Object makeLeftShip()
    {
        game::vcr::Object left;
        left.setMass(150);
        left.setCrew(2);
        left.setId(14);
        left.setOwner(2);
        left.setBeamType(0);
        left.setNumBeams(0);
        left.setNumBays(0);
        left.setTorpedoType(0);
        left.setNumLaunchers(0);
        left.setNumTorpedoes(0);
        left.setNumFighters(0);
        left.setShield(100);
        left.setName("Liz");
        return left;
    }

    game::vcr::Object makeRightShip()
    {
        game::vcr::Object right;
        right.setMass(233);
        right.setCrew(240);
        right.setId(434);
        right.setOwner(3);
        right.setBeamType(5);
        right.setNumBeams(6);
        right.setNumBays(0);
        right.setTorpedoType(7);
        right.setNumLaunchers(4);
        right.setNumTorpedoes(0);
        right.setNumFighters(0);
        right.setShield(100);
        right.setName("Bird");
        return right;
    }

    struct EventReceiver {
        EventReceiver()
            : m_events(), m_done(false), m_error()
            { }

        void onEvent(util::StringInstructionList& events, bool done)
            { m_events = events; m_done = done; }

        void onError(String_t err)
            { m_error = err; }

        util::StringInstructionList m_events;
        bool m_done;
        String_t m_error;
    };

    class PlacementVerifier : public game::vcr::classic::EventListener {
     public:
        PlacementVerifier()
            { m_position[LeftSide] = m_position[RightSide] = -1; }
        virtual void placeObject(Side side, const UnitInfo& info)
            { m_position[side] = info.position; }
        virtual void updateTime(game::vcr::classic::Time_t /*time*/, int32_t /*distance*/)
            { }
        virtual void startFighter(Side /*side*/, int /*track*/, int /*position*/, int /*distance*/, int /*fighterDiff*/)
            { }
        virtual void landFighter(Side /*side*/, int /*track*/, int /*fighterDiff*/)
            { }
        virtual void killFighter(Side /*side*/, int /*track*/)
            { }
        virtual void fireBeam(Side /*side*/, int /*track*/, int /*target*/, int /*hit*/, int /*damage*/, int /*kill*/, const HitEffect& /*effect*/)
            { }
        virtual void fireTorpedo(Side /*side*/, int /*hit*/, int /*launcher*/, int /*torpedoDiff*/, const HitEffect& /*effect*/)
            { }
        virtual void updateBeam(Side /*side*/, int /*id*/, int /*value*/)
            { }
        virtual void updateLauncher(Side /*side*/, int /*id*/, int /*value*/)
            { }
        virtual void moveObject(Side /*side*/, int /*position*/)
            { }
        virtual void moveFighter(Side /*side*/, int /*track*/, int /*position*/, int /*distance*/, FighterStatus /*status*/)
            { }
        virtual void killObject(Side /*side*/)
            { }
        virtual void updateObject(Side /*side*/, int /*damage*/, int /*crew*/, int /*shield*/)
            { }
        virtual void updateAmmo(Side /*side*/, int /*numTorpedoes*/, int /*numFighters*/)
            { }
        virtual void updateFighter(Side /*side*/, int /*track*/, int /*position*/, int /*distance*/, FighterStatus /*status*/)
            { }
        virtual void setResult(game::vcr::classic::BattleResult_t /*result*/)
            { }

        int getPosition(Side side) const
            { return m_position[side]; }

     private:
        int m_position[2];
    };

    void testError(afl::test::Assert a, Environment& env, size_t index)
    {
        // Set up tasking
        util::SimpleRequestDispatcher disp;
        TestAdaptor ad(env);
        util::RequestReceiver<game::proxy::VcrDatabaseAdaptor> recv(disp, ad);

        // Make proxy
        game::proxy::ClassicVcrPlayerProxy proxy(recv.getSender(), disp);
        EventReceiver event;
        proxy.sig_event.add(&event, &EventReceiver::onEvent);
        proxy.sig_error.add(&event, &EventReceiver::onError);

        // Load the fight
        proxy.initRequest(index);
        while (disp.wait(0))
            ;

        a.checkEqual("01. size", event.m_events.size(), 0U);
        a.check("02. m_done", event.m_done);
        a.checkDifferent("03. m_error", event.m_error, "");
    }
}

/** Test normal scenario (happy path).
    A: define a battle. Play it; rewind it.
    E: events generated as expected */
AFL_TEST("game.proxy.ClassicVcrPlayerProxy:normal", a)
{
    // Make simple environment
    Environment env;
    env.battles->addNewBattle(new game::vcr::classic::Battle(makeLeftShip(), makeRightShip(), 42, 0))
        ->setType(game::vcr::classic::PHost4, 0);

    // Set up tasking
    // Use just one RequestDispatcher to serve both sides
    util::SimpleRequestDispatcher disp;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::VcrDatabaseAdaptor> recv(disp, ad);

    // Make proxy
    game::proxy::ClassicVcrPlayerProxy proxy(recv.getSender(), disp);
    EventReceiver event;
    proxy.sig_event.add(&event, &EventReceiver::onEvent);
    proxy.sig_error.add(&event, &EventReceiver::onError);

    // Load first fight
    proxy.initRequest(0);
    while (disp.wait(0))
        ;
    a.check("01. events", event.m_events.size() > 0);
    a.check("02. m_done", !event.m_done);
    a.checkEqual("03. m_error", event.m_error, "");

    // Verify
    {
        PlacementVerifier v;
        game::vcr::classic::EventRecorder rec;
        rec.swapContent(event.m_events);
        rec.replay(v);
        a.checkEqual("11. left position", v.getPosition(LeftSide), 37);
        a.checkEqual("12. right position", v.getPosition(RightSide), 603);
    }

    // Load more events until we're done.
    // Fetching events after done is harmless.
    for (int i = 0; i < 30; ++i) {
        proxy.eventRequest();
        while (disp.wait(0))
            ;
        a.check("21. events", event.m_events.size() > 0);
        event.m_events.clear();
    }
    a.check("22. m_done", event.m_done);
    a.checkEqual("23. m_error", event.m_error, "");

    // Jump. This will produce a new position.
    proxy.jumpRequest(52);
    while (disp.wait(0))
        ;
    a.check("31. events", event.m_events.size() > 0);
    a.check("32. m_done", !event.m_done);
    a.checkEqual("33. m_error", event.m_error, "");
}

/** Test error: bad algorithm.
    A: define a battle with an unknown algorithm.
    E: error generated but no events */
AFL_TEST("game.proxy.ClassicVcrPlayerProxy:error:bad-algorithm", a)
{
    Environment env;
    env.battles->addNewBattle(new game::vcr::classic::Battle(makeLeftShip(), makeRightShip(), 42, 0))
        ->setType(game::vcr::classic::UnknownPHost, 0);

    testError(a, env, 0);
}

/** Test error: bad content.
    A: define a battle with bad conten (too many beams).
    E: error generated but no events */
AFL_TEST("game.proxy.ClassicVcrPlayerProxy:error:bad-content", a)
{
    Environment env;
    game::vcr::Object leftShip = makeLeftShip();
    leftShip.setNumBeams(77);
    env.battles->addNewBattle(new game::vcr::classic::Battle(leftShip, makeRightShip(), 42, 0))
        ->setType(game::vcr::classic::PHost4, 0);

    testError(a, env, 0);
}

/** Test error: bad index.
    A: try to play a battle with an out-of-range index.
    E: error generated but no events */
AFL_TEST("game.proxy.ClassicVcrPlayerProxy:error:bad-index", a)
{
    Environment env;
    testError(a, env, 1);
}

/** Test error: bad capabilities.
    A: try to play a battle with bad capabilities.
    E: error generated but no events */
AFL_TEST("game.proxy.ClassicVcrPlayerProxy:error:bad-capabilities", a)
{
    Environment env;
    game::vcr::Object leftShip = makeLeftShip();
    leftShip.setNumBeams(77);
    env.battles->addNewBattle(new game::vcr::classic::Battle(leftShip, makeRightShip(), 42, 0))
        ->setType(game::vcr::classic::PHost4, -1);    /* all bits set = lots of unknown capabilities */

    testError(a, env, 0);
}
