/**
  *  \file u/t_game_proxy_imperialstatsproxy.cpp
  *  \brief Test for game::proxy::ImperialStatsProxy
  */

#include "game/proxy/imperialstatsproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/xml/writer.hpp"
#include "game/game.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"

using game::proxy::ImperialStatsProxy;
using game::test::SessionThread;
using game::test::WaitIndicator;

namespace {
    // Event handler for sig_pageContent
    class NodeReceiver {
     public:
        NodeReceiver()
            : m_count(),
              m_nodes()
            { }

        void onPageContent(ImperialStatsProxy::Nodes_t& nodes)
            {
                ++m_count;
                m_nodes.swap(nodes);
            }

        String_t get() const
            {
                afl::io::InternalSink sink;
                afl::io::xml::Writer(sink).visit(m_nodes);
                return afl::string::fromBytes(sink.getContent());
            }

        int getCount() const
            { return m_count; }

     private:
        int m_count;
        ImperialStatsProxy::Nodes_t m_nodes;
    };

    // Event handler for sig_pageOptions
    class OptionReceiver {
     public:
        OptionReceiver()
            : m_list(), m_options()
            { }

        void onPageOptions(const util::StringList& list, game::map::info::PageOptions_t opts)
            {
                m_list = list;
                m_options = opts;
            }

        bool hasOption(String_t title, uint8_t value) const
            {
                for (size_t i = 0; i < m_list.size(); ++i) {
                    String_t foundTitle;
                    int32_t foundKey;
                    if (m_list.get(i, foundKey, foundTitle) && foundKey == value && foundTitle == title) {
                        return true;
                    }
                }
                return false;
            }

        game::map::info::PageOptions_t get() const
            { return m_options; }

     private:
        util::StringList m_list;
        game::map::info::PageOptions_t m_options;
    };
}

/** Test behaviour on empty session.
    A: set up an empty session. Request a page.
    E: a response must be generated, even if game-side throws an exception */
void
TestGameProxyImperialStatsProxy::testEmpty()
{
    SessionThread t;
    WaitIndicator ind;
    ImperialStatsProxy testee(t.gameSender(), ind);

    NodeReceiver recv;
    testee.sig_pageContent.add(&recv, &NodeReceiver::onPageContent);

    testee.requestPageContent(game::map::info::ColonyPage);

    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(recv.getCount(), 1);
}


/** Test behaviour on nonempty session.
    A: set up a session and place Root/Game/ShipList in it (need not be populated). Request a page.
    E: a response with correct text must be generated */
void
TestGameProxyImperialStatsProxy::testNonempty()
{
    SessionThread t;
    t.session().setRoot(new game::test::Root(game::HostVersion()));
    t.session().setShipList(new game::spec::ShipList());
    t.session().setGame(new game::Game());

    WaitIndicator ind;
    ImperialStatsProxy testee(t.gameSender(), ind);

    NodeReceiver recv;
    testee.sig_pageContent.add(&recv, &NodeReceiver::onPageContent);

    testee.requestPageContent(game::map::info::ColonyPage);

    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(recv.get(),
                     "<h1>Colony</h1>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Colonists Planets</font></td><td align=\"right\" width=\"8\">(clans)</td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Supplies Planets</font></td><td align=\"right\" width=\"8\">(kt)</td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Money Planets</font></td><td align=\"right\" width=\"8\">(mc)</td></tr></table>");
    TS_ASSERT_EQUALS(recv.getCount(), 1);
}

/** Test option handling.
    A: set up a session. Request options. Change options.
    E: correct option list reported (content of session does not matter) */
void
TestGameProxyImperialStatsProxy::testOptions()
{
    SessionThread t;
    WaitIndicator ind;
    ImperialStatsProxy testee(t.gameSender(), ind);

    OptionReceiver recv;
    testee.sig_pageOptions.add(&recv, &OptionReceiver::onPageOptions);

    // Request initial options
    testee.requestPageOptions(game::map::info::StarshipPage);

    t.sync();
    ind.processQueue();

    // Verify initial options
    TS_ASSERT(recv.hasOption("Show only hull list", game::map::info::Ships_HideTop));
    TS_ASSERT_EQUALS(recv.get(), 0);

    // Modify options and request again
    testee.setPageOptions(game::map::info::StarshipPage, game::map::info::Ships_HideTop);
    testee.requestPageOptions(game::map::info::StarshipPage);

    t.sync();
    ind.processQueue();

    // Verify changed options
    TS_ASSERT(recv.hasOption("Show all info", 0));
    TS_ASSERT_EQUALS(recv.get(), game::map::info::Ships_HideTop);
}

/** Test that options actually affect content.
    A: set up a session and place Root/Game/ShipList in it (need not be populated). Set options and request a page.
    E: a response with correct text must be generated */
void
TestGameProxyImperialStatsProxy::testContentOptions()
{
    SessionThread t;
    t.session().setRoot(new game::test::Root(game::HostVersion()));
    t.session().setShipList(new game::spec::ShipList());
    t.session().setGame(new game::Game());

    WaitIndicator ind;
    ImperialStatsProxy testee(t.gameSender(), ind);

    NodeReceiver recv;
    testee.sig_pageContent.add(&recv, &NodeReceiver::onPageContent);

    testee.setPageOptions(game::map::info::ColonyPage, game::map::info::Colony_ShowOnlySupplies);
    testee.requestPageContent(game::map::info::ColonyPage);

    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(recv.get(),
                     "<h1>Colony</h1>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 24 Supplies Planets</font></td><td align=\"right\" width=\"8\">(kt)</td></tr></table>");
    TS_ASSERT_EQUALS(recv.getCount(), 1);
}

