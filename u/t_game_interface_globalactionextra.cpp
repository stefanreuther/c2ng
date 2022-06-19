/**
  *  \file u/t_game_interface_globalactionextra.cpp
  *  \brief Test for game::interface::GlobalActionExtra
  */

#include "game/interface/globalactionextra.hpp"

#include "t_game_interface.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/filecommandsource.hpp"
#include "interpreter/statementcompilationcontext.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/values.hpp"

using game::interface::GlobalActionExtra;
using game::interface::GlobalActions;
using game::map::Universe;
using interpreter::Process;
using util::TreeList;

namespace {
    /* Test universe with some objects.
       (Same test harness as for GlobalActions, so we can also run the action.) */
    struct TestUniverse {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        TestUniverse()
            : tx(), fs(), session(tx, fs)
            {
                const game::map::Point P(1000,1000);
                const game::PlayerSet_t S(3);
                session.setRoot(new game::test::Root(game::HostVersion()));
                session.setShipList(new game::spec::ShipList());
                session.setGame(new game::Game());
                Universe& univ = session.getGame()->currentTurn().universe();
                univ.ships().create(10)->addShipXYData(P, 10, 100, S);
                univ.ships().create(20)->addShipXYData(P, 10, 100, S);
                univ.planets().create(15)->setPosition(P);
                univ.planets().create(23)->setPosition(P);
                univ.planets().create(47)->setPosition(P);
                session.postprocessTurn(session.getGame()->currentTurn(), S, S, game::map::Object::Playable);
            }
    };

    void runCode(game::Session& session, const char* code, Process::State expectedState)
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes(code));
        afl::io::TextFile tf(ms);
        interpreter::FileCommandSource fcs(tf);
        interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
        interpreter::StatementCompiler(fcs).compileList(*bco, interpreter::DefaultStatementCompilationContext(session.world()));

        Process& proc = session.processList().create(session.world(), "p");
        proc.pushFrame(bco, false);
        proc.run();
        TSM_ASSERT_EQUALS(code, proc.getState(), expectedState);
    }

    void runFailTestCase(const char* code, Process::State expectedState)
    {
        TestUniverse u;
        GlobalActionExtra& extra = GlobalActionExtra::create(u.session);
        runCode(u.session, code, expectedState);
        TSM_ASSERT_EQUALS(code, extra.actionNames().getFirstChild(TreeList::root), TreeList::nil);
    }
}

/** Test creation and use of a GlobalActionExtra. */
void
TestGameInterfaceGlobalActionExtra::testIt()
{
    TestUniverse u;

    // At startup, no GlobalActionExtra is present
    TS_ASSERT(GlobalActionExtra::get(u.session) == 0);

    // Create one; must be empty
    GlobalActionExtra& extra = GlobalActionExtra::create(u.session);
    TS_ASSERT(GlobalActionExtra::get(u.session) == &extra);
    TS_ASSERT(extra.actions().getActionByIndex(0) == 0);

    // Constness, for coverage
    const GlobalActionExtra& cextra = extra;
    TS_ASSERT_EQUALS(&extra.actions(), &cextra.actions());
    TS_ASSERT_EQUALS(&extra.actionNames(), &cextra.actionNames());

    // Define one
    const char CODE[] =
        "a := ''\n"
        "Function xprep()\n"
        "  a := a & 'pr()'\n"
        "  Return 42\n"
        "EndFunction\n"
        "Sub xexec(obj,st)\n"
        "  a := a & 'ex(' & obj->Id & ')'\n"
        "EndSub\n"
        "Sub xfinish(st,gs)\n"
        "  a := a & 'fi()'\n"
        "EndSub\n"
        "AddGlobalAction 'a|b', xprep, xexec, xfinish\n";
    runCode(u.session, CODE, Process::Ended);

    // Must now have a global action: check the tree
    size_t aNode = extra.actionNames().getFirstChild(TreeList::root);
    TS_ASSERT_DIFFERS(aNode, TreeList::nil);

    size_t bNode = extra.actionNames().getFirstChild(aNode);
    TS_ASSERT_DIFFERS(bNode, TreeList::nil);

    int32_t key = 0;
    String_t label;
    TS_ASSERT_EQUALS(extra.actionNames().get(bNode, key, label), true);
    TS_ASSERT_EQUALS(label, "b");
    TS_ASSERT_DIFFERS(key, 0);

    // Check the action
    const GlobalActions::Action* p = extra.actions().getActionByIndex(key-1);
    TS_ASSERT(p != 0);

    // Run the action
    Process& proc = u.session.processList().create(u.session.world(), "p");
    proc.pushFrame(extra.actions().compileGlobalAction(p, u.session.world(), GlobalActions::Flags_t()), false);
    proc.run();
    TS_ASSERT_EQUALS(proc.getState(), Process::Ended);

    // Verify result
    TS_ASSERT_EQUALS(interpreter::toString(u.session.world().getGlobalValue("A"), false),
                     "pr()ex(10)ex(20)ex(15)ex(23)ex(47)fi()");
}

/** Test failure cases of AddGlobalAction. */
void
TestGameInterfaceGlobalActionExtra::testFailures()
{
    // Null name (ignored successfully)
    runFailTestCase("Sub qq\n"
                    "EndSub\n"
                    "AddGlobalAction Z(0), qq, qq, qq\n", Process::Ended);

    // Null function (ignored successfully)
    runFailTestCase("Sub qq\n"
                    "EndSub\n"
                    "AddGlobalAction 'foo', Z(0), qq, qq\n", Process::Ended);

    // Empty name (failure)
    runFailTestCase("Sub qq\n"
                    "EndSub\n"
                    "AddGlobalAction '', qq, qq, qq\n", Process::Failed);

    // Type error
    runFailTestCase("Sub qq\n"
                    "EndSub\n"
                    "AddGlobalAction 'foo', qq, qq, 3\n", Process::Failed);
}

