/**
  *  \file u/t_game_proxy_flakvcrplayerproxy.cpp
  *  \brief Test for game::proxy::FlakVcrPlayerProxy
  */

#include "game/proxy/flakvcrplayerproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/vcr/flak/database.hpp"
#include "util/simplerequestdispatcher.hpp"

namespace {
    /* A single 1:1 fight */
    const uint8_t FILE_CONTENT[] = {
        0x46, 0x4c, 0x41, 0x4b, 0x56, 0x43, 0x52, 0x1a, 0x00, 0x00, 0x05, 0x00, 0x0b, 0x00, 0x01, 0x00,
        0x30, 0x34, 0x2d, 0x32, 0x35, 0x2d, 0x32, 0x30, 0x32, 0x31, 0x31, 0x31, 0x3a, 0x31, 0x31, 0x3a,
        0x34, 0x33, 0x00, 0x00, 0x00, 0x00, 0xec, 0x00, 0x00, 0x00, 0xe8, 0x03, 0xe8, 0x03, 0x95, 0xec,
        0x60, 0x92, 0xf1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x18, 0x00,
        0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x68, 0x00,
        0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xe4, 0x00, 0x00, 0x00, 0x05, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x4b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xa0, 0x92,
        0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x01, 0x00, 0x4b, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x60, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x00, 0x00, 0x66, 0x00, 0x64, 0x00, 0x05, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x04, 0x00,
        0x09, 0x00, 0x04, 0x00, 0x32, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5a, 0x00, 0x64, 0x00,
        0x01, 0x00, 0xa2, 0x00, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x30, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x00, 0x00, 0x06, 0x00, 0xc8, 0x00, 0x06, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x01, 0x00, 0x2c, 0x00, 0x00, 0x00,
        0x22, 0x00
    };


    struct Environment {
        game::test::Root root;
        game::spec::ShipList shipList;
        game::TeamSettings* pTeamSettings;
        game::vcr::flak::Database battles;
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

    struct EventReceiver {
        EventReceiver()
            : m_result(), m_done(false)
            { }

        void onEvent(game::proxy::FlakVcrPlayerProxy::Result_t& result, bool done)
            { m_result.swap(result); m_done = done; }

        game::proxy::FlakVcrPlayerProxy::Result_t m_result;
        bool m_done;
    };
}

void
TestGameProxyFlakVcrPlayerProxy::testIt()
{
    // Make simple environment
    Environment env;
    game::test::initStandardBeams(env.shipList);
    game::test::initStandardTorpedoes(env.shipList);
    afl::charset::Utf8Charset cs;
    afl::io::ConstMemoryStream file(FILE_CONTENT);
    env.battles.load(file, cs, env.translator);

    // Set up tasking
    // Use just one RequestDispatcher to serve both sides
    util::SimpleRequestDispatcher disp;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::VcrDatabaseAdaptor> recv(disp, ad);

    // Make proxy
    game::proxy::FlakVcrPlayerProxy proxy(recv.getSender(), disp);
    EventReceiver event;
    proxy.sig_event.add(&event, &EventReceiver::onEvent);

    // Load first fight
    proxy.initRequest(0);
    while (disp.wait(0))
        ;
    TS_ASSERT(event.m_result.size() > 0);
    TS_ASSERT(!event.m_done);
    event.m_result.clear();

    // Load more events until we're done.
    // Fetching events after done is harmless.
    for (int i = 0; i < 300; ++i) {
        proxy.eventRequest();
        while (disp.wait(0))
            ;
        TS_ASSERT(event.m_result.size() > 0);
        event.m_result.clear();
    }
    TS_ASSERT(event.m_done);

    // Jump. This will produce a new position.
    proxy.jumpRequest(52);
    while (disp.wait(0))
        ;
    TS_ASSERT(event.m_result.size() > 0);
    TS_ASSERT(!event.m_done);
}

