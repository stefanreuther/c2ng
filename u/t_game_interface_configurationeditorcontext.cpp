/**
  *  \file u/t_game_interface_configurationeditorcontext.cpp
  *  \brief Test for game::interface::ConfigurationEditorContext
  */

#include <stdexcept>
#include "game/interface/configurationeditorcontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/nullstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/root.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/filecommandsource.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"

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

    void runCode(game::Session& session, interpreter::Context& ctx, const char* code, Process::State expectedState)
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

    void runFailTestCase(const char* code, Process::State expectedState, bool expectTree)
    {
        Environment env;
        ConfigurationEditorContext ctx(env.session);
        runCode(env.session, ctx, code, expectedState);
        if (expectTree) {
            TSM_ASSERT_DIFFERS(code, ctx.data().ref->optionNames.getFirstChild(TreeList::root), TreeList::nil);
        } else {
            TSM_ASSERT_EQUALS(code, ctx.data().ref->optionNames.getFirstChild(TreeList::root), TreeList::nil);
        }
    }
}

/** Test Context properties. */
void
TestGameInterfaceConfigurationEditorContext::testBasics()
{
    Environment env;
    ConfigurationEditorContext testee(env.session);

    // General verification
    ContextVerifier(testee, "testBasics").verifyTypes();

    // Some properties
    TS_ASSERT(testee.getObject() == 0);
    TS_ASSERT_DIFFERS(testee.toString(true), "");
    TS_ASSERT_DIFFERS(testee.toString(false), "");
    TS_ASSERT_EQUALS(testee.next(), false);

    // Cloning
    std::auto_ptr<ConfigurationEditorContext> clone(testee.clone());
    TS_ASSERT(clone.get() != 0);
    TS_ASSERT_EQUALS(clone->toString(false), testee.toString(false));
    TS_ASSERT_EQUALS(&*clone->data().ref, &*testee.data().ref);
    TS_ASSERT_EQUALS(clone->data().root, testee.data().root);

    // Storing
    interpreter::TagNode out;
    afl::io::NullStream aux;
    interpreter::vmio::NullSaveContext ctx;
    TS_ASSERT_THROWS(testee.store(out, aux, ctx), std::exception);

    // Ids
    TS_ASSERT_EQUALS(ConfigurationEditorContext::getTreeIdFromEditorIndex(0), 1);
    TS_ASSERT_EQUALS(ConfigurationEditorContext::getEditorIndexFromTreeId(1), 0U);
}

/** Test IFConfigurationEditorContext(). */
void
TestGameInterfaceConfigurationEditorContext::testMake()
{
    // Call it
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    std::auto_ptr<afl::data::Value> result(game::interface::IFConfigurationEditorContext(env.session, args));

    // Result must not be null
    TS_ASSERT(result.get() != 0);

    // Result must be a Context
    interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(result.get());
    TS_ASSERT(ctx != 0);

    // Context must have a ADD attribute
    std::auto_ptr<afl::data::Value> adder(ContextVerifier(*ctx, "testMake").getValue("ADD"));
    TS_ASSERT(adder.get() != 0);
}

/** Test general usage sequence. */
void
TestGameInterfaceConfigurationEditorContext::testSequence()
{
    // Create ConfigurationEditorContext; must be empty
    Environment env;
    ConfigurationEditorContext ctx(env.session);
    TS_ASSERT_EQUALS(ctx.data().ref->optionNames.hasChildren(TreeList::root), false);

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
    runCode(env.session, ctx, CODE, Process::Ended);

    // Verify tree
    size_t aNode = ctx.data().ref->optionNames.getFirstChild(TreeList::root);
    TS_ASSERT_DIFFERS(aNode, TreeList::nil);

    size_t bNode = ctx.data().ref->optionNames.getFirstChild(aNode);
    TS_ASSERT_DIFFERS(bNode, TreeList::nil);

    int32_t key = 0;
    String_t label;
    TS_ASSERT_EQUALS(ctx.data().ref->optionNames.get(aNode, key, label), true);
    TS_ASSERT_EQUALS(label, "group");
    TS_ASSERT_EQUALS(key, 0);

    TS_ASSERT_EQUALS(ctx.data().ref->optionNames.get(bNode, key, label), true);
    TS_ASSERT_EQUALS(label, "opt");
    TS_ASSERT_DIFFERS(key, 0);

    // Verify status: value must be 'v1', storage must be Default
    game::config::Configuration& conf = env.session.getRoot()->userConfiguration();
    game::config::ConfigurationEditor::Node* n = ctx.data().ref->editor.getNodeByIndex(ConfigurationEditorContext::getEditorIndexFromTreeId(key));
    TS_ASSERT(n != 0);
    TS_ASSERT_EQUALS(n->getType(), ConfigurationEditorContext::ScriptEditor);
    TS_ASSERT_EQUALS(n->getValue(conf, env.tx), "v1");
    TS_ASSERT_EQUALS(n->getSource(conf), game::config::ConfigurationEditor::Default);

    // Modify it
    Process& proc = env.session.processList().create(env.session.world(), "p");
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    ctx.compileEditor(*bco, ConfigurationEditorContext::getEditorIndexFromTreeId(key));
    proc.pushFrame(bco, false);
    proc.run();
    TS_ASSERT_EQUALS(proc.getState(), Process::Ended);

    // Verify updated value
    TS_ASSERT_EQUALS(n->getValue(conf, env.tx), "nvex-Chart.Marker0");

    // Update and verify storage
    n->setSource(conf, game::config::ConfigurationOption::Game);
    TS_ASSERT_EQUALS(n->getSource(conf), game::config::ConfigurationEditor::Game);
}

