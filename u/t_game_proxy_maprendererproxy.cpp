/**
  *  \file u/t_game_proxy_maprendererproxy.cpp
  *  \brief Test for game::proxy::MapRendererProxy
  */

#include "game/proxy/maprendererproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/test/root.hpp"
#include "game/game.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"

using game::test::SessionThread;
using game::test::WaitIndicator;
using game::proxy::MapRendererProxy;
using game::map::Point;

namespace {
    /* Receiver to capture RenderList callbacks */
    class ResultReceiver {
     public:
        ResultReceiver(MapRendererProxy& proxy)
            : conn_update(proxy.sig_update.add(this, &ResultReceiver::onUpdate)),
              m_result()
            { }

        bool hasResult()
            { return m_result.get() != 0; }

        void replay(game::map::RendererListener& r)
            {
                if (m_result.get() != 0) {
                    m_result->replay(r);
                }
            }

     private:
        void onUpdate(afl::base::Ptr<game::map::RenderList> list)
            { m_result = list; }

        afl::base::SignalConnection conn_update;
        afl::base::Ptr<game::map::RenderList> m_result;
    };

    /* Receiver to capture configuration callbacks */
    class ConfigReceiver {
     public:
        ConfigReceiver()
            : m_result(), m_ok()
            { }
        void onConfiguration(game::map::RenderOptions opts)
            { m_result = opts; m_ok = true; }
        bool hasResult() const
            { return m_ok; }
        const game::map::RenderOptions& get() const
            { return m_result; }
     private:
        game::map::RenderOptions m_result;
        bool m_ok;
    };

    /* RendererListener to capture received marker colors */
    class MarkerCollector : public game::map::RenderList {
     public:
        typedef afl::bits::SmallSet<int> Colors_t;

        virtual void drawGridLine(Point, Point)
            { }
        virtual void drawBorderLine(Point, Point)
            { }
        virtual void drawBorderCircle(Point, int)
            { }
        virtual void drawSelection(Point)
            { }
        virtual void drawMessageMarker(Point)
            { }
        virtual void drawPlanet(Point, int, int, String_t)
            { }
        virtual void drawShip(Point, int, Relation_t, int, String_t)
            { }
        virtual void drawMinefield(Point, int, int, bool, Relation_t, bool)
            { }
        virtual void drawUfo(Point, int, int, int, int, int, bool)
            { }
        virtual void drawUfoConnection(Point, Point, int)
            { }
        virtual void drawIonStorm(Point, int, int, int, int, bool)
            { }
        virtual void drawUserCircle(Point, int, int color)
            { m_colors += color; }
        virtual void drawUserLine(Point, Point, int color)
            { m_colors += color; }
        virtual void drawUserRectangle(Point, Point, int color)
            { m_colors += color; }
        virtual void drawUserMarker(Point, int, int color, String_t)
            { m_colors += color; }
        virtual void drawExplosion(Point)
            { }
        virtual void drawShipTrail(Point, Point, Relation_t, int, int)
            { }
        virtual void drawShipWaypoint(Point, Point, Relation_t)
            { }
        virtual void drawShipVector(Point, Point, Relation_t)
            { }
        virtual void drawWarpWellEdge(Point, Edge)
            { }

        Colors_t getColors() const
            { return m_colors; }
     private:
        Colors_t m_colors;
    };

    /* Preparation */
    void prepare(SessionThread& t)
    {
        afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))).asPtr();
        t.session().setRoot(r);

        afl::base::Ptr<game::Game> g = new game::Game();
        t.session().setGame(g);

        afl::base::Ptr<game::spec::ShipList> sl = new game::spec::ShipList();
        t.session().setShipList(sl);
    }

    void addMarker(SessionThread& h, int x, int y, uint8_t color, util::Atom_t tag)
    {
        game::map::Drawing* d = new game::map::Drawing(game::map::Point(x, y), game::map::Drawing::MarkerDrawing);
        d->setColor(color);
        d->setTag(tag);
        h.session().getGame()->currentTurn().universe().drawings().addNew(d);
    }
}

/** Test behaviour on empty session.
    A: create empty session. Create and configure MapRendererProxy.
    E: no callback generated */
void
TestGameProxyMapRendererProxy::testEmpty()
{
    SessionThread h;
    WaitIndicator ind;
    MapRendererProxy testee(h.gameSender(), ind);
    ResultReceiver recv(testee);

    testee.setRange(Point(100, 100), Point(300, 300));
    h.sync();
    ind.processQueue();

    TS_ASSERT(!recv.hasResult());
}

/** Test normal behaviour.
    A: create session with some markers. Create and configure MapRendererProxy.
    E: callback generated with correct content */
void
TestGameProxyMapRendererProxy::testNormal()
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    addMarker(h, 1010, 1010, 1, 1);
    addMarker(h, 1020, 1030, 2, 2);
    addMarker(h, 1040, 1200, 3, 3);
    MapRendererProxy testee(h.gameSender(), ind);
    ResultReceiver recv(testee);

    testee.setRange(Point(1000, 1000), Point(1030, 1050));
    h.sync();
    ind.processQueue();

    TS_ASSERT(recv.hasResult());

    MarkerCollector coll;
    recv.replay(coll);
    TS_ASSERT_EQUALS(coll.getColors(), MarkerCollector::Colors_t() + 1 + 2);
}

/** Test tag filter.
    A: create session with some markers. Create and configure MapRendererProxy. Enable/disable tag filter.
    E: callback generated with correct content */
