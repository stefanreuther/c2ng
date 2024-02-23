/**
  *  \file test/game/proxy/drawingproxytest.cpp
  *  \brief Test for game::proxy::DrawingProxy
  */

#include "game/proxy/drawingproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/drawingcontainer.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

using game::test::SessionThread;
using game::test::WaitIndicator;
using game::proxy::DrawingProxy;
using game::map::Point;
using game::map::Drawing;
using game::map::DrawingContainer;

namespace {
    class StatusReceiver {
     public:
        StatusReceiver(DrawingProxy& proxy)
            : conn_update(proxy.sig_update.add(this, &StatusReceiver::onUpdate)),
              m_status()
            { }

        const DrawingProxy::Status_t& get() const
            { return m_status; }

     private:
        void onUpdate(const DrawingProxy::Status_t& st)
            { m_status = st; }

        afl::base::SignalConnection conn_update;
        DrawingProxy::Status_t m_status;
    };

    void prepare(SessionThread& h)
    {
        afl::base::Ptr<game::Game> g = new game::Game();
        g->currentTurn().setLocalDataPlayers(game::PlayerSet_t(1));
        h.session().setGame(g);
    }

    void addMarker(SessionThread& h, int x, int y, uint8_t color)
    {
        Drawing* d = new Drawing(game::map::Point(x, y), Drawing::MarkerDrawing);
        d->setColor(color);
        h.session().getGame()->currentTurn().universe().drawings().addNew(d);
    }

    void addLine(SessionThread& h, int x, int y, int x2, int y2, uint8_t color, util::Atom_t tag)
    {
        Drawing* d = new Drawing(game::map::Point(x, y), Drawing::LineDrawing);
        d->setColor(color);
        d->setTag(tag);
        d->setPos2(Point(x2, y2));
        h.session().getGame()->currentTurn().universe().drawings().addNew(d);
    }
}

/** Test behaviour on empty session.
    A: create empty session. Create DrawingProxy. Call some methods. Query status.
    E: no crash. Status reports empty. */
AFL_TEST("game.proxy.DrawingProxy:empty", a)
{
    SessionThread h;
    WaitIndicator ind;
    DrawingProxy testee(h.gameSender(), ind);

    // Some dummy calls
    AFL_CHECK_SUCCEEDS(a("01. create"), testee.create(Point(2000, 2000), Drawing::MarkerDrawing));
    AFL_CHECK_SUCCEEDS(a("02. setPos"), testee.setPos(Point(2000, 2020)));

    // Querying should yield nothing
    DrawingProxy::Status_t st;
    AFL_CHECK_SUCCEEDS(a("11. setStatus"), testee.getStatus(ind, st));
    a.check("12. isValid", !st.isValid());
}

/** Test creating a marker.
    A: create session with turn. Create DrawingProxy. Create and populate a marker.
    E: marker created; correct status reported. Verify all stages. */
