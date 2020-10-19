/**
  *  \file u/t_game_proxy_taxationproxy.cpp
  *  \brief Test for game::proxy::TaxationProxy
  */

#include "game/proxy/taxationproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/game.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"
#include "util/simplerequestdispatcher.hpp"

using afl::base::Ptr;
using game::Game;
using game::HostVersion;
using game::map::Planet;
using game::map::Universe;
using game::proxy::TaxationProxy;
using game::actions::TaxationAction;

namespace {
    Planet& addPlanet(Universe& univ, int id, int owner)
    {
        Planet& p = *univ.planets().create(id);
        p.setPosition(game::map::Point(1,2));

        game::map::PlanetData pd;
        pd.owner = owner;
        pd.minedNeutronium = 1000;
        pd.minedTritanium = 1000;
        pd.minedDuranium = 1000;
        pd.minedMolybdenum = 1000;
        pd.money = 1000;
        pd.supplies = 1000;

        // Same setup as in TaxationAction test
        pd.colonistClans = 1000;
        pd.nativeRace = game::ReptilianNatives;
        pd.nativeGovernment = 5;
        pd.nativeClans = 20000;
        pd.colonistHappiness = 100;
        pd.nativeHappiness = 100;
        pd.temperature = 50;
        pd.colonistTax = 1;
        pd.nativeTax = 2;

        p.addCurrentPlanetData(pd, game::PlayerSet_t(owner));

        afl::string::NullTranslator tx;
        afl::sys::Log log;
        p.internalCheck(game::map::Configuration(), tx, log);
        p.setPlayability(game::map::Object::Playable);

        return p;
    }

    const int PLANET_ID = 42;
    const int OWNER = 3;

    Planet& setup(game::test::SessionThread& h)
    {
        // Root
        h.session().setRoot(new game::test::Root(HostVersion(HostVersion::PHost, MKVERSION(3,4,0))));

        // Game
        Ptr<Game> g = new Game();
        Universe& u = g->currentTurn().universe();
        Planet& p = addPlanet(u, PLANET_ID, OWNER);
        h.session().setGame(g);
        return p;
    }

    class StatusReceiver {
     public:
        StatusReceiver()
            : m_status(),
              m_ok(false)
            { }
        void onChange(const TaxationProxy::Status& st)
            {
                m_status = st;
                m_ok = true;
            }
        void wait(util::SimpleRequestDispatcher& disp)
            {
                CxxTest::setAbortTestOnFail(true);
                m_ok = false;
                while (!m_ok) {
                    TS_ASSERT(disp.wait(100));
                }
            }
        const TaxationProxy::Status& status() const
            { return m_status; }

     private:
        TaxationProxy::Status m_status;
        bool m_ok;
    };
}

/** Test empty universe.
    A: create a TaxationProxy on an empty universe.
    E: proxy must report all values unavailable */
void
TestGameProxyTaxationProxy::testEmpty()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    TaxationProxy testee(ind, h.gameSender(), PLANET_ID);

    TaxationProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.valid, false);
    TS_ASSERT_EQUALS(st.colonists.available, false);
    TS_ASSERT_EQUALS(st.natives.available, false);
}

/** Test normal situation.
    A: create a TaxationProxy on an normal situation.
    E: proxy must report expected values, change must have expected effect. */
void
TestGameProxyTaxationProxy::testNormal()
{
    // Derived from TestGameActionsTaxationAction::testNormal()
    game::test::SessionThread h;
    Planet& p = setup(h);

    // Testee
    game::test::WaitIndicator ind;
    TaxationProxy testee(ind, h.gameSender(), PLANET_ID);

    // Get status
    TaxationProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.valid, true);
    TS_ASSERT_EQUALS(st.colonists.available, true);
    TS_ASSERT_EQUALS(st.colonists.tax, 1);
    TS_ASSERT_EQUALS(st.colonists.change, 8);
    TS_ASSERT(st.colonists.changeLabel.find("LOVE") != String_t::npos);
    TS_ASSERT(st.colonists.description.find("pay 1 mc") != String_t::npos);
    TS_ASSERT(st.colonists.title.find("olon") != String_t::npos);  // to anticipate Colony, Colonists, etc.
    TS_ASSERT_EQUALS(st.natives.available, true);
    TS_ASSERT_EQUALS(st.natives.tax, 2);
    TS_ASSERT_EQUALS(st.natives.change, 4);
    TS_ASSERT(st.natives.changeLabel.find("like") != String_t::npos);
    TS_ASSERT(st.natives.description.find("pay 40 mc") != String_t::npos);
    TS_ASSERT(st.natives.title.find("Reptilian") != String_t::npos);

    // Change
    testee.setTaxLimited(TaxationAction::Colonists, 2);
    testee.getStatus(ind, st);

    TS_ASSERT_EQUALS(st.colonists.tax, 2);
    TS_ASSERT_EQUALS(st.colonists.change, 8);
    TS_ASSERT(st.colonists.description.find("pay 2 mc") != String_t::npos);

    // Commit
    TS_ASSERT_THROWS_NOTHING(testee.commit());

    // Verify
    h.sync();

    TS_ASSERT_EQUALS(p.getColonistTax().orElse(-1), 2);
}

