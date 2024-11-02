/**
  *  \file test/game/sessiontest.cpp
  *  \brief Test for game::Session
  */

#include "game/session.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/data/access.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/loglistener.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/ufo.hpp"
#include "game/map/universe.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/root.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/turn.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/values.hpp"
#include <memory>

using afl::base::Ptr;
using afl::base::Ref;
using afl::except::FileProblemException;
using afl::io::FileSystem;
using afl::io::InternalDirectory;
using afl::io::InternalFileSystem;
using afl::io::NullFileSystem;
using afl::string::NullTranslator;
using game::Reference;
using game::Root;
using interpreter::Error;
using interpreter::Process;
using interpreter::World;

/** Test initialisation.
    A: create a session
    E: verify initial values */
AFL_TEST("game.Session:init", a)
{
    NullFileSystem fs;
    NullTranslator tx;
    game::Session testee(tx, fs);

    // Initial values
    a.checkEqual    ("01. translator",             testee.translator()("foo"), "foo");
    a.checkNull     ("02. getRoot",                testee.getRoot().get());
    a.checkNull     ("03. getShipList",            testee.getShipList().get());
    a.checkNull     ("04. getGame",                testee.getGame().get());
    a.checkDifferent("05. fileTable",              testee.world().fileTable().getFreeFile(), 0U);
    a.check         ("06. globalPropertyNames",    testee.world().globalPropertyNames().getIndexByName("HULL") != afl::data::NameMap::nil);
    a.checkEqual    ("07. getPluginDirectoryName", testee.getPluginDirectoryName(), "");
    a.checkEqual    ("08. getSystemInformation",   testee.getSystemInformation().numProcessors, 1U);

    // Plugin directory is modifiable
    testee.setPluginDirectoryName("/pp");
    a.checkEqual("11. getPluginDirectoryName", testee.getPluginDirectoryName(), "/pp");

    // System information is modifiable
    util::SystemInformation sysInfo;
    sysInfo.numProcessors = 3;
    testee.setSystemInformation(sysInfo);
    a.checkEqual("21. getSystemInformation", testee.getSystemInformation().numProcessors, 3U);
}

/** Test subobjects.
    A: create a session. Access subobjects.
    E: subobject references match */
AFL_TEST("game.Session:subobjects", a)
{
    NullFileSystem fs;
    NullTranslator tx;
    game::Session s(tx, fs);
    const game::Session& cs(s);

    a.checkEqual("01. translator", &s.translator(), &tx);
    // a.checkEqual("02. fileSystem", &s.world().fileSystem(), &fs); -- moved to next test

    a.checkEqual("11. uiPropertyStack", &s.uiPropertyStack(), &cs.uiPropertyStack());
    a.checkEqual("12. notifications", &s.notifications(), &cs.notifications());
    // a.checkEqual("13", &s.world(), &cs.world());
    a.checkEqual("14. processList", &s.processList(), &cs.processList());
}

/** Test file system subobject, metadata operations.
    Session now provides a virtualized file system.
    Verify its correct operation.
    A: create a session. Check FileSystem methods.
    E: correct result */