AFL_TEST("game.proxy.DrawingProxy:create:MarkerDrawing", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    DrawingProxy testee(h.gameSender(), ind);
    StatusReceiver recv(testee);

    // Create and configure a marker
    testee.create(Point(2100, 2200), Drawing::MarkerDrawing);
    testee.setMarkerKind(3);
    testee.setColor(4, false);
    testee.setComment("hi");
    testee.setTag(7777, false);
    testee.setPos(Point(2200, 2300));

    // Verify that correct update is eventually received
    h.sync();
    ind.processQueue();
    a.check("01. isValid",            recv.get().isValid());
    a.checkEqual("02. getType",       recv.get().get()->getType(), Drawing::MarkerDrawing);
    a.checkEqual("03. getPos",        recv.get().get()->getPos(), Point(2200, 2300));
    a.checkEqual("04. getColor",      recv.get().get()->getColor(), 4);
    a.checkEqual("05. getMarkerKind", recv.get().get()->getMarkerKind(), 3);
    a.checkEqual("06. getComment",    recv.get().get()->getComment(), "hi");
    a.checkEqual("07. getTag",        recv.get().get()->getTag(), 7777U);

    // Explicitly query
    DrawingProxy::Status_t st;
    AFL_CHECK_SUCCEEDS(a("11. getStatus"), testee.getStatus(ind, st));
    a.check("12. isValid",            st.isValid());
    a.checkEqual("13. getType",       st.get()->getType(), Drawing::MarkerDrawing);
    a.checkEqual("14. getPos",        st.get()->getPos(), Point(2200, 2300));
    a.checkEqual("15. getColor",      st.get()->getColor(), 4);
    a.checkEqual("16. getMarkerKind", st.get()->getMarkerKind(), 3);
    a.checkEqual("17. getComment",    st.get()->getComment(), "hi");
    a.checkEqual("18. getTag",        st.get()->getTag(), 7777U);

    // Verify that marker is present
    DrawingContainer::Iterator_t it = h.session().getGame()->currentTurn().universe().drawings().begin();
    a.check("21. first drawing", *it);
    a.checkEqual("22. getType",       (*it)->getType(), Drawing::MarkerDrawing);
    a.checkEqual("23. getPos",        (*it)->getPos(), Point(2200, 2300));
    a.checkEqual("24. getColor",      (*it)->getColor(), 4);
    a.checkEqual("25. getMarkerKind", (*it)->getMarkerKind(), 3);
    a.checkEqual("26. getComment",    (*it)->getComment(), "hi");
    a.checkEqual("27. getTag",        (*it)->getTag(), 7777U);

    // Finish; verify that report is invalidated
    testee.finish();
    h.sync();
    ind.processQueue();
    a.check("31. isValid", !recv.get().isValid());
}

/** Test creating a marker, not-editable case.
    A: create session with turn. Create DrawingProxy. Create and populate a marker.
    E: marker created; correct status reported. Verify all stages. */
AFL_TEST("game.proxy.DrawingProxy:create:MarkerDrawing:not-editable", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    h.session().getGame()->currentTurn().setLocalDataPlayers(game::PlayerSet_t());
    DrawingProxy testee(h.gameSender(), ind);
    StatusReceiver recv(testee);

    // Create a marker
    testee.create(Point(2100, 2200), Drawing::MarkerDrawing);

    // Verify that correct update is eventually received
    h.sync();
    ind.processQueue();
    a.check("isValid", !recv.get().isValid());
}

/** Test creating lines.
    A: create session with turn. Create DrawingProxy. Create multiple lines.
    E: verify result. */
AFL_TEST("game.proxy.DrawingProxy:create:LineDrawing", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    DrawingProxy testee(h.gameSender(), ind);

    // Create some lines
    //   (2000,2000) -> (2000,2010), color 4
    testee.create(Point(2000, 2000), Drawing::LineDrawing);
    testee.setColor(4, false);
    testee.setPos2(Point(2000, 2010));
    //   (2000,2010) -> (2000,2020), color 4
    testee.continueLine();
    testee.setPos2(Point(2000, 2020));
    //   (2000,2020) -> (2000,2030), color 5
    testee.continueLine();
    testee.setColor(5, false);
    testee.setPos2(Point(2000, 2030));
    //   (2000,2030) -> (2000,2030), color 5 [ignored segment]
    testee.continueLine();
    //   (2000,2030) -> (2000,2040), color 5
    testee.continueLine();
    testee.setPos2(Point(2000, 2040));

    // Verify
    h.sync();
    ind.processQueue();

    DrawingContainer::Iterator_t it = h.session().getGame()->currentTurn().universe().drawings().begin();
    a.check("01. it", *it);
    a.checkEqual("02. getPos",   (**it).getPos(),   Point(2000, 2000));
    a.checkEqual("03. getPos2",  (**it).getPos2(),  Point(2000, 2010));
    a.checkEqual("04. getColor", (**it).getColor(), 4);

    ++it;
    a.check("11. it", *it);
    a.checkEqual("12. getPos",   (**it).getPos(),   Point(2000, 2010));
    a.checkEqual("13. getPos2",  (**it).getPos2(),  Point(2000, 2020));
    a.checkEqual("14. getColor", (**it).getColor(), 4);

    ++it;
    a.check("21. it", *it);
    a.checkEqual("22. getPos",   (**it).getPos(),   Point(2000, 2020));
    a.checkEqual("23. getPos2",  (**it).getPos2(),  Point(2000, 2030));
    a.checkEqual("24. getColor", (**it).getColor(), 5);

    ++it;
    a.check("31. it", *it);
    a.checkEqual("32. getPos",   (**it).getPos(),   Point(2000, 2030));
    a.checkEqual("33. getPos2",  (**it).getPos2(),  Point(2000, 2040));
    a.checkEqual("34. getColor", (**it).getColor(), 5);

    ++it;
    a.check("41. end", !*it);
}