/** Test changeRevenue().
    A: prepare normal planet. Call changeRevenue().
    E: tax rate and revenue must change */
void
TestGameProxyTaxationProxy::testChangeRevenue()
{
    // Derived from TestGameActionsTaxationAction::testChangeRevenue()
    game::test::SessionThread h;
    Planet& p = setup(h);
    p.setCargo(game::Element::Colonists, 100);

    // Testee
    game::test::WaitIndicator ind;
    TaxationProxy testee(ind, h.gameSender(), PLANET_ID);

    // Get status
    TaxationProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.colonists.tax, 1);

    // Change up
    testee.changeRevenue(TaxationAction::Colonists, TaxationAction::Up);
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.colonists.tax, 5);

    // Change down
    testee.changeRevenue(TaxationAction::Colonists, TaxationAction::Down);
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.colonists.tax, 4);
}

/** Test changeTax(), revert().
    A: prepare planet. Call changeTax(), revert().
    E: tax rate must change accordingly */
void
TestGameProxyTaxationProxy::testModifyRevert()
{
    // Derived from TestGameActionsTaxationAction::testModifyRevert()
    game::test::SessionThread h;
    setup(h);

    // Testee
    game::test::WaitIndicator ind;
    TaxationProxy testee(ind, h.gameSender(), PLANET_ID);

    // Check initial state
    TaxationProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.colonists.tax, 1);
    TS_ASSERT_EQUALS(st.natives.tax, 2);

    // Modify
    testee.changeTax(TaxationAction::Colonists, 10);
    testee.changeTax(TaxationAction::Natives, -1);
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.colonists.tax, 11);
    TS_ASSERT_EQUALS(st.natives.tax, 1);

    // Revert
    testee.revert(TaxationAction::Areas_t(TaxationAction::Natives));
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.colonists.tax, 11);
    TS_ASSERT_EQUALS(st.natives.tax, 2);

    // Revert more
    testee.revert(TaxationAction::Areas_t(TaxationAction::Colonists));
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.colonists.tax, 1);
    TS_ASSERT_EQUALS(st.natives.tax, 2);
}

/** Test setSafeTax().
    A: prepare planet. Call setSafeTax().
    E: tax rate must be set for a change of 0 */
void
TestGameProxyTaxationProxy::testSafeTax()
{
    // Derived from TestGameActionsTaxationAction::testNormal()
    game::test::SessionThread h;
    setup(h);

    // Testee
    game::test::WaitIndicator ind;
    TaxationProxy testee(ind, h.gameSender(), PLANET_ID);

    // Check initial state
    TaxationProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.colonists.tax, 1);
    TS_ASSERT_EQUALS(st.natives.tax, 2);

    // Colonists
    testee.setSafeTax(TaxationAction::Areas_t(TaxationAction::Colonists));
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.colonists.tax, 13);
    TS_ASSERT_EQUALS(st.colonists.change, 0);

    // Natives
    testee.setSafeTax(TaxationAction::Areas_t(TaxationAction::Natives));
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.natives.tax, 8);
    TS_ASSERT_EQUALS(st.natives.change, 0);
}

/** Test setNumBuildings().
    A: prepare planet. Call setNumBuildings().
    E: happiness must change according to number of buildings */
void
TestGameProxyTaxationProxy::testSetNumBuildings()
{
    // Derived from TestGameActionsTaxationAction::testNormal()
    game::test::SessionThread h;
    setup(h);

    // Testee
    game::test::WaitIndicator ind;
    TaxationProxy testee(ind, h.gameSender(), PLANET_ID);

    // Check initial state
    TaxationProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.colonists.change, 8);
    TS_ASSERT_EQUALS(st.natives.change, 4);

    // Change number of buildings
    testee.setNumBuildings(300);
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.colonists.change, 7);
    TS_ASSERT_EQUALS(st.natives.change, 2);
}

/** Test signalisation. */
void
TestGameProxyTaxationProxy::testSignal()
{
    // Derived from TestGameActionsTaxationAction::testNormal()
    game::test::SessionThread h;
    setup(h);

    // Testee
    StatusReceiver rx;
    util::SimpleRequestDispatcher disp;
    TaxationProxy testee(disp, h.gameSender(), PLANET_ID);
    testee.sig_change.add(&rx, &StatusReceiver::onChange);

    // Change
    testee.setTaxLimited(TaxationAction::Colonists, 2);
    rx.wait(disp);

    TS_ASSERT_EQUALS(rx.status().colonists.tax, 2);
    TS_ASSERT_EQUALS(rx.status().colonists.change, 8);
}

