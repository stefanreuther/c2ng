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
        s.session().setRoot(new game::test::Root(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,2,0))));
        s.session().setGame(new game::Game());
        s.session().setShipList(new game::spec::ShipList());

        // We need a CC$AUTOEXEC procedure
        BCORef_t bco = *new BytecodeObject();
        bco->setIsProcedure(true);
        bco->addArgument("A", false);
        bco->addInstruction(Opcode::maPush, Opcode::sLocal, 0);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEvalStatement, 1);
        s.session().world().setNewGlobalValue("CC$AUTOEXEC", new interpreter::SubroutineValue(bco));
    }

    void addShip(SessionThread& s, int id, Point pos)
    {
        game::map::Ship* sh = s.session().getGame()->currentTurn().universe().ships().create(id);
        sh->addShipXYData(pos, 1, 100, game::PlayerSet_t(1));
        sh->internalCheck();
    }

    struct StatusReceiver {
        TaskEditorProxy::Status status;
        bool ok;

        StatusReceiver()
            : status(), ok(false)
            { }

        void onChange(const TaskEditorProxy::Status& st)
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

    StatusReceiver recv;
    testee.sig_change.add(&recv, &StatusReceiver::onChange);

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

    StatusReceiver recv;
    testee.sig_change.add(&recv, &StatusReceiver::onChange);

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

