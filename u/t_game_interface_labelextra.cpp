/**
  *  \file u/t_game_interface_labelextra.cpp
  *  \brief Test for game::interface::LabelExtra
  */

#include "game/interface/labelextra.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/counter.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/values.hpp"

using game::interface::LabelExtra;
using game::config::ConfigurationOption;
using game::test::Counter;

namespace {
    /* Shortcut to set up a session */
    struct TestHarness {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        TestHarness()
            : session(tx, fs)
            { }
    };

    /* Add connections (=root, shiplist, game).
       Although LabelExtra does not require a ship list, PlanetFunction and ShipFunction do. */
    void addConnections(TestHarness& h)
    {
        h.session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        h.session.setGame(new game::Game());
        h.session.setShipList(new game::spec::ShipList());
        h.session.sig_runRequest.add(&h.session.processList(), &interpreter::ProcessList::run);
    }

    /* Add planet. It doesn't need any specific status, it just needs to be visible on the map. */
    void addPlanet(TestHarness& h, game::Id_t id, int x, int y, String_t name)
    {
        game::map::Planet* pl = h.session.getGame()->currentTurn().universe().planets().create(id);
        pl->setName(name);
        pl->setPosition(game::map::Point(x, y));
    }

    /* Add ship. It doesn't need any specific status, it just needs to be visible on the map, so we make a shipxy target. */
    void addShip(TestHarness& h, game::Id_t id, int x, int y, String_t name)
    {
        game::map::Ship* sh = h.session.getGame()->currentTurn().universe().ships().create(id);
        sh->setName(name);
        sh->addShipXYData(game::map::Point(x, y), 1, 100, game::PlayerSet_t(2));
    }

    /* Add some generic units */
    void addObjects(TestHarness& h)
    {
        addPlanet(h, 1, 1000, 1100, "Mercury");
        addPlanet(h, 2, 1100, 1200, "Venus");
        addPlanet(h, 3, 1200, 1300, "Terra");
        addPlanet(h, 4, 1300, 1400, "Mars");
        addPlanet(h, 5, 1400, 1500, "Jupiter");

        addShip(h, 10, 1000, 1010, "Titanic");
        addShip(h, 20, 1020, 1020, "Ever Given");
        addShip(h, 30, 1040, 1030, "Exxon Valdez");

        h.session.postprocessTurn(h.session.getGame()->currentTurn(), game::PlayerSet_t(2), game::PlayerSet_t(2), game::map::Object::Playable);
        h.session.getGame()->setViewpointPlayer(2);
    }
}

/** Test object accesses. */
void
TestGameInterfaceLabelExtra::testLink()
{
    // Session starts with no LabelExtra
    TestHarness h;
    TS_ASSERT(LabelExtra::get(h.session) == 0);

    // Create one
    LabelExtra& t = LabelExtra::create(h.session);
    TS_ASSERT(LabelExtra::get(h.session) == &t);

    // Check accessors (mostly for coverage)
    const LabelExtra& ct = t;
    TS_ASSERT_EQUALS(&t.shipLabels(), &ct.shipLabels());
    TS_ASSERT_EQUALS(&t.planetLabels(), &ct.planetLabels());
}

/** Test LabelExtra early registration.
    Labels need to be computed correctly when the LabelExtra is created before objects are connected to the session. */
void
TestGameInterfaceLabelExtra::testEarly()
{
    // Create LabelExtra first
    TestHarness h;
    LabelExtra& t = LabelExtra::create(h.session);

    // Add stuff
    addConnections(h);
    h.session.getRoot()->userConfiguration().setOption("Label.Planet", "Name", ConfigurationOption::User);
    h.session.getRoot()->userConfiguration().setOption("Label.Ship", "Loc.X", ConfigurationOption::User);
    addObjects(h);
    h.session.notifyListeners();

    // Labels now present
    TS_ASSERT_EQUALS(t.planetLabels().getLabel(2), "Venus");
    TS_ASSERT_EQUALS(t.shipLabels().getLabel(30), "1040");

    // Modify configuration. This will update labels.
    h.session.getRoot()->userConfiguration().setOption("Label.Planet", "Id", ConfigurationOption::User);
    h.session.notifyListeners();
    TS_ASSERT_EQUALS(t.planetLabels().getLabel(2), "2");
}