AFL_TEST("game.Session:filesystem:meta", a)
{
    InternalFileSystem fs;
    NullTranslator tx;
    game::Session s(tx, fs);

    FileSystem& sfs = s.world().fileSystem();

    // isPathSeparator
    a.check("01. isPathSeparator",  sfs.isPathSeparator('/'));
    a.check("02. isPathSeparator", !sfs.isPathSeparator('a'));

    // makePathName
    a.checkEqual("11. makePathName", sfs.makePathName("a",       "b"),  "a/b");
    a.checkEqual("12. makePathName", sfs.makePathName("a/",      "b"),  "a/b");
    a.checkEqual("13. makePathName", sfs.makePathName("game:a/", "b"),  "game:a/b");
    a.checkEqual("14. makePathName", sfs.makePathName("game:",   "b"),  "game:b");
    a.checkEqual("15. makePathName", sfs.makePathName("game:",   "/b"), "/b");
    a.checkEqual("16. makePathName", sfs.makePathName("game:.",  "c"),  "game:c");

    // getCanonicalPathName
    a.checkEqual("21. getCanonicalPathName", sfs.getCanonicalPathName("a/b/../c"),      "a/c");
    a.checkEqual("22. getCanonicalPathName", sfs.getCanonicalPathName("game:a/b/../c"), "game:a/c");

    // getAbsolutePathName
    a.checkEqual("21. getAbsolutePathName", sfs.getAbsolutePathName("a/b/../c"),      "/a/c");
    a.checkEqual("22. getAbsolutePathName", sfs.getAbsolutePathName("game:a/b/../c"), "game:a/c");

    // getFileName
    a.checkEqual("31. getFileName", sfs.getFileName("a/b/c"),    "c");
    a.checkEqual("32. getFileName", sfs.getFileName("a"),        "a");
    a.checkEqual("33. getFileName", sfs.getFileName("game:x"),   "x");
    a.checkEqual("34. getFileName", sfs.getFileName("game:x/y"), "y");

    // getDirectoryName
    a.checkEqual("41. getDirectoryName", sfs.getDirectoryName("a/b/c"),    "a/b");
    a.checkEqual("42. getDirectoryName", sfs.getDirectoryName("a"),        ".");
    a.checkEqual("43. getDirectoryName", sfs.getDirectoryName("game:x"),   "game:.");
    a.checkEqual("44. getDirectoryName", sfs.getDirectoryName("game:x/y"), "game:x");

    // getWorkingDirectoryName
    a.checkEqual("51. getWorkingDirectoryName", sfs.getWorkingDirectoryName(), fs.getWorkingDirectoryName());
}

/** Test opening files.
    A: create a session. Use open() methods
    E: calls targeted at FileSystem succeed, targeted at game fail */
AFL_TEST("game.Session:filesystem:open:no-game", a)
{
    InternalFileSystem fs;
    fs.openFile("/x", FileSystem::Create);
    fs.openFile("game:", FileSystem::Create);
    NullTranslator tx;
    game::Session s(tx, fs);

    FileSystem& sfs = s.world().fileSystem();

    // Success cases
    AFL_CHECK_SUCCEEDS(a("01. open root"), sfs.openFile("/x", FileSystem::OpenRead));
    AFL_CHECK_SUCCEEDS(a("02. read dir"),  sfs.openDirectory("/")->getDirectoryEntries());
    AFL_CHECK_SUCCEEDS(a("03. read root"), sfs.openRootDirectory()->getDirectoryEntries());

    // Error cases
    AFL_CHECK_THROWS(a("11. open root"), sfs.openFile("game:", FileSystem::OpenRead), FileProblemException);
    AFL_CHECK_THROWS(a("12. read dir"),  sfs.openDirectory("game:")->getDirectoryEntries(), FileProblemException);
}

/** Test opening files, with game present.
    A: create a session. Add root with game directory. Use open() methods
    E: calls targeted at game succeed */
AFL_TEST("game.Session:filesystem:open:game", a)
{
    InternalFileSystem fs;
    NullTranslator tx;
    game::Session s(tx, fs);

    // Root with game directory
    Ref<InternalDirectory> dir = InternalDirectory::create("gamedir");
    dir->openFile("a", FileSystem::Create);
    s.setRoot(new game::Root(dir,
                             *new game::test::SpecificationLoader(),
                             game::HostVersion(),
                             std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Registered, 9)),
                             std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                             std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                             game::Root::Actions_t()));

    FileSystem& sfs = s.world().fileSystem();

    // These now succeed
    AFL_CHECK_SUCCEEDS(a("01. open file"), sfs.openFile("game:a", FileSystem::OpenRead));
    AFL_CHECK_SUCCEEDS(a("02. open file"), sfs.openFile("game:/a", FileSystem::OpenRead));
    AFL_CHECK_SUCCEEDS(a("03. read dir"),  sfs.openDirectory("game:")->getDirectoryEntries());
    AFL_CHECK_SUCCEEDS(a("04. read dir"),  sfs.openDirectory("game:.")->getDirectoryEntries());
    AFL_CHECK_SUCCEEDS(a("05. read dir"),  sfs.openDirectory("game:/")->getDirectoryEntries());
}

/** Test getReferenceName().
    A: create empty session. Call getReferenceName().
    E: must report unknown for all objects */
