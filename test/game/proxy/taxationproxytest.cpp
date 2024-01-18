/**
  *  \file test/game/proxy/taxationproxytest.cpp
  *  \brief Test for game::proxy::TaxationProxy
  */

#include "game/proxy/taxationproxy.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
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
        p.internalCheck(game::map::Configuration(), game::PlayerSet_t(owner), 15, tx, log);
        p.setPlayability(game::map::Object::Playable);

        return p;
    }

    const int PLANET_ID = 42;
    const int OWNER = 3;

    Planet& setup(game::test::SessionThread& h)
    {
        // Root
        h.session().setRoot(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,4,0))).asPtr());

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
        void wait(afl::test::Assert a, util::SimpleRequestDispatcher& disp)
            {
                m_ok = false;
                while (!m_ok) {
                    a.check("01. wait", disp.wait(100));
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
AFL_TEST("game.proxy.TaxationProxy:empty", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    TaxationProxy testee(ind, h.gameSender(), PLANET_ID);

    TaxationProxy::Status st;
    testee.getStatus(ind, st);
    a.checkEqual("01. valid", st.valid, false);
    a.checkEqual("02. colonists", st.colonists.available, false);
    a.checkEqual("03. natives", st.natives.available, false);
}

/** Test normal situation.
    A: create a TaxationProxy on an normal situation.
    E: proxy must report expected values, change must have expected effect. */
AFL_TEST("game.proxy.TaxationProxy:normal", a)
{
    // Derived from TestGameActionsTaxationAction::testNormal()
    game::test::SessionThread h;
    Planet& p = setup(h);
    p.setColonistHappiness(91);

    // Testee
    game::test::WaitIndicator ind;
    TaxationProxy testee(ind, h.gameSender(), PLANET_ID);

    // Effectors
    game::map::PlanetEffectors eff;
    eff.set(game::map::PlanetEffectors::Hiss, 1);
    testee.setEffectors(eff);

    // Get status
    TaxationProxy::Status st;
    testee.getStatus(ind, st);
    a.checkEqual("01. valid", st.valid, true);
    a.checkEqual("02. colonists", st.colonists.available, true);
    a.checkEqual("03. ctax", st.colonists.tax, 1);
    a.checkEqual("04. cchange", st.colonists.change, 8);
    a.check("05. clabel", st.colonists.changeLabel.find("LOVE") != String_t::npos);
    a.check("06. cdesc", st.colonists.description.find("happy (104)") != String_t::npos);
    a.check("07. cdesc", st.colonists.description.find("pay 1 mc") != String_t::npos);
    a.check("08. ctitle", st.colonists.title.find("olon") != String_t::npos);  // to anticipate Colony, Colonists, etc.
    a.checkEqual("09. natives", st.natives.available, true);
    a.checkEqual("10. ntax", st.natives.tax, 2);
    a.checkEqual("11. nchange", st.natives.change, 4);
    a.check("12. nlabel", st.natives.changeLabel.find("like") != String_t::npos);
    a.check("13. ndesc", st.natives.description.find("pay 40 mc") != String_t::npos);
    a.check("14. ntitle", st.natives.title.find("Reptilian") != String_t::npos);

    // Change
    testee.setTaxLimited(TaxationAction::Colonists, 2);
    testee.getStatus(ind, st);

    a.checkEqual("21. ctax", st.colonists.tax, 2);
    a.checkEqual("22. cchange", st.colonists.change, 8);
    a.check("23. cdesc", st.colonists.description.find("pay 2 mc") != String_t::npos);

    // Commit
    AFL_CHECK_SUCCEEDS(a("31. commit"), testee.commit());

    // Verify
    h.sync();

    a.checkEqual("41. getColonistTax", p.getColonistTax().orElse(-1), 2);
}

/** Test changeRevenue().
    A: prepare normal planet. Call changeRevenue().
    E: tax rate and revenue must change */
AFL_TEST("game.proxy.TaxationProxy:changeRevenue", a)
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
    a.checkEqual("01. ctax", st.colonists.tax, 1);

    // Change up
    testee.changeRevenue(TaxationAction::Colonists, TaxationAction::Up);
    testee.getStatus(ind, st);
    a.checkEqual("11. ctax", st.colonists.tax, 5);

    // Change down
    testee.changeRevenue(TaxationAction::Colonists, TaxationAction::Down);
    testee.getStatus(ind, st);
    a.checkEqual("21. ctax", st.colonists.tax, 4);
}

/** Test changeTax(), revert().
    A: prepare planet. Call changeTax(), revert().
    E: tax rate must change accordingly */
AFL_TEST("game.proxy.TaxationProxy:modify+revert", a)
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
    a.checkEqual("01. ctax", st.colonists.tax, 1);
    a.checkEqual("02. ntax", st.natives.tax, 2);

    // Modify
    testee.changeTax(TaxationAction::Colonists, 10);
    testee.changeTax(TaxationAction::Natives, -1);
    testee.getStatus(ind, st);
    a.checkEqual("11. ctax", st.colonists.tax, 11);
    a.checkEqual("12. ntax", st.natives.tax, 1);

    // Revert
    testee.revert(TaxationAction::Areas_t(TaxationAction::Natives));
    testee.getStatus(ind, st);
    a.checkEqual("21. ctax", st.colonists.tax, 11);
    a.checkEqual("22. ntax", st.natives.tax, 2);

    // Revert more
    testee.revert(TaxationAction::Areas_t(TaxationAction::Colonists));
    testee.getStatus(ind, st);
    a.checkEqual("31. ctax", st.colonists.tax, 1);
    a.checkEqual("32. ntax", st.natives.tax, 2);
}

/** Test setSafeTax().
    A: prepare planet. Call setSafeTax().
    E: tax rate must be set for a change of 0 */
AFL_TEST("game.proxy.TaxationProxy:setSafeTax", a)
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
    a.checkEqual("01. ctax", st.colonists.tax, 1);
    a.checkEqual("02. ntax", st.natives.tax, 2);

    // Colonists
    testee.setSafeTax(TaxationAction::Areas_t(TaxationAction::Colonists));
    testee.getStatus(ind, st);
    a.checkEqual("11. ctax", st.colonists.tax, 13);
    a.checkEqual("12. cchange", st.colonists.change, 0);

    // Natives
    testee.setSafeTax(TaxationAction::Areas_t(TaxationAction::Natives));
    testee.getStatus(ind, st);
    a.checkEqual("21. ntax", st.natives.tax, 8);
    a.checkEqual("22. nchange", st.natives.change, 0);
}

/** Test setNumBuildings().
    A: prepare planet. Call setNumBuildings().
    E: happiness must change according to number of buildings */
AFL_TEST("game.proxy.TaxationProxy:setNumBuildings", a)
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
    a.checkEqual("01. cchange", st.colonists.change, 8);
    a.checkEqual("02. nchange", st.natives.change, 4);

    // Change number of buildings
    testee.setNumBuildings(300);
    testee.getStatus(ind, st);
    a.checkEqual("11. cchange", st.colonists.change, 7);
    a.checkEqual("12. nchange", st.natives.change, 2);
}

/** Test signalisation. */
AFL_TEST("game.proxy.TaxationProxy:signal", a)
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
    rx.wait(a, disp);

    a.checkEqual("01. ctax", rx.status().colonists.tax, 2);
    a.checkEqual("02. cchange", rx.status().colonists.change, 8);
}
