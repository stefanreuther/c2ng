/**
  *  \file test/game/proxy/taskeditorproxytest.cpp
  *  \brief Test for game::proxy::TaskEditorProxy
  */

#include "game/proxy/taskeditorproxy.hpp"

#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.proxy.TaskEditorProxy:empty", a)
{
    // Environment
    /* FIXME: this crashes when the declarations of disp and s are swapped - why? */
    SimpleRequestDispatcher disp;
    SessionThread s;
    TaskEditorProxy testee(s.gameSender(), disp);

    StatusReceiver<TaskEditorProxy::Status> recv;
    testee.sig_change.add(&recv, &StatusReceiver<TaskEditorProxy::Status>::onChange);

    // Wait for status update
    testee.selectTask(99, Process::pkShipTask, true);
    while (!recv.ok) {
        a.check("01. wait", disp.wait(1000));
    }

    a.check("11. status", !recv.status.valid);
}

/** Test non-empty session.
    A: make session containing a ship and a ship task.
    E: status correctly reported */
AFL_TEST("game.proxy.TaskEditorProxy:normal", a)
{
    const int SHIP_ID = 43;

    // Environment
    SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);
    addShip(s, SHIP_ID, Point(1000,1000));

    // Add a task
    {
        Ptr<TaskEditor> ed = s.session().getAutoTaskEditor(SHIP_ID, Process::pkShipTask, true);
        a.check("01. get", ed.get());

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
        a.check("11. wait", disp.wait(1000));
    }

    a.check("21. status", recv.status.valid);
    a.checkEqual("22. size", recv.status.commands.size(), 3U);
    a.checkEqual("23. command", recv.status.commands[0], "stop");
    a.checkEqual("24. pc", recv.status.pc, 0U);
    a.checkEqual("25. cursor", recv.status.cursor, 3U);
    a.checkEqual("26. isInSubroutineCall", recv.status.isInSubroutineCall, true);

    // Move the cursor
    recv.ok = false;
    testee.setCursor(1);
    while (!recv.ok) {
        a.check("31. wait", disp.wait(1000));
    }
    a.check("32. status", recv.status.valid);
    a.checkEqual("33. cursor", recv.status.cursor, 1U);
}

/** Test ship status reporting. */
AFL_TEST("game.proxy.TaskEditorProxy:ship-status", a)
{
    const int SHIP_ID = 43;

    // Environment
    SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);
    addShip(s, SHIP_ID, Point(1000,1000));

    game::map::Point pt(333,333);
    s.session().getGame()->currentTurn().universe().ships().get(SHIP_ID)->getPosition().get(pt);

    // Add a task
    {
        Ptr<TaskEditor> ed = s.session().getAutoTaskEditor(SHIP_ID, Process::pkShipTask, true);
        a.check("01. get", ed.get());

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
        a.check("11. wait", disp.wait(1000));
    }

    a.check("21. ok", recv.ok);
    a.check("22. status", recv.status.valid);
    a.checkEqual("23. positions", recv.status.positions.size(), 2U);
    a.checkEqual("24. positions", recv.status.positions[0].getX(), 1000);
    a.checkEqual("25. positions", recv.status.positions[0].getY(), 1036);
    a.checkEqual("26. positions", recv.status.positions[1].getX(), 1000);
    a.checkEqual("27. positions", recv.status.positions[1].getY(), 1050);
    a.checkEqual("28. distances", recv.status.distances2.size(), 2U);
    a.checkEqual("29. distances", recv.status.distances2[0], 36*36);
    a.checkEqual("30. distances", recv.status.distances2[1], 14*14);

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
        a.check("31. wait", disp.wait(1000));
    }

    a.check("41. ok", recv.ok);
    a.check("42. status", recv.status.valid);
    a.checkEqual("43. positions", recv.status.positions.size(), 2U);
    a.checkEqual("44. distances2", recv.status.distances2.size(), 0U);  // no longer reported because option disabled
}

/** Test message status reporting. */
AFL_TEST("game.proxy.TaskEditorProxy:message", a)
{
    const int SHIP_ID = 43;

    // Environment
    SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);
    addShip(s, SHIP_ID, Point(1000,1000));

    // Add a task and a message
    {
        Ptr<TaskEditor> ed = s.session().getAutoTaskEditor(SHIP_ID, Process::pkShipTask, true);
        a.check("01. get", ed.get());

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
        a.check("11. wait", disp.wait(1000));
    }

    // Verify
    a.check("21. ok", recv.ok);
    a.check("22. hasUnconfirmedMessage", recv.status.hasUnconfirmedMessage);
    a.checkEqual("23. text", recv.status.text, "the message body");
}

/** Test starbase status reporting. */
AFL_TEST("game.proxy.TaskEditorProxy:base", a)
{
    const int BASE_ID = 78;

    // Environment
    SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);
    addBase(s, BASE_ID, Point(1200, 2300));

    // Add a task
    {
        Ptr<TaskEditor> ed = s.session().getAutoTaskEditor(BASE_ID, Process::pkBaseTask, true);
        a.check("01. get", ed.get());

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
        a.check("11. wait", disp.wait(1000));
    }

    // Verify
    a.check("21. ok", recv.ok);
    a.checkEqual("22. buildOrder", recv.status.buildOrder.size(), 2U);
    a.checkEqual("23. buildOrder", recv.status.buildOrder[0], "OUTRIDER CLASS SCOUT");
    a.checkEqual("24. buildOrder", recv.status.buildOrder[1], "Transwarp Drive");
    a.checkEqual("25. missingMinerals", recv.status.missingMinerals, "4,650sup");  // FIXME: should be mc; see game::actions::CargoCostAction::getMissingAmount
}

/** Test editing.
    A: make session containing a ship and a ship task.
    E: status correctly reported */
AFL_TEST("game.proxy.TaskEditorProxy:edit", a)
{
    const int SHIP_ID = 43;

    // Environment
    game::test::WaitIndicator ind;          // must be first because SessionThread will post updates into it
    SessionThread s;
    prepare(s);
    addShip(s, SHIP_ID, Point(1000,1000));

    // Add a task
    {
        Ptr<TaskEditor> ed = s.session().getAutoTaskEditor(SHIP_ID, Process::pkShipTask, true);
        a.check("01. get", ed.get());

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
    a.check("11. valid", st.valid);
    a.checkEqual("12. commands", st.commands.size(), 3U);
    a.checkEqual("13. commands", st.commands[0], "stop");
    a.checkEqual("14. pc", st.pc, 0U);
    a.checkEqual("15. cursor", st.cursor, 3U);
    a.checkEqual("16. isInSubroutineCall", st.isInSubroutineCall, true);

    // Manipulate
    testee.addAsCurrent("stop %2");
    testee.addAtEnd("again");

    // Check status again
    testee.getStatus(ind, st);
    a.check("21. valid", st.valid);
    a.checkEqual("22. commands", st.commands.size(), 5U);
    a.checkEqual("23. commands", st.commands[0], "stop %2");
    a.checkEqual("24. commands", st.commands[1], "stop");
    a.checkEqual("25. commands", st.commands[2], "hammer");
    a.checkEqual("26. commands", st.commands[3], "time");
    a.checkEqual("27. commands", st.commands[4], "again");
    a.checkEqual("28. pc", st.pc, 0U);
    a.checkEqual("29. cursor", st.cursor, 5U);
    a.checkEqual("30. isInSubroutineCall", st.isInSubroutineCall, false);
}
