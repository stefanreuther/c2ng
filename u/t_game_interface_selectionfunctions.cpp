/**
  *  \file u/t_game_interface_selectionfunctions.cpp
  *  \brief Test for game::interface::SelectionFunctions
  */

#include "game/interface/selectionfunctions.hpp"

#include "t_game_interface.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
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
    int getFilePosition(Environment& env, size_t fd)
    {
        afl::io::TextFile* tf = env.session.world().fileTable().getFile(fd);
        TS_ASSERT(tf != 0);
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
    afl::data::Value* callReadHeader(Environment& env, int fd)
    {
        afl::data::Segment seg;
        seg.pushBackInteger(fd);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> state(game::interface::IFCCSelReadHeader(env.session, args));
        TS_ASSERT(state.get() != 0);

        interpreter::BaseValue* bv = dynamic_cast<interpreter::BaseValue*>(state.get());
        TS_ASSERT(bv != 0);
        interpreter::test::ValueVerifier verif(*bv, "CC$SelReadHeader state");
        verif.verifyBasics();

        return state.release();
    }

    // Call CC$SelReadHeader with options
    afl::data::Value* callReadHeaderWithFlags(Environment& env, int fd, String_t opts)
    {
        afl::data::Segment seg;
        seg.pushBackInteger(fd);
        seg.pushBackString(opts);
        interpreter::Arguments args(seg, 0, 2);
        std::auto_ptr<afl::data::Value> state(game::interface::IFCCSelReadHeader(env.session, args));
        TS_ASSERT(state.get() != 0);

        interpreter::BaseValue* bv = dynamic_cast<interpreter::BaseValue*>(state.get());
        TS_ASSERT(bv != 0);
        interpreter::test::ValueVerifier verif(*bv, "CC$SelReadHeader state");
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

/** Test IFSelectionSave(). */
void
TestGameInterfaceSelectionFunctions::testSelectionSave()
{
    // Default case (save all)
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
        TS_ASSERT_EQUALS(getFile(env, "/foo"),
                         "CCsel0 12-24-202113:50:15 8\n"
                         "s20 1\n"
                         "p30 1\n");
    }

    // Save all, timeless
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
        TS_ASSERT_EQUALS(getFile(env, "/foo"),
                         "CCsel0 - 8\n"
                         "s20 1\n"
                         "p30 1\n");
    }

    // Save one
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
        TS_ASSERT_EQUALS(getFile(env, "/foo"),
                         "CCsel0 12-24-202113:50:15 1\n"
                         "s20 1\n"
                         "p30 1\n");
    }

    // Save one, timeless
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
        TS_ASSERT_EQUALS(getFile(env, "/foo"),
                         "CCsel0 - 1\n"
                         "s20 1\n"
                         "p30 1\n");
    }

    // Error case: file not open
    {
        Environment env;
        addGame(env);
        markShip(env, 20);
        markPlanet(env, 30);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFSelectionSave(env.session, env.proc, args), interpreter::Error);
    }

    // Error case: no game
    {
        Environment env;
        openFile(env, 5, "/foo", afl::io::FileSystem::Create);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFSelectionSave(env.session, env.proc, args), game::Exception);

        closeFile(env, 5);
        TS_ASSERT_EQUALS(getFile(env, "/foo"), "");  // File has not been written
    }

    // Error case: arity error
    {
        Environment env;
        addGame(env);
        markShip(env, 20);
        markPlanet(env, 30);
        openFile(env, 5, "/foo", afl::io::FileSystem::Create);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFSelectionSave(env.session, env.proc, args), interpreter::Error);

        closeFile(env, 5);
        TS_ASSERT_EQUALS(getFile(env, "/foo"), "");  // File has not been written
    }

    // Error case: out of range index
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
        TS_ASSERT_THROWS(game::interface::IFSelectionSave(env.session, env.proc, args), interpreter::Error);

        closeFile(env, 5);
        TS_ASSERT_EQUALS(getFile(env, "/foo"), "");  // File has not been written
    }

    // Null FD
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
}

