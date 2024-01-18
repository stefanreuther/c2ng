/**
  *  \file test/game/interface/globalactioncontexttest.cpp
  *  \brief Test for game::interface::GlobalActionContext
  */

#include "game/interface/globalactioncontext.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
                session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
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

    void runCode(afl::test::Assert a, game::Session& session, GlobalActionContext& ctx, const char* code, Process::State expectedState)
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
        a.checkEqual("process state", proc.getState(), expectedState);
    }

    void runFailTestCase(afl::test::Assert a, const char* code, Process::State expectedState)
    {
        TestUniverse u;
        GlobalActionContext ctx;
        runCode(a, u.session, ctx, code, expectedState);
        a.checkEqual("getFirstChild", ctx.data()->actionNames.getFirstChild(TreeList::root), TreeList::nil);
    }
}

/** Test creation and use of a GlobalActionContext. */
AFL_TEST("game.interface.GlobalActionContext:basics", a)
{
    TestUniverse u;

    // Create GlobalActionContext; must be empty
    GlobalActionContext ctx;
    a.checkNull("01. getActionByIndex", ctx.data()->actions.getActionByIndex(0));

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
    runCode(a("02. runCode"), u.session, ctx, CODE, Process::Ended);

    // Must now have a global action: check the tree
    size_t aNode = ctx.data()->actionNames.getFirstChild(TreeList::root);
    a.checkDifferent("11. getFirstChild", aNode, TreeList::nil);

    size_t bNode = ctx.data()->actionNames.getFirstChild(aNode);
    a.checkDifferent("21. getFirstChild", bNode, TreeList::nil);

    int32_t key = 0;
    String_t label;
    a.checkEqual("31. actionNames", ctx.data()->actionNames.get(bNode, key, label), true);
    a.checkEqual("32. label", label, "b");
    a.checkDifferent("33. key", key, 0);

    // Check the action
    const GlobalActions::Action* p = ctx.data()->actions.getActionByIndex(key-1);
    a.checkNonNull("41. getActionByIndex", p);

    // Run the action
    Process& proc = u.session.processList().create(u.session.world(), "p");
    proc.pushFrame(ctx.data()->actions.compileGlobalAction(p, u.session.world(), GlobalActions::Flags_t()), false);
    proc.run();
    a.checkEqual("51. run action", proc.getState(), Process::Ended);

    // Verify result
    a.checkEqual("61. getGlobalValue", interpreter::toString(u.session.world().getGlobalValue("A"), false),
                 "pr()ex(10)ex(20)ex(15)ex(23)ex(47)fi()");
}

/*
 *  Test failure cases of GlobalActions().Add.
 */

// Null name (ignored successfully)
AFL_TEST("game.interface.GlobalActionContext:Add:error:null-name", a)
{
    runFailTestCase(a, "Sub qq\n"
                    "EndSub\n"
                    "Add Z(0), qq, qq, qq\n", Process::Ended);
}

// Null function (ignored successfully)
AFL_TEST("game.interface.GlobalActionContext:Add:error:null-function", a)
{
    runFailTestCase(a, "Sub qq\n"
                    "EndSub\n"
                    "Add 'foo', Z(0), qq, qq\n", Process::Ended);
}

// Empty name (failure)
AFL_TEST("game.interface.GlobalActionContext:Add:error:empty-name", a)
{
    runFailTestCase(a, "Sub qq\n"
                    "EndSub\n"
                    "Add '', qq, qq, qq\n", Process::Failed);
}

// Type error
AFL_TEST("game.interface.GlobalActionContext:Add:error:type", a)
{
    runFailTestCase(a, "Sub qq\n"
                    "EndSub\n"
                    "Add 'foo', qq, qq, 3\n", Process::Failed);
}

/** Test Context properties. */
AFL_TEST("game.interface.GlobalActionContext:context", a)
{
    GlobalActionContext testee;

    // General verification
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyTypes();
    verif.verifyBasics();
    verif.verifyNotSerializable();

    // Some properties
    a.checkNull("01. getObject", testee.getObject());

    // Cloning
    std::auto_ptr<GlobalActionContext> clone(testee.clone());
    a.checkNonNull("11. clone", clone.get());
    a.checkEqual("12. data", &*clone->data(), &*testee.data());
}

/** Test IFGlobalActionContext, success case. */
AFL_TEST("game.interface.GlobalActionContext:IFGlobalActionContext", a)
{
    // Call it
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    std::auto_ptr<afl::data::Value> result(game::interface::IFGlobalActionContext(args));

    // Result must not be null
    a.checkNonNull("01. get", result.get());

    // Result must be a Context
    interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(result.get());
    a.checkNonNull("11. ctx", ctx);

    // Context must have a ADD attribute
    std::auto_ptr<afl::data::Value> adder(interpreter::test::ContextVerifier(*ctx, a).getValue("ADD"));
    a.checkNonNull("21. ADD", adder.get());
}

/** Test IFGlobalActionContext, failure case. */
AFL_TEST("game.interface.GlobalActionContext:IFGlobalActionContext:fail", a)
{
    // Call it with too many args
    afl::data::Segment seg;
    seg.pushBackInteger(1);
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<afl::data::Value> result;

    AFL_CHECK_THROWS(a, result.reset(game::interface::IFGlobalActionContext(args)), std::exception);
}
