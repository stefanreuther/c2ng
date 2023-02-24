/**
  *  \file u/t_game_proxy_taskeditorproxy.cpp
  *  \brief Test for game::proxy::TaskEditorProxy
  */

#include "game/proxy/taskeditorproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "util/simplerequestdispatcher.hpp"

using afl::base::Ptr;
using game::map::Point;
using game::proxy::TaskEditorProxy;
using game::test::SessionThread;
using interpreter::BCORef_t;
using interpreter::BytecodeObject;
using interpreter::Opcode;
using interpreter::Process;
using interpreter::TaskEditor;
using util::SimpleRequestDispatcher;

namespace {
    void prepare(SessionThread& s)
    {
        // Objects
        s.session().setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,2,0))).asPtr());
        s.session().setGame(new game::Game());
        s.session().setShipList(new game::spec::ShipList());
        game::test::addOutrider(*s.session().getShipList());
        game::test::addTranswarp(*s.session().getShipList());
        s.session().getShipList()->hullAssignments().add(1, 1, game::test::OUTRIDER_HULL_ID);

        // We need a CC$AUTOEXEC procedure
        BCORef_t bco = BytecodeObject::create(true);
        bco->addArgument("A", false);
        bco->addInstruction(Opcode::maPush, Opcode::sLocal, 0);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEvalStatement, 1);
        s.session().world().setNewGlobalValue("CC$AUTOEXEC", new interpreter::SubroutineValue(bco));
    }

    void addShip(SessionThread& s, int id, Point pos)
    {
        game::map::ShipData data;
        data.owner = 1;
        data.x = pos.getX();
        data.y = pos.getY();
        data.engineType = game::test::TRANSWARP_ENGINE_ID;
        data.hullType = game::test::OUTRIDER_HULL_ID;
        data.neutronium = 100;

        game::map::Ship* sh = s.session().getGame()->currentTurn().universe().ships().create(id);
        sh->addCurrentShipData(data, game::PlayerSet_t(1));  // needed to enable ship prediction
        sh->internalCheck(game::PlayerSet_t(1), 15);
    }

    void addBase(SessionThread& s, int id, Point pos)
    {
        game::map::Planet* pl = s.session().getGame()->currentTurn().universe().planets().create(id);
        pl->setPosition(pos);
        pl->setName("Giedi Prime");

        game::map::PlanetData data;
        data.owner = 1;
        data.money = 100;
        data.supplies = 100;
        data.minedTritanium = 1000;
        data.minedDuranium = 1000;
        data.minedMolybdenum = 1000;
        data.minedNeutronium = 1000;
        data.colonistClans = 10;
        data.colonistHappiness = 100;
        data.temperature = 50;
        pl->addCurrentPlanetData(data, game::PlayerSet_t(1));

        game::map::BaseData base;
        for (size_t i = 0; i < game::NUM_TECH_AREAS; ++i) {
            base.techLevels[i] = 1;
        }
        pl->addCurrentBaseData(base, game::PlayerSet_t(1));
        pl->internalCheck(s.session().getGame()->mapConfiguration(),
                          game::PlayerSet_t(1),
                          s.session().getGame()->currentTurn().getTurnNumber(),
                          s.session().translator(),
                          s.session().log());
        pl->setPlayability(game::map::Object::Playable);
    }

    template<typename T>
    struct StatusReceiver {
        T status;
        bool ok;

        StatusReceiver()
            : status(), ok(false)
            { }

        void onChange(const T& st)
            { this->status = st; this->ok = true; }
    };
}

/** Test empty session.
    A: make empty session.
    E: status correctly reported as not valid */
void
TestGameProxyTaskEditorProxy::testEmpty()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    /* FIXME: this crashes when the declarations of disp and s are swapped - why? */
    SimpleRequestDispatcher disp;
    SessionThread s;
    TaskEditorProxy testee(s.gameSender(), disp);

    StatusReceiver<TaskEditorProxy::Status> recv;
    testee.sig_change.add(&recv, &StatusReceiver<TaskEditorProxy::Status>::onChange);

    // Wait for status update
    testee.selectTask(99, Process::pkShipTask, true);
    while (!recv.ok) {
        TS_ASSERT(disp.wait(1000));
    }

    TS_ASSERT(!recv.status.valid);
}

/** Test non-empty session.
    A: make session containing a ship and a ship task.
    E: status correctly reported */