AFL_TEST("game.Session:getReferenceName:empty", a)
{

    NullFileSystem fs;
    NullTranslator tx;
    game::Session testee(tx, fs);

    String_t s;
    a.checkEqual("01", testee.getReferenceName(Reference(),                            game::PlainName).isValid(), false);
    a.checkEqual("02", testee.getReferenceName(Reference(Reference::Player, 3),        game::PlainName).isValid(), false);
    a.checkEqual("03", testee.getReferenceName(Reference(game::map::Point(2000,3000)), game::PlainName).orElse(""), "(2000,3000)");
    a.checkEqual("04", testee.getReferenceName(Reference(Reference::Ship, 17),         game::PlainName).isValid(), false);
    a.checkEqual("05", testee.getReferenceName(Reference(Reference::Planet, 9),        game::PlainName).isValid(), false);
    a.checkEqual("06", testee.getReferenceName(Reference(Reference::Starbase, 9),      game::PlainName).isValid(), false);
    a.checkEqual("07", testee.getReferenceName(Reference(Reference::IonStorm, 4),      game::PlainName).isValid(), false);
    a.checkEqual("08", testee.getReferenceName(Reference(Reference::Minefield, 150),   game::PlainName).isValid(), false);
    a.checkEqual("09", testee.getReferenceName(Reference(Reference::Ufo, 42),          game::PlainName).isValid(), false);
    a.checkEqual("10", testee.getReferenceName(Reference(Reference::Hull, 15),         game::PlainName).isValid(), false);
    a.checkEqual("11", testee.getReferenceName(Reference(Reference::Engine, 2),        game::PlainName).isValid(), false);
    a.checkEqual("12", testee.getReferenceName(Reference(Reference::Beam, 3),          game::PlainName).isValid(), false);
    a.checkEqual("13", testee.getReferenceName(Reference(Reference::Torpedo, 7),       game::PlainName).isValid(), false);
}

/** Test getReferenceName().
    A: create session, add some objects. Call getReferenceName().
    E: must report correct names for all objects */