/** Test selection loading. */
void
TestGameInterfaceSelectionFunctions::testSelectionLoad()
{
    // Standard case: read a file, successfully
    {
        Environment env;
        addGame(env);
        markShip(env, 20);
        markPlanet(env, 30);
        putFile(env, "/test", defaultFile());
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeader(env, 7));
        verifyNewNull("default question", callGetQuestion(env, state.get()));
        verifyNewNull("default read", callReadContent(env, state.get()));

        TS_ASSERT(!isShipMarked(env, 20));
        TS_ASSERT( isShipMarked(env, 25));
        TS_ASSERT(!isPlanetMarked(env, 30));
        TS_ASSERT( isPlanetMarked(env, 35));
    }

    // Read into different layer
    {
        Environment env;
        addGame(env);
        markShip(env, 20);
        markPlanet(env, 30);
        putFile(env, "/test", defaultFile());
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(env, 7, "3"));
        verifyNewNull("target question", callGetQuestion(env, state.get()));
        verifyNewNull("target read", callReadContent(env, state.get()));

        TS_ASSERT( isShipMarked(env, 20));
        TS_ASSERT(!isShipMarked(env, 25));
        TS_ASSERT( isShipMarkedOnLayer(env, 25, 3));
        TS_ASSERT( isPlanetMarked(env, 30));
        TS_ASSERT(!isPlanetMarked(env, 35));
        TS_ASSERT( isPlanetMarkedOnLayer(env, 35, 3));
    }

    // Merge
    {
        Environment env;
        addGame(env);
        markShip(env, 20);
        markPlanet(env, 30);
        putFile(env, "/test", defaultFile());
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(env, 7, "m"));
        verifyNewNull("merge question", callGetQuestion(env, state.get()));
        verifyNewNull("merge read", callReadContent(env, state.get()));

        TS_ASSERT(isShipMarked(env, 20));
        TS_ASSERT(isShipMarked(env, 25));
        TS_ASSERT(isPlanetMarked(env, 30));
        TS_ASSERT(isPlanetMarked(env, 35));
    }

    // Timeless file
    {
        Environment env;
        addGame(env);
        putFile(env, "/test",
                "CCsel0 - 1\n"
                "s25 1\n"
                "p35 1\n");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeader(env, 7));
        verifyNewNull("timeless question", callGetQuestion(env, state.get()));
        verifyNewNull("timeless read", callReadContent(env, state.get()));

        TS_ASSERT(isShipMarked(env, 25));
        TS_ASSERT(isPlanetMarked(env, 35));
    }

    // Mismatching timestamp
    {
        Environment env;
        addGame(env);
        putFile(env, "/test",
                "CCsel0 12-25-202113:50:15 1\n"
                "s25 1\n"
                "p35 1\n");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        TS_ASSERT_THROWS(callReadHeader(env, 7), interpreter::Error);
        TS_ASSERT_EQUALS(getFilePosition(env, 7), 0);
    }

    // Accepting mismatching timestamp
    {
        Environment env;
        addGame(env);
        putFile(env, "/test",
                "CCsel0 12-25-202113:50:15 1\n"
                "s25 1\n"
                "p35 1\n");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(env, 7, "t"));
        verifyNewNull("mismatch question", callGetQuestion(env, state.get()));
        verifyNewNull("mismatch read", callReadContent(env, state.get()));

        TS_ASSERT(isShipMarked(env, 25));
        TS_ASSERT(isPlanetMarked(env, 35));
    }

    // Mismatching timestamp with UI
    {
        Environment env;
        addGame(env);
        putFile(env, "/test",
                "CCsel0 12-25-202113:50:15 1\n"
                "s25 1\n"
                "p35 1\n");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(env, 7, "u"));
        TS_ASSERT_DIFFERS(verifyNewString("ui question", callGetQuestion(env, state.get())), "");
        verifyNewNull("ui read", callReadContent(env, state.get()));

        TS_ASSERT(isShipMarked(env, 25));
        TS_ASSERT(isPlanetMarked(env, 35));
    }

    // Multiple layers
    {
        Environment env;
        addGame(env);
        putFile(env, "/test", multiFile());
        openFile(env, 7, "/test", FileSystem::OpenRead);

        TS_ASSERT_THROWS(callReadHeader(env, 7), interpreter::Error);
        TS_ASSERT_EQUALS(getFilePosition(env, 7), 0);
    }

    // Accept multiple layers
    {
        Environment env;
        addGame(env);
        putFile(env, "/test", multiFile());
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(env, 7, "a"));
        verifyNewNull("multi question", callGetQuestion(env, state.get()));
        verifyNewNull("multi read", callReadContent(env, state.get()));

        TS_ASSERT( isShipMarked(env, 25));
        TS_ASSERT( isPlanetMarkedOnLayer(env, 30, 7));
        TS_ASSERT(!isPlanetMarkedOnLayer(env, 30, 6));
        TS_ASSERT( isPlanetMarked(env, 35));
        TS_ASSERT( isPlanetMarkedOnLayer(env, 35, 7));
        TS_ASSERT( isPlanetMarkedOnLayer(env, 35, 6));
    }

    // Multiple layers, UI
    {
        Environment env;
        addGame(env);
        putFile(env, "/test", multiFile());
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(env, 7, "u"));
        TS_ASSERT_DIFFERS(verifyNewString("multi ui question", callGetQuestion(env, state.get())), "");
        verifyNewNull("multi ui read", callReadContent(env, state.get()));

        TS_ASSERT(isShipMarked(env, 25));
        TS_ASSERT(isPlanetMarked(env, 35));
    }

    // Multiple layers, timeless, UI
    {
        Environment env;
        addGame(env);
        putFile(env, "/test",
                "CCsel0 - 8\n"
                "s25 1\n"
                "p30 128\n"
                "p35 255\n");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeaderWithFlags(env, 7, "u"));
        TS_ASSERT_DIFFERS(verifyNewString("timeless multi ui question", callGetQuestion(env, state.get())), "");
        verifyNewNull("timeless multi ui read", callReadContent(env, state.get()));

        TS_ASSERT(isShipMarked(env, 25));
        TS_ASSERT(isPlanetMarked(env, 35));
    }

    // Read a file with EOF marker
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

        std::auto_ptr<afl::data::Value> state(callReadHeader(env, 7));
        verifyNewNull("eof read", callReadContent(env, state.get()));

        TS_ASSERT(isShipMarked(env, 25));
        TS_ASSERT(isPlanetMarked(env, 35));

        std::string line;
        TS_ASSERT(env.session.world().fileTable().getFile(7)->readLine(line));
        TS_ASSERT_EQUALS(line, "next");
    }

    // Syntax error: bad type
    {
        Environment env;
        addGame(env);
        putFile(env, "/test",
                "CCsel0 12-24-202113:50:15 1\n"
                "s25 1\n"
                "x99 1\n");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeader(env, 7));
        TS_ASSERT_THROWS(callReadContent(env, state.get()), afl::except::FileProblemException);
        TS_ASSERT_EQUALS(getFilePosition(env, 7), 0);

        TS_ASSERT(!isShipMarked(env, 25));    // No modification
    }

    // Syntax error: bad Id
    {
        Environment env;
        addGame(env);
        putFile(env, "/test",
                "CCsel0 12-24-202113:50:15 1\n"
                "s25 1\n"
                "p51 1\n");                   // limit is 50
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeader(env, 7));
        TS_ASSERT_THROWS(callReadContent(env, state.get()), afl::except::FileProblemException);
        TS_ASSERT_EQUALS(getFilePosition(env, 7), 0);

        TS_ASSERT(!isShipMarked(env, 25));    // No modification
    }

    // Syntax error: no separator
    {
        Environment env;
        addGame(env);
        putFile(env, "/test",
                "CCsel0 12-24-202113:50:15 1\n"
                "s25 1\n"
                "p35\n");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeader(env, 7));
        TS_ASSERT_THROWS(callReadContent(env, state.get()), afl::except::FileProblemException);
        TS_ASSERT_EQUALS(getFilePosition(env, 7), 0);

        TS_ASSERT(!isShipMarked(env, 25));    // No modification
    }

    // Syntax error: missing Id
    {
        Environment env;
        addGame(env);
        putFile(env, "/test",
                "CCsel0 12-24-202113:50:15 1\n"
                "s25 1\n"
                "p 1\n");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeader(env, 7));
        TS_ASSERT_THROWS(callReadContent(env, state.get()), afl::except::FileProblemException);
        TS_ASSERT_EQUALS(getFilePosition(env, 7), 0);

        TS_ASSERT(!isShipMarked(env, 25));    // No modification
    }

    // Syntax error: bad mask
    {
        Environment env;
        addGame(env);
        putFile(env, "/test",
                "CCsel0 12-24-202113:50:15 1\n"
                "s25 1\n"
                "p35 999999\n");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeader(env, 7));
        TS_ASSERT_THROWS(callReadContent(env, state.get()), afl::except::FileProblemException);
        TS_ASSERT_EQUALS(getFilePosition(env, 7), 0);

        TS_ASSERT(!isShipMarked(env, 25));    // No modification
    }

    // Error: bad target layer
    {
        Environment env;
        addGame(env);
        putFile(env, "/test", "CCsel0 12-24-202113:50:15 1\n");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        TS_ASSERT_THROWS(callReadHeaderWithFlags(env, 7, "99"), interpreter::Error);
        TS_ASSERT_EQUALS(getFilePosition(env, 7), 0);
    }

    // Null fd
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        verifyNewNull("null fd", game::interface::IFCCSelReadHeader(env.session, args));
    }

    // Bad fd
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFCCSelReadHeader(env.session, args), interpreter::Error);
    }

    // Bad signature
    {
        Environment env;
        addGame(env);
        putFile(env, "/test", "CCsel99 12-24-202113:50:15 1\n");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        TS_ASSERT_THROWS(callReadHeader(env, 7), afl::except::FileProblemException);
    }

    // No signature
    {
        Environment env;
        addGame(env);
        putFile(env, "/test", "");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        TS_ASSERT_THROWS(callReadHeader(env, 7), afl::except::FileProblemException);
    }

    // Bad layer count
    {
        Environment env;
        addGame(env);
        putFile(env, "/test", "CCsel0 12-24-202113:50:15 99\n");
        openFile(env, 7, "/test", FileSystem::OpenRead);

        TS_ASSERT_THROWS(callReadHeader(env, 7), afl::except::FileProblemException);
    }

    // Error case: file not open
    {
        Environment env;
        addGame(env);
        TS_ASSERT_THROWS(callReadHeader(env, 7), interpreter::Error);
    }

    // Error case: no game
    {
        Environment env;
        putFile(env, "/test", defaultFile());
        openFile(env, 7, "/test", FileSystem::OpenRead);
        TS_ASSERT_THROWS(callReadHeader(env, 7), game::Exception);
    }

    // Error case: no game for CC$SelReadContent
    {
        Environment env;
        addGame(env);
        markShip(env, 20);
        markPlanet(env, 30);
        putFile(env, "/test", defaultFile());
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeader(env, 7));
        env.session.setGame(0);
        TS_ASSERT_THROWS(callReadContent(env, state.get()), game::Exception);
    }

    // Error case: file closed for CC$SelReadContent
    {
        Environment env;
        addGame(env);
        markShip(env, 20);
        markPlanet(env, 30);
        putFile(env, "/test", defaultFile());
        openFile(env, 7, "/test", FileSystem::OpenRead);

        std::auto_ptr<afl::data::Value> state(callReadHeader(env, 7));
        env.session.world().fileTable().closeFile(7);
        TS_ASSERT_THROWS(callReadContent(env, state.get()), interpreter::Error);
    }

    // Error case: bad state
    {
        Environment env;
        addGame(env);

        afl::data::IntegerValue iv(10);
        TS_ASSERT_THROWS(callGetQuestion(env, &iv), interpreter::Error);
        TS_ASSERT_THROWS(callReadContent(env, &iv), interpreter::Error);
    }
}

