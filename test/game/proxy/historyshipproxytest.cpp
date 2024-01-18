/**
  *  \file test/game/proxy/historyshipproxytest.cpp
  *  \brief Test for game::proxy::HistoryShipProxy
  */

#include "game/proxy/historyshipproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"
#include <memory>

using game::proxy::HistoryShipProxy;
using game::map::Point;

namespace {
    const int TURN_NR = 10;

    // Let source be different from ship owner, to make "true" scans; see TestGameRefHistoryShipSelection.
    const int SOURCE_PLAYER = 1;
    const int OWNER = 2;

    class Listener {
     public:
        void onChange(const HistoryShipProxy::Status& st)
            { m_status.reset(new HistoryShipProxy::Status(st)); }

        const HistoryShipProxy::Status* get() const
            { return m_status.get(); }

        void reset()
            { m_status.reset(); }

     private:
        std::auto_ptr<HistoryShipProxy::Status> m_status;
    };

    void prepare(game::test::SessionThread& h)
    {
        h.session().setGame(new game::Game());
        h.session().setShipList(new game::spec::ShipList());
        h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        h.session().getGame()->currentTurn().setTurnNumber(TURN_NR);
    }

    game::map::Ship& addShip(game::test::SessionThread& h, game::Id_t id, Point pos, int owner)
    {

        game::map::Universe& u = h.session().getGame()->currentTurn().universe();
        game::map::Ship& sh = *u.ships().create(id);
        sh.addShipXYData(pos, owner, 100, game::PlayerSet_t(SOURCE_PLAYER));
        sh.setPlayability(game::map::Object::NotPlayable);
        return sh;
    }

    void addShips(game::test::SessionThread& h)
    {
        addShip(h, 10, Point(1000, 1000), OWNER);
        addShip(h, 20, Point(1000, 1020), OWNER);
        addShip(h, 30, Point(1000, 1000), OWNER);
        addShip(h, 40, Point(1000, 1000), OWNER);
        addShip(h, 50, Point(1000, 1000), OWNER);
        h.session().postprocessTurn(h.session().getGame()->currentTurn(), game::PlayerSet_t(SOURCE_PLAYER), game::PlayerSet_t(SOURCE_PLAYER), game::map::Object::Playable);
    }
}


/** Test behaviour on empty session. */
AFL_TEST_NOARG("game.proxy.HistoryShipProxy:empty")
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    HistoryShipProxy testee(h.gameSender(), ind);

    h.sync();
    ind.processQueue();

    // Cannot check for results, as HistoryShipProxy::Trampoline fails to construct, but it shall not crash
}

/** Test behaviour on populated session with no ships. */
AFL_TEST("game.proxy.HistoryShipProxy:no-ship", a)
{
    // Set up
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    prepare(h);

    HistoryShipProxy testee(h.gameSender(), ind);
    Listener lis;
    testee.sig_change.add(&lis, &Listener::onChange);

    // Process tasks
    h.sync();
    ind.processQueue();

    // Check
    a.checkNonNull("01. get", lis.get());
    a.checkEqual("02. shipId", lis.get()->shipId, 0);
}