AFL_TEST("game.Session:getReferenceName:nonempty", a)
{
    NullFileSystem fs;
    NullTranslator tx;
    game::Session testee(tx, fs);

    // Populate ship list
    Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
    shipList->hulls().create(15)->setName("SMALL FREIGHTER");
    shipList->engines().create(2)->setName("2-cyl. engine");
    shipList->beams().create(3)->setName("Pink Laser");
    shipList->launchers().create(7)->setName("Mark 7 Torpedo");
    testee.setShipList(shipList);

    // Populate root
    Ptr<Root> root = game::test::makeRoot(game::HostVersion()).asPtr();
    root->playerList().create(3)->setName(game::Player::ShortName, "The Romulans");
    testee.setRoot(root);

    // Populate game
    Ptr<game::Game> g = new game::Game();
    g->currentTurn().universe().planets().create(9)->setName("Pluto");
    g->currentTurn().universe().planets().create(80)->setName("Sedna");
    g->currentTurn().universe().ships().create(17)->setName("Voyager");
    g->currentTurn().universe().ships().create(50)->setName("Pioneer");
    g->currentTurn().universe().ships().create(90);
    g->currentTurn().universe().ionStorms().create(4)->setName("Kathrina");
    g->currentTurn().universe().minefields().create(150);
    g->currentTurn().universe().ufos().addUfo(42, 1, 1)->setName("Hui");
    testee.setGame(g);

    testee.world().shipProperties()  .create(50)->setNew(World::sp_Comment, interpreter::makeStringValue("com s50"));
    testee.world().planetProperties().create(80)->setNew(World::pp_Comment, interpreter::makeStringValue("com p80"));

    // Query plain names
    a.checkEqual("01", testee.getReferenceName(Reference(),                            game::PlainName).isValid(), false);
    a.checkEqual("02", testee.getReferenceName(Reference(Reference::Player, 3),        game::PlainName).orElse(""), "The Romulans");
    a.checkEqual("03", testee.getReferenceName(Reference(game::map::Point(2000,3000)), game::PlainName).orElse(""), "(2000,3000)");
    a.checkEqual("04", testee.getReferenceName(Reference(Reference::Ship, 17),         game::PlainName).orElse(""), "Voyager");
    a.checkEqual("05", testee.getReferenceName(Reference(Reference::Planet, 9),        game::PlainName).orElse(""), "Pluto");
    a.checkEqual("06", testee.getReferenceName(Reference(Reference::Starbase, 9),      game::PlainName).orElse(""), "Pluto");
    a.checkEqual("07", testee.getReferenceName(Reference(Reference::IonStorm, 4),      game::PlainName).orElse(""), "Kathrina");
    a.checkEqual("08", testee.getReferenceName(Reference(Reference::Minefield, 150),   game::PlainName).orElse(""), "Deleted Mine Field #150");
    a.checkEqual("09", testee.getReferenceName(Reference(Reference::Ufo, 42),          game::PlainName).orElse(""), "Hui");
    a.checkEqual("10", testee.getReferenceName(Reference(Reference::Hull, 15),         game::PlainName).orElse(""), "SMALL FREIGHTER");
    a.checkEqual("11", testee.getReferenceName(Reference(Reference::Engine, 2),        game::PlainName).orElse(""), "2-cyl. engine");
    a.checkEqual("12", testee.getReferenceName(Reference(Reference::Beam, 3),          game::PlainName).orElse(""), "Pink Laser");
    a.checkEqual("13", testee.getReferenceName(Reference(Reference::Torpedo, 7),       game::PlainName).orElse(""), "Mark 7 Torpedo");

    // Query detailed names
    a.checkEqual("21", testee.getReferenceName(Reference(),                            game::DetailedName).isValid(), false);
    a.checkEqual("22", testee.getReferenceName(Reference(Reference::Player, 3),        game::DetailedName).orElse(""), "Player #3: The Romulans");
    a.checkEqual("23", testee.getReferenceName(Reference(game::map::Point(2000,3000)), game::DetailedName).orElse(""), "(2000,3000)");
    a.checkEqual("24", testee.getReferenceName(Reference(Reference::Ship, 17),         game::DetailedName).orElse(""), "Ship #17: Voyager");
    a.checkEqual("25", testee.getReferenceName(Reference(Reference::Planet, 9),        game::DetailedName).orElse(""), "Planet #9: Pluto");
    a.checkEqual("26", testee.getReferenceName(Reference(Reference::Starbase, 9),      game::DetailedName).orElse(""), "Starbase #9: Pluto");
    a.checkEqual("27", testee.getReferenceName(Reference(Reference::IonStorm, 4),      game::DetailedName).orElse(""), "Ion storm #4: Kathrina");
    a.checkEqual("28", testee.getReferenceName(Reference(Reference::Minefield, 150),   game::DetailedName).orElse(""), "Deleted Mine Field #150");
    a.checkEqual("29", testee.getReferenceName(Reference(Reference::Ufo, 42),          game::DetailedName).orElse(""), "Ufo #42: Hui");
    a.checkEqual("30", testee.getReferenceName(Reference(Reference::Hull, 15),         game::DetailedName).orElse(""), "Hull #15: SMALL FREIGHTER");
    a.checkEqual("31", testee.getReferenceName(Reference(Reference::Engine, 2),        game::DetailedName).orElse(""), "Engine #2: 2-cyl. engine");
    a.checkEqual("32", testee.getReferenceName(Reference(Reference::Beam, 3),          game::DetailedName).orElse(""), "Beam Weapon #3: Pink Laser");
    a.checkEqual("33", testee.getReferenceName(Reference(Reference::Torpedo, 7),       game::DetailedName).orElse(""), "Torpedo Type #7: Mark 7 Torpedo");
    a.checkEqual("34", testee.getReferenceName(Reference(Reference::Ship, 50),         game::DetailedName).orElse(""), "Ship #50: Pioneer: com s50");
    a.checkEqual("35", testee.getReferenceName(Reference(Reference::Planet, 80),       game::DetailedName).orElse(""), "Planet #80: Sedna: com p80");
    a.checkEqual("36", testee.getReferenceName(Reference(Reference::Starbase, 80),     game::DetailedName).orElse(""), "Starbase #80: Sedna: com p80");
    a.checkEqual("37", testee.getReferenceName(Reference(Reference::Ship, 90),         game::DetailedName).orElse(""), "Ship #90"); // Object exists but has no name

    // Access off-by-one Ids (that is, container exists but object doesn't)
    a.checkEqual("41", testee.getReferenceName(Reference(),                            game::PlainName).isValid(), false);
    a.checkEqual("42", testee.getReferenceName(Reference(Reference::Player, 4),        game::PlainName).isValid(), false);
    a.checkEqual("43", testee.getReferenceName(Reference(Reference::Ship, 18),         game::PlainName).isValid(), false);
    a.checkEqual("44", testee.getReferenceName(Reference(Reference::Planet, 8),        game::PlainName).isValid(), false);
    a.checkEqual("45", testee.getReferenceName(Reference(Reference::Starbase, 8),      game::PlainName).isValid(), false);
    a.checkEqual("46", testee.getReferenceName(Reference(Reference::IonStorm, 5),      game::PlainName).isValid(), false);
    a.checkEqual("47", testee.getReferenceName(Reference(Reference::Minefield, 152),   game::PlainName).isValid(), false);
    a.checkEqual("48", testee.getReferenceName(Reference(Reference::Ufo, 43),          game::PlainName).isValid(), false);
    a.checkEqual("49", testee.getReferenceName(Reference(Reference::Hull, 16),         game::PlainName).isValid(), false);
    a.checkEqual("50", testee.getReferenceName(Reference(Reference::Engine, 3),        game::PlainName).isValid(), false);
    a.checkEqual("51", testee.getReferenceName(Reference(Reference::Beam, 4),          game::PlainName).isValid(), false);
    a.checkEqual("52", testee.getReferenceName(Reference(Reference::Torpedo, 8),       game::PlainName).isValid(), false);
    a.checkEqual("53", testee.getReferenceName(Reference(Reference::Ship, 90),         game::PlainName).isValid(), false); // Object exists but has no name
}

