/**
  *  \file u/t_game_interface_globalactioncontext.cpp
  *  \brief Test for game::interface::GlobalActionContext
  */

#include "game/interface/globalactioncontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/nullstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/filecommandsource.hpp"
#include "interpreter/statementcompilationcontext.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/values.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"

using game::interface::GlobalActionContext;
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

    void runCode(game::Session& session, GlobalActionContext& ctx, const char* code, Process::State expectedState)
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes(code));
        afl::io::TextFile tf(ms);
        interpreter::FileCommandSource fcs(tf);
        interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
        interpreter::StatementCompiler(fcs).compileList(*bco, interpreter::DefaultStatementCompilationContext(session.world()));

        Process& proc = session.processList().create(session.world(), "p");
        proc.pushFrame(bco, false);
        proc.pushNewContext(ctx.clone());
        proc.run();
        TSM_ASSERT_EQUALS(code, proc.getState(), expectedState);
    }

    void runFailTestCase(const char* code, Process::State expectedState)
    {
        TestUniverse u;
        GlobalActionContext ctx;
        runCode(u.session, ctx, code, expectedState);
        TSM_ASSERT_EQUALS(code, ctx.data()->actionNames.getFirstChild(TreeList::root), TreeList::nil);
    }
}

/** Test creation and use of a GlobalActionContext. */
void
TestGameInterfaceGlobalActionContext::testIt()
{
    TestUniverse u;

    // Create GlobalActionContext; must be empty
    GlobalActionContext ctx;
    TS_ASSERT(ctx.data()->actions.getActionByIndex(0) == 0);

    // Define an action
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
        "Add 'a|b', xprep, xexec, xfinish\n";
    runCode(u.session, ctx, CODE, Process::Ended);

    // Must now have a global action: check the tree
    size_t aNode = ctx.data()->actionNames.getFirstChild(TreeList::root);
    TS_ASSERT_DIFFERS(aNode, TreeList::nil);

    size_t bNode = ctx.data()->actionNames.getFirstChild(aNode);
    TS_ASSERT_DIFFERS(bNode, TreeList::nil);

    int32_t key = 0;
    String_t label;
    TS_ASSERT_EQUALS(ctx.data()->actionNames.get(bNode, key, label), true);
    TS_ASSERT_EQUALS(label, "b");
    TS_ASSERT_DIFFERS(key, 0);

    // Check the action
    const GlobalActions::Action* p = ctx.data()->actions.getActionByIndex(key-1);
    TS_ASSERT(p != 0);

    // Run the action
    Process& proc = u.session.processList().create(u.session.world(), "p");
    proc.pushFrame(ctx.data()->actions.compileGlobalAction(p, u.session.world(), GlobalActions::Flags_t()), false);
    proc.run();
    TS_ASSERT_EQUALS(proc.getState(), Process::Ended);

    // Verify result
    TS_ASSERT_EQUALS(interpreter::toString(u.session.world().getGlobalValue("A"), false),
                     "pr()ex(10)ex(20)ex(15)ex(23)ex(47)fi()");
}

/** Test failure cases of GlobalActions().Add. */
void
TestGameInterfaceGlobalActionContext::testFailures()
{
    // Null name (ignored successfully)
    runFailTestCase("Sub qq\n"
                    "EndSub\n"
                    "Add Z(0), qq, qq, qq\n", Process::Ended);

    // Null function (ignored successfully)
    runFailTestCase("Sub qq\n"
                    "EndSub\n"
                    "Add 'foo', Z(0), qq, qq\n", Process::Ended);

    // Empty name (failure)
    runFailTestCase("Sub qq\n"
                    "EndSub\n"
                    "Add '', qq, qq, qq\n", Process::Failed);

    // Type error
    runFailTestCase("Sub qq\n"
                    "EndSub\n"
                    "Add 'foo', qq, qq, 3\n", Process::Failed);
}

/** Test Context properties. */
void
TestGameInterfaceGlobalActionContext::testContext()
{
    GlobalActionContext testee;

    // General verification
    interpreter::test::ContextVerifier(testee, "testContext").verifyTypes();

    // Some properties
    TS_ASSERT(testee.getObject() == 0);
    TS_ASSERT_DIFFERS(testee.toString(true), "");
    TS_ASSERT_DIFFERS(testee.toString(false), "");

    // Cloning
    std::auto_ptr<GlobalActionContext> clone(testee.clone());
    TS_ASSERT(clone.get() != 0);
    TS_ASSERT_EQUALS(clone->toString(false), testee.toString(false));
    TS_ASSERT_EQUALS(&*clone->data(), &*testee.data());

    // Storing
    interpreter::TagNode out;
    afl::io::NullStream aux;
    interpreter::vmio::NullSaveContext ctx;
    TS_ASSERT_THROWS(testee.store(out, aux, ctx), std::exception);
}

/** Test IFGlobalActionContext, success case. */
void
TestGameInterfaceGlobalActionContext::testMake()
{
    // Call it
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    std::auto_ptr<afl::data::Value> result(game::interface::IFGlobalActionContext(args));

    // Result must not be null
    TS_ASSERT(result.get() != 0);

    // Result must be a Context
    interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(result.get());
    TS_ASSERT(ctx != 0);

    // Context must have a ADD attribute
    std::auto_ptr<afl::data::Value> adder(interpreter::test::ContextVerifier(*ctx, "testMake").getValue("ADD"));
    TS_ASSERT(adder.get() != 0);
}

/** Test IFGlobalActionContext, failure case. */
void
TestGameInterfaceGlobalActionContext::testMakeFail()
{
    // Call it with too many args
    afl::data::Segment seg;
    seg.pushBackInteger(1);
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<afl::data::Value> result;

    TS_ASSERT_THROWS(result.reset(game::interface::IFGlobalActionContext(args)), std::exception);
}

