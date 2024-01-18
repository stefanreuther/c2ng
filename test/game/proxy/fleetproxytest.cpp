/**
  *  \file test/game/proxy/fleetproxytest.cpp
  *  \brief Test for game::proxy::FleetProxy
  */

#include "game/proxy/fleetproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/fleetmember.hpp"
#include "game/test/counter.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

namespace {
    /* Prepare a SessionThread */
    void prepare(game::test::SessionThread& h)
    {
        h.session().setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,2,0))).asPtr());
        h.session().setShipList(new game::spec::ShipList());
        h.session().setGame(new game::Game());
    }

    /* Add a ship */
    game::map::Ship& addShip(game::map::Universe& univ, game::Id_t id, String_t name, String_t friendlyCode, int x, int y, int fleetNumber)
    {
        game::map::Ship& sh = *univ.ships().create(id);
        game::map::ShipData sd;
        sd.owner = 1;
        sd.x = x;
        sd.y = y;
        sh.addCurrentShipData(sd, game::PlayerSet_t(1));
        sh.setName(name);
        sh.setFleetNumber(fleetNumber);
        sh.setPlayability(game::map::Object::Playable);
        sh.setFriendlyCode(friendlyCode);
        sh.internalCheck(game::PlayerSet_t(1), 15);
        return sh;
    }

    /* Postprocess a SessionThread after ships have been added to it */
    void postprocess(game::test::SessionThread& h, game::Turn& t)
    {
        h.session().postprocessTurn(t, game::PlayerSet_t::allUpTo(20), game::PlayerSet_t::allUpTo(20), game::map::Object::Playable);
    }


    /*
     *  Task to change a fleet number
     */
    class SetFleetNumberTask : public util::Request<game::Session> {
     public:
        SetFleetNumberTask(game::Id_t shipId, game::Id_t fleetNumber)
            : m_shipId(shipId), m_fleetNumber(fleetNumber)
            { }
        virtual void handle(game::Session& s)
            {
                game::Game& g = *s.getGame();
                game::map::Universe& univ = g.currentTurn().universe();
                game::map::FleetMember(univ, *univ.ships().get(m_shipId), g.mapConfiguration()).setFleetNumber(m_fleetNumber, s.getRoot()->hostConfiguration(), *s.getShipList());
                s.notifyListeners();
            }
     private:
        game::Id_t m_shipId;
        game::Id_t m_fleetNumber;
    };


    /*
     *  Callback for FleetProxy::sig_change: record all received Ids
     */
    class IdChecker {
     public:
        IdChecker(game::proxy::FleetProxy& proxy)
            : m_proxy(proxy)
            { }
        void onFleetChange()
            { m_set.insert(m_proxy.getSelectedFleetMember()); }
        void clear()
            { m_set.clear(); }
        bool contains(int n)
            { return m_set.find(n) != m_set.end(); }
     private:
        game::proxy::FleetProxy& m_proxy;
        std::set<int> m_set;
    };
}

/** Test behaviour on empty session. */
AFL_TEST("game.proxy.FleetProxy:empty", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    game::proxy::FleetProxy testee(h.gameSender(), ind);

    h.sync();
    ind.processQueue();

    a.checkEqual("01. getSelectedFleetMember", testee.getSelectedFleetMember(), 0);
    a.checkEqual("02. getFleetMemberList", testee.getFleetMemberList().size(), 0U);
}

