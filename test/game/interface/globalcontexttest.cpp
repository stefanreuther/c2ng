/**
  *  \file test/game/interface/globalcontexttest.cpp
  *  \brief Test for game::interface::GlobalContext
  */

#include "game/interface/globalcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/values.hpp"

namespace {
    /*
     *  UserInterfacePropertyAccessor implementation that publishes the iuiScreenNumber property
     */
    class ScreenNumberMock : public game::interface::UserInterfacePropertyAccessor {
     public:
        ScreenNumberMock()
            : m_number(4)
            { }
        virtual bool get(game::interface::UserInterfaceProperty prop, std::auto_ptr<afl::data::Value>& result)
            {
                if (prop == game::interface::iuiScreenNumber) {
                    result.reset(interpreter::makeIntegerValue(m_number));
                    return true;
                } else {
                    return false;
                }
            }
        virtual bool set(game::interface::UserInterfaceProperty prop, const afl::data::Value* p)
            {
                if (prop == game::interface::iuiScreenNumber) {
                    interpreter::checkIntegerArg(m_number, p);
                    return true;
                } else {
                    return false;
                }
            }
        int32_t get() const
            { return m_number; }
     private:
        int32_t m_number;
    };
}

/** General tests.
    Tests property access with all required objects present. */
AFL_TEST("game.interface.GlobalContext:general", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    session.rng().setSeed(55);
    session.world().setNewGlobalValue("GV", interpreter::makeStringValue("t"));

    // - Game (for turn, viewpoint player)
    afl::base::Ptr<game::Game> g = new game::Game();
    g->currentTurn().setTurnNumber(42);
    g->setViewpointPlayer(5);
    session.setGame(g);

    // - Root (required for player properties)
    afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion()).asPtr();
    session.setRoot(r);

    // - UI
    ScreenNumberMock ui;
    session.uiPropertyStack().add(ui);

    // General properties
    game::interface::GlobalContext testee(session);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifySerializable(interpreter::TagNode::Tag_Global, 0, afl::base::Nothing);

    // Reading specific properties
    // - Global Property
    verif.verifyString("SYSTEM.PROGRAM", "PCC");
    verif.verifyString("GLOBAL.SYSTEM.PROGRAM", "PCC");
    verif.verifyInteger("TURN", 42);
    verif.verifyInteger("GLOBAL.TURN", 42);
    verif.verifyInteger("SYSTEM.RANDOMSEED", 55);
    verif.verifyInteger("GLOBAL.SYSTEM.RANDOMSEED", 55);

    // - Player Property
    verif.verifyInteger("MY.RACE$", 5);
    verif.verifyInteger("GLOBAL.MY.RACE$", 5);

    // - UI property
    verif.verifyInteger("UI.SCREEN", 4);
    verif.verifyInteger("GLOBAL.UI.SCREEN", 4);

    // - Global variable
    verif.verifyString("GV", "t");
    verif.verifyString("GLOBAL.GV", "t");

    // Writing specific properies
    verif.setIntegerValue("GLOBAL.SYSTEM.RANDOMSEED", 77);
    verif.setIntegerValue("UI.SCREEN", 10);
    verif.setIntegerValue("GV", 55);

    a.checkEqual("01. screen number", ui.get(), 10);
    a.checkEqual("02. rng seed", session.rng().getSeed(), 77U);

    int32_t gv = 0;
    a.check("11. getGlobalValue", interpreter::checkIntegerArg(gv, session.world().getGlobalValue("GV")));
    a.checkEqual("12. value", gv, 55);

    // Failure to write
    AFL_CHECK_THROWS(a("21. set TURN"), verif.setIntegerValue("TURN", 100), interpreter::Error);
    AFL_CHECK_THROWS(a("22. set MY.RACE$"), verif.setIntegerValue("MY.RACE$", 100), interpreter::Error);
}

/** General tests.
    Tests property access on empty session. */
AFL_TEST("game.interface.GlobalContext:empty", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // General properties
    game::interface::GlobalContext testee(session);
    interpreter::test::ContextVerifier verif(testee, a);

    // Reading specific properties
    // - Global Property
    verif.verifyNull("TURN");
    verif.verifyNull("GLOBAL.TURN");

    // - Player Property
    verif.verifyNull("MY.RACE$");
    verif.verifyNull("GLOBAL.MY.RACE$");

    // - UI property
    verif.verifyNull("UI.SCREEN");
    verif.verifyNull("GLOBAL.UI.SCREEN");

    // Writing specific properies
    AFL_CHECK_THROWS(a("01. set UI.SCREEN"), verif.setIntegerValue("UI.SCREEN", 10), interpreter::Error);
    AFL_CHECK_THROWS(a("02. set TURN"),      verif.setIntegerValue("TURN", 100), interpreter::Error);
    AFL_CHECK_THROWS(a("03. set MY.RACE$"),  verif.setIntegerValue("MY.RACE$", 100), interpreter::Error);
}
