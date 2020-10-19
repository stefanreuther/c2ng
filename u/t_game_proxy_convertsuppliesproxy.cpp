/**
  *  \file u/t_game_proxy_convertsuppliesproxy.cpp
  *  \brief Test for game::proxy::ConvertSuppliesProxy
  */

#include "game/proxy/convertsuppliesproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/map/planet.hpp"
#include "game/turn.hpp"
#include "game/map/reverter.hpp"

namespace {
    using afl::base::Ptr;
    using game::Element;
    using game::Game;
    using game::HostVersion;
    using game::map::Planet;
    using game::proxy::ConvertSuppliesProxy;
    using game::test::SessionThread;
    using game::test::WaitIndicator;

    class TestReverter : public game::map::Reverter {
     public:
        virtual afl::base::Optional<int> getMinBuildings(int /*planetId*/, game::PlanetaryBuilding /*building*/) const
            { return 0; }
        virtual int getSuppliesAllowedToBuy(int /*planetId*/) const
            { return 3000; }
        virtual afl::base::Optional<int> getMinTechLevel(int /*planetId*/, game::TechLevel /*techLevel*/) const
            { return 0; }
        virtual afl::base::Optional<int> getMinBaseStorage(int /*planetId*/, game::TechLevel /*area*/, int /*slot*/) const
            { return 0; }
        virtual int getNumTorpedoesAllowedToSell(int /*planetId*/, int /*slot*/) const
            { return 0; }
        virtual int getNumFightersAllowedToSell(int /*planetId*/) const
            { return 0; }
        virtual afl::base::Optional<String_t> getPreviousShipFriendlyCode(game::Id_t /*shipId*/) const
            { return afl::base::Nothing; }
        virtual afl::base::Optional<String_t> getPreviousPlanetFriendlyCode(game::Id_t /*planetId*/) const
            { return afl::base::Nothing; }
        virtual bool getPreviousShipMission(int /*shipId*/, int& /*m*/, int& /*i*/, int& /*t*/) const
            { return false; }
        virtual bool getPreviousShipBuildOrder(int /*planetId*/, game::ShipBuildOrder& /*result*/) const
            { return false; }
        virtual game::map::LocationReverter* createLocationReverter(game::map::Point /*pt*/) const
            { return 0; }
    };

    const int PLANET_ID = 333;

    Planet& prepare(SessionThread& h)
    {
        // Create a game with a planet
        Ptr<Game> g = new Game();
        Planet& p = *g->currentTurn().universe().planets().create(PLANET_ID);

        game::map::PlanetData pd;
        pd.owner = 3;
        pd.colonistClans = 100;
        pd.supplies = 1000;
        pd.money = 500;
        p.addCurrentPlanetData(pd, game::PlayerSet_t(3));

        afl::string::NullTranslator tx;
        afl::sys::Log log;
        p.internalCheck(game::map::Configuration(), h.session().translator(), h.session().log());
        p.setPlayability(Planet::Playable);

        h.session().setGame(g);

        return p;
    }
}

/** Test behaviour on empty universe.
    A: create empty universe. Initialize proxy.
    E: status must be reported as invalid */
void
TestGameProxyConvertSuppliesProxy::testEmpty()
{
    SessionThread h;
    ConvertSuppliesProxy testee(h.gameSender());

    WaitIndicator ind;
    ConvertSuppliesProxy::Status st = testee.init(ind, 99, 0, 0);
    TS_ASSERT_EQUALS(st.valid, false);
}

/** Test supply sale.
    A: create universe with a planet. Sell supplies.
    E: correct results reported. */
void
TestGameProxyConvertSuppliesProxy::testSell()
{
    // Environment
    SessionThread h;
    Planet& p = prepare(h);
    ConvertSuppliesProxy testee(h.gameSender());

    // Set up
    WaitIndicator ind;
    ConvertSuppliesProxy::Status st = testee.init(ind, PLANET_ID, 0, 0);
    TS_ASSERT_EQUALS(st.valid, true);
    TS_ASSERT_EQUALS(st.maxSuppliesToSell, 1000);
    TS_ASSERT_EQUALS(st.maxSuppliesToBuy, 0);

    // Sell supplies
    testee.sellSupplies(300);

    // Verify
    h.sync();
    TS_ASSERT_EQUALS(p.getCargo(Element::Supplies).orElse(-1), 700);
    TS_ASSERT_EQUALS(p.getCargo(Element::Money).orElse(-1), 800);
}

/** Test buying supplies.
    A: create universe with a planet and a reverter. Buy supplies.
    E: correct results reported. */
void
TestGameProxyConvertSuppliesProxy::testBuy()
{
    // Environment
    SessionThread h;
    Planet& p = prepare(h);
    h.session().getGame()->currentTurn().universe().setNewReverter(new TestReverter());
    ConvertSuppliesProxy testee(h.gameSender());

    // Set up
    WaitIndicator ind;
    ConvertSuppliesProxy::Status st = testee.init(ind, PLANET_ID, 0, 0);
    TS_ASSERT_EQUALS(st.valid, true);
    TS_ASSERT_EQUALS(st.maxSuppliesToSell, 1000);
    TS_ASSERT_EQUALS(st.maxSuppliesToBuy, 500);

    // Sell supplies
    testee.buySupplies(300);

    // Verify
    h.sync();
    TS_ASSERT_EQUALS(p.getCargo(Element::Supplies).orElse(-1), 1300);
    TS_ASSERT_EQUALS(p.getCargo(Element::Money).orElse(-1), 200);
}

