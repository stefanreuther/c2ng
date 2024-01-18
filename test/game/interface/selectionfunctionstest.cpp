/**
  *  \file test/game/interface/selectionfunctionstest.cpp
  *  \brief Test for game::interface::SelectionFunctions
  */

#include "game/interface/selectionfunctions.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/valueverifier.hpp"

using afl::io::FileSystem;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

namespace {
    /*
     *  Test environment
     */
    struct Environment {
        afl::io::InternalFileSystem fs;
        afl::string::NullTranslator tx;
        game::Session session;
        interpreter::Process proc;

        Environment()
            : fs(), tx(), session(tx, fs), proc(session.world(), "tester", 777)
            { }
    };

    // Add a game
    void addGame(Environment& env)
    {
        afl::base::Ptr<game::Game> g = new game::Game();
        const game::PlayerSet_t availablePlayers(1);
        const int turnNr = 10;
        for (int i = 1; i < 50; ++i) {
            game::map::Planet* pl = g->currentTurn().universe().planets().create(i);
            pl->setPosition(game::map::Point(1000 + i, 2000 - i));
            pl->internalCheck(g->mapConfiguration(), availablePlayers, turnNr, env.tx, env.session.log());
        }
        for (int i = 1; i < 50; ++i) {
            game::map::Ship* sh = g->currentTurn().universe().ships().create(i);
            sh->addShipXYData(game::map::Point(2000 - i, 1000 + i), 2, 100, game::PlayerSet_t(1));
            sh->internalCheck(availablePlayers, turnNr);
        }
        g->currentTurn().setTimestamp(game::Timestamp(2021, 12, 24, 13, 50, 15));

        env.session.setGame(g);
    }

    // Mark ship on current layer
    void markShip(Environment& env, game::Id_t id)
    {
        env.session.getGame()->currentTurn().universe().ships().get(id)->setIsMarked(true);
    }

    // Mark planet on current layer
    void markPlanet(Environment& env, game::Id_t id)
    {
        env.session.getGame()->currentTurn().universe().planets().get(id)->setIsMarked(true);
    }

    // Check whether ship is marked on current layer
    bool isShipMarked(Environment& env, game::Id_t id)
    {
        return env.session.getGame()->currentTurn().universe().ships().get(id)->isMarked();
    }

    // Check whether planet is marked on current layer
    bool isPlanetMarked(Environment& env, game::Id_t id)
    {
        return env.session.getGame()->currentTurn().universe().planets().get(id)->isMarked();
    }

    // Check whether ship is marked on given layer
    bool isShipMarkedOnLayer(Environment& env, game::Id_t id, int layer)
    {
        return env.session.getGame()->selections().get(game::map::Selections::Ship, layer)->get(id);
    }

    // Check whether planet is marked on given layer
    bool isPlanetMarkedOnLayer(Environment& env, game::Id_t id, int layer)
    {
        return env.session.getGame()->selections().get(game::map::Selections::Planet, layer)->get(id);
    }

    // Open file
    void openFile(Environment& env, size_t fd, String_t name, FileSystem::OpenMode mode)
    {
        env.session.world().fileTable().openFile(fd, env.fs.openFile(name, mode));
    }

    // Close file. Required to flush buffered output.
    void closeFile(Environment& env, size_t fd)
    {
        env.session.world().fileTable().closeFile(fd);
    }

    // Get file position.
    int getFilePosition(afl::test::Assert a, Environment& env, size_t fd)
    {
        afl::io::TextFile* tf = env.session.world().fileTable().getFile(fd);
        a.checkNonNull("file is open", tf);
        return static_cast<int>(tf->getPos());
    }

    // Get file content
    String_t getFile(Environment& env, String_t name)
    {
        afl::base::Ref<afl::io::Stream> in(env.fs.openFile(name, FileSystem::OpenRead));
        afl::io::TextFile tf(*in);
        String_t result;
        String_t line;
        while (tf.readLine(line)) {
            result += line;
            result += "\n";
        }
        return result;
    }