/** Test creating rectangels.
    A: create session with turn. Create DrawingProxy. Create a rectangle.
    E: verify result. */
AFL_TEST("game.proxy.DrawingProxy:create:RectangleDrawing", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    DrawingProxy testee(h.gameSender(), ind);

    // Create and verify initial state
    testee.create(Point(1500, 1600), Drawing::RectangleDrawing);
    DrawingProxy::Status_t st;
    AFL_CHECK_SUCCEEDS(a("01. getStatus"), testee.getStatus(ind, st));
    a.check("02. isValid", st.isValid());
    a.checkEqual("03. getPos", st.get()->getPos(), Point(1500, 1600));
    a.checkEqual("04. getPos2", st.get()->getPos2(), Point(1500, 1600));

    // Finish it
    testee.setPos2(Point(1700, 1800));
    testee.finish();

    // Verify
    h.sync();
    ind.processQueue();

    DrawingContainer::Iterator_t it = h.session().getGame()->currentTurn().universe().drawings().begin();
    a.check("11. it", *it);
    a.checkEqual("12. getPos",  (**it).getPos(),   Point(1500, 1600));
    a.checkEqual("13. getPos2", (**it).getPos2(),  Point(1700, 1800));
    a.checkEqual("14. getType", (**it).getType(),  Drawing::RectangleDrawing);
}

/** Test creating circles.
    A: create session with turn. Create DrawingProxy. Create a circle.
    E: verify result. */
AFL_TEST("game.proxy.DrawingProxy:create:CircleDrawing", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    DrawingProxy testee(h.gameSender(), ind);

    // Create
    testee.create(Point(1300, 1400), Drawing::CircleDrawing);
    testee.setCircleRadius(20);
    testee.changeCircleRadius(50);
    testee.finish();

    // Verify
    h.sync();
    ind.processQueue();

    DrawingContainer::Iterator_t it = h.session().getGame()->currentTurn().universe().drawings().begin();
    a.check("01. it", *it);
    a.checkEqual("02. getPos",          (**it).getPos(),          Point(1300, 1400));
    a.checkEqual("03. getCircleRadius", (**it).getCircleRadius(), 70);
    a.checkEqual("04. getType",         (**it).getType(),         Drawing::CircleDrawing);
}

/** Test selectNearestVisibleDrawing().
    A: create session with turn and some drawings. Create DrawingProxy. Call selectNearestVisibleDrawing().
    E: verify correct drawing is selected. */