void
TestGameProxyTaskEditorProxy::testNormal()
{
    const int SHIP_ID = 43;

    // Environment
    CxxTest::setAbortTestOnFail(true);
    SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);
    addShip(s, SHIP_ID, Point(1000,1000));

    // Add a task
    {
        Ptr<TaskEditor> ed = s.session().getAutoTaskEditor(SHIP_ID, Process::pkShipTask, true);
        TS_ASSERT(ed.get());

        // releaseAutoTaskEditor will run the task, so the first command needs to be 'stop'
        String_t code[] = { "stop", "hammer", "time" };
        ed->replace(0, 0, code, TaskEditor::DefaultCursor, TaskEditor::PlacePCBefore);

        s.session().releaseAutoTaskEditor(ed);
    }

    // Testee
    TaskEditorProxy testee(s.gameSender(), disp);

    StatusReceiver<TaskEditorProxy::Status> recv;
    testee.sig_change.add(&recv, &StatusReceiver<TaskEditorProxy::Status>::onChange);

    // Wait for status update
    testee.selectTask(SHIP_ID, Process::pkShipTask, true);
    while (!recv.ok) {
        TS_ASSERT(disp.wait(1000));
    }

    TS_ASSERT(recv.status.valid);
    TS_ASSERT_EQUALS(recv.status.commands.size(), 3U);
    TS_ASSERT_EQUALS(recv.status.commands[0], "stop");
    TS_ASSERT_EQUALS(recv.status.pc, 0U);
    TS_ASSERT_EQUALS(recv.status.cursor, 3U);
    TS_ASSERT_EQUALS(recv.status.isInSubroutineCall, true);

    // Move the cursor
    recv.ok = false;
    testee.setCursor(1);
    while (!recv.ok) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT(recv.status.valid);
    TS_ASSERT_EQUALS(recv.status.cursor, 1U);
}

/** Test ship status reporting. */
void
TestGameProxyTaskEditorProxy::testShipStatus()
{
    const int SHIP_ID = 43;

    // Environment
    CxxTest::setAbortTestOnFail(true);
    SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);
    addShip(s, SHIP_ID, Point(1000,1000));

    game::map::Point pt(333,333);
    s.session().getGame()->currentTurn().universe().ships().get(SHIP_ID)->getPosition().get(pt);

    // Add a task
    {
        Ptr<TaskEditor> ed = s.session().getAutoTaskEditor(SHIP_ID, Process::pkShipTask, true);
        TS_ASSERT(ed.get());

        // releaseAutoTaskEditor will run the task, so the first command needs to be 'stop'. Following commands will be predicted.
        String_t code[] = { "stop", "setspeed 6", "moveto 1000, 1050" };
        ed->replace(0, 0, code, TaskEditor::DefaultCursor, TaskEditor::PlacePCBefore);

        s.session().releaseAutoTaskEditor(ed);
    }

    // Testee
    TaskEditorProxy testee(s.gameSender(), disp);

    StatusReceiver<TaskEditorProxy::ShipStatus> recv;
    testee.sig_shipChange.add(&recv, &StatusReceiver<TaskEditorProxy::ShipStatus>::onChange);

    // Wait for status update
    testee.selectTask(SHIP_ID, Process::pkShipTask, true);
    while (!recv.ok) {
        TS_ASSERT(disp.wait(1000));
    }

    TS_ASSERT(recv.ok);
    TS_ASSERT(recv.status.valid);
    TS_ASSERT_EQUALS(recv.status.positions.size(), 2U);
    TS_ASSERT_EQUALS(recv.status.positions[0].getX(), 1000);
    TS_ASSERT_EQUALS(recv.status.positions[0].getY(), 1036);
    TS_ASSERT_EQUALS(recv.status.positions[1].getX(), 1000);
    TS_ASSERT_EQUALS(recv.status.positions[1].getY(), 1050);
    TS_ASSERT_EQUALS(recv.status.distances2.size(), 2U);
    TS_ASSERT_EQUALS(recv.status.distances2[0], 36*36);
    TS_ASSERT_EQUALS(recv.status.distances2[1], 14*14);

    // Update configuration: should send update
    recv.ok = false;
    class Task : public util::Request<game::Session> {
     public:
        virtual void handle(game::Session& session)
            {
                session.getRoot()->userConfiguration()[game::config::UserConfiguration::Task_ShowDistances].set(0);
                session.notifyListeners();
            }
    };
    s.gameSender().postNewRequest(new Task());
    while (!recv.ok) {
        TS_ASSERT(disp.wait(1000));
    }

    TS_ASSERT(recv.ok);
    TS_ASSERT(recv.status.valid);
    TS_ASSERT_EQUALS(recv.status.positions.size(), 2U);
    TS_ASSERT_EQUALS(recv.status.distances2.size(), 0U);  // no longer reported because option disabled
}