    // Store file content
    void putFile(Environment& env, String_t name, String_t content)
    {
        env.fs.openFile(name, FileSystem::Create)
            ->fullWrite(afl::string::toBytes(content));
    }

    // Call CC$SelReadHeader
    afl::data::Value* callReadHeader(afl::test::Assert a, Environment& env, int fd)
    {
        afl::data::Segment seg;
        seg.pushBackInteger(fd);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> state(game::interface::IFCCSelReadHeader(env.session, args));
        a.checkNonNull("IFCCSelReadHeader returns non-null", state.get());

        interpreter::BaseValue* bv = dynamic_cast<interpreter::BaseValue*>(state.get());
        a.checkNonNull("IFCCSelReadHeader return value is BaseValue", bv);
        interpreter::test::ValueVerifier verif(*bv, a("CC$SelReadHeader state"));
        verif.verifyBasics();

        return state.release();
    }

    // Call CC$SelReadHeader with options
    afl::data::Value* callReadHeaderWithFlags(afl::test::Assert a, Environment& env, int fd, String_t opts)
    {
        afl::data::Segment seg;
        seg.pushBackInteger(fd);
        seg.pushBackString(opts);
        interpreter::Arguments args(seg, 0, 2);
        std::auto_ptr<afl::data::Value> state(game::interface::IFCCSelReadHeader(env.session, args));
        a.checkNonNull("IFCCSelReadHeader returns non-null", state.get());

        interpreter::BaseValue* bv = dynamic_cast<interpreter::BaseValue*>(state.get());
        a.checkNonNull("IFCCSelReadHeader return value is BaseValue", bv);
        interpreter::test::ValueVerifier verif(*bv, a("CC$SelReadHeader state"));
        verif.verifyBasics();

        return state.release();
    }

    // Call CC$SelGetQuestion
    afl::data::Value* callGetQuestion(Environment& env, afl::data::Value* state)
    {
        afl::data::Segment seg;
        seg.pushBackNew(state->clone());
        interpreter::Arguments args(seg, 0, 1);
        return game::interface::IFCCSelGetQuestion(env.session, args);
    }

    // Call CC$SelReadContent
    afl::data::Value* callReadContent(Environment& env, afl::data::Value* state)
    {
        afl::data::Segment seg;
        seg.pushBackNew(state->clone());
        interpreter::Arguments args(seg, 0, 1);
        return game::interface::IFCCSelReadContent(env.session, args);
    }

    // Default file content for single-layer file
    String_t defaultFile()
    {
        return "CCsel0 12-24-202113:50:15 1\n"
            "s25 1\n"
            "p35 1\n";
    }

    // Default file content for multi-layer file
    String_t multiFile()
    {
        return "CCsel0 12-24-202113:50:15 8\n"
            "s25 1\n"
            "p30 128\n"
            "p35 255\n";
    }
}

/*
 *  IFSelectionSave
 */

// Default case (save all)
AFL_TEST("game.interface.SelectionFunctions:IFSelectionSave:default", a)
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);
    openFile(env, 5, "/foo", afl::io::FileSystem::Create);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    interpreter::Arguments args(seg, 0, 1);
    game::interface::IFSelectionSave(env.session, env.proc, args);

    closeFile(env, 5);
    a.checkEqual("file content", getFile(env, "/foo"),
                 "CCsel0 12-24-202113:50:15 8\n"
                 "s20 1\n"
                 "p30 1\n");
}

// Save all, timeless
AFL_TEST("game.interface.SelectionFunctions:IFSelectionSave:timeless", a)
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);
    openFile(env, 5, "/foo", afl::io::FileSystem::Create);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    seg.pushBackString("t");
    interpreter::Arguments args(seg, 0, 2);
    game::interface::IFSelectionSave(env.session, env.proc, args);

    closeFile(env, 5);
    a.checkEqual("file content", getFile(env, "/foo"),
                 "CCsel0 - 8\n"
                 "s20 1\n"
                 "p30 1\n");
}