AFL_TEST("game.proxy.DrawingProxy:selectNearestVisibleDrawing", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    addMarker(h, 1000, 1000, 1);
    addMarker(h, 1000, 1010, 2);
    addMarker(h, 1000, 1020, 3);
    addMarker(h, 1000, 1030, 4);

    DrawingProxy testee(h.gameSender(), ind);
    StatusReceiver recv(testee);

    // Select marker, unsuccessfully
    testee.selectNearestVisibleDrawing(Point(2000, 2000), 10, afl::base::Nothing);
    h.sync();
    ind.processQueue();
    a.check("01. isValid", !recv.get().isValid());

    // Select marker, successfully
    testee.selectNearestVisibleDrawing(Point(1000, 1011), 10, afl::base::Nothing);
    h.sync();
    ind.processQueue();
    a.check("11. isValid", recv.get().isValid());
    a.checkEqual("12. getColor", recv.get().get()->getColor(), 2);

    // Select again, unsuccessfully. This does not change anything.
    testee.selectNearestVisibleDrawing(Point(2000, 2000), 10, afl::base::Nothing);
    h.sync();
    ind.processQueue();
    a.check("21. isValid", recv.get().isValid());
    a.checkEqual("22. getColor", recv.get().get()->getColor(), 2);

    // Select again, successfully
    testee.selectNearestVisibleDrawing(Point(1000, 1019), 10, afl::base::Nothing);
    h.sync();
    ind.processQueue();
    a.check("31. isValid", recv.get().isValid());
    a.checkEqual("32. getColor", recv.get().get()->getColor(), 3);
}

/** Test erase().
    A: create session with turn and some drawings. Create DrawingProxy. Call selectNearestVisibleDrawing(), then erase().
    E: verify correct result. */
AFL_TEST("game.proxy.DrawingProxy:erase", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    addMarker(h, 1000, 1000, 1);
    addMarker(h, 1000, 1010, 2);
    addMarker(h, 1000, 1020, 3);

    DrawingProxy testee(h.gameSender(), ind);
    StatusReceiver recv(testee);

    // Select and erase
    testee.selectNearestVisibleDrawing(Point(1000, 1011), 10, afl::base::Nothing);
    testee.erase(false);

    // Verify
    h.sync();
    ind.processQueue();
    a.check("01. isValud", !recv.get().isValid());

    DrawingContainer::Iterator_t it = h.session().getGame()->currentTurn().universe().drawings().begin();
    a.check("11. it", *it);
    a.checkEqual("12. getColor", (**it).getColor(), 1);
    ++it;
    a.check("13. it", *it);
    a.checkEqual("14. getColor", (**it).getColor(), 3);
    ++it;
    a.check("15. end", !*it);
}

/** Test setColor() for adjacent lines.
    A: create session with turn and some lines. Create DrawingProxy. Call selectNearestVisibleDrawing(). Call setColor(adjacent=true).
    E: verify correct result. */
AFL_TEST("game.proxy.DrawingProxy:setColor:adjacent", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    addLine(h, 1000, 1000, 1000, 1010, 4, 500);
    addLine(h, 1000, 1010, 1000, 1020, 6, 501);
    addLine(h, 1000, 1020, 1000, 1030, 7, 502);

    // Action
    DrawingProxy testee(h.gameSender(), ind);
    testee.selectNearestVisibleDrawing(Point(1005, 1015), 10, afl::base::Nothing);
    testee.setColor(9, true);

    // Verify
    h.sync();
    ind.processQueue();
    DrawingContainer::Iterator_t it = h.session().getGame()->currentTurn().universe().drawings().begin();
    for (int i = 0; i < 3; ++i) {
        a.check("01. it", *it);
        a.checkEqual("02. getColor", (**it).getColor(), 9);
        ++it;
    }
    a.check("03. end", !*it);
}

/** Test setTag() for adjacent lines.
    A: create session with turn and some lines. Create DrawingProxy. Call selectNearestVisibleDrawing(). Call setTag(adjacent=true).
    E: verify correct result. */