/** Test Subtree(). */
void
TestGameInterfaceConfigurationEditorContext::testSubtree()
{
    // Create ConfigurationEditorContext; must be empty
    Environment env;
    ConfigurationEditorContext ctx(env.session);
    TS_ASSERT_EQUALS(ctx.data().ref->optionNames.hasChildren(TreeList::root), false);

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
    runCode(env.session, ctx, CODE, Process::Ended);

    // Verify tree
    size_t aNode = ctx.data().ref->optionNames.getFirstChild(TreeList::root);
    TS_ASSERT_DIFFERS(aNode, TreeList::nil);

    size_t bNode = ctx.data().ref->optionNames.getFirstChild(aNode);
    TS_ASSERT_DIFFERS(bNode, TreeList::nil);

    int32_t key = 0;
    String_t label;
    TS_ASSERT_EQUALS(ctx.data().ref->optionNames.get(aNode, key, label), true);
    TS_ASSERT_EQUALS(label, "subgroup");
    TS_ASSERT_EQUALS(key, 0);

    TS_ASSERT_EQUALS(ctx.data().ref->optionNames.get(bNode, key, label), true);
    TS_ASSERT_EQUALS(label, "opt");
    TS_ASSERT_DIFFERS(key, 0);

    // Verify status: value must be 'vx', storage must be Default
    game::config::Configuration& conf = env.session.getRoot()->userConfiguration();
    game::config::ConfigurationEditor::Node* n = ctx.data().ref->editor.getNodeByIndex(ConfigurationEditorContext::getEditorIndexFromTreeId(key));
    TS_ASSERT(n != 0);
    TS_ASSERT_EQUALS(n->getType(), ConfigurationEditorContext::ScriptEditor);
    TS_ASSERT_EQUALS(n->getValue(conf, env.tx), "vx");
    TS_ASSERT_EQUALS(n->getSource(conf), game::config::ConfigurationEditor::NotStored);
}

void
TestGameInterfaceConfigurationEditorContext::testFailures()
{
    // Null name (ignored successfully)
    runFailTestCase("Function xval\nEndFunction\n"
                    "Sub xmod\nEndSub\n"
                    "Add Z(0), xmod, xval\n", Process::Ended, false);

    // Null function (ignored successfully)
    runFailTestCase("Function xval\nEndFunction\n"
                    "Add 'a', Z(0), xval\n", Process::Ended, false);

    // Null function (ignored successfully)
    runFailTestCase("Sub xmod\nEndSub\n"
                    "Add 'a', xmod, Z(0)\n", Process::Ended, false);

    // Empty name (failure)
    runFailTestCase("Function xval\nEndFunction\n"
                    "Sub xmod\nEndSub\n"
                    "Add '', xmod, xval\n", Process::Failed, false);

    // Type error (failure)
    runFailTestCase("Function xval\nEndFunction\n"
                    "Add 'x', 9, xval\n", Process::Failed, false);

    // Sequence error: LinkExtra
    runFailTestCase("LinkExtra 3", Process::Failed, false);

    // Sequence error: LinkPref
    runFailTestCase("LinkPref 'Chart.Marker0'", Process::Failed, false);

    // Arity error: Add
    runFailTestCase("Add 'x'", Process::Failed, false);

    // Arity error: LinkPref
    runFailTestCase("Function xval\nEndFunction\n"
                    "Sub xmod\nEndSub\n"
                    "Add 'a', xmod, xval\n"
                    "LinkPref\n", Process::Failed, true);

    // Arity error: LinkExtra
    runFailTestCase("Function xval\nEndFunction\n"
                    "Sub xmod\nEndSub\n"
                    "Add 'a', xmod, xval\n"
                    "LinkExtra\n", Process::Failed, true);
}