// Save one
AFL_TEST("game.interface.SelectionFunctions:IFSelectionSave:single-layer", a)
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);
    openFile(env, 5, "/foo", afl::io::FileSystem::Create);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    seg.pushBackInteger(0);
    interpreter::Arguments args(seg, 0, 2);
    game::interface::IFSelectionSave(env.session, env.proc, args);

    closeFile(env, 5);
    a.checkEqual("file content", getFile(env, "/foo"),
                 "CCsel0 12-24-202113:50:15 1\n"
                 "s20 1\n"
                 "p30 1\n");
}

// Save one, timeless
AFL_TEST("game.interface.SelectionFunctions:IFSelectionSave:single-timeless", a)
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);
    openFile(env, 5, "/foo", afl::io::FileSystem::Create);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    seg.pushBackString("t0");
    interpreter::Arguments args(seg, 0, 2);
    game::interface::IFSelectionSave(env.session, env.proc, args);

    closeFile(env, 5);
    a.checkEqual("file content", getFile(env, "/foo"),
                 "CCsel0 - 1\n"
                 "s20 1\n"
                 "p30 1\n");
}

// Error case: file not open
AFL_TEST("game.interface.SelectionFunctions:IFSelectionSave:error:not-open", a)
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFSelectionSave(env.session, env.proc, args), interpreter::Error);
}

// Error case: no game
AFL_TEST("game.interface.SelectionFunctions:IFSelectionSave:error:no-game", a)
{
    Environment env;
    openFile(env, 5, "/foo", afl::io::FileSystem::Create);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFSelectionSave(env.session, env.proc, args), game::Exception);

    closeFile(env, 5);
    a.checkEqual("file content", getFile(env, "/foo"), "");  // File has not been written
}

// Error case: arity error
AFL_TEST("game.interface.SelectionFunctions:IFSelectionSave:error:arity", a)
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);
    openFile(env, 5, "/foo", afl::io::FileSystem::Create);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFSelectionSave(env.session, env.proc, args), interpreter::Error);

    closeFile(env, 5);
    a.checkEqual("file content", getFile(env, "/foo"), "");  // File has not been written
}

// Error case: out of range index
AFL_TEST("game.interface.SelectionFunctions:IFSelectionSave:error:range", a)
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);
    openFile(env, 5, "/foo", afl::io::FileSystem::Create);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    seg.pushBackInteger(999);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFSelectionSave(env.session, env.proc, args), interpreter::Error);

    closeFile(env, 5);
    a.checkEqual("file content", getFile(env, "/foo"), "");  // File has not been written
}

// Null FD
AFL_TEST_NOARG("game.interface.SelectionFunctions:IFSelectionSave:null-fd")
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 1);
    game::interface::IFSelectionSave(env.session, env.proc, args);
}

/*
 *  Loading
 */


// Standard case: read a file, successfully
AFL_TEST("game.interface.SelectionFunctions:load:default", a)
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);
    putFile(env, "/test", defaultFile());
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeader(a, env, 7));
    verifyNewNull(a("question"), callGetQuestion(env, state.get()));
    verifyNewNull(a("read"), callReadContent(env, state.get()));

    a.check("01. isShipMarked",   !isShipMarked(env, 20));
    a.check("02. isShipMarked",    isShipMarked(env, 25));
    a.check("03. isPlanetMarked", !isPlanetMarked(env, 30));
    a.check("04. isPlanetMarked",  isPlanetMarked(env, 35));
}

// Read into different layer
AFL_TEST("game.interface.SelectionFunctions:load:target-layer", a)
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);
    putFile(env, "/test", defaultFile());
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(a, env, 7, "3"));
    verifyNewNull(a("question"), callGetQuestion(env, state.get()));
    verifyNewNull(a("read"), callReadContent(env, state.get()));

    a.check("01. isShipMarked",    isShipMarked(env, 20));
    a.check("02. isShipMarked",   !isShipMarked(env, 25));
    a.check("03. isShipMarked",    isShipMarkedOnLayer(env, 25, 3));
    a.check("04. isPlanetMarked",  isPlanetMarked(env, 30));
    a.check("05. isPlanetMarked", !isPlanetMarked(env, 35));
    a.check("06. isPlanetMarked",  isPlanetMarkedOnLayer(env, 35, 3));
}

