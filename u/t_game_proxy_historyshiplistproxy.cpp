/**
  *  \file u/t_game_proxy_historyshiplistproxy.cpp
  *  \brief Test for game::proxy::HistoryShipListProxy
  */

#include "game/proxy/historyshiplistproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/game.hpp"

using game::map::Point;

namespace {
    class UpdateReceiver {
     public:
        void onListChange(const game::ref::HistoryShipList& list)
            { m_list = list; }
        const game::ref::HistoryShipList& list() const
            { return m_list; }
     private:
        game::ref::HistoryShipList m_list;
    };

    game::ref::HistoryShipSelection makeSelection()
    {
        game::ref::HistoryShipSelection sel;
        sel.setPosition(game::map::Point(1000, 1000));
        sel.setMode(game::ref::HistoryShipSelection::LocalShips);
        return sel;
    }

    /*
     *  Adding a ship
     */

    const int TURN_NR = 32;

    game::map::Ship& addShip(game::test::SessionThread& h, game::Id_t id, Point pos, int owner)
    {
        // Let source be different from owner, to make this "true" scans; see TestGameRefHistoryShipSelection.
        game::PlayerSet_t source(owner+1);

        game::map::Universe& u = h.session().getGame()->currentTurn().universe();
        game::map::Ship& sh = *u.ships().create(id);
        sh.addShipXYData(pos, owner, 100, source);
        sh.internalCheck(source, TURN_NR);
        sh.setPlayability(game::map::Object::NotPlayable);
        return sh;
    }

}

void
TestGameProxyHistoryShipListProxy::testEmpty()
{
    // Set up empty session
    game::test::SessionThread h;
    game::test::WaitIndicator ind;

    // Set up testee
    game::proxy::HistoryShipListProxy testee(h.gameSender(), ind);
    UpdateReceiver recv;
    testee.sig_listChange.add(&recv, &UpdateReceiver::onListChange);

    // Request
    testee.setSelection(makeSelection());
    h.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(recv.list().size(), 0U);
}

void
TestGameProxyHistoryShipListProxy::testNormal()
{
    // Set up session
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    h.session().setGame(new game::Game());
    addShip(h, 10, Point(1000, 1000), 3);
    addShip(h, 20, Point(1000, 1020), 3);
    addShip(h, 30, Point(1000, 1000), 3);

    // Set up testee
    game::proxy::HistoryShipListProxy testee(h.gameSender(), ind);
    UpdateReceiver recv;
    testee.sig_listChange.add(&recv, &UpdateReceiver::onListChange);

    // Request
    testee.setSelection(makeSelection());
    h.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(recv.list().size(), 2U);
    TS_ASSERT_EQUALS(recv.list().get(0)->name, "Ship #10");
    TS_ASSERT_EQUALS(recv.list().get(1)->name, "Ship #30");
    TS_ASSERT_EQUALS(recv.list().get(0)->marked, false);
    TS_ASSERT_EQUALS(recv.list().get(1)->marked, false);

    // Exercise unsolicited updates
    h.session().getGame()->currentTurn().universe().ships().get(30)->setIsMarked(true);
    h.session().notifyListeners();
    h.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(recv.list().size(), 2U);
    TS_ASSERT_EQUALS(recv.list().get(0)->name, "Ship #10");
    TS_ASSERT_EQUALS(recv.list().get(1)->name, "Ship #30");
    TS_ASSERT_EQUALS(recv.list().get(0)->marked, false);
    TS_ASSERT_EQUALS(recv.list().get(1)->marked, true);
}
