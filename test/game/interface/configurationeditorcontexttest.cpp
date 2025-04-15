/**
  *  \file test/game/interface/configurationeditorcontexttest.cpp
  *  \brief Test for game::interface::ConfigurationEditorContext
  */

#include "game/interface/configurationeditorcontext.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/root.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/filecommandsource.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/test/contextverifier.hpp"
#include <stdexcept>

using game::interface::ConfigurationEditorContext;
using interpreter::Process;
using interpreter::test::ContextVerifier;
using util::TreeList;

namespace {
    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        Environment()
            : tx(), fs(), session(tx, fs)
            { session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr()); }
    };

    void runCode(afl::test::Assert a, game::Session& session, interpreter::Context& ctx, const char* code, Process::State expectedState)
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes(code));
        afl::io::TextFile tf(ms);
        interpreter::FileCommandSource fcs(tf);
        interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
        interpreter::StatementCompiler(fcs).compileList(*bco, interpreter::DefaultStatementCompilationContext(session.world()));

        Process& proc = session.processList().create(session.world(), "p");
        proc.pushFrame(bco, false);
        proc.pushNewContext(ctx.clone());
        proc.run(0);
        a.checkEqual("code execution result", proc.getState(), expectedState);
    }

    void runFailTestCase(afl::test::Assert a, const char* code, Process::State expectedState, bool expectTree)
    {
        Environment env;
        ConfigurationEditorContext ctx(env.session);
        runCode(a, env.session, ctx, code, expectedState);
        if (expectTree) {
            a.checkDifferent("result", ctx.data().ref->optionNames.getFirstChild(TreeList::root), TreeList::nil);
        } else {
            a.checkEqual("result", ctx.data().ref->optionNames.getFirstChild(TreeList::root), TreeList::nil);
        }
    }
}

/** Test Context properties. */
AFL_TEST("game.interface.ConfigurationEditorContext:basics", a)
{
    Environment env;
    ConfigurationEditorContext testee(env.session);

    // General verification
    ContextVerifier verif(testee, a);
    verif.verifyTypes();
    verif.verifyBasics();
    verif.verifyNotSerializable();

    // Some properties
    a.checkNull("01. getObject", testee.getObject());
    a.checkEqual("02. next", testee.next(), false);

    // Cloning
    std::auto_ptr<ConfigurationEditorContext> clone(testee.clone());
    a.checkNonNull("11. clone", clone.get());
    a.checkEqual("12. cloned ref", &*clone->data().ref, &*testee.data().ref);
    a.checkEqual("13. cloned root", clone->data().root, testee.data().root);

    // Ids
    a.checkEqual("21. getTreeIdFromEditorIndex", ConfigurationEditorContext::getTreeIdFromEditorIndex(0), 1);
    a.checkEqual("22. getEditorIndexFromTreeId", ConfigurationEditorContext::getEditorIndexFromTreeId(1), 0U);
}

/** Test IFConfigurationEditorContext(). */
AFL_TEST("game.interface.ConfigurationEditorContext:IFConfigurationEditorContext", a)
{
    // Call it
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    std::auto_ptr<afl::data::Value> result(game::interface::IFConfigurationEditorContext(env.session, args));

    // Result must not be null
    a.checkNonNull("01. result", result.get());

    // Result must be a Context
    interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(result.get());
    a.checkNonNull("11. type", ctx);

    // Context must have a ADD attribute
    std::auto_ptr<afl::data::Value> adder(ContextVerifier(*ctx, a).getValue("ADD"));
    a.check("21. has ADD", adder.get());
}