// Merge
AFL_TEST("game.interface.SelectionFunctions:load:merge", a)
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);
    putFile(env, "/test", defaultFile());
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(a, env, 7, "m"));
    verifyNewNull(a("question"), callGetQuestion(env, state.get()));
    verifyNewNull(a("read"), callReadContent(env, state.get()));

    a.check("01. isShipMarked",   isShipMarked(env, 20));
    a.check("02. isShipMarked",   isShipMarked(env, 25));
    a.check("03. isPlanetMarked", isPlanetMarked(env, 30));
    a.check("04. isPlanetMarked", isPlanetMarked(env, 35));
}

// Timeless file
AFL_TEST("game.interface.SelectionFunctions:load:timeless", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test",
            "CCsel0 - 1\n"
            "s25 1\n"
            "p35 1\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeader(a, env, 7));
    verifyNewNull(a("question"), callGetQuestion(env, state.get()));
    verifyNewNull(a("read"), callReadContent(env, state.get()));

    a.check("01. isShipMarked",   isShipMarked(env, 25));
    a.check("02. isPlanetMarked", isPlanetMarked(env, 35));
}

// Mismatching timestamp
AFL_TEST("game.interface.SelectionFunctions:load:error:timestamp-mismatch", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test",
            "CCsel0 12-25-202113:50:15 1\n"
            "s25 1\n"
            "p35 1\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    AFL_CHECK_THROWS(a("read header"), callReadHeader(a, env, 7), interpreter::Error);
    a.checkEqual("getFilePosition", getFilePosition(a, env, 7), 0);
}

// Accepting mismatching timestamp
AFL_TEST("game.interface.SelectionFunctions:load:timestamp-ignored", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test",
            "CCsel0 12-25-202113:50:15 1\n"
            "s25 1\n"
            "p35 1\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(a, env, 7, "t"));
    verifyNewNull(a("question"), callGetQuestion(env, state.get()));
    verifyNewNull(a("read"), callReadContent(env, state.get()));

    a.check("01. isShipMarked",   isShipMarked(env, 25));
    a.check("02. isPlanetMarked", isPlanetMarked(env, 35));
}

// Mismatching timestamp with UI
AFL_TEST("game.interface.SelectionFunctions:load:timestamp-mismatch:ui", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test",
            "CCsel0 12-25-202113:50:15 1\n"
            "s25 1\n"
            "p35 1\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(a, env, 7, "u"));
    a.checkDifferent("question", verifyNewString(a("question"), callGetQuestion(env, state.get())), "");
    verifyNewNull(a("read"), callReadContent(env, state.get()));

    a.check("01. isShipMarked",   isShipMarked(env, 25));
    a.check("02. isPlanetMarked", isPlanetMarked(env, 35));
}

// Multiple layers
AFL_TEST("game.interface.SelectionFunctions:load:multilayer-file", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test", multiFile());
    openFile(env, 7, "/test", FileSystem::OpenRead);

    AFL_CHECK_THROWS(a, callReadHeader(a, env, 7), interpreter::Error);
    a.checkEqual("getFilePosition", getFilePosition(a, env, 7), 0);
}

// Accept multiple layers
AFL_TEST("game.interface.SelectionFunctions:load:multilayer-accepted", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test", multiFile());
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(a, env, 7, "a"));
    verifyNewNull(a("question"), callGetQuestion(env, state.get()));
    verifyNewNull(a("read"), callReadContent(env, state.get()));

    a.check("01",  isShipMarked(env, 25));
    a.check("02",  isPlanetMarkedOnLayer(env, 30, 7));
    a.check("03", !isPlanetMarkedOnLayer(env, 30, 6));
    a.check("04",  isPlanetMarked(env, 35));
    a.check("05",  isPlanetMarkedOnLayer(env, 35, 7));
    a.check("06",  isPlanetMarkedOnLayer(env, 35, 6));
}