/** Test InterpreterInterface implementation.
    A: create session. Call InterpreterInterface methods.
    E: correct results produced. */
AFL_TEST("game.Session:InterpreterInterface", a)
{
    NullFileSystem fs;
    NullTranslator tx;
    game::Session testee(tx, fs);

    // Populate ship list
    Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
    shipList->hulls().create(3)->setName("SCOUT");
    testee.setShipList(shipList);

    // Populate root
    Ptr<Root> root = game::test::makeRoot(game::HostVersion()).asPtr();
    root->playerList().create(5)->setName(game::Player::AdjectiveName, "Pirate");
    testee.setRoot(root);

    // Populate game
    Ptr<game::Game> g = new game::Game();
    g->currentTurn().universe().ships().create(17)->setName("Voyager");
    testee.setGame(g);

    // Verify
    game::InterpreterInterface& iface = testee.interface();

    // - getComment(), hasTask() - return defaults because not configured in this test
    a.checkEqual("01. getComment", iface.getComment(iface.Ship, 17), "");
    a.checkEqual("02. hasTask", iface.hasTask(iface.Ship, 17), false);

    // - getHullShortName
    a.checkEqual("11. getHullShortName", iface.getHullShortName(3).orElse(""), "SCOUT");
    a.checkEqual("12. getHullShortName", iface.getHullShortName(10).isValid(), false);

    // - getPlayerAdjective
    a.checkEqual("21. getPlayerAdjective", iface.getPlayerAdjective(5).orElse(""), "Pirate");
    a.checkEqual("22. getPlayerAdjective", iface.getPlayerAdjective(10).isValid(), false);
}

/** Test task handling/inquiry. */
AFL_TEST("game.Session:tasks", a)
{
    NullFileSystem fs;
    NullTranslator tx;
    game::Session testee(tx, fs);

    // Populate root
    Ptr<Root> root = game::test::makeRoot(game::HostVersion()).asPtr();
    testee.setRoot(root);

    // Populate game
    Ptr<game::Game> g = new game::Game();
    game::map::Planet* p = g->currentTurn().universe().planets().create(17);
    testee.setGame(g);

    // Initial inquiry
    a.checkEqual("01. getTaskStatus", testee.getTaskStatus(p, Process::pkPlanetTask, false), game::Session::NoTask);
    a.checkEqual("02. getTaskStatus", testee.getTaskStatus(p, Process::pkBaseTask, false),   game::Session::NoTask);
    a.checkEqual("03. getTaskStatus", testee.getTaskStatus(p, Process::pkPlanetTask, true),  game::Session::NoTask);
    a.checkEqual("04. getTaskStatus", testee.getTaskStatus(p, Process::pkBaseTask, true),    game::Session::NoTask);

    // Create CC$AUTOEXEC mock (we only want the process to suspend)
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    bco->addArgument("A", false);
    bco->addInstruction(interpreter::Opcode::maSpecial, interpreter::Opcode::miSpecialSuspend, 0);
    testee.world().setNewGlobalValue("CC$AUTOEXEC", new interpreter::SubroutineValue(bco));

    // Create auto task (content doesn't matter; it's all given to CC$AUTOEXEC)
    Ptr<interpreter::TaskEditor> editor = testee.getAutoTaskEditor(17, Process::pkPlanetTask, true);
    a.checkNonNull("11. getAutoTaskEditor", editor.get());
    String_t command[] = { "whatever" };
    editor->addAtEnd(command);
    editor->setPC(0);
    testee.releaseAutoTaskEditor(editor);

    // Inquiry
    a.checkEqual("21. getTaskStatus", testee.getTaskStatus(p, Process::pkPlanetTask, false), game::Session::ActiveTask);
    a.checkEqual("22. getTaskStatus", testee.getTaskStatus(p, Process::pkBaseTask, false),   game::Session::OtherTask);
    a.checkEqual("23. getTaskStatus", testee.getTaskStatus(p, Process::pkPlanetTask, true),  game::Session::NoTask);
    a.checkEqual("24. getTaskStatus", testee.getTaskStatus(p, Process::pkBaseTask, true),    game::Session::NoTask);
}