/** Test LabelExtra late registration.
    Labels need to be computed correctly when the LabelExtra is added to a populated session. */
void
TestGameInterfaceLabelExtra::testLate()
{
    // Create and populate a session
    TestHarness h;
    addConnections(h);
    h.session.getRoot()->userConfiguration().setOption("Label.Planet", "Name", ConfigurationOption::User);
    h.session.getRoot()->userConfiguration().setOption("Label.Ship", "Loc.X", ConfigurationOption::User);
    addObjects(h);

    // Create a LabelExtra. This will immediately produce labels.
    LabelExtra& t = LabelExtra::create(h.session);
    TS_ASSERT_EQUALS(t.planetLabels().getLabel(2), "Venus");
    TS_ASSERT_EQUALS(t.shipLabels().getLabel(30), "1040");
}

/** Test self-modifying labels.
    Labels must be computed correctly if they modify the object being labeled. */
void
TestGameInterfaceLabelExtra::testSelfModify()
{
    // Create and populate a session
    TestHarness h;
    addConnections(h);
    h.session.getRoot()->userConfiguration().setOption("Label.Ship", "Name:=RandomFCode()", ConfigurationOption::User);
    h.session.getRoot()->userConfiguration().setOption("Label.Planet", "Comment:=RandomFCode()", ConfigurationOption::User);
    addObjects(h);

    // Create a LabelExtra. This will immediately produce labels.
    LabelExtra& t = LabelExtra::create(h.session);
    String_t shipLabel = t.shipLabels().getLabel(30);
    String_t shipName = h.session.getGame()->currentTurn().universe().ships().get(30)->getName();
    TS_ASSERT_DIFFERS(shipLabel, "");
    TS_ASSERT_EQUALS(shipLabel, shipName);

    String_t planetLabel = t.planetLabels().getLabel(2);
    String_t planetComment = interpreter::toString(h.session.world().planetProperties().get(2, interpreter::World::pp_Comment), false);
    TS_ASSERT_DIFFERS(planetLabel, "");
    TS_ASSERT_EQUALS(planetLabel, planetComment);

    // Trigger incremental change. This must recompute (=change) the label of the changed object.
    h.session.getGame()->currentTurn().universe().ships().get(30)->markDirty();
    h.session.notifyListeners();

    String_t newLabel = t.shipLabels().getLabel(30);
    String_t newName = h.session.getGame()->currentTurn().universe().ships().get(30)->getName();
    TS_ASSERT_DIFFERS(newLabel, "");
    TS_ASSERT_DIFFERS(newLabel, shipLabel);
    TS_ASSERT_EQUALS(newLabel, newName);

    // Unrelated label does not change
    TS_ASSERT_EQUALS(planetLabel, t.planetLabels().getLabel(2));
}

/** Test labels that modify other objects.
    This exercises the paranoia-counter logic.
    Labels must be computed correctly if they modify a different object. */