// Multiple layers, UI
AFL_TEST("game.interface.SelectionFunctions:load:multilayer-ui", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test", multiFile());
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(a, env, 7, "u"));
    a.checkDifferent("question", verifyNewString(a("question"), callGetQuestion(env, state.get())), "");
    verifyNewNull(a("read"), callReadContent(env, state.get()));

    a.check("01. isShipMarked",   isShipMarked(env, 25));
    a.check("02. isPlanetMarked", isPlanetMarked(env, 35));
}

// Multiple layers, timeless, UI
AFL_TEST("game.interface.SelectionFunctions:load:multilayer-timeless-ui", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test",
            "CCsel0 - 8\n"
            "s25 1\n"
            "p30 128\n"
            "p35 255\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(a, env, 7, "u"));
    a.checkDifferent("question", verifyNewString(a("question"), callGetQuestion(env, state.get())), "");
    verifyNewNull(a("read"), callReadContent(env, state.get()));

    a.check("01. isShipMarked",   isShipMarked(env, 25));
    a.check("02. isPlanetMarked", isPlanetMarked(env, 35));
}

// Read a file with EOF marker
AFL_TEST("game.interface.SelectionFunctions:load:eof-marker", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test",
            "CCsel0 12-24-202113:50:15 1\n"
            "s25 1\n"
            "p35 1\n"
            "}\n"
            "next\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeader(a, env, 7));
    verifyNewNull(a("read"), callReadContent(env, state.get()));

    a.check("01. isShipMarked",   isShipMarked(env, 25));
    a.check("02. isPlanetMarked", isPlanetMarked(env, 35));

    std::string line;
    a.check("11. readLine", env.session.world().fileTable().getFile(7)->readLine(line));
    a.checkEqual("12. content", line, "next");
}

// Syntax error: bad type
AFL_TEST("game.interface.SelectionFunctions:load:error:file-content:bad-type", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test",
            "CCsel0 12-24-202113:50:15 1\n"
            "s25 1\n"
            "x99 1\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeader(a, env, 7));
    AFL_CHECK_THROWS(a("read"), callReadContent(env, state.get()), afl::except::FileProblemException);
    a.checkEqual("getFilePosition", getFilePosition(a, env, 7), 0);

    a.check("isShipMarked", !isShipMarked(env, 25));    // No modification
}

// Syntax error: bad Id
AFL_TEST("game.interface.SelectionFunctions:load:error:file-content:bad-id", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test",
            "CCsel0 12-24-202113:50:15 1\n"
            "s25 1\n"
            "p51 1\n");                   // limit is 50
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeader(a, env, 7));
    AFL_CHECK_THROWS(a("read"), callReadContent(env, state.get()), afl::except::FileProblemException);
    a.checkEqual("getFilePosition", getFilePosition(a, env, 7), 0);

    a.check("isShipMarked", !isShipMarked(env, 25));    // No modification
}

// Syntax error: no separator
AFL_TEST("game.interface.SelectionFunctions:load:error:file-content:no-separator", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test",
            "CCsel0 12-24-202113:50:15 1\n"
            "s25 1\n"
            "p35\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeader(a, env, 7));
    AFL_CHECK_THROWS(a("read"), callReadContent(env, state.get()), afl::except::FileProblemException);
    a.checkEqual("getFilePosition", getFilePosition(a, env, 7), 0);

    a.check("isShipMarked", !isShipMarked(env, 25));    // No modification
}

// Syntax error: missing Id
AFL_TEST("game.interface.SelectionFunctions:load:error:file-content:no-id", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test",
            "CCsel0 12-24-202113:50:15 1\n"
            "s25 1\n"
            "p 1\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeader(a, env, 7));
    AFL_CHECK_THROWS(a("read"), callReadContent(env, state.get()), afl::except::FileProblemException);
    a.checkEqual("getFilePosition", getFilePosition(a, env, 7), 0);

    a.check("isShipMarked", !isShipMarked(env, 25));    // No modification
}

