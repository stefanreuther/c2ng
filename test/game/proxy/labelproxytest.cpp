/**
  *  \file test/game/proxy/labelproxytest.cpp
  *  \brief Test for game::proxy::LabelProxy
  */

#include "game/proxy/labelproxy.hpp"

#include "afl/test/testrunner.hpp"
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
        afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
        afl::base::Ref<game::Game> g = *new game::Game();
        t.session().setRoot(r.asPtr());
        t.session().setGame(g.asPtr());
        t.session().setShipList(new game::spec::ShipList());

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
AFL_TEST("game.proxy.LabelProxy:empty", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::LabelProxy testee(t.gameSender(), ind);

    // Get status: must report empty
    String_t shipExpr = "?", planetExpr = "?";
    testee.getConfiguration(ind, shipExpr, planetExpr);
    a.checkEqual("01. shipExpr", shipExpr, "");
    a.checkEqual("02. planetExpr", planetExpr, "");

    // Set configuration: must report error
    Receiver recv;
    testee.sig_configurationApplied.add(&recv, &Receiver::onConfigurationApplied);
    testee.setConfiguration(String_t("Name"), String_t("Id"));
    t.sync();
    ind.processQueue();

    a.check("11. isOK", recv.isOK());
    a.check("12. shipError", recv.getStatus().shipError.isValid());
    a.check("13. planetError", recv.getStatus().planetError.isValid());
}

/** Test normal behaviour. */
AFL_TEST("game.proxy.LabelProxy:normal", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::LabelProxy testee(t.gameSender(), ind);

    // Get status
    String_t shipExpr = "?", planetExpr = "?";
    testee.getConfiguration(ind, shipExpr, planetExpr);
    a.checkEqual("01. shipExpr", shipExpr, "Id");
    a.checkEqual("02. planetExpr", planetExpr, "Name");

    // Set configuration: must succeed
    Receiver recv;
    testee.sig_configurationApplied.add(&recv, &Receiver::onConfigurationApplied);
    testee.setConfiguration(String_t("Id+1"), String_t("Id+2"));
    t.sync();
    ind.processQueue();

    a.check("11. isOK", recv.isOK());
    a.check("12. shipError", !recv.getStatus().shipError.isValid());
    a.check("13. planetError", !recv.getStatus().planetError.isValid());
    a.checkEqual("14. ship label", game::interface::LabelExtra::get(t.session())->shipLabels().getLabel(42), "43");
    a.checkEqual("15. planet label", game::interface::LabelExtra::get(t.session())->planetLabels().getLabel(1), "3");
    recv.clear();

    // Set (partial) error configuration
    testee.setConfiguration(String_t("*"), String_t("Id+3"));
    t.sync();
    ind.processQueue();

    a.check("21. isOK", recv.isOK());
    a.check("22. shipError", recv.getStatus().shipError.isValid());
    a.check("23. planetError", !recv.getStatus().planetError.isValid());
    a.checkEqual("24. ship label", game::interface::LabelExtra::get(t.session())->shipLabels().getLabel(42), "");
    a.checkEqual("25. planet label", game::interface::LabelExtra::get(t.session())->planetLabels().getLabel(1), "4");
    recv.clear();

    // Set success again
    testee.setConfiguration(String_t("Name"), String_t("Id+4"));
    t.sync();
    ind.processQueue();

    a.check("31. isOK", recv.isOK());
    a.check("32. shipError", !recv.getStatus().shipError.isValid());
    a.check("33. planetError", !recv.getStatus().planetError.isValid());
    a.checkEqual("34. ship label", game::interface::LabelExtra::get(t.session())->shipLabels().getLabel(42), "Unsinkable II");
    a.checkEqual("35. planet label", game::interface::LabelExtra::get(t.session())->planetLabels().getLabel(1), "5");
    recv.clear();
}