void
TestGameProxyMapRendererProxy::testTagFilter()
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    addMarker(h, 1010, 1010, 1, 1);
    addMarker(h, 1020, 1030, 2, 2);
    MapRendererProxy testee(h.gameSender(), ind);
    ResultReceiver recv(testee);

    // Enable filter
    testee.setRange(Point(1000, 1000), Point(2000, 2000));
    testee.setDrawingTagFilter(1);
    h.sync();
    ind.processQueue();

    // Verify filter active
    {
        TS_ASSERT(recv.hasResult());
        MarkerCollector coll;
        recv.replay(coll);
        TS_ASSERT_EQUALS(coll.getColors(), MarkerCollector::Colors_t() + 1);
    }

    // Disable filter
    testee.clearDrawingTagFilter();
    h.sync();
    ind.processQueue();

    // Verify filter inactive
    {
        TS_ASSERT(recv.hasResult());
        MarkerCollector coll;
        recv.replay(coll);
        TS_ASSERT_EQUALS(coll.getColors(), MarkerCollector::Colors_t() + 1 + 2);
    }
}

/** Test toggleOptions().
    A: create session with some markers. Create and configure MapRendererProxy. Enable/disable drawing display using toggleOptions.
    E: callback generated with correct content */
void
TestGameProxyMapRendererProxy::testToggleOptions()
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    addMarker(h, 1010, 1010, 7, 1);
    MapRendererProxy testee(h.gameSender(), ind);
    ResultReceiver recv(testee);

    // Toggle
    testee.setRange(Point(1000, 1000), Point(2000, 2000));
    testee.toggleOptions(game::map::RenderOptions::Options_t(game::map::RenderOptions::ShowDrawings));
    h.sync();
    ind.processQueue();

    // Verify drawings disabled
    {
        TS_ASSERT(recv.hasResult());
        MarkerCollector coll;
        recv.replay(coll);
        TS_ASSERT_EQUALS(coll.getColors(), MarkerCollector::Colors_t());
    }

    // Toggle again
    testee.toggleOptions(game::map::RenderOptions::Options_t(game::map::RenderOptions::ShowDrawings));
    h.sync();
    ind.processQueue();

    // Verify filter inactive
    {
        TS_ASSERT(recv.hasResult());
        MarkerCollector coll;
        recv.replay(coll);
        TS_ASSERT_EQUALS(coll.getColors(), MarkerCollector::Colors_t() + 7);
    }
}

/** Test setConfiguration().
    A: create session with some markers, display disabled in default. Create and configure MapRendererProxy. Enable drawing using different config.
    E: callback generated with correct content */
void
TestGameProxyMapRendererProxy::testSetConfiguration()
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    addMarker(h, 1010, 1010, 7, 1);
    game::config::UserConfiguration& pref = h.session().getRoot()->userConfiguration();
    pref.setOption("Chart.Normal.Show", "ion", game::config::ConfigurationOption::Game);
    pref.setOption("Chart.Small.Show", "drawings", game::config::ConfigurationOption::Game);

    MapRendererProxy testee(h.gameSender(), ind);
    ResultReceiver recv(testee);

    // Render with default config
    testee.setRange(Point(1000, 1000), Point(2000, 2000));
    h.sync();
    ind.processQueue();

    // Verify drawings hidden
    {
        TS_ASSERT(recv.hasResult());
        MarkerCollector coll;
        recv.replay(coll);
        TS_ASSERT_EQUALS(coll.getColors(), MarkerCollector::Colors_t());
    }

    // Switch to small mode
    testee.setConfiguration(game::map::RenderOptions::Small);
    h.sync();
    ind.processQueue();

    // Verify drawings shown
    {
        TS_ASSERT(recv.hasResult());
        MarkerCollector coll;
        recv.replay(coll);
        TS_ASSERT_EQUALS(coll.getColors(), MarkerCollector::Colors_t() + 7);
    }
}

/** Test preferences modification.
    A: create session with some markers. Create and configure MapRendererProxy. Modify configuration outside the MapRendererProxy.
    E: callback generated with correct content */
void
TestGameProxyMapRendererProxy::testModifyPreferences()
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    addMarker(h, 1010, 1010, 7, 1);
    MapRendererProxy testee(h.gameSender(), ind);
    ResultReceiver recv(testee);
    ConfigReceiver cfg;
    testee.sig_configuration.add(&cfg, &ConfigReceiver::onConfiguration);

    // Toggle
    testee.setRange(Point(1000, 1000), Point(2000, 2000));
    testee.toggleOptions(game::map::RenderOptions::Options_t(game::map::RenderOptions::ShowDrawings));
    h.sync();
    ind.processQueue();

    // Verify drawings disabled
    {
        TS_ASSERT(recv.hasResult());
        MarkerCollector coll;
        recv.replay(coll);
        TS_ASSERT_EQUALS(coll.getColors(), MarkerCollector::Colors_t());
        TS_ASSERT_EQUALS(cfg.hasResult(), true);
        TS_ASSERT_EQUALS(cfg.get().getOption(game::map::RenderOptions::ShowDrawings), game::map::RenderOptions::Disabled);
    }

    // Enable by modifying preferences
    class Task : public util::Request<game::Session> {
     public:
        virtual void handle(game::Session& s)
            {
                game::config::UserConfiguration& pref = s.getRoot()->userConfiguration();
                pref.setOption("Chart.Normal.Show", "drawings", game::config::ConfigurationOption::Game);
                pref.notifyListeners();
            }
    };
    h.gameSender().postNewRequest(new Task());
    h.sync();
    ind.processQueue();

    // Verify filter inactive
    {
        TS_ASSERT(recv.hasResult());
        MarkerCollector coll;
        recv.replay(coll);
        TS_ASSERT_EQUALS(coll.getColors(), MarkerCollector::Colors_t() + 7);
        TS_ASSERT_EQUALS(cfg.get().getOption(game::map::RenderOptions::ShowDrawings), game::map::RenderOptions::Enabled);
    }
}