/** Test behaviour on empty game (no ship). */
AFL_TEST("game.proxy.FleetProxy:no-ship", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    prepare(h);
    postprocess(h, h.session().getGame()->currentTurn());
    game::proxy::FleetProxy testee(h.gameSender(), ind);

    h.sync();
    ind.processQueue();

    a.checkEqual("01. getSelectedFleetMember", testee.getSelectedFleetMember(), 0);
    a.checkEqual("02. getFleetMemberList", testee.getFleetMemberList().size(), 0U);
    a.checkEqual("03. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 0);
    a.checkEqual("04. currentShip", h.session().getGame()->cursors().currentShip().getCurrentIndex(), 0);
}

/** Test behaviour on normal game.
    Set up a situation with multiple fleets.
    Verify information is reported correctly for use-cases:
    - initialisation
    - selectFleetMember()
    - game-side browsing
    - game-side fleet modification */
AFL_TEST("game.proxy.FleetProxy:normal", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    prepare(h);
    game::map::Universe& univ = h.session().getGame()->currentTurn().universe();
    addShip(univ, 1,  "s1", "one",  1000, 1200, 0);
    addShip(univ, 3,  "s3", "thr",  1000, 1200, 3);
    addShip(univ, 5,  "s5", "fiv",  1000, 1200, 9);
    addShip(univ, 7,  "s7", "sev",  2000, 1200, 3);
    addShip(univ, 9,  "s9", "nin",  1000, 1200, 9);
    addShip(univ, 11, "s11", "ele", 1000, 1200, 9);
    postprocess(h, h.session().getGame()->currentTurn());

    // Initial state: "current" must be lowest Ids
    a.checkEqual("01. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 3);
    a.checkEqual("02. currentShip",  h.session().getGame()->cursors().currentShip().getCurrentIndex(), 1);

    // Set up FleetProxy. This will select ship #3
    game::proxy::FleetProxy testee(h.gameSender(), ind);
    game::test::Counter ctr;
    testee.sig_change.add(&ctr, &game::test::Counter::increment);
    h.sync();
    ind.processQueue();
    int n1 = ctr.get();
    a.check("10. got notification", n1 > 0);
    a.checkEqual("11. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 3);
    a.checkEqual("12. currentShip",  h.session().getGame()->cursors().currentShip().getCurrentIndex(), 3);
    a.checkEqual("13. getSelectedFleetMember", testee.getSelectedFleetMember(), 3);
    a.checkEqual("14. getFleetMemberList", testee.getFleetMemberList().size(), 2U);
    a.checkEqual("15. getFleetMemberList", testee.getFleetMemberList().get(0)->reference.getId(), 3);
    a.checkEqual("16. getFleetMemberList", testee.getFleetMemberList().get(1)->reference.getId(), 7);

    // Select another fleet member
    testee.selectFleetMember(7);
    h.sync();
    ind.processQueue();
    int n2 = ctr.get();
    a.check("20. got notification", n2 > n1);
    a.checkEqual("21", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 3);
    a.checkEqual("22", h.session().getGame()->cursors().currentShip().getCurrentIndex(), 7);
    a.checkEqual("23. getSelectedFleetMember", testee.getSelectedFleetMember(), 7);

    // Select different fleet [irregular case]
    testee.selectFleetMember(11);
    h.sync();
    ind.processQueue();
    int n3 = ctr.get();
    a.check("30. got notification", n3 > n2);
    a.checkEqual("31. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 9);
    a.checkEqual("32. currentShip",  h.session().getGame()->cursors().currentShip().getCurrentIndex(), 11);
    a.checkEqual("33. getSelectedFleetMember", testee.getSelectedFleetMember(), 11);
    a.checkEqual("34. getFleetMemberList", testee.getFleetMemberList().size(), 3U);
    a.checkEqual("35. getFleetMemberList", testee.getFleetMemberList().get(0)->reference.getId(), 9);
    a.checkEqual("36. getFleetMemberList", testee.getFleetMemberList().get(1)->reference.getId(), 5);
    a.checkEqual("37. getFleetMemberList", testee.getFleetMemberList().get(2)->reference.getId(), 11);

    // Game-side browsing
    class Task : public util::Request<game::Session> {
     public:
        virtual void handle(game::Session& s)
            {
                s.getGame()->cursors().currentFleet().browse(game::map::ObjectCursor::Next, false);
                s.notifyListeners();
            }
    };
    h.gameSender().postNewRequest(new Task());
    h.sync();
    ind.processQueue();
    int n4 = ctr.get();
    a.check("40. got notification", n4 > n3);
    a.checkEqual("41. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 3);
    a.checkEqual("42. currentShip",  h.session().getGame()->cursors().currentShip().getCurrentIndex(), 3);
    a.checkEqual("43. getSelectedFleetMember", testee.getSelectedFleetMember(), 3);
    a.checkEqual("44. getFleetMemberList", testee.getFleetMemberList().size(), 2U);

    // Game-side fleet modification
    h.gameSender().postNewRequest(new SetFleetNumberTask(11, 3));
    h.sync();
    ind.processQueue();
    int n5 = ctr.get();
    a.check("50. got notification", n5 > n4);
    a.checkEqual("51. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 3);
    a.checkEqual("52. currentShip",  h.session().getGame()->cursors().currentShip().getCurrentIndex(), 3);
    a.checkEqual("53. getSelectedFleetMember", testee.getSelectedFleetMember(), 3);
    a.checkEqual("54. getFleetMemberList", testee.getFleetMemberList().size(), 3U);
    a.checkEqual("55. getFleetMemberList", testee.getFleetMemberList().get(0)->reference.getId(), 3);
    a.checkEqual("56. getFleetMemberList", testee.getFleetMemberList().get(1)->reference.getId(), 7);
    a.checkEqual("57. getFleetMemberList", testee.getFleetMemberList().get(2)->reference.getId(), 11);
}

/** Delete ship in the middle.
    Cursor must remain at sensible place (not at leader). */
AFL_TEST("game.proxy.FleetProxy:delete-mid", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    prepare(h);
    game::map::Universe& univ = h.session().getGame()->currentTurn().universe();
    addShip(univ, 1,  "s1", "one",  1000, 1200, 0);
    addShip(univ, 3,  "s3", "thr",  1000, 1200, 5);
    addShip(univ, 5,  "s5", "fiv",  1000, 1200, 5);
    addShip(univ, 7,  "s7", "sev",  2000, 1200, 5);
    addShip(univ, 9,  "s9", "nin",  1000, 1200, 5);
    postprocess(h, h.session().getGame()->currentTurn());

    // Set up FleetProxy and select #7.
    game::proxy::FleetProxy testee(h.gameSender(), ind);
    testee.selectFleetMember(7);
    h.sync();
    ind.processQueue();
    a.checkEqual("01. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 5);
    a.checkEqual("02. currentShip", h.session().getGame()->cursors().currentShip().getCurrentIndex(), 7);
    a.checkEqual("03. getSelectedFleetMember", testee.getSelectedFleetMember(), 7);
    a.checkEqual("04. getFleetMemberList", testee.getFleetMemberList().size(), 4U);

    // Delete #7. Current should now be #9.
    h.gameSender().postNewRequest(new SetFleetNumberTask(7, 0));
    h.sync();
    ind.processQueue();

    a.checkEqual("11. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 5);
    a.checkEqual("12. currentShip", h.session().getGame()->cursors().currentShip().getCurrentIndex(), 9);
    a.checkEqual("13. getSelectedFleetMember", testee.getSelectedFleetMember(), 9);
    a.checkEqual("14. getFleetMemberList", testee.getFleetMemberList().size(), 3U);
}

/** Delete ship at end.
    Cursor must remain at sensible place (not at leader). */
AFL_TEST("game.proxy.FleetProxy:delete-end", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    prepare(h);
    game::map::Universe& univ = h.session().getGame()->currentTurn().universe();
    addShip(univ, 1,  "s1", "one",  1000, 1200, 0);
    addShip(univ, 3,  "s3", "thr",  1000, 1200, 5);
    addShip(univ, 5,  "s5", "fiv",  1000, 1200, 5);
    addShip(univ, 7,  "s7", "sev",  2000, 1200, 5);
    addShip(univ, 9,  "s9", "nin",  1000, 1200, 5);
    postprocess(h, h.session().getGame()->currentTurn());

    // Set up FleetProxy and select #9.
    game::proxy::FleetProxy testee(h.gameSender(), ind);
    testee.selectFleetMember(9);
    h.sync();
    ind.processQueue();
    a.checkEqual("01. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 5);
    a.checkEqual("02. currentShip", h.session().getGame()->cursors().currentShip().getCurrentIndex(), 9);
    a.checkEqual("03. getSelectedFleetMember", testee.getSelectedFleetMember(), 9);
    a.checkEqual("04. getFleetMemberList", testee.getFleetMemberList().size(), 4U);

    // Delete #9. Current should now be #7.
    h.gameSender().postNewRequest(new SetFleetNumberTask(9, 0));
    h.sync();
    ind.processQueue();

    a.checkEqual("11. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 5);
    a.checkEqual("12. currentShip", h.session().getGame()->cursors().currentShip().getCurrentIndex(), 7);
    a.checkEqual("13. getSelectedFleetMember", testee.getSelectedFleetMember(), 7);
    a.checkEqual("14. getFleetMemberList", testee.getFleetMemberList().size(), 3U);
}

/** Delete all fleets.
    Cursor must automatically advance.
    Proxy must not report 0, because control screen would take that to mean "no more fleets". */
AFL_TEST("game.proxy.FleetProxy:delete-all", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    prepare(h);
    game::map::Universe& univ = h.session().getGame()->currentTurn().universe();
    addShip(univ, 1,  "s1", "one",  1000, 1200, 0);
    addShip(univ, 3,  "s3", "thr",  1000, 1200, 3);
    addShip(univ, 5,  "s5", "fiv",  1000, 1200, 5);
    postprocess(h, h.session().getGame()->currentTurn());

    // Set up FleetProxy. This selects #3.
    game::proxy::FleetProxy testee(h.gameSender(), ind);
    IdChecker checker(testee);
    testee.sig_change.add(&checker, &IdChecker::onFleetChange);
    h.sync();
    ind.processQueue();
    a.checkEqual("01. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 3);
    a.checkEqual("02. getSelectedFleetMember", testee.getSelectedFleetMember(), 3);
    a.check("03. contains 3", checker.contains(3));
    a.check("04. contains 0", !checker.contains(0));

    // Delete this fleet. Current should now be #5. Proxy must not report an intermediate 0.
    checker.clear();
    h.gameSender().postNewRequest(new SetFleetNumberTask(3, 0));
    h.sync();
    ind.processQueue();
    a.checkEqual("11. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 5);
    a.checkEqual("12. getSelectedFleetMember", testee.getSelectedFleetMember(), 5);
    a.check("13. contains 5", checker.contains(5));
    a.check("14. contains 0", !checker.contains(0));

    // Delete #5. Should now report 0.
    checker.clear();
    h.gameSender().postNewRequest(new SetFleetNumberTask(5, 0));
    h.sync();
    ind.processQueue();
    a.checkEqual("21. currentFleet", h.session().getGame()->cursors().currentFleet().getCurrentIndex(), 0);
    a.checkEqual("22. getSelectedFleetMember", testee.getSelectedFleetMember(), 0);
    a.check("23. contains 0", checker.contains(0));
}