// Syntax error: bad mask
AFL_TEST("game.interface.SelectionFunctions:load:error:file-content:bad-mask", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test",
            "CCsel0 12-24-202113:50:15 1\n"
            "s25 1\n"
            "p35 999999\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeader(a, env, 7));
    AFL_CHECK_THROWS(a("read"), callReadContent(env, state.get()), afl::except::FileProblemException);
    a.checkEqual("getFilePosition", getFilePosition(a, env, 7), 0);

    a.check("isShipMarked", !isShipMarked(env, 25));    // No modification
}

// Error: bad target layer
AFL_TEST("game.interface.SelectionFunctions:load:error:bad-target", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test", "CCsel0 12-24-202113:50:15 1\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    AFL_CHECK_THROWS(a("read"), callReadHeaderWithFlags(a, env, 7, "99"), interpreter::Error);
    a.checkEqual("getFilePosition", getFilePosition(a, env, 7), 0);
}

// Null fd
AFL_TEST("game.interface.SelectionFunctions:load:null-fd", a)
{
    Environment env;
    addGame(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull(a, game::interface::IFCCSelReadHeader(env.session, args));
}

// Bad fd
AFL_TEST("game.interface.SelectionFunctions:load:error:bad-fd", a)
{
    Environment env;
    addGame(env);

    afl::data::Segment seg;
    seg.pushBackString("X");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFCCSelReadHeader(env.session, args), interpreter::Error);
}

// Bad signature
AFL_TEST("game.interface.SelectionFunctions:load:error:file-content:bad-signature", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test", "CCsel99 12-24-202113:50:15 1\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    AFL_CHECK_THROWS(a, callReadHeader(a, env, 7), afl::except::FileProblemException);
}

// No signature
AFL_TEST("game.interface.SelectionFunctions:load:error:file-content:missing-signature", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test", "");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    AFL_CHECK_THROWS(a, callReadHeader(a, env, 7), afl::except::FileProblemException);
}

// Bad layer count
AFL_TEST("game.interface.SelectionFunctions:load:error:file-content:bad-layer-count", a)
{
    Environment env;
    addGame(env);
    putFile(env, "/test", "CCsel0 12-24-202113:50:15 99\n");
    openFile(env, 7, "/test", FileSystem::OpenRead);

    AFL_CHECK_THROWS(a, callReadHeader(a, env, 7), afl::except::FileProblemException);
}

// Error case: file not open
AFL_TEST("game.interface.SelectionFunctions:load:error:file-not-open", a)
{
    Environment env;
    addGame(env);
    AFL_CHECK_THROWS(a, callReadHeader(a, env, 7), interpreter::Error);
}

// Error case: no game
AFL_TEST("game.interface.SelectionFunctions:load:error:no-game", a)
{
    Environment env;
    putFile(env, "/test", defaultFile());
    openFile(env, 7, "/test", FileSystem::OpenRead);
    AFL_CHECK_THROWS(a, callReadHeader(a, env, 7), game::Exception);
}

// Error case: no game for CC$SelReadContent
AFL_TEST("game.interface.SelectionFunctions:load:error:no-game-for-content", a)
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);
    putFile(env, "/test", defaultFile());
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeader(a, env, 7));
    env.session.setGame(0);
    AFL_CHECK_THROWS(a, callReadContent(env, state.get()), game::Exception);
}

// Error case: file closed for CC$SelReadContent
AFL_TEST("game.interface.SelectionFunctions:load:error:file-not-open-for-content", a)
{
    Environment env;
    addGame(env);
    markShip(env, 20);
    markPlanet(env, 30);
    putFile(env, "/test", defaultFile());
    openFile(env, 7, "/test", FileSystem::OpenRead);

    std::auto_ptr<afl::data::Value> state(callReadHeader(a, env, 7));
    env.session.world().fileTable().closeFile(7);
    AFL_CHECK_THROWS(a, callReadContent(env, state.get()), interpreter::Error);
}

// Error case: bad state
AFL_TEST("game.interface.SelectionFunctions:load:error:bad-state", a)
{
    Environment env;
    addGame(env);

    afl::data::IntegerValue iv(10);
    AFL_CHECK_THROWS(a, callGetQuestion(env, &iv), interpreter::Error);
    AFL_CHECK_THROWS(a, callReadContent(env, &iv), interpreter::Error);
}