void
TestGameInterfaceLabelExtra::testOtherModify()
{
    // Create and populate a session
    TestHarness h;
    addConnections(h);
    h.session.getRoot()->userConfiguration().setOption("Label.Ship", "Ship(Id-1).Name:=RandomFCode()", ConfigurationOption::User);
    for (game::Id_t i = 100; i <= 500; ++i) {
        addShip(h, i, 1000+i, 1000, "Extra");
    }
    addObjects(h);

    // Create a LabelExtra. This will immediately produce labels and change ship names. Verify them.
    // Our expression modifies each ship's predecessor.
    // On the initial run, this is done in one pass, because a label that is updating does not trigger recomputation.
    LabelExtra& t = LabelExtra::create(h.session);
    for (int i = 100; i <= 499; ++i) {
        String_t label = t.shipLabels().getLabel(i+1);
        String_t name = h.session.getGame()->currentTurn().universe().ships().get(i)->getName();
        TS_ASSERT_EQUALS(name.size(), 3U);
        TS_ASSERT_EQUALS(name, label);
    }
    TS_ASSERT_EQUALS(h.session.getGame()->currentTurn().universe().ships().get(500)->getName(), "Extra");
    String_t shipLabel = t.shipLabels().getLabel(500);
    TS_ASSERT_EQUALS(shipLabel.size(), 3U);

    String_t firstLabel = t.shipLabels().getLabel(100);

    // Trigger incremental change. This will repeatedly trigger updates until the paranoia limit kicks in.
    // Therefore, it will not update everything.
    h.session.getGame()->currentTurn().universe().ships().get(500)->markDirty();
    h.session.notifyListeners();

    TS_ASSERT_DIFFERS(t.shipLabels().getLabel(500), shipLabel);     // last ship > changed
    TS_ASSERT_EQUALS(t.shipLabels().getLabel(100), firstLabel);     // first ship > not changed
}

/** Test configuration handling.
    A configuration change must always produce a sig_change, even if it doesn't actually change anything. */
void
TestGameInterfaceLabelExtra::testConfig()
{
    // Create and populate a session
    TestHarness h;
    addConnections(h);
    addObjects(h);
    LabelExtra& t = LabelExtra::create(h.session);

    Counter c;
    t.sig_change.add(&c, &Counter::increment);

    // Change configuration
    int n1 = c.get();
    t.setConfiguration(String_t("Id"), String_t("Name"));
    int n2 = c.get();
    TS_ASSERT(n2 > n1);
    TS_ASSERT_EQUALS(t.shipLabels().getLabel(10), "10");
    TS_ASSERT_EQUALS(t.planetLabels().getLabel(1), "Mercury");

    // Change configuration (no-op)
    t.setConfiguration(String_t("Id"), String_t("Name"));
    int n3 = c.get();
    TS_ASSERT(n3 > n2);

    // Change configuration (another no-op)
    t.setConfiguration(afl::base::Nothing, afl::base::Nothing);
    int n4 = c.get();
    TS_ASSERT(n4 > n3);
}

/** Test configuration error handling: compile-time error.
    Setting an erroneous expression must make an error report available after sig_change. */
void
TestGameInterfaceLabelExtra::testConfigError()
{
    // Create and populate a session
    TestHarness h;
    addConnections(h);
    addObjects(h);
    LabelExtra& t = LabelExtra::create(h.session);

    Counter c;
    t.sig_change.add(&c, &Counter::increment);

    // Change configuration
    int n1 = c.get();
    t.setConfiguration(String_t("Id"), String_t("Name"));
    int n2 = c.get();
    TS_ASSERT(n2 > n1);
    TS_ASSERT_EQUALS(t.shipLabels().getLabel(10), "10");
    TS_ASSERT_EQUALS(t.planetLabels().getLabel(1), "Mercury");

    // Change configuration to some error
    t.setConfiguration(String_t("*"), String_t("*"));
    int n3 = c.get();
    TS_ASSERT(n3 > n2);
    TS_ASSERT(t.shipLabels().hasError());
    TS_ASSERT(t.planetLabels().hasError());

    // Change configuration back
    t.setConfiguration(String_t("Id"), String_t("Name"));
    int n4 = c.get();
    TS_ASSERT(n4 > n3);
    TS_ASSERT(!t.shipLabels().hasError());
    TS_ASSERT(!t.planetLabels().hasError());
}

/** Test configuration error handling: run-time error.
    Setting an erroneous expression must make an error report available after sig_change. */