AFL_TEST("game.proxy.DrawingProxy:setTag:adjacent", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    addLine(h, 1000, 1000, 1000, 1010, 4, 500);
    addLine(h, 1000, 1010, 1000, 1020, 6, 501);
    addLine(h, 1000, 1020, 1000, 1030, 7, 502);

    // Action
    DrawingProxy testee(h.gameSender(), ind);
    testee.selectNearestVisibleDrawing(Point(1005, 1015), 10, afl::base::Nothing);
    testee.setTag(600, true);

    // Verify
    h.sync();
    ind.processQueue();
    DrawingContainer::Iterator_t it = h.session().getGame()->currentTurn().universe().drawings().begin();
    for (int i = 0; i < 3; ++i) {
        a.check("01. it", *it);
        a.checkEqual("02. getTag", (**it).getTag(), 600U);
        ++it;
    }
    a.check("03. it", !*it);
}

/** Test erase() for adjacent lines.
    A: create session with turn and some lines. Create DrawingProxy. Call selectNearestVisibleDrawing(). Call erase(adjacent=true).
    E: verify correct result. */
AFL_TEST("game.proxy.DrawingProxy:erase:adjacent", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    addLine(h, 1000, 1000, 1000, 1010, 4, 500);
    addLine(h, 1000, 1010, 1000, 1020, 6, 501);
    addLine(h, 1000, 1020, 1000, 1030, 7, 502);

    // Action
    DrawingProxy testee(h.gameSender(), ind);
    testee.selectNearestVisibleDrawing(Point(1005, 1015), 10, afl::base::Nothing);
    testee.erase(true);

    // Verify
    h.sync();
    ind.processQueue();
    DrawingContainer::Iterator_t it = h.session().getGame()->currentTurn().universe().drawings().begin();
    a.check("01. empty", !*it);
}

/** Test parallel usage.
    A: create a session. Create two DrawingProxy instances and observe both. Create a drawing with one, erase it with the other.
    E: verify correct status updates. */
AFL_TEST("game.proxy.DrawingProxy:parallel-usage", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    DrawingProxy p1(h.gameSender(), ind);
    DrawingProxy p2(h.gameSender(), ind);
    StatusReceiver r1(p1);
    StatusReceiver r2(p2);

    // Create a circle and verify
    p1.create(Point(1500, 1500), Drawing::CircleDrawing);
    p1.setCircleRadius(200);
    h.sync();
    ind.processQueue();

    a.check("01. isValid", r1.get().isValid());
    a.checkEqual("02. getCircleRadius", r1.get().get()->getCircleRadius(), 200);
    a.check("03. isValid", !r2.get().isValid());

    // Select circle with second instance
    p2.selectNearestVisibleDrawing(Point(1700, 1500), 10, afl::base::Nothing);
    h.sync();
    ind.processQueue();

    a.check("11. isValid", r1.get().isValid());
    a.checkEqual("12. getCircleRadius", r1.get().get()->getCircleRadius(), 200);
    a.check("13. isValid", r2.get().isValid());
    a.checkEqual("14. getCircleRadius", r2.get().get()->getCircleRadius(), 200);

    // Modify tag with second instance
    p2.setTag(7777, false);
    h.sync();
    ind.processQueue();

    a.check("21. isValid", r1.get().isValid());
    a.checkEqual("22. getTag", r1.get().get()->getTag(), 7777U);
    a.check("23. isValid", r2.get().isValid());
    a.checkEqual("24. getTag", r2.get().get()->getTag(), 7777U);

    // Erase with second instance
    p2.erase(false);
    h.sync();
    ind.processQueue();

    a.check("31. isValid", !r1.get().isValid());
    a.check("32. isValid", !r2.get().isValid());
}

/** Test selectMarkerAt().
    A: create session with turn and some markers. Create DrawingProxy. Call selectMarkerAt().
    E: verify correct drawing is selected. */