/** Test general usage sequence. */
AFL_TEST("game.interface.ConfigurationEditorContext:sequence", a)
{
    // Create ConfigurationEditorContext; must be empty
    Environment env;
    ConfigurationEditorContext ctx(env.session);
    a.checkEqual("01. hasChildren", ctx.data().ref->optionNames.hasChildren(TreeList::root), false);

    // Action sequence
    const char CODE[] =
        "v := 'v1'\n"
        "n := 'nv'\n"
        "Function xval()\n"
        "  Return v\n"
        "EndFunction\n"
        "Sub xmod\n"
        "  v := n & Extra & '-' & Option\n"
        "EndSub\n"
        "Add 'group|opt', xmod, xval\n"
        "LinkExtra 'ex'\n"
        "LinkPref 'Chart.Marker0', 'something.that.does.not.exist'\n"
        "UpdateAll\n";
    runCode(a("02. run code"), env.session, ctx, CODE, Process::Ended);

    // Verify tree
    size_t aNode = ctx.data().ref->optionNames.getFirstChild(TreeList::root);
    a.checkDifferent("11. getFirstChild", aNode, TreeList::nil);

    size_t bNode = ctx.data().ref->optionNames.getFirstChild(aNode);
    a.checkDifferent("21. getFirstChild", bNode, TreeList::nil);

    int32_t key = 0;
    String_t label;
    a.checkEqual("31. optionNames", ctx.data().ref->optionNames.get(aNode, key, label), true);
    a.checkEqual("32. label", label, "group");
    a.checkEqual("33. key", key, 0);

    a.checkEqual("41. optionNames", ctx.data().ref->optionNames.get(bNode, key, label), true);
    a.checkEqual("42. label", label, "opt");
    a.checkDifferent("43. key", key, 0);

    // Verify status: value must be 'v1', storage must be Default
    game::config::Configuration& conf = env.session.getRoot()->userConfiguration();
    game::config::ConfigurationEditor::Node* n = ctx.data().ref->editor.getNodeByIndex(ConfigurationEditorContext::getEditorIndexFromTreeId(key));
    a.checkNonNull("51. node", n);
    a.checkEqual("52. type", n->getType(), ConfigurationEditorContext::ScriptEditor);
    a.checkEqual("53. value", n->getValue(conf, env.tx), "v1");
    a.checkEqual("54. source", n->getSource(conf), game::config::ConfigurationEditor::Default);

    // Modify it
    Process& proc = env.session.processList().create(env.session.world(), "p");
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    ctx.compileEditor(*bco, ConfigurationEditorContext::getEditorIndexFromTreeId(key));
    proc.pushFrame(bco, false);
    proc.run(0);
    a.checkEqual("61. getState", proc.getState(), Process::Ended);

    // Verify updated value
    a.checkEqual("71. value", n->getValue(conf, env.tx), "nvex-Chart.Marker0");

    // Update and verify storage
    n->setSource(conf, game::config::ConfigurationOption::Game);
    a.checkEqual("81. source", n->getSource(conf), game::config::ConfigurationEditor::Game);
}

/** Test Subtree(). */
AFL_TEST("game.interface.ConfigurationEditorContext:subtree", a)
{
    // Create ConfigurationEditorContext; must be empty
    Environment env;
    ConfigurationEditorContext ctx(env.session);
    a.checkEqual("01. hasChildren", ctx.data().ref->optionNames.hasChildren(TreeList::root), false);

    // Action sequence [reduced version of testSequence]
    const char CODE[] =
        "v := 'vx'\n"
        "Function xval()\n"
        "  Return v\n"
        "EndFunction\n"
        "Sub xmod\n"
        "EndSub\n"
        "With Subtree('subgroup') Do Add 'opt', xmod, xval\n"
        "UpdateAll\n";
    runCode(a("02. run code"), env.session, ctx, CODE, Process::Ended);

    // Verify tree
    size_t aNode = ctx.data().ref->optionNames.getFirstChild(TreeList::root);
    a.checkDifferent("11. getFirstChild", aNode, TreeList::nil);

    size_t bNode = ctx.data().ref->optionNames.getFirstChild(aNode);
    a.checkDifferent("21. getFirstChild", bNode, TreeList::nil);

    int32_t key = 0;
    String_t label;
    a.checkEqual("31. optionNames", ctx.data().ref->optionNames.get(aNode, key, label), true);
    a.checkEqual("32. label", label, "subgroup");
    a.checkEqual("33. key", key, 0);

    a.checkEqual("41. optionNames", ctx.data().ref->optionNames.get(bNode, key, label), true);
    a.checkEqual("42. label", label, "opt");
    a.checkDifferent("43. key", key, 0);

    // Verify status: value must be 'vx', storage must be Default
    game::config::Configuration& conf = env.session.getRoot()->userConfiguration();
    game::config::ConfigurationEditor::Node* n = ctx.data().ref->editor.getNodeByIndex(ConfigurationEditorContext::getEditorIndexFromTreeId(key));
    a.checkNonNull("51. node", n);
    a.checkEqual("52. type", n->getType(), ConfigurationEditorContext::ScriptEditor);
    a.checkEqual("53. value", n->getValue(conf, env.tx), "vx");
    a.checkEqual("54. source", n->getSource(conf), game::config::ConfigurationEditor::NotStored);
}

