/**
  *  \file test/game/proxy/imperialstatsproxytest.cpp
  *  \brief Test for game::proxy::ImperialStatsProxy
  */

#include "game/proxy/imperialstatsproxy.hpp"

#include "afl/io/filemapping.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/xml/writer.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/info/scriptlinkbuilder.hpp"
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

    // Shortcut to create LinkBuilder
    std::auto_ptr<game::map::info::LinkBuilder> makeLinkBuilder()
    {
        return std::auto_ptr<game::map::info::LinkBuilder>(new game::map::info::ScriptLinkBuilder());
    }

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
AFL_TEST("game.proxy.ImperialStatsProxy:empty", a)
{
    SessionThread t;
    WaitIndicator ind;
    ImperialStatsProxy testee(t.gameSender(), ind, makeLinkBuilder());

    NodeReceiver recv;
    testee.sig_pageContent.add(&recv, &NodeReceiver::onPageContent);

    testee.requestPageContent(game::map::info::ColonyPage);

    t.sync();
    ind.processQueue();

    a.checkEqual("01. getCount", recv.getCount(), 1);
}


/** Test behaviour on nonempty session.
    A: set up a session and place Root/Game/ShipList in it (need not be populated). Request a page.
    E: a response with correct text must be generated */
AFL_TEST("game.proxy.ImperialStatsProxy:normal", a)
{
    SessionThread t;
    t.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    t.session().setShipList(new game::spec::ShipList());
    t.session().setGame(new game::Game());

    WaitIndicator ind;
    ImperialStatsProxy testee(t.gameSender(), ind, makeLinkBuilder());

    NodeReceiver recv;
    testee.sig_pageContent.add(&recv, &NodeReceiver::onPageContent);

    testee.requestPageContent(game::map::info::ColonyPage);

    t.sync();
    ind.processQueue();

    a.checkEqual("01. get", recv.get(),
                 "<h1>Colony</h1>"
                 "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Colonists Planets</font></td><td align=\"right\" width=\"8\">(clans)</td></tr></table>"
                 "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Supplies Planets</font></td><td align=\"right\" width=\"8\">(kt)</td></tr></table>"
                 "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Money Planets</font></td><td align=\"right\" width=\"8\">(mc)</td></tr></table>");
    a.checkEqual("02. getCount", recv.getCount(), 1);
}

/** Test option handling.
    A: set up a session. Request options. Change options.
    E: correct option list reported (content of session does not matter) */
AFL_TEST("game.proxy.ImperialStatsProxy:requestPageOptions", a)
{
    SessionThread t;
    WaitIndicator ind;
    ImperialStatsProxy testee(t.gameSender(), ind, makeLinkBuilder());

    OptionReceiver recv;
    testee.sig_pageOptions.add(&recv, &OptionReceiver::onPageOptions);

    // Request initial options
    testee.requestPageOptions(game::map::info::StarshipPage);

    t.sync();
    ind.processQueue();

    // Verify initial options
    a.check("01. hasOption", recv.hasOption("Show only hull list", game::map::info::Ships_HideTop));
    a.checkEqual("02. get", recv.get(), 0);

    // Modify options and request again
    testee.setPageOptions(game::map::info::StarshipPage, game::map::info::Ships_HideTop);
    testee.requestPageOptions(game::map::info::StarshipPage);

    t.sync();
    ind.processQueue();

    // Verify changed options
    a.check("11. hasOption", recv.hasOption("Show all info", 0));
    a.checkEqual("12. get", recv.get(), game::map::info::Ships_HideTop);
}

/** Test that options actually affect content.
    A: set up a session and place Root/Game/ShipList in it (need not be populated). Set options and request a page.
    E: a response with correct text must be generated */
AFL_TEST("game.proxy.ImperialStatsProxy:setPageOptions", a)
{
    SessionThread t;
    t.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    t.session().setShipList(new game::spec::ShipList());
    t.session().setGame(new game::Game());

    WaitIndicator ind;
    ImperialStatsProxy testee(t.gameSender(), ind, makeLinkBuilder());

    NodeReceiver recv;
    testee.sig_pageContent.add(&recv, &NodeReceiver::onPageContent);

    testee.setPageOptions(game::map::info::ColonyPage, game::map::info::Colony_ShowOnlySupplies);
    testee.requestPageContent(game::map::info::ColonyPage);

    t.sync();
    ind.processQueue();

    a.checkEqual("01. get", recv.get(),
                 "<h1>Colony</h1>"
                 "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 24 Supplies Planets</font></td><td align=\"right\" width=\"8\">(kt)</td></tr></table>");
    a.checkEqual("02. getCount", recv.getCount(), 1);
}

/** Test savePageAsHTML().
    A: set up a situation with mock file system and call savePageAsHTML()
    E: correct result generated */
AFL_TEST("game.proxy.ImperialStatsProxy:savePageAsHTML", a)
{
    afl::io::InternalFileSystem fs;
    SessionThread t(fs);
    t.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    t.session().setShipList(new game::spec::ShipList());
    t.session().setGame(new game::Game());

    WaitIndicator ind;
    ImperialStatsProxy testee(t.gameSender(), ind, makeLinkBuilder());
    testee.setPageOptions(game::map::info::ColonyPage, game::map::info::Colony_ShowOnlySupplies);

    String_t error;
    bool ok = testee.savePageAsHTML(ind, game::map::info::ColonyPage, "/out.html", error);
    a.check("01. savePageAsHTML", ok);

    // Verify file content
    String_t content = afl::string::fromBytes(fs.openFile("/out.html", afl::io::FileSystem::OpenRead)->createVirtualMapping()->get());
    a.check("11. content: title", content.find("<title>Colony</title>") != String_t::npos);
    a.check("12. content: table", content.find("<table align=\"left\" class=\"normaltable\"><tr><td valign=\"top\" width=\"256\"><span class=\"color-white\">Top 24 Supplies Planets</span></td><td valign=\"top\" align=\"right\" width=\"128\">(kt)</td></tr></table>") != String_t::npos);
}

/** Test savePageAsHTML(), error case.
    A: set up a situation with mock file system and call savePageAsHTML() with a failing file name.
    E: correct result generated */
AFL_TEST("game.proxy.ImperialStatsProxy:savePageAsHTML:error", a)
{
    afl::io::InternalFileSystem fs;
    SessionThread t(fs);
    t.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    t.session().setShipList(new game::spec::ShipList());
    t.session().setGame(new game::Game());

    WaitIndicator ind;
    ImperialStatsProxy testee(t.gameSender(), ind, makeLinkBuilder());
    testee.setPageOptions(game::map::info::ColonyPage, game::map::info::Colony_ShowOnlySupplies);

    String_t error;
    bool ok = testee.savePageAsHTML(ind, game::map::info::ColonyPage, "/nonexistant-subdir/out.html", error);
    a.check("01. savePageAsHTML", !ok);
    a.checkDifferent("02. error", error, "");
}