AFL_TEST("game.proxy.DrawingProxy:selectMarkerAt", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    addMarker(h, 1000, 1000, 1);
    addMarker(h, 1000, 1010, 2);

    DrawingProxy testee(h.gameSender(), ind);
    StatusReceiver recv(testee);

    // Select marker, unsuccessfully
    testee.selectMarkerAt(Point(2000, 2000), afl::base::Nothing);
    h.sync();
    ind.processQueue();
    a.check("01. isValid", !recv.get().isValid());

    // Select marker, successfully
    testee.selectMarkerAt(Point(1000, 1000), afl::base::Nothing);
    h.sync();
    ind.processQueue();
    a.check("11. isValid", recv.get().isValid());
    a.checkEqual("12. getColor", recv.get().get()->getColor(), 1);

    // Select other marker, successfully
    testee.selectMarkerAt(Point(1000, 1010), afl::base::Nothing);
    h.sync();
    ind.processQueue();
    a.check("21. isValid", recv.get().isValid());
    a.checkEqual("22. getColor", recv.get().get()->getColor(), 2);
}

/** Test setTagName.
    A: create session with turn and a marker. Create DrawingProxy. Call setTagName() with different parameters.
    E: verify correct values set */
AFL_TEST("game.proxy.DrawingProxy:setTagName", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    addMarker(h, 1000, 1000, 1);

    // Create and set number
    DrawingProxy testee(h.gameSender(), ind);
    testee.selectMarkerAt(Point(1000, 1000), afl::base::Nothing);
    testee.setTagName("17", false);
    h.sync();
    ind.processQueue();

    // Verify
    DrawingContainer::Iterator_t it = h.session().getGame()->currentTurn().universe().drawings().begin();
    a.check("01. it", *it);
    a.checkEqual("02. getTag", (**it).getTag(), 17U);

    // Try a string
    testee.setTagName("foo", false);
    h.sync();
    ind.processQueue();
    a.check("11. it", *it);
    a.checkDifferent("12. getTag", (**it).getTag(), 0U);
    a.checkEqual("13. getStringFromAtom", h.session().world().atomTable().getStringFromAtom((**it).getTag()), "foo");

    // Try empty
    testee.setTagName("", false);
    h.sync();
    ind.processQueue();
    a.check("21. it", *it);
    a.checkEqual("22. getTag", (**it).getTag(), 0U);
}

/** Test packTagList.
    A: create session with turn and a some markers. Create DrawingProxy. Call getTagList().
    E: verify result */
AFL_TEST("game.proxy.DrawingProxy:getTagList", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    util::AtomTable& tab = h.session().world().atomTable();
    addLine(h, 1000, 1000, 1010, 1010, 7, 1);
    addLine(h, 1000, 1000, 1010, 1010, 7, 20);
    addLine(h, 1000, 1000, 1010, 1010, 7, tab.getAtomFromString("hi"));
    addLine(h, 1000, 1000, 1010, 1010, 7, tab.getAtomFromString("hi"));
    addLine(h, 1000, 1000, 1010, 1010, 7, tab.getAtomFromString("what"));
    addLine(h, 1000, 1000, 1010, 1010, 7, 3);

    // Retrieve result
    DrawingProxy testee(h.gameSender(), ind);
    util::StringList list;
    testee.getTagList(ind, list);

    // Verify
    list.sortAlphabetically();
    a.checkEqual("01. size", list.size(), 5U);

    int32_t k;
    String_t s;
    a.check("11. get 0", list.get(0, k, s));
    a.checkEqual("12. k", k, 1);
    a.checkEqual("13. s", s, "1");

    a.check("21. get 1", list.get(1, k, s));
    a.checkEqual("22. k", k, 20);
    a.checkEqual("23. s", s, "20");

    a.check("31. get 2", list.get(2, k, s));
    a.checkEqual("32. k", k, 3);
    a.checkEqual("33. s", s, "3");

    a.check("41. get 3", list.get(3, k, s));
    a.checkEqual("42. s", s, "hi");

    a.check("51. get 4", list.get(4, k, s));
    a.checkEqual("52. s", s, "what");
}

