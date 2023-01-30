/**
  *  \file u/t_game_proxy_scoreproxy.cpp
  *  \brief Test for game::proxy::ScoreProxy
  */

#include "game/proxy/scoreproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/test/root.hpp"
#include "game/game.hpp"

using game::proxy::ScoreProxy;

namespace {
    void prepare(game::test::SessionThread& h)
    {
        // Game with TurnScoreList, TeamSettings
        afl::base::Ptr<game::Game> g = new game::Game();
        game::score::TurnScoreList& scores = g->scores();
        game::score::TurnScoreList::Slot_t cap = scores.addSlot(game::score::ScoreId_Capital);
        game::score::TurnScoreList::Slot_t fre = scores.addSlot(game::score::ScoreId_Freighters);

        // - one turn
        game::score::TurnScore& ta = scores.addTurn(10, game::Timestamp(2000, 10, 10, 12, 0, 0));
        ta.set(cap, 4, 10);
        ta.set(fre, 4, 3);
        ta.set(cap, 5, 4);
        ta.set(fre, 5, 7);

        // - another turn
        game::score::TurnScore& tb = scores.addTurn(11, game::Timestamp(2000, 10, 11, 12, 0, 0));
        tb.set(cap, 4, 11);
        tb.set(fre, 4, 3);
        tb.set(cap, 5, 4);
        tb.set(fre, 5, 9);

        game::TeamSettings& teams = g->teamSettings();
        teams.setPlayerTeam(4, 4);
        teams.setPlayerTeam(5, 4);
        teams.setTeamName(4, "Me");
        h.session().setGame(g);

        // Root with PlayerList, HostVersion, Configuration
        afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))).asPtr();
        r->playerList().create(4)->setName(game::Player::ShortName, "The Klingons");
        r->playerList().create(5)->setName(game::Player::ShortName, "The Orions");
        h.session().setRoot(r);
    }

    class DataReceiver {
     public:
        void onUpdate(std::auto_ptr<util::DataTable>& tab)
            { m_table = tab; }
        util::DataTable* get() const
            { return m_table.get(); }

     private:
        std::auto_ptr<util::DataTable> m_table;
    };
}


/** Test that ScoreProxy can be constructed on empty universe. */
void
TestGameProxyScoreProxy::testEmpty()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    ScoreProxy testee(ind, h.gameSender());

    {
        ScoreProxy::Variants_t var;
        testee.getChartVariants(ind, var);
        TS_ASSERT(var.empty());
    }

    {
        ScoreProxy::Variants_t var;
        testee.getTableVariants(ind, var);
        TS_ASSERT(var.empty());
    }
}

/** Test that ScoreProxy can produce charts. */
void
TestGameProxyScoreProxy::testChart()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    prepare(h);
    ScoreProxy testee(ind, h.gameSender());

    // Must have more than 0 variants
    ScoreProxy::Variants_t var;
    testee.getChartVariants(ind, var);
    TS_ASSERT(!var.empty());

    // Retrieve default chart
    DataReceiver recv;
    testee.sig_chartUpdate.add(&recv, &DataReceiver::onUpdate);
    testee.setChartIndex(0);
    h.sync();
    ind.processQueue();

    TS_ASSERT(recv.get() != 0);
    TS_ASSERT_EQUALS(recv.get()->getNumRows(), 2U);
    TS_ASSERT_EQUALS(recv.get()->getRow(0)->getName(), "The Klingons");

    // Toggle options (mostly for coverage...)
    testee.setByTeam(true);
    testee.setCumulativeMode(true);
    h.sync();
    ind.processQueue();

    TS_ASSERT(recv.get() != 0);
    TS_ASSERT_EQUALS(recv.get()->getNumRows(), 1U);
    TS_ASSERT_EQUALS(recv.get()->getRow(0)->getName(), "Me");
}

/** Test that ScoreProxy can produce tables. */
void
TestGameProxyScoreProxy::testTable()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    prepare(h);
    ScoreProxy testee(ind, h.gameSender());

    // Must have more than 0 variants
    ScoreProxy::Variants_t var;
    testee.getTableVariants(ind, var);
    TS_ASSERT(!var.empty());

    // Retrieve default table
    DataReceiver recv;
    testee.sig_tableUpdate.add(&recv, &DataReceiver::onUpdate);
    testee.setTableTurnIndex(0);
    h.sync();
    ind.processQueue();

    TS_ASSERT(recv.get() != 0);
    TS_ASSERT_EQUALS(recv.get()->getNumRows(), 2U);
    TS_ASSERT_EQUALS(recv.get()->getRow(0)->getName(), "The Klingons");
    TS_ASSERT_EQUALS(recv.get()->getColumnName(0), var[0].name);

    // Retrieve difference-by-team table (mostly for coverage)
    testee.setByTeam(true);
    testee.setTableTurnDifferenceIndexes(1, 0);
    h.sync();
    ind.processQueue();

    TS_ASSERT(recv.get() != 0);
    TS_ASSERT_EQUALS(recv.get()->getNumRows(), 1U);
    TS_ASSERT_EQUALS(recv.get()->getRow(0)->getName(), "Me");
    TS_ASSERT_EQUALS(recv.get()->getColumnName(0), var[0].name);
}

/** Test getTurns(). */
void
TestGameProxyScoreProxy::testTurns()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    prepare(h);
    ScoreProxy testee(ind, h.gameSender());

    // Retrieve turns
    util::StringList list;
    testee.getTurns(ind, list);

    // Verify
    TS_ASSERT_EQUALS(list.size(), 2U);

    int32_t turn;
    String_t timeStamp;
    TS_ASSERT_EQUALS(list.get(0, turn, timeStamp), true);
    TS_ASSERT_EQUALS(turn, 10);
    TS_ASSERT_EQUALS(timeStamp, "10-10-200012:00:00");

    TS_ASSERT_EQUALS(list.get(1, turn, timeStamp), true);
    TS_ASSERT_EQUALS(turn, 11);
    TS_ASSERT_EQUALS(timeStamp, "10-11-200012:00:00");
}

/** Test getOverviewInformation. */
void
TestGameProxyScoreProxy::testOverview()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    prepare(h);
    ScoreProxy testee(ind, h.gameSender());

    // Retrieve information
    ScoreProxy::Info info;
    testee.getOverviewInformation(ind, info);

    // Verify
    TS_ASSERT_EQUALS(info.numTurns, 2U);
    TS_ASSERT_EQUALS(info.hasTeams, true);
    TS_ASSERT_EQUALS(info.players, game::PlayerSet_t() + 4 + 5);
}

