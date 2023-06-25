/**
  *  \file u/t_game_proxy_historyturnproxy.cpp
  *  \brief Test for game::proxy::HistoryTurnProxy
  */

#include "game/proxy/historyturnproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "game/game.hpp"
#include "game/test/waitindicator.hpp"
#include "game/task.hpp"

namespace {
    game::Timestamp makeTimestamp(int turnNumber)
    {
        return game::Timestamp(1990 + turnNumber, 5, 7, 12, 30, 25);
    }

    void configureTurn(game::Turn& t, int turnNumber)
    {
        t.setTimestamp(makeTimestamp(turnNumber));
        t.setTurnNumber(turnNumber);
    }

    class TestTurnLoader : public game::TurnLoader {
     public:
        virtual PlayerStatusSet_t getPlayerStatus(int /*player*/, String_t& /*extra*/, afl::string::Translator& /*tx*/) const
            { return PlayerStatusSet_t(); }
        virtual std::auto_ptr<game::Task_t> loadCurrentTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            {
                TS_FAIL("unexpected: loadCurrentTurn");
                return game::makeConfirmationTask(true, then);
            }
        virtual std::auto_ptr<game::Task_t> saveCurrentTurn(const game::Turn& /*turn*/, const game::Game& /*game*/, game::PlayerSet_t /*players*/, SaveOptions_t /*opts*/, const game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            {
                TS_FAIL("unexpected: saveCurrentTurn");
                return game::makeConfirmationTask(true, then);
            }
        virtual void getHistoryStatus(int /*player*/, int turn, afl::base::Memory<HistoryStatus> status, const game::Root& /*root*/)
            {
                while (HistoryStatus* p = status.eat()) {
                    *p = turnStatus[turn];
                    ++turn;
                }
            }
        virtual std::auto_ptr<game::Task_t> loadHistoryTurn(game::Turn& turn, game::Game& /*game*/, int /*player*/, int turnNumber, game::Root& /*root*/, std::auto_ptr<game::StatusTask_t> then)
            {
                configureTurn(turn, turnNumber);
                return game::makeConfirmationTask(this->loadStatus[turnNumber], then);
            }
        virtual std::auto_ptr<game::Task_t> saveConfiguration(const game::Root& /*root*/, std::auto_ptr<game::Task_t> then)
            {
                TS_FAIL("unexpected: saveConfiguration");
                return then;
            }
        virtual String_t getProperty(Property /*p*/)
            { return ""; }

        std::map<int, HistoryStatus> turnStatus;
        std::map<int, bool> loadStatus;
    };


    struct UpdateReceiver {
        game::proxy::HistoryTurnProxy::Items_t items;
        int turnNumber;