void
TestGameInterfaceLabelExtra::testConfigError2()
{
    // Create and populate a session
    TestHarness h;
    addConnections(h);
    addObjects(h);
    LabelExtra& t = LabelExtra::create(h.session);

    Counter c;
    t.sig_change.add(&c, &Counter::increment);

    // Change configuration
    int n1 = c.get();
    t.setConfiguration(String_t("Id"), String_t("Name"));
    int n2 = c.get();
    TS_ASSERT(n2 > n1);
    TS_ASSERT_EQUALS(t.shipLabels().getLabel(10), "10");
    TS_ASSERT_EQUALS(t.planetLabels().getLabel(1), "Mercury");

    // Change configuration to some error
    t.setConfiguration(String_t("xyxyyxxyyxyx"), String_t("Id*Name"));
    int n3 = c.get();
    TS_ASSERT(n3 > n2);
    TS_ASSERT(t.shipLabels().hasError());
    TS_ASSERT(t.planetLabels().hasError());

    // Change configuration back
    t.setConfiguration(String_t("Id"), String_t("Name"));
    int n4 = c.get();
    TS_ASSERT(n4 > n3);
    TS_ASSERT(!t.shipLabels().hasError());
    TS_ASSERT(!t.planetLabels().hasError());
}

/** Test configuration, empty session (no connections).
    setConfiguration() must produce a callback even if there is no game/root to configure. */
void
TestGameInterfaceLabelExtra::testConfigEmpty()
{
    // Create an empty session
    TestHarness h;
    LabelExtra& t = LabelExtra::create(h.session);

    Counter c;
    t.sig_change.add(&c, &Counter::increment);

    // Change configuration
    int n1 = c.get();
    t.setConfiguration(String_t("Id"), String_t("Name"));
    int n2 = c.get();
    TS_ASSERT(n2 > n1);
}

/** Test configuration, empty session (no objects).
    setConfiguration() must produce a callback even if there are no objects to update. */
void
TestGameInterfaceLabelExtra::testConfigEmpty2()
{
    // Create an empty session
    TestHarness h;
    addConnections(h);
    LabelExtra& t = LabelExtra::create(h.session);

    Counter c;
    t.sig_change.add(&c, &Counter::increment);

    // Change configuration
    int n1 = c.get();
    t.setConfiguration(String_t("Id"), String_t("Name"));
    int n2 = c.get();
    TS_ASSERT(n2 > n1);
}

/** Test clearing a session.
    If the game is reoved, labels must disappear. */
void
TestGameInterfaceLabelExtra::testClear()
{
    // Set up
    TestHarness h;
    addConnections(h);
    h.session.getRoot()->userConfiguration().setOption("Label.Planet", "Name", ConfigurationOption::User);
    addObjects(h);
    LabelExtra& t = LabelExtra::create(h.session);
    TS_ASSERT_EQUALS(t.planetLabels().getLabel(2), "Venus");

    // Remove the game. Labels must go away.
    h.session.setGame(0);
    TS_ASSERT_EQUALS(t.planetLabels().getLabel(2), "");
}

/** Test process exiting with wrong state. */
void
TestGameInterfaceLabelExtra::testBadState()
{
    // Set up
    TestHarness h;
    addConnections(h);
    h.session.getRoot()->userConfiguration().setOption("Label.Planet", "Name", ConfigurationOption::User);
    addObjects(h);
    LabelExtra& t = LabelExtra::create(h.session);
    TS_ASSERT_EQUALS(t.planetLabels().getLabel(1), "Mercury");
    TS_ASSERT_EQUALS(t.planetLabels().getLabel(2), "Venus");

    // Create a function that stops a process
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    bco->addInstruction(interpreter::Opcode::maSpecial, interpreter::Opcode::miSpecialSuspend, 0);
    h.session.world().setNewGlobalValue("FXN", new interpreter::SubroutineValue(bco));

    // Configure
    t.setConfiguration(afl::base::Nothing, String_t("fxn()"));

    // Labels remain unchanged as expression never completes
    TS_ASSERT_EQUALS(t.planetLabels().getLabel(1), "Mercury");
    TS_ASSERT_EQUALS(t.planetLabels().getLabel(2), "Venus");

    // Process is gone
    h.session.processList().removeTerminatedProcesses();
    TS_ASSERT_EQUALS(h.session.processList().getProcessList().size(), 0U);
}

