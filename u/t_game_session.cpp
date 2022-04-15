/**
  *  \file u/t_game_session.cpp
  *  \brief Test for game::Session
  */

#include <memory>
#include "game/session.hpp"

#include "t_game.hpp"
#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/ufo.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/subroutinevalue.hpp"

/** Test initialisation.
    A: create a session
    E: verify initial values */
void
TestGameSession::testInit()
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session testee(tx, fs);

    // Initial values
    TS_ASSERT_EQUALS(testee.translator()("foo"), "foo");
    TS_ASSERT(testee.getRoot().get() == 0);
    TS_ASSERT(testee.getShipList().get() == 0);
    TS_ASSERT(testee.getGame().get() == 0);
    TS_ASSERT(testee.getEditableAreas().empty());
    TS_ASSERT(testee.world().fileTable().getFreeFile() != 0);
    TS_ASSERT(testee.world().globalPropertyNames().getIndexByName("HULL") != afl::data::NameMap::nil);

    // EditableAreas is modifiable
    game::Session::AreaSet_t a(game::Session::CommandArea);
    testee.setEditableAreas(a);
    TS_ASSERT_EQUALS(testee.getEditableAreas(), a);
}

/** Test subobjects.
    A: create a session. Access subobjects.
    E: subobject references match */
void
TestGameSession::testSubobjects()
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session s(tx, fs);
    const game::Session& cs(s);

    TS_ASSERT_EQUALS(&s.translator(), &tx);
    TS_ASSERT_EQUALS(&s.world().fileSystem(), &fs);

    TS_ASSERT_EQUALS(&s.uiPropertyStack(), &cs.uiPropertyStack());
    TS_ASSERT_EQUALS(&s.notifications(), &cs.notifications());
    // TS_ASSERT_EQUALS(&s.world(), &cs.world());
    TS_ASSERT_EQUALS(&s.processList(), &cs.processList());
}

/** Test getReferenceName().
    A: create empty session. Call getReferenceName().
    E: must report unknown for all objects */
void
TestGameSession::testReferenceNameEmpty()
{
    using game::Reference;

    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session testee(tx, fs);

    String_t s;
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(),                            game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Player, 3),        game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(game::map::Point(2000,3000)), game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "(2000,3000)");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ship, 17),         game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Planet, 9),        game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Starbase, 9),      game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Storm, 4),         game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Minefield, 150),   game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ufo, 42),          game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Hull, 15),         game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Engine, 2),        game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Beam, 3),          game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Torpedo, 7),       game::PlainName, s), false);
}

/** Test getReferenceName().
    A: create session, add some objects. Call getReferenceName().
    E: must report correct names for all objects */
void
TestGameSession::testReferenceNameNonempty()
{
    using game::Reference;

    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session testee(tx, fs);

    // Populate ship list
    afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
    shipList->hulls().create(15)->setName("SMALL FREIGHTER");
    shipList->engines().create(2)->setName("2-cyl. engine");
    shipList->beams().create(3)->setName("Pink Laser");
    shipList->launchers().create(7)->setName("Mark 7 Torpedo");
    testee.setShipList(shipList);

    // Populate root
    afl::base::Ptr<game::Root> root = new game::test::Root(game::HostVersion());
    root->playerList().create(3)->setName(game::Player::ShortName, "The Romulans");
    testee.setRoot(root);

    // Populate game
    afl::base::Ptr<game::Game> g = new game::Game();
    g->currentTurn().universe().planets().create(9)->setName("Pluto");
    g->currentTurn().universe().ships().create(17)->setName("Voyager");
    g->currentTurn().universe().ionStorms().create(4)->setName("Kathrina");
    g->currentTurn().universe().minefields().create(150);
    g->currentTurn().universe().ufos().addUfo(42, 1, 1)->setName("Hui");
    testee.setGame(g);

    String_t s;

    // Query plain names
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(),                            game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Player, 3),        game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "The Romulans");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(game::map::Point(2000,3000)), game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "(2000,3000)");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ship, 17),         game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "Voyager");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Planet, 9),        game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "Pluto");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Starbase, 9),      game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "Pluto");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Storm, 4),         game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "Kathrina");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Minefield, 150),   game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "Deleted Mine Field #150");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ufo, 42),          game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "Hui");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Hull, 15),         game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "SMALL FREIGHTER");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Engine, 2),        game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "2-cyl. engine");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Beam, 3),          game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "Pink Laser");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Torpedo, 7),       game::PlainName, s), true);
    TS_ASSERT_EQUALS(s, "Mark 7 Torpedo");

    // Query detailed names
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(),                            game::DetailedName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Player, 3),        game::DetailedName, s), true);
    TS_ASSERT_EQUALS(s, "Player #3: The Romulans");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(game::map::Point(2000,3000)), game::DetailedName, s), true);
    TS_ASSERT_EQUALS(s, "(2000,3000)");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ship, 17),         game::DetailedName, s), true);
    TS_ASSERT_EQUALS(s, "Ship #17: Voyager");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Planet, 9),        game::DetailedName, s), true);
    TS_ASSERT_EQUALS(s, "Planet #9: Pluto");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Starbase, 9),      game::DetailedName, s), true);
    TS_ASSERT_EQUALS(s, "Starbase #9: Pluto");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Storm, 4),         game::DetailedName, s), true);
    TS_ASSERT_EQUALS(s, "Ion storm #4: Kathrina");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Minefield, 150),   game::DetailedName, s), true);
    TS_ASSERT_EQUALS(s, "Deleted Mine Field #150");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ufo, 42),          game::DetailedName, s), true);
    TS_ASSERT_EQUALS(s, "Ufo #42: Hui");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Hull, 15),         game::DetailedName, s), true);
    TS_ASSERT_EQUALS(s, "Hull #15: SMALL FREIGHTER");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Engine, 2),        game::DetailedName, s), true);
    TS_ASSERT_EQUALS(s, "Engine #2: 2-cyl. engine");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Beam, 3),          game::DetailedName, s), true);
    TS_ASSERT_EQUALS(s, "Beam Weapon #3: Pink Laser");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Torpedo, 7),       game::DetailedName, s), true);
    TS_ASSERT_EQUALS(s, "Torpedo Type #7: Mark 7 Torpedo");

    // Access off-by-one Ids (that is, container exists but object doesn't)
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(),                            game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Player, 4),        game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ship, 18),         game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Planet, 8),        game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Starbase, 8),      game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Storm, 5),         game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Minefield, 152),   game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ufo, 43),          game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Hull, 16),         game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Engine, 3),        game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Beam, 4),          game::PlainName, s), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Torpedo, 8),       game::PlainName, s), false);
}

