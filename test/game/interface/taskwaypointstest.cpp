/**
  *  \file test/game/interface/taskwaypointstest.cpp
  *  \brief Test for game::interface::TaskWaypoints
  */

#include "game/interface/taskwaypoints.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/subroutinevalue.hpp"

using afl::base::Ptr;
using interpreter::TaskEditor;
using interpreter::Opcode;
using interpreter::Process;
using game::interface::TaskWaypoints;
using game::Game;
using game::PlayerSet_t;

namespace {
    const int PLAYER = 5;

    const String_t COMMANDS[] = {
        "MoveTo 1000,1300",
        "MoveTo 1500,1000",
    };

    void addShip(Game& g, game::Id_t id, int x, int y)
    {
        game::map::Ship* s = g.currentTurn().universe().ships().create(id);
        game::map::ShipData sd;
        sd.x = x;
        sd.y = y;
        sd.waypointDX = 0;
        sd.waypointDY = 0;
        sd.warpFactor = 3;
        sd.owner = PLAYER;
        s->addCurrentShipData(sd, PlayerSet_t(PLAYER));
        s->internalCheck(PlayerSet_t(PLAYER), 20);
    }

    struct Environment {
        afl::io::NullFileSystem fs;
        afl::string::NullTranslator tx;
        game::Session session;

        Environment()
            : fs(), tx(), session(tx, fs)
            {
                // Environment
                session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
                session.setShipList(new game::spec::ShipList());

                // Create CC$AUTOEXEC mock.
                // This is "do / stop / loop", i.e. will suspend indefinitely.
                // If we do not use this, the auto tasks will fail (which largely produces the same net effect but is unrealistic)
                interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
                bco->addArgument("A", false);
                bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
                bco->addInstruction(Opcode::maJump, Opcode::jAlways, 0);
                session.world().setNewGlobalValue("CC$AUTOEXEC", new interpreter::SubroutineValue(bco));
            }
    };
}

/* Normal operation */
AFL_TEST("game.interface.TaskWaypoints:normal", a)
{
    // Environment
    Environment env;

    // Create game
    Ptr<Game> g(new Game());
    addShip(*g, 10, 1000, 1100);
    addShip(*g, 20, 2000, 1100);
    env.session.setGame(g);

    // Create auto task using TaskEditor
    Ptr<TaskEditor> ed = env.session.getAutoTaskEditor(20, Process::pkShipTask, true);
    a.checkNonNull("01. ed", ed.get());
    ed->addAtEnd(COMMANDS);
    env.session.releaseAutoTaskEditor(ed);

    // Create TaskWaypoints object; this will inspect all tasks
    TaskWaypoints& testee = TaskWaypoints::create(env.session);

    // Verify result
    const TaskWaypoints::Track* t = testee.getTrack(20);
    a.checkNonNull("11. track", t);
    a.checkEqual("12. size", t->waypoints.size(), 2U);
    a.checkEqual("13. x0", t->waypoints[0].getX(), 1000);
    a.checkEqual("14. y0", t->waypoints[0].getY(), 1300);
    a.checkEqual("15. x1", t->waypoints[1].getX(), 1500);
    a.checkEqual("16. y1", t->waypoints[1].getY(), 1000);

    a.checkNull("21. track", testee.getTrack(10));
}

/* Use of pre-existing TaskEditor */
AFL_TEST("game.interface.TaskWaypoints:preexisting", a)
{
    // Environment
    Environment env;

    // Create game
    Ptr<Game> g(new Game());
    addShip(*g, 10, 1000, 1100);
    addShip(*g, 20, 2000, 1100);
    env.session.setGame(g);

    // Create auto task using TaskEditor, not releasing it
    Ptr<TaskEditor> ed = env.session.getAutoTaskEditor(20, Process::pkShipTask, true);
    a.checkNonNull("01. ed", ed.get());
    ed->addAtEnd(COMMANDS);

    // Explicitly sync this process
    TaskWaypoints testee(env.session);
    testee.updateProcess(ed->process(), false);

    // Verify
    const TaskWaypoints::Track* t = testee.getTrack(20);
    a.checkNonNull("11. track", t);
    a.checkEqual("12. size", t->waypoints.size(), 2U);
    a.checkEqual("13. x0", t->waypoints[0].getX(), 1000);
    a.checkEqual("14. y0", t->waypoints[0].getY(), 1300);
    a.checkEqual("15. x1", t->waypoints[1].getX(), 1500);
    a.checkEqual("16. y1", t->waypoints[1].getY(), 1000);

    a.checkNull("21. track", testee.getTrack(10));
}