/** Test normal behaviour, including history-ship specific browsing. */
AFL_TEST("game.proxy.HistoryShipProxy:normal", a)
{
    // Set up
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    prepare(h);
    addShips(h);

    HistoryShipProxy testee(h.gameSender(), ind);
    Listener lis;
    testee.sig_change.add(&lis, &Listener::onChange);

    // Process tasks
    h.sync();
    ind.processQueue();

    // Verify
    a.checkNonNull("01. get", lis.get());
    a.checkEqual("02. shipId", lis.get()->shipId, 10);
    a.check("03. locations", lis.get()->locations.size() > 0);
    a.checkEqual("04. locations", lis.get()->locations[0].turnNumber, TURN_NR);
    a.check("05. turnNumber", !lis.get()->turnNumber.isValid());                // no turn suggestion

    // Browse - next
    {
        lis.reset();
        testee.browseAt(Point(1000, 1000), HistoryShipProxy::Next, false);
        h.sync();
        ind.processQueue();

        a.checkNonNull("11. get", lis.get());
        a.checkEqual("12. shipId", lis.get()->shipId, 30);
        a.check("13. locations", lis.get()->locations.size() > 0);
        a.checkEqual("14. locations", lis.get()->locations[0].turnNumber, TURN_NR);
        a.checkEqual("15. turnNumber", lis.get()->turnNumber.orElse(-1), TURN_NR);       // turn suggested because explicitly browsed
    }

    // Browse - last
    {
        lis.reset();
        testee.browseAt(Point(1000, 1000), HistoryShipProxy::Last, false);
        h.sync();
        ind.processQueue();

        a.checkNonNull("21. lis", lis.get());
        a.checkEqual("22. shipId", lis.get()->shipId, 50);
        a.check("23. locations", lis.get()->locations.size() > 0);
        a.checkEqual("24. locations", lis.get()->locations[0].turnNumber, TURN_NR);
        a.checkEqual("25. turnNumber", lis.get()->turnNumber.orElse(-1), TURN_NR);       // turn suggested because explicitly browsed
    }

    // Browse - prev
    {
        lis.reset();
        testee.browseAt(Point(1000, 1000), HistoryShipProxy::Previous, false);
        h.sync();
        ind.processQueue();

        a.checkNonNull("31. lis", lis.get());
        a.checkEqual("32. shipId", lis.get()->shipId, 40);
        a.check("33. locations", lis.get()->locations.size() > 0);
        a.checkEqual("34. locations", lis.get()->locations[0].turnNumber, TURN_NR);
        a.checkEqual("35. turnNumber", lis.get()->turnNumber.orElse(-1), TURN_NR);       // turn suggested because explicitly browsed
    }

    // Browse - first
    {
        lis.reset();
        testee.browseAt(Point(1000, 1000), HistoryShipProxy::First, false);
        h.sync();
        ind.processQueue();

        a.checkNonNull("41. lis", lis.get());
        a.checkEqual("42. shipId", lis.get()->shipId, 10);
        a.check("43. locations", lis.get()->locations.size() > 0);
        a.checkEqual("44. locations", lis.get()->locations[0].turnNumber, TURN_NR);
        a.checkEqual("45. turnNumber", lis.get()->turnNumber.orElse(-1), TURN_NR);       // turn suggested because explicitly browsed
    }
}

/** Test interaction with external browse operations. */
AFL_TEST("game.proxy.HistoryShipProxy:external-browsing", a)
{
    // Set up
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    prepare(h);
    addShips(h);

    HistoryShipProxy testee(h.gameSender(), ind);
    Listener lis;
    testee.sig_change.add(&lis, &Listener::onChange);

    // Process tasks
    h.sync();
    ind.processQueue();

    // Verify
    a.checkNonNull("01. lis", lis.get());
    a.checkEqual("02. shipId", lis.get()->shipId, 10);
    a.check("03. locations", lis.get()->locations.size() > 0);
    a.checkEqual("04. locations", lis.get()->locations[0].turnNumber, TURN_NR);
    a.check("05. turnNumber", !lis.get()->turnNumber.isValid());                // no turn suggestion

    // Browse externally
    class Task : public util::Request<game::Session> {
     public:
        virtual void handle(game::Session& s)
            { s.getGame()->cursors().currentHistoryShip().browse(game::map::ObjectCursor::Next, false); }
    };
    h.gameSender().postNewRequest(new Task());
    h.sync();
    ind.processQueue();

    // Verify
    a.checkNonNull("11. lis", lis.get());
    a.checkEqual("12. shipId", lis.get()->shipId, 20);
    a.check("13. locations", lis.get()->locations.size() > 0);
    a.checkEqual("14. locations", lis.get()->locations[0].turnNumber, TURN_NR);
    a.check("15. turnNumber", !lis.get()->turnNumber.isValid());                // no turn suggestion
}