/** Test message status reporting. */
void
TestGameProxyTaskEditorProxy::testMessage()
{
    const int SHIP_ID = 43;

    // Environment
    CxxTest::setAbortTestOnFail(true);
    SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);
    addShip(s, SHIP_ID, Point(1000,1000));

    // Add a task and a message
    {
        Ptr<TaskEditor> ed = s.session().getAutoTaskEditor(SHIP_ID, Process::pkShipTask, true);
        TS_ASSERT(ed.get());

        // releaseAutoTaskEditor will run the task, so the first command needs to be 'stop'.
        String_t code[] = { "stop" };
        ed->replace(0, 0, code, TaskEditor::DefaultCursor, TaskEditor::PlacePCBefore);

        // Message
        s.session().notifications().addMessage(ed->process().getProcessId(), "header", "the message body", game::Reference());

        s.session().releaseAutoTaskEditor(ed);
    }

    // Testee
    TaskEditorProxy testee(s.gameSender(), disp);

    StatusReceiver<TaskEditorProxy::MessageStatus> recv;
    testee.sig_messageChange.add(&recv, &StatusReceiver<TaskEditorProxy::MessageStatus>::onChange);

    // Wait for status update
    testee.selectTask(SHIP_ID, Process::pkShipTask, true);
    while (!recv.ok) {
        TS_ASSERT(disp.wait(1000));
    }

    // Verify
    TS_ASSERT(recv.ok);
    TS_ASSERT(recv.status.hasUnconfirmedMessage);
    TS_ASSERT_EQUALS(recv.status.text, "the message body");
}

/** Test starbase status reporting. */
void
TestGameProxyTaskEditorProxy::testBase()
{
    const int BASE_ID = 78;

    // Environment
    CxxTest::setAbortTestOnFail(true);
    SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);
    addBase(s, BASE_ID, Point(1200, 2300));

    // Add a task
    {
        Ptr<TaskEditor> ed = s.session().getAutoTaskEditor(BASE_ID, Process::pkBaseTask, true);
        TS_ASSERT(ed.get());

        String_t code[] = { "stop", "buildship 1, 9" };
        ed->replace(0, 0, code, TaskEditor::DefaultCursor, TaskEditor::PlacePCBefore);

        s.session().releaseAutoTaskEditor(ed);
    }

    // Testee
    TaskEditorProxy testee(s.gameSender(), disp);

    StatusReceiver<TaskEditorProxy::BaseStatus> recv;
    testee.sig_baseChange.add(&recv, &StatusReceiver<TaskEditorProxy::BaseStatus>::onChange);

    // Wait for status update
    testee.selectTask(BASE_ID, Process::pkBaseTask, true);
    testee.setCursor(1);
    while (!recv.ok || recv.status.buildOrder.empty()) {
        TS_ASSERT(disp.wait(1000));
    }

    // Verify
    TS_ASSERT(recv.ok);
    TS_ASSERT_EQUALS(recv.status.buildOrder.size(), 2U);
    TS_ASSERT_EQUALS(recv.status.buildOrder[0], "OUTRIDER CLASS SCOUT");
    TS_ASSERT_EQUALS(recv.status.buildOrder[1], "Transwarp Drive");
    TS_ASSERT_EQUALS(recv.status.missingMinerals, "4,650sup");  // FIXME: should be mc; see game::actions::CargoCostAction::getMissingAmount
}

/** Test editing.
    A: make session containing a ship and a ship task.
    E: status correctly reported */
void
TestGameProxyTaskEditorProxy::testEdit()
{
    const int SHIP_ID = 43;

    // Environment
    CxxTest::setAbortTestOnFail(true);
    game::test::WaitIndicator ind;          // must be first because SessionThread will post updates into it
    SessionThread s;
    prepare(s);
    addShip(s, SHIP_ID, Point(1000,1000));

    // Add a task
    {
        Ptr<TaskEditor> ed = s.session().getAutoTaskEditor(SHIP_ID, Process::pkShipTask, true);
        TS_ASSERT(ed.get());

        // releaseAutoTaskEditor will run the task, so the first command needs to be 'stop'
        String_t code[] = { "stop", "hammer", "time" };
        ed->replace(0, 0, code, TaskEditor::DefaultCursor, TaskEditor::PlacePCBefore);

        s.session().releaseAutoTaskEditor(ed);
    }

    // Testee
    TaskEditorProxy testee(s.gameSender(), ind);
    testee.selectTask(SHIP_ID, Process::pkShipTask, true);

    // Get status, synchronously
    TaskEditorProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT(st.valid);
    TS_ASSERT_EQUALS(st.commands.size(), 3U);
    TS_ASSERT_EQUALS(st.commands[0], "stop");
    TS_ASSERT_EQUALS(st.pc, 0U);
    TS_ASSERT_EQUALS(st.cursor, 3U);
    TS_ASSERT_EQUALS(st.isInSubroutineCall, true);

    // Manipulate
    testee.addAsCurrent("stop %2");
    testee.addAtEnd("again");

    // Check status again
    testee.getStatus(ind, st);
    TS_ASSERT(st.valid);
    TS_ASSERT_EQUALS(st.commands.size(), 5U);
    TS_ASSERT_EQUALS(st.commands[0], "stop %2");
    TS_ASSERT_EQUALS(st.commands[1], "stop");
    TS_ASSERT_EQUALS(st.commands[2], "hammer");
    TS_ASSERT_EQUALS(st.commands[3], "time");
    TS_ASSERT_EQUALS(st.commands[4], "again");
    TS_ASSERT_EQUALS(st.pc, 0U);
    TS_ASSERT_EQUALS(st.cursor, 5U);
    TS_ASSERT_EQUALS(st.isInSubroutineCall, false);
}
