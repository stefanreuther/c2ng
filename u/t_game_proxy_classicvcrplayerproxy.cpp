/**
  *  \file u/t_game_proxy_classicvcrplayerproxy.cpp
  *  \brief Test for game::proxy::ClassicVcrPlayerProxy
  */

#include "game/proxy/classicvcrplayerproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
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
        game::test::Root root;
        game::spec::ShipList shipList;
        game::TeamSettings* pTeamSettings;
        game::vcr::classic::Database battles;
        afl::string::NullTranslator translator;
        afl::sys::Log log;
        size_t currentBattle;

        Environment()
            : root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))),
              shipList(), pTeamSettings(0), battles(), translator(), currentBattle(0)
            { }
    };
    
    class TestAdaptor : public game::proxy::VcrDatabaseAdaptor {
     public:
        TestAdaptor(Environment& env)
            : m_env(env)
            { }
        virtual const game::Root& root() const
            { return m_env.root; }
        virtual const game::spec::ShipList& shipList() const
            { return m_env.shipList; }
        virtual const game::TeamSettings* getTeamSettings() const
            { return m_env.pTeamSettings; }
        virtual game::vcr::Database& battles()
            { return m_env.battles; }
        virtual afl::string::Translator& translator()
            { return m_env.translator; }
        virtual afl::sys::LogListener& log()
            { return m_env.log; }
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
            : m_events(), m_done(false)
            { }

        void onEvent(util::StringInstructionList& events, bool done)
            { m_events = events; m_done = done; }

        util::StringInstructionList m_events;
        bool m_done;
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
}

void
TestGameProxyClassicVcrPlayerProxy::testIt()
{
    // Make simple environment
    Environment env;
    game::test::initStandardBeams(env.shipList);
    game::test::initStandardTorpedoes(env.shipList);
    env.battles.addNewBattle(new game::vcr::classic::Battle(makeLeftShip(), makeRightShip(), 42, 0, 0))
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

    // Load first fight
    proxy.initRequest(0);
    while (disp.wait(0))
        ;
    TS_ASSERT(event.m_events.size() > 0);
    TS_ASSERT(!event.m_done);

    // Verify
    {
        PlacementVerifier v;
        game::vcr::classic::EventRecorder rec;
        rec.swapContent(event.m_events);
        rec.replay(v);
        TS_ASSERT_EQUALS(v.getPosition(LeftSide), 37);
        TS_ASSERT_EQUALS(v.getPosition(RightSide), 603);
    }

    // Load more events until we're done.
    // Fetching events after done is harmless.
    for (int i = 0; i < 30; ++i) {
        proxy.eventRequest();
        while (disp.wait(0))
            ;
        TS_ASSERT(event.m_events.size() > 0);
        event.m_events.clear();
    }
    TS_ASSERT(event.m_done);

    // Jump. This will produce a new position.
    proxy.jumpRequest(52);
    while (disp.wait(0))
        ;
    TS_ASSERT(event.m_events.size() > 0);
    TS_ASSERT(!event.m_done);
}