        UpdateReceiver()
            : items(), turnNumber()
            { }
        void onSetup(const game::proxy::HistoryTurnProxy::Items_t& items, int turnNumber)
            {
                this->items = items;
                this->turnNumber = turnNumber;
            }
        void onUpdate(const game::proxy::HistoryTurnProxy::Items_t& items)
            { onSetup(items, -1); }
    };
}

/** Test normal operation sequence. */
void
TestGameProxyHistoryTurnProxy::testIt()
{
    // A fully populated session
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    afl::base::Ref<game::Root> r(game::test::makeRoot(game::HostVersion()));
    t.session().setRoot(r.asPtr());

    afl::base::Ref<game::Game> g(*new game::Game());
    configureTurn(g->currentTurn(), 30);
    t.session().setGame(g.asPtr());

    t.session().setShipList(new game::spec::ShipList());

    // Turn loader with configured reactions
    afl::base::Ref<TestTurnLoader> tl(*new TestTurnLoader());
    r->setTurnLoader(tl.asPtr());

    tl->turnStatus[29] = game::TurnLoader::WeaklyPositive;
    tl->turnStatus[28] = game::TurnLoader::StronglyPositive;
    tl->turnStatus[27] = game::TurnLoader::Negative;
    tl->turnStatus[26] = game::TurnLoader::WeaklyPositive;

    tl->loadStatus[29] = true;       // WeaklyPositive -> Loaded
    tl->loadStatus[28] = false;      // StronglyPositive -> Failed
    tl->loadStatus[26] = false;      // WeaklyPositive -> Unavailable

    // Object under test
    game::proxy::HistoryTurnProxy testee(t.gameSender(), ind);
    UpdateReceiver sr, ur;
    testee.sig_setup.add(&sr, &UpdateReceiver::onSetup);
    testee.sig_update.add(&ur, &UpdateReceiver::onUpdate);

    // Receive setup into 'sr' - must receive current status [Unknown,Unknown,....,Current]
    testee.requestSetup(20);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(sr.items.size(), 20U);
    TS_ASSERT_EQUALS(sr.items[0].turnNumber, 11);
    TS_ASSERT_EQUALS(sr.items[0].status, game::proxy::HistoryTurnProxy::Unknown);
    TS_ASSERT_EQUALS(sr.items[19].turnNumber, 30);
    TS_ASSERT_EQUALS(sr.items[19].status, game::proxy::HistoryTurnProxy::Current);

    // Try to update 5 turns - must update according to configured reactions
    testee.requestUpdate(21, 5);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(ur.items.size(), 5U);
    TS_ASSERT_EQUALS(ur.items[0].turnNumber, 25);
    TS_ASSERT_EQUALS(ur.items[0].status, game::proxy::HistoryTurnProxy::Unavailable);
    TS_ASSERT_EQUALS(ur.items[1].turnNumber, 26);
    TS_ASSERT_EQUALS(ur.items[1].status, game::proxy::HistoryTurnProxy::WeaklyAvailable)
    TS_ASSERT_EQUALS(ur.items[2].turnNumber, 27);
    TS_ASSERT_EQUALS(ur.items[2].status, game::proxy::HistoryTurnProxy::Unavailable);
    TS_ASSERT_EQUALS(ur.items[3].turnNumber, 28);
    TS_ASSERT_EQUALS(ur.items[3].status, game::proxy::HistoryTurnProxy::StronglyAvailable);
    TS_ASSERT_EQUALS(ur.items[4].turnNumber, 29);
    TS_ASSERT_EQUALS(ur.items[4].status, game::proxy::HistoryTurnProxy::WeaklyAvailable);

    // Update 5 more; first-turn limit kicks in
    testee.requestUpdate(21, 5);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(ur.items.size(), 4U);
    TS_ASSERT_EQUALS(ur.items[0].turnNumber, 21);
    TS_ASSERT_EQUALS(ur.items[0].status, game::proxy::HistoryTurnProxy::Unavailable);

    // Try to load 29 (WeaklyAvailable/positive result)
    testee.requestLoad(29);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(ur.items.size(), 1U);
    TS_ASSERT_EQUALS(ur.items[0].turnNumber, 29);
    TS_ASSERT_EQUALS(ur.items[0].status, game::proxy::HistoryTurnProxy::Loaded);

    // Try to load 28 (StronglyAvailable/negative result)
    testee.requestLoad(28);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(ur.items.size(), 1U);
    TS_ASSERT_EQUALS(ur.items[0].turnNumber, 28);
    TS_ASSERT_EQUALS(ur.items[0].status, game::proxy::HistoryTurnProxy::Failed);

    // Try to load 26 (WeaklyAvailable/negative result)
    testee.requestLoad(26);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(ur.items.size(), 1U);
    TS_ASSERT_EQUALS(ur.items[0].turnNumber, 26);
    TS_ASSERT_EQUALS(ur.items[0].status, game::proxy::HistoryTurnProxy::Unavailable);

    // Load 29 again (no change, but still generates an update)
    testee.requestLoad(29);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(ur.items.size(), 1U);
    TS_ASSERT_EQUALS(ur.items[0].turnNumber, 29);
    TS_ASSERT_EQUALS(ur.items[0].status, game::proxy::HistoryTurnProxy::Loaded);
}

/** Test error case: no TurnLoader.
    Responses must still make sense; no crash. */
void
TestGameProxyHistoryTurnProxy::testNoTurnLoader()
{
    // A fully populated session (but no TurnLoader)
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    afl::base::Ref<game::Root> r(game::test::makeRoot(game::HostVersion()));
    t.session().setRoot(r.asPtr());

    afl::base::Ref<game::Game> g(*new game::Game());
    configureTurn(g->currentTurn(), 30);
    t.session().setGame(g.asPtr());

    t.session().setShipList(new game::spec::ShipList());

    // Object under test
    game::proxy::HistoryTurnProxy testee(t.gameSender(), ind);
    UpdateReceiver sr, ur;
    testee.sig_setup.add(&sr, &UpdateReceiver::onSetup);
    testee.sig_update.add(&ur, &UpdateReceiver::onUpdate);

    // Receive setup into 'sr' - always succeeds
    testee.requestSetup(20);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(sr.items.size(), 20U);
    TS_ASSERT_EQUALS(sr.items[0].turnNumber, 11);
    TS_ASSERT_EQUALS(sr.items[0].status, game::proxy::HistoryTurnProxy::Unknown);
    TS_ASSERT_EQUALS(sr.items[19].turnNumber, 30);
    TS_ASSERT_EQUALS(sr.items[19].status, game::proxy::HistoryTurnProxy::Current);

    // Update - must receive an empty update [not strictly contractual]
    testee.requestUpdate(21, 5);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(ur.items.size(), 0U);

    // Try to load a turn - must receive a failure response
    testee.requestLoad(29);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(ur.items.size(), 1U);
    TS_ASSERT_EQUALS(ur.items[0].turnNumber, 29);
    TS_ASSERT_EQUALS(ur.items[0].status, game::proxy::HistoryTurnProxy::Unknown);
}

/** Test error case: empty session.
    Responses must still make sense; no crash. */
void
TestGameProxyHistoryTurnProxy::testEmpty()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;

    // Object under test
    game::proxy::HistoryTurnProxy testee(t.gameSender(), ind);
    UpdateReceiver sr, ur;
    testee.sig_setup.add(&sr, &UpdateReceiver::onSetup);
    testee.sig_update.add(&ur, &UpdateReceiver::onUpdate);

    // Receive setup into 'sr' - must receive an empty update
    testee.requestSetup(20);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(sr.items.size(), 0U);

    // Update - must receive an empty update
    testee.requestUpdate(21, 5);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(ur.items.size(), 0U);

    // Try to load a turn - must receive an empty update
    testee.requestLoad(29);
    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(ur.items.size(), 0U);
}

