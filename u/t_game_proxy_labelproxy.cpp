/**
  *  \file u/t_game_proxy_labelproxy.cpp
  *  \brief Test for game::proxy::LabelProxy
  */

#include "game/proxy/labelproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/interface/labelextra.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

namespace {
    class Receiver {
     public:
        Receiver()
            : m_ok(false),
              m_status()
            { }
        void onConfigurationApplied(const game::proxy::LabelProxy::Status& st)
            {
                m_ok = true;
                m_status = st;
            }
        void clear()
            { m_ok = false; }
        bool isOK() const
            { return m_ok; }
        const game::proxy::LabelProxy::Status& getStatus() const
            { return m_status; }
     private:
        bool m_ok;
        game::proxy::LabelProxy::Status m_status;
    };

    /* Add planet. It doesn't need any specific status, it just needs to be visible on the map. */
    void addPlanet(game::test::SessionThread& t, game::Id_t id, int x, int y, String_t name)
    {
        game::map::Planet* pl = t.session().getGame()->currentTurn().universe().planets().create(id);
        pl->setName(name);
        pl->setPosition(game::map::Point(x, y));
    }

    /* Add ship. It doesn't need any specific status, it just needs to be visible on the map, so we make a shipxy target. */
    void addShip(game::test::SessionThread& t, game::Id_t id, int x, int y, String_t name)
    {
        game::map::Ship* sh = t.session().getGame()->currentTurn().universe().ships().create(id);
        sh->setName(name);
        sh->addShipXYData(game::map::Point(x, y), 1, 100, game::PlayerSet_t(2));
    }

    void prepare(game::test::SessionThread& t)
    {
        // Add connections
        afl::base::Ref<game::Root> r = *new game::test::Root(game::HostVersion());
        afl::base::Ref<game::Game> g = *new game::Game();
        t.session().setRoot(r.asPtr());
        t.session().setGame(g.asPtr());
        t.session().setShipList(new game::spec::ShipList());
        t.session().sig_runRequest.add(&t.session().processList(), &interpreter::ProcessList::run);

        // Add objects
        addPlanet(t, 1, 1000, 1001, "Romulus");
        addShip(t, 42, 2000, 2001, "Unsinkable II");

        // Configure
        r->userConfiguration().setOption("Label.Planet", "Name", game::config::ConfigurationOption::User);
        r->userConfiguration().setOption("Label.Ship",   "Id",   game::config::ConfigurationOption::User);

        // Finish
        t.session().postprocessTurn(g->currentTurn(), game::PlayerSet_t(2), game::PlayerSet_t(2), game::map::Object::Playable);
        g->setViewpointPlayer(2);

        // Attach LabelExtra
        game::interface::LabelExtra::create(t.session());
    }
}

/** Test behaviour on empty session.
    Calls must complete without error. */
void
TestGameProxyLabelProxy::testEmpty()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::LabelProxy testee(t.gameSender(), ind);

    // Get status: must report empty
    String_t shipExpr = "?", planetExpr = "?";
    testee.getConfiguration(ind, shipExpr, planetExpr);
    TS_ASSERT_EQUALS(shipExpr, "");
    TS_ASSERT_EQUALS(planetExpr, "");

    // Set configuration: must report error
    Receiver recv;
    testee.sig_configurationApplied.add(&recv, &Receiver::onConfigurationApplied);
    testee.setConfiguration(String_t("Name"), String_t("Id"));
    t.sync();
    ind.processQueue();

    TS_ASSERT(recv.isOK());
    TS_ASSERT(recv.getStatus().shipError.isValid());
    TS_ASSERT(recv.getStatus().planetError.isValid());
}

/** Test normal behaviour. */
void
TestGameProxyLabelProxy::testNormal()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::LabelProxy testee(t.gameSender(), ind);

    // Get status
    String_t shipExpr = "?", planetExpr = "?";
    testee.getConfiguration(ind, shipExpr, planetExpr);
    TS_ASSERT_EQUALS(shipExpr, "Id");
    TS_ASSERT_EQUALS(planetExpr, "Name");

    // Set configuration: must succeed
    Receiver recv;
    testee.sig_configurationApplied.add(&recv, &Receiver::onConfigurationApplied);
    testee.setConfiguration(String_t("Id+1"), String_t("Id+2"));
    t.sync();
    ind.processQueue();

    TS_ASSERT(recv.isOK());
    TS_ASSERT(!recv.getStatus().shipError.isValid());
    TS_ASSERT(!recv.getStatus().planetError.isValid());
    TS_ASSERT_EQUALS(game::interface::LabelExtra::get(t.session())->shipLabels().getLabel(42), "43");
    TS_ASSERT_EQUALS(game::interface::LabelExtra::get(t.session())->planetLabels().getLabel(1), "3");
    recv.clear();

    // Set (partial) error configuration
    testee.setConfiguration(String_t("*"), String_t("Id+3"));
    t.sync();
    ind.processQueue();

    TS_ASSERT(recv.isOK());
    TS_ASSERT(recv.getStatus().shipError.isValid());
    TS_ASSERT(!recv.getStatus().planetError.isValid());
    TS_ASSERT_EQUALS(game::interface::LabelExtra::get(t.session())->shipLabels().getLabel(42), "");
    TS_ASSERT_EQUALS(game::interface::LabelExtra::get(t.session())->planetLabels().getLabel(1), "4");
    recv.clear();

    // Set success again
    testee.setConfiguration(String_t("Name"), String_t("Id+4"));
    t.sync();
    ind.processQueue();

    TS_ASSERT(recv.isOK());
    TS_ASSERT(!recv.getStatus().shipError.isValid());
    TS_ASSERT(!recv.getStatus().planetError.isValid());
    TS_ASSERT_EQUALS(game::interface::LabelExtra::get(t.session())->shipLabels().getLabel(42), "Unsinkable II");
    TS_ASSERT_EQUALS(game::interface::LabelExtra::get(t.session())->planetLabels().getLabel(1), "5");
    recv.clear();
}