/* Editing task while TaskWaypoints exists */
AFL_TEST("game.interface.TaskWaypoints:edit", a)
{
    // Environment
    Environment env;

    // Create game
    Ptr<Game> g(new Game());
    addShip(*g, 10, 1000, 1100);
    addShip(*g, 20, 2000, 1100);
    env.session.setGame(g);

    // Create TaskWaypoints object
    TaskWaypoints& testee = TaskWaypoints::create(env.session);
    a.checkNull("00. track", testee.getTrack(20));

    // Create auto task using TaskEditor. Releasing it will invoke the TaskWaypoints.
    Ptr<TaskEditor> ed = env.session.getAutoTaskEditor(20, Process::pkShipTask, true);
    a.checkNonNull("01. ed", ed.get());
    ed->addAtEnd(COMMANDS);
    env.session.releaseAutoTaskEditor(ed);

    // Verify result
    const TaskWaypoints::Track* t = testee.getTrack(20);
    a.checkNonNull("11. track", t);
    a.checkEqual("12. size", t->waypoints.size(), 2U);
    a.checkEqual("13. x0", t->waypoints[0].getX(), 1000);
    a.checkEqual("14. y0", t->waypoints[0].getY(), 1300);
    a.checkEqual("15. x1", t->waypoints[1].getX(), 1500);
    a.checkEqual("16. y1", t->waypoints[1].getY(), 1000);
}

/* Change an existing task */
AFL_TEST("game.interface.TaskWaypoints:change", a)
{
    // Environment
    Environment env;

    // Create game
    Ptr<Game> g(new Game());
    addShip(*g, 10, 1000, 1100);
    addShip(*g, 20, 2000, 1100);
    env.session.setGame(g);

    // Create auto task using TaskEditor
    Ptr<TaskEditor> ed = env.session.getAutoTaskEditor(20, Process::pkShipTask, true);
    a.checkNonNull("01. ed", ed.get());
    ed->addAtEnd(COMMANDS);
    env.session.releaseAutoTaskEditor(ed);

    // Create TaskWaypoints object; this will inspect all tasks
    TaskWaypoints& testee = TaskWaypoints::create(env.session);

    // Delete the task
    ed = env.session.getAutoTaskEditor(20, Process::pkShipTask, true);
    ed->addAtEnd(COMMANDS);
    env.session.releaseAutoTaskEditor(ed);

    // Verify result
    const TaskWaypoints::Track* t = testee.getTrack(20);
    a.checkNonNull("11. track", t);
    a.checkEqual("12. size", t->waypoints.size(), 4U);
}

/* Change from existing to empty task */
AFL_TEST("game.interface.TaskWaypoints:change-to-empty", a)
{
    // Environment
    Environment env;

    // Create game
    Ptr<Game> g(new Game());
    addShip(*g, 10, 1000, 1100);
    addShip(*g, 20, 2000, 1100);
    env.session.setGame(g);

    // Create auto task using TaskEditor
    Ptr<TaskEditor> ed = env.session.getAutoTaskEditor(20, Process::pkShipTask, true);
    a.checkNonNull("01. ed", ed.get());
    ed->addAtEnd(COMMANDS);
    env.session.releaseAutoTaskEditor(ed);

    // Create TaskWaypoints object; this will inspect all tasks
    TaskWaypoints& testee = TaskWaypoints::create(env.session);

    // Delete the task
    ed = env.session.getAutoTaskEditor(20, Process::pkShipTask, true);
    ed->replace(0, 100, afl::base::Nothing, TaskEditor::DefaultCursor, TaskEditor::DefaultPC);
    env.session.releaseAutoTaskEditor(ed);

    // Verify result
    a.checkNull("11. track", testee.getTrack(20));
}