/** Test InterpreterInterface implementation.
    A: create session. Call InterpreterInterface methods.
    E: correct results produced. */
void
TestGameSession::testInterpreterInterface()
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session testee(tx, fs);

    // Populate ship list
    afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
    shipList->hulls().create(3)->setName("SCOUT");
    testee.setShipList(shipList);

    // Populate root
    afl::base::Ptr<game::Root> root = new game::test::Root(game::HostVersion());
    root->playerList().create(5)->setName(game::Player::AdjectiveName, "Pirate");
    testee.setRoot(root);

    // Populate game
    afl::base::Ptr<game::Game> g = new game::Game();
    g->currentTurn().universe().ships().create(17)->setName("Voyager");
    testee.setGame(g);

    // Verify
    game::InterpreterInterface& iface = testee.interface();

    // - getComment(), hasTask() - return defaults because not configured in this test
    TS_ASSERT_EQUALS(iface.getComment(iface.Ship, 17), "");
    TS_ASSERT_EQUALS(iface.hasTask(iface.Ship, 17), false);

    // - getHullShortName
    String_t s;
    TS_ASSERT_EQUALS(iface.getHullShortName(3, s), true);
    TS_ASSERT_EQUALS(s, "SCOUT");

    TS_ASSERT_EQUALS(iface.getHullShortName(10, s), false);

    // - getPlayerAdjective
    TS_ASSERT_EQUALS(iface.getPlayerAdjective(5, s), true);
    TS_ASSERT_EQUALS(s, "Pirate");

    TS_ASSERT_EQUALS(iface.getPlayerAdjective(10, s), false);
}

/** Test task handling/inquiry. */
void
TestGameSession::testTask()
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session testee(tx, fs);

    // Populate root
    afl::base::Ptr<game::Root> root = new game::test::Root(game::HostVersion());
    testee.setRoot(root);

    // Populate game
    afl::base::Ptr<game::Game> g = new game::Game();
    game::map::Planet* p = g->currentTurn().universe().planets().create(17);
    testee.setGame(g);

    // Initial inquiry
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkPlanetTask, false), game::Session::NoTask);
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkBaseTask, false),   game::Session::NoTask);
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkPlanetTask, true),  game::Session::NoTask);
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkBaseTask, true),    game::Session::NoTask);

    // Create CC$AUTOEXEC mock (we only want the process to suspend)
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    bco->addArgument("A", false);
    bco->addInstruction(interpreter::Opcode::maSpecial, interpreter::Opcode::miSpecialSuspend, 0);
    testee.world().setNewGlobalValue("CC$AUTOEXEC", new interpreter::SubroutineValue(bco));

    // Create auto task (content doesn't matter; it's all given to CC$AUTOEXEC)
    afl::base::Ptr<interpreter::TaskEditor> editor = testee.getAutoTaskEditor(17, interpreter::Process::pkPlanetTask, true);
    TS_ASSERT(editor.get() != 0);
    String_t command[] = { "whatever" };
    editor->addAtEnd(command);
    editor->setPC(0);
    testee.releaseAutoTaskEditor(editor);

    // Inquiry
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkPlanetTask, false), game::Session::ActiveTask);
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkBaseTask, false),   game::Session::OtherTask);
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkPlanetTask, true),  game::Session::NoTask);
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkBaseTask, true),    game::Session::NoTask);
}