/** Test packTagList on empty universe.
    A: create session. Create DrawingProxy. Call getTagList().
    E: result must be empty */
AFL_TEST("game.proxy.DrawingProxy:getTagList:empty", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    DrawingProxy testee(h.gameSender(), ind);
    util::StringList list;
    testee.getTagList(ind, list);

    a.checkEqual("01. size", list.size(), 0U);
}

/** Test creating a canned marker.
    A: create session with turn and root. Create DrawingProxy. Create and populate a canned marker.
    E: marker created; correct status reported. */
AFL_TEST("game.proxy.DrawingProxy:createCannedMarker", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Need a root for this test for configuration.
    // Hardwire the configuration here to be independant from changing defaults.
    afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion()).asPtr();
    r->userConfiguration().setOption("chart.marker4", "5,7,x", game::config::ConfigurationOption::User);
    h.session().setRoot(r);

    DrawingProxy testee(h.gameSender(), ind);
    StatusReceiver recv(testee);

    // Create and configure a marker
    testee.createCannedMarker(Point(1111, 2222), 4);

    // Verify that correct update is eventually received
    h.sync();
    ind.processQueue();
    a.check("01. isValid",            recv.get().isValid());
    a.checkEqual("02. getType",       recv.get().get()->getType(), Drawing::MarkerDrawing);
    a.checkEqual("03. getPos",        recv.get().get()->getPos(), Point(1111, 2222));
    a.checkEqual("04. getColor",      recv.get().get()->getColor(), 7);
    a.checkEqual("05. getMarkerKind", recv.get().get()->getMarkerKind(), 5);

    // Verify that marker is present
    DrawingContainer::Iterator_t it = h.session().getGame()->currentTurn().universe().drawings().begin();
    a.check("11. it", *it);
    a.checkEqual("12. getType",       (*it)->getType(), Drawing::MarkerDrawing);
    a.checkEqual("13. getPos",        (*it)->getPos(), Point(1111, 2222));
    a.checkEqual("14. getColor",      (*it)->getColor(), 7);
    a.checkEqual("15. getMarkerKind", (*it)->getMarkerKind(), 5);
}

/** Test queueing.
    A: create session with turn. Create DrawingProxy. Create a circle. Call setCircleRadius repeatedly.
    E: verify result: circle eventually settles at the last radius given. */
AFL_TEST("game.proxy.DrawingProxy:queueing", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    DrawingProxy testee(h.gameSender(), ind);

    // Create
    testee.create(Point(1300, 1400), Drawing::CircleDrawing);
    for (int i = 1; i < 20; ++i) {
        testee.setCircleRadius(10*i);
    }

    // Verify
    for (int i = 1; i < 20; ++i) {
        h.sync();
        ind.processQueue();
    }

    DrawingContainer::Iterator_t it = h.session().getGame()->currentTurn().universe().drawings().begin();
    a.check("01. it", *it);
    a.checkEqual("02. getPos",          (**it).getPos(),          Point(1300, 1400));
    a.checkEqual("03. getCircleRadius", (**it).getCircleRadius(), 190);
    a.checkEqual("04. getType",         (**it).getType(),         Drawing::CircleDrawing);
}

/** Test lifetime behaviour.
    A: create session and DrawingProxy. Create a circle. Clear session.
    E: verify result. */
AFL_TEST_NOARG("game.proxy.DrawingProxy:create:lifecycle")
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);
    DrawingProxy testee(h.gameSender(), ind);

    // Create
    testee.create(Point(1300, 1400), Drawing::CircleDrawing);
    h.sync();
    ind.processQueue();

    // Clear session
    h.session().setGame(0);
    h.session().setShipList(0);
    h.session().setRoot(0);

    // Continue operating. Must not crash.
    testee.setCircleRadius(20);
    testee.changeCircleRadius(50);
    testee.finish();
    h.sync();
    ind.processQueue();
}