/** Test file character set handling. */
AFL_TEST("game.Session:charset", a)
{
    InternalFileSystem fs;
    NullTranslator tx;
    game::Session testee(tx, fs);

    // Initial file system content
    const char*const SCRIPT =
        "t := chr(246)\n"
        "open '/file.txt' for output as #1\n"
        "print #1, t\n"
        "close #1\n"
        "a := ''\n"
        "open '/data.dat' for output as #1\n"
        "setstr a, 0, 20, t\n"
        "put #1, a, 20\n"
        "close #1\n";
    fs.createDirectory("/gd");
    fs.openFile("/gd/t.q", FileSystem::Create)->fullWrite(afl::string::toBytes(SCRIPT));

    // Create a root. This sets the charset.
    testee.setRoot(new Root(fs.openDirectory("/gd"),
                            *new game::test::SpecificationLoader(),
                            game::HostVersion(),
                            std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Registered, 10)),
                            std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                            std::auto_ptr<afl::charset::Charset>(new afl::charset::CodepageCharset(afl::charset::g_codepage437)),
                            Root::Actions_t()));

    // Build a script process
    World& w = testee.world();
    Process& p = testee.processList().create(w, "testFileCharsetHandling");
    Ptr<afl::io::Stream> in = w.openLoadFile("t.q");
    a.checkNonNull("01. openLoadFile", in.get());               // Fails if Session/Root does not correctly provide the load directory
    p.pushFrame(w.compileFile(*in, "origin", 1), false);

    // Run the process
    uint32_t pgid = testee.processList().allocateProcessGroup();
    testee.processList().resumeProcess(p, pgid);
    testee.processList().startProcessGroup(pgid);
    testee.processList().run();

    // Verify
    a.checkEqual("11. getState", p.getState(), Process::Ended);

    // Verify file content
    uint8_t tmp[100];
    size_t n = 0;

    // - text file
    AFL_CHECK_SUCCEEDS(a("21. openFile"), n = fs.openFile("/file.txt", FileSystem::OpenRead)->read(tmp));
    a.checkGreaterEqual("22. size read", n, 2U); // at least two characters [first is payload, second (and more) for system newline]
    a.checkEqual("22. char", tmp[0], 0x94);      // 0x94 = U+00F6 in codepage 437, fails if Session/Root does not correctly provide the charset

    // - binary file
    AFL_CHECK_SUCCEEDS(a("31. openFile"), n = fs.openFile("/data.dat", FileSystem::OpenRead)->read(tmp));
    a.checkEqual("32. bytes read", n, 20U);
    a.checkEqual("33. content", tmp[0], 0x94);
    a.checkEqual("34. content", tmp[1], 0x20);
    a.checkEqual("35. content", tmp[2], 0x20);
    a.checkEqual("36. content", tmp[19], 0x20);
}

/** Test error logging. */
AFL_TEST("game.Session:logError", a)
{
    InternalFileSystem fs;
    NullTranslator tx;
    game::Session testee(tx, fs);

    afl::test::LogListener log;
    testee.log().addListener(log);

    testee.logError(Error::notAssignable());

    a.checkDifferent("01", log.getNumErrors(), 0U);
}