// Null name (ignored successfully)
AFL_TEST("game.interface.ConfigurationEditorContext:error:null-name", a)
{
    runFailTestCase(a, "Function xval\nEndFunction\n"
                    "Sub xmod\nEndSub\n"
                    "Add Z(0), xmod, xval\n", Process::Ended, false);
}

// Null function (ignored successfully)
AFL_TEST("game.interface.ConfigurationEditorContext:error:null-mod", a)
{
    runFailTestCase(a, "Function xval\nEndFunction\n"
                    "Add 'a', Z(0), xval\n", Process::Ended, false);
}

// Null function (ignored successfully)
AFL_TEST("game.interface.ConfigurationEditorContext:error:null-val", a)
{
    runFailTestCase(a, "Sub xmod\nEndSub\n"
                    "Add 'a', xmod, Z(0)\n", Process::Ended, false);
}

// Empty name (failure)
AFL_TEST("game.interface.ConfigurationEditorContext:error:empty-name", a)
{
    runFailTestCase(a, "Function xval\nEndFunction\n"
                    "Sub xmod\nEndSub\n"
                    "Add '', xmod, xval\n", Process::Failed, false);
}

// Type error (failure)
AFL_TEST("game.interface.ConfigurationEditorContext:error:type", a)
{
    runFailTestCase(a, "Function xval\nEndFunction\n"
                    "Add 'x', 9, xval\n", Process::Failed, false);
}

// Sequence error: LinkExtra
AFL_TEST("game.interface.ConfigurationEditorContext:error:bad-context:LinkExtra", a)
{
    runFailTestCase(a, "LinkExtra 3", Process::Failed, false);
}

// Sequence error: LinkPref
AFL_TEST("game.interface.ConfigurationEditorContext:error:bad-context:LinkPref", a)
{
    runFailTestCase(a, "LinkPref 'Chart.Marker0'", Process::Failed, false);
}

// Arity error: Add
AFL_TEST("game.interface.ConfigurationEditorContext:error:arity-error:Add", a)
{
    runFailTestCase(a, "Add 'x'", Process::Failed, false);
}

// Arity error: LinkPref
AFL_TEST("game.interface.ConfigurationEditorContext:error:arity-error:LinkPref", a)
{
    runFailTestCase(a, "Function xval\nEndFunction\n"
                    "Sub xmod\nEndSub\n"
                    "Add 'a', xmod, xval\n"
                    "LinkPref\n", Process::Failed, true);
}

// Arity error: LinkExtra
AFL_TEST("game.interface.ConfigurationEditorContext:error:arity-error:LinkExtra", a)
{
    runFailTestCase(a, "Function xval\nEndFunction\n"
                    "Sub xmod\nEndSub\n"
                    "Add 'a', xmod, xval\n"
                    "LinkExtra\n", Process::Failed, true);
}
