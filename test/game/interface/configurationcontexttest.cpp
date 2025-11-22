/**
  *  \file test/game/interface/configurationcontexttest.cpp
  *  \brief Test for game::interface::ConfigurationContext
  */

#include "game/interface/configurationcontext.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/session.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/values.hpp"

using afl::base::Ref;
using afl::data::Segment;
using game::config::Configuration;
using game::config::ConfigurationOption;
using game::interface::ConfigurationContext;
using interpreter::Arguments;
using interpreter::Context;
using interpreter::Error;
using interpreter::IndexableValue;
using interpreter::test::ContextVerifier;
using interpreter::test::ValueVerifier;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

namespace {
    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::InternalFileSystem fs;
        game::Session session;
        interpreter::Process proc;

        Environment()
            : tx(), fs(), session(tx, fs), proc(session.world(), "proc-name", 42)
            { }
    };

    /* Retrieve the "ENTRY" property, and verify its basic properties.
       Return as IndexableValue.
       Caller takes ownership. */
    IndexableValue* getEntryProperty(afl::test::Assert a, Environment& env, Ref<Configuration> config)
    {
        ConfigurationContext ctx(env.session, config);
        std::auto_ptr<afl::data::Value> propertyValue(ContextVerifier(ctx, a("getEntryProperty(ConfigurationContext)")).getValue("ENTRY"));
        a.checkNonNull("getEntryProperty: value must not be null", propertyValue.get());

        IndexableValue* indexable = dynamic_cast<IndexableValue*>(propertyValue.get());
        a.checkNonNull("getEntryProperty: value must be indexable", indexable);

        ValueVerifier verif(*indexable, a("ValueVerifier"));
        verif.verifyBasics();
        verif.verifyNotSerializable();
        a.checkEqual("getEntryProperty: value must not have dimension", indexable->getDimension(0), 0U);

        propertyValue.release(); // ownership taken over by caller
        return indexable;
    }
}

/* Test basics */
AFL_TEST("game.interface.ConfigurationContext:basics", a)
{
    // Empty session (root required for HConfig only)
    Environment env;

    // Testee
    ConfigurationContext testee(env.session, Configuration::create());
    testee.config().setOption("testkey", "testvalue", ConfigurationOption::User);

    // Basic properties
    ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyTypes();
    a.checkNull("getObject", testee.getObject());
    a.checkDifferent("toString", testee.toString(false), "");

    // Lookup error
    Context::PropertyIndex_t idx;
    a.checkNull("lookup", testee.lookup("INVALID", idx));
}

/* Test the check() function */
AFL_TEST("game.interface.ConfigurationContext:check", a)
{
    // Empty session (root required for HConfig only)
    Environment env;

    // Testee
    ConfigurationContext testee(env.session, Configuration::create());

    // Verify
    a.checkEqual("ConfigurationContext must be returned as is", ConfigurationContext::check(&testee), &testee);
    a.checkNull ("Null must be returned as is",                 ConfigurationContext::check(0));

    afl::data::IntegerValue iv(99);
    AFL_CHECK_THROWS("Type mismatch must be rejected", ConfigurationContext::check(&iv), Error);
}

/*
 *  IFConfiguration
 */

/* Nullary invocation, should create an empty configuration */
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration:nullary", a)
{
    Environment env;
    Segment seg;
    Arguments args(seg, 0, 0);

    std::auto_ptr<afl::data::Value> result(game::interface::IFConfiguration(env.session, args));
    a.checkNonNull("got a ConfigurationContext", dynamic_cast<ConfigurationContext*>(result.get()));
}

/* Invocation with parameter 0, should create an empty configuration */
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration:unary:normal", a)
{
    Environment env;
    Segment seg;
    seg.pushBackInteger(0);
    Arguments args(seg, 0, 1);

    std::auto_ptr<afl::data::Value> result(game::interface::IFConfiguration(env.session, args));
    a.checkNonNull("got a ConfigurationContext", dynamic_cast<ConfigurationContext*>(result.get()));
}

/* Invocation with parameter 1, should create a HostConfiguration */
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration:unary:host", a)
{
    Environment env;
    Segment seg;
    seg.pushBackInteger(1);
    Arguments args(seg, 0, 1);

    std::auto_ptr<afl::data::Value> result(game::interface::IFConfiguration(env.session, args));
    ConfigurationContext* cc = dynamic_cast<ConfigurationContext*>(result.get());
    a.checkNonNull("got a ConfigurationContext", cc);
    a.checkNonNull("got a HostConfiguration", dynamic_cast<game::config::HostConfiguration*>(&cc->config()));
}

/* Invocation with parameter 2, should create a UserConfiguration */
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration:unary:user", a)
{
    Environment env;
    Segment seg;
    seg.pushBackInteger(2);
    Arguments args(seg, 0, 1);

    std::auto_ptr<afl::data::Value> result(game::interface::IFConfiguration(env.session, args));
    ConfigurationContext* cc = dynamic_cast<ConfigurationContext*>(result.get());
    a.checkNonNull("got a ConfigurationContext", cc);
    a.checkNonNull("got a HostConfiguration", dynamic_cast<game::config::UserConfiguration*>(&cc->config()));
}

/* Invocation with wrong type */
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration:error:type", a)
{
    Environment env;
    Segment seg;
    seg.pushBackString("X");
    Arguments args(seg, 0, 1);

    AFL_CHECK_THROWS(a, game::interface::IFConfiguration(env.session, args), Error);
}

/* Invocation with out-of-range value */
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration:error:range", a)
{
    Environment env;
    Segment seg;
    seg.pushBackInteger(3);
    Arguments args(seg, 0, 1);

    AFL_CHECK_THROWS(a, game::interface::IFConfiguration(env.session, args), Error);
}

/* Invocation with too many parameters */
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration:error:arity", a)
{
    Environment env;
    Segment seg;
    seg.pushBackInteger(1);
    seg.pushBackInteger(1);
    Arguments args(seg, 0, 2);

    AFL_CHECK_THROWS(a, game::interface::IFConfiguration(env.session, args), Error);
}

/*
 *  IFConfiguration_Add
 *
 *  More cases in tests for IFAddConfig, IFAddPref.
 */

// Success case
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Add:success", a)
{
    Environment env;
    Segment seg;
    seg.pushBackString("newkey=value");
    Arguments args(seg, 0, 1);

    Ref<Configuration> config = Configuration::create();
    AFL_CHECK_SUCCEEDS(a, game::interface::IFConfiguration_Add(ConfigurationContext::Data(env.session, config), env.proc, args));

    ConfigurationOption* opt = config->getOptionByName("NEWKEY");
    a.checkNonNull("option must have been set", opt);
    a.checkEqual("option value", opt->toString(), "value");
}

// Error case, wrong number of parameters
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Add:error:arity", a)
{
    Environment env;
    Segment seg;
    seg.pushBackString("newkey=value");
    seg.pushBackInteger(9);
    Arguments args(seg, 0, 2);

    Ref<Configuration> config = Configuration::create();
    AFL_CHECK_THROWS(a, game::interface::IFConfiguration_Add(ConfigurationContext::Data(env.session, config), env.proc, args), Error);

    a.checkNull("option must not have been set", config->getOptionByName("NEWKEY"));
}

/*
 *  IFConfiguration_Create
 *
 *  More cases in tests for IFCreateConfigOption, IFCreatePrefOption.
 */

// Success case
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Create:success", a)
{
    Environment env;
    Segment seg;
    seg.pushBackString("newkey");
    seg.pushBackString("int");
    Arguments args(seg, 0, 2);

    Ref<Configuration> config = Configuration::create();
    AFL_CHECK_SUCCEEDS(a, game::interface::IFConfiguration_Create(ConfigurationContext::Data(env.session, config), env.proc, args));

    ConfigurationOption* opt = config->getOptionByName("NEWKEY");
    a.checkNonNull("option must have been created", opt);

    // Set and get: value is parsed as integer inbetween
    opt->set("00003");
    a.checkEqual("option value", opt->toString(), "3");
}

// Error case
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Create:error", a)
{
    Environment env;
    Segment seg;
    seg.pushBackString("newkey");
    Arguments args(seg, 0, 1);

    Ref<Configuration> config = Configuration::create();
    AFL_CHECK_THROWS(a, game::interface::IFConfiguration_Create(ConfigurationContext::Data(env.session, config), env.proc, args), Error);

    a.checkNull("option must not have been created", config->getOptionByName("NEWKEY"));
}

/*
 *  IFConfiguration_Load
 */

// Unary invocation ("Load #30")
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Load:unary", a)
{
    Environment env;

    // Create file
    env.fs.openFile("/file.txt", afl::io::FileSystem::Create)
        ->fullWrite(afl::string::toBytes("first=o1\nsecond=o2\n"));

    // Open file
    env.session.world().fileTable().setMaxFiles(100);
    env.session.world().fileTable().openFile(30, env.fs.openFile("/file.txt", afl::io::FileSystem::OpenRead));

    // Create config
    Ref<Configuration> config = Configuration::create();

    // Load
    Segment seg;
    seg.pushBackInteger(30);
    Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFConfiguration_Load(ConfigurationContext::Data(env.session, config), env.proc, args));

    // Verify
    ConfigurationOption* opt1 = config->getOptionByName("FIRST");
    a.checkNonNull("first option must have been created", opt1);
    a.checkEqual("first option must have correct value", opt1->toString(), "o1");

    ConfigurationOption* opt2 = config->getOptionByName("second");
    a.checkNonNull("second option must have been created", opt2);
    a.checkEqual("second option must have correct value", opt2->toString(), "o2");
}

// Ternary invocation ("Load #30, 'sec', 1")
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Load:ternary", a)
{
    Environment env;

    // Create file
    env.fs.openFile("/file.txt", afl::io::FileSystem::Create)
        ->fullWrite(afl::string::toBytes("first=o1\n%sec\nsecond=o2\n"));

    // Open file
    env.session.world().fileTable().setMaxFiles(100);
    env.session.world().fileTable().openFile(30, env.fs.openFile("/file.txt", afl::io::FileSystem::OpenRead));

    // Create config
    Ref<Configuration> config = Configuration::create();

    // Load
    Segment seg;
    seg.pushBackInteger(30);
    seg.pushBackString("sec");
    seg.pushBackInteger(0);
    Arguments args(seg, 0, 3);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFConfiguration_Load(ConfigurationContext::Data(env.session, config), env.proc, args));

    // Verify
    a.checkNull("first option must not have been created", config->getOptionByName("FIRST"));

    ConfigurationOption* opt2 = config->getOptionByName("second");
    a.checkNonNull("second option must have been created", opt2);
    a.checkEqual("second option must have correct value", opt2->toString(), "o2");
}

// Error: file not open
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Load:error:not-open", a)
{
    Environment env;
    env.session.world().fileTable().setMaxFiles(100);
    Ref<Configuration> config = Configuration::create();

    // Load
    Segment seg;
    seg.pushBackInteger(30);
    Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFConfiguration_Load(ConfigurationContext::Data(env.session, config), env.proc, args), Error);
}

// Error: missing parameter
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Load:error:missing-args", a)
{
    Environment env;
    env.session.world().fileTable().setMaxFiles(100);
    Ref<Configuration> config = Configuration::create();

    // Load
    Segment seg;
    Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFConfiguration_Load(ConfigurationContext::Data(env.session, config), env.proc, args), Error);
}

// Error: type
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Load:error:type", a)
{
    Environment env;
    env.session.world().fileTable().setMaxFiles(100);
    Ref<Configuration> config = Configuration::create();

    // Load
    Segment seg;
    seg.pushBackString("X");
    Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFConfiguration_Load(ConfigurationContext::Data(env.session, config), env.proc, args), Error);
}

// Special case: null file
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Load:null-file", a)
{
    Environment env;
    env.session.world().fileTable().setMaxFiles(100);
    Ref<Configuration> config = Configuration::create();

    // Load
    Segment seg;
    Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFConfiguration_Load(ConfigurationContext::Data(env.session, config), env.proc, args));
}

/*
 *  IFConfiguration_Merge
 */

// Normal case
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Merge:normal", a)
{
    Environment env;
    Ref<Configuration> one = Configuration::create();
    one->setOption("first", "first-one", ConfigurationOption::User);
    one->setOption("both", "both-one", ConfigurationOption::User);

    Ref<Configuration> two = Configuration::create();
    two->setOption("second", "second-two", ConfigurationOption::User);
    two->setOption("both", "both-two", ConfigurationOption::User);

    Segment seg;
    seg.pushBackNew(new ConfigurationContext(env.session, two));
    Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFConfiguration_Merge(ConfigurationContext::Data(env.session, one), env.proc, args));

    ConfigurationOption* o1 = one->getOptionByName("first");
    a.checkNonNull("o1 non-null", o1);
    a.checkEqual("o1 value", o1->toString(), "first-one");

    ConfigurationOption* o2 = one->getOptionByName("second");
    a.checkNonNull("o2 non-null", o2);
    a.checkEqual("o2 value", o2->toString(), "second-two");

    ConfigurationOption* o3 = one->getOptionByName("both");
    a.checkNonNull("o3 non-null", o3);
    a.checkEqual("o3 value", o3->toString(), "both-two");
}

// Merge null
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Merge:null", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();
    config->setOption("a", "value", ConfigurationOption::User);

    Segment seg;
    seg.pushBackNew(0);
    Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFConfiguration_Merge(ConfigurationContext::Data(env.session, config), env.proc, args));
}

// Self-merge
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Merge:self", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();
    config->setOption("a", "value", ConfigurationOption::User);

    Segment seg;
    seg.pushBackNew(new ConfigurationContext(env.session, config));
    Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFConfiguration_Merge(ConfigurationContext::Data(env.session, config), env.proc, args));

    ConfigurationOption* opt = config->getOptionByName("a");
    a.checkNonNull("opt non-null", opt);
    a.checkEqual("opt value", opt->toString(), "value");
}

// Arity error
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Merge:error:arity", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();

    Segment seg;
    Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFConfiguration_Merge(ConfigurationContext::Data(env.session, config), env.proc, args), Error);
}

// Type error
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Merge:error:type", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();

    Segment seg;
    seg.pushBackInteger(42);
    Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFConfiguration_Merge(ConfigurationContext::Data(env.session, config), env.proc, args), Error);
}

/*
 *  IFConfiguration_Subtract
 */

// Normal case
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Subtract:normal", a)
{
    // Same test case as game.config.Configuration:subtract
    Environment env;
    Ref<Configuration> ca = Configuration::create();
    ca->setOption("one", "1a", ConfigurationOption::User);
    ca->setOption("two", "2a", ConfigurationOption::User);
    ca->setOption("three", "3a", ConfigurationOption::User);

    Ref<Configuration> cb = Configuration::create();
    cb->setOption("One", "1a", ConfigurationOption::User);
    cb->setOption("two", "2b", ConfigurationOption::User);

    Segment seg;
    seg.pushBackNew(new ConfigurationContext(env.session, cb));
    Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFConfiguration_Subtract(ConfigurationContext::Data(env.session, ca), env.proc, args));

    ConfigurationOption* p1 = ca->getOptionByName("one");
    a.checkNonNull("01. getOptionByName one", p1);
    a.checkEqual("02. toString", p1->toString(), "1a");
    a.checkEqual("03. getSource", p1->getSource(), ConfigurationOption::Default);

    ConfigurationOption* p2 = ca->getOptionByName("two");
    a.checkNonNull("11. getOptionByName two", p2);
    a.checkEqual("12. toString", p2->toString(), "2a");
    a.checkEqual("13. getSource", p2->getSource(), ConfigurationOption::User);

    ConfigurationOption* p3 = ca->getOptionByName("three");
    a.checkNonNull("21. getOptionByName three", p3);
    a.checkEqual("22. toString", p3->toString(), "3a");
    a.checkEqual("23. getSource", p3->getSource(), ConfigurationOption::User);
}

// Subtract null
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Subtract:null", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();
    config->setOption("a", "value", ConfigurationOption::User);

    Segment seg;
    seg.pushBackNew(0);
    Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFConfiguration_Subtract(ConfigurationContext::Data(env.session, config), env.proc, args));
}

// Self-subtract
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Subtract:error:self", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();
    config->setOption("a", "value", ConfigurationOption::User);

    Segment seg;
    seg.pushBackNew(new ConfigurationContext(env.session, config));
    Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFConfiguration_Subtract(ConfigurationContext::Data(env.session, config), env.proc, args), Error);
}

// Arity error
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Subtract:error:arity", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();

    Segment seg;
    Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFConfiguration_Subtract(ConfigurationContext::Data(env.session, config), env.proc, args), Error);
}

// Type error
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Subtract:error:type", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();

    Segment seg;
    seg.pushBackInteger(42);
    Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFConfiguration_Subtract(ConfigurationContext::Data(env.session, config), env.proc, args), Error);
}

/*
 *  IFConfiguration_Get
 *
 *  More (older) test cases for IFCfg, IFPref
 */

AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Get:normal", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();
    config->setOption("a", "value", ConfigurationOption::User);

    Segment seg;
    seg.pushBackString("A");
    Arguments args(seg, 0, 1);
    String_t result = verifyNewString(a("result value"), game::interface::IFConfiguration_Get(ConfigurationContext::Data(env.session, config), args));

    a.checkEqual("result string", result, "value");
}

/*
 *  Entry
 */

// Successful unary call 'conf->Entry("thename")'
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Entry:call-unary", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();
    config->setOption("thename", "value", ConfigurationOption::User);

    std::auto_ptr<IndexableValue> idx(getEntryProperty(a, env, config));

    Segment seg;
    seg.pushBackString("TheName");
    Arguments args(seg, 0, 1);
    std::auto_ptr<afl::data::Value> result(idx->get(args));

    a.checkNonNull("result of ENTRY() must be non-null", result.get());
    Context* ctx = dynamic_cast<Context*>(result.get());
    a.checkNonNull("result of ENTRY() must be a context", ctx);

    ContextVerifier verif(*ctx, a("ContextVerifier"));
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyTypes();
    a.checkNull("result of ENTRY() must not have an object", ctx->getObject());
    a.check("result of ENTRY() is not iterable", !ctx->next());    // Not contractual

    verif.verifyString("NAME", "TheName");             // Taken from invocation - not contractual
    verif.verifyString("VALUE", "value");
    verif.verifyInteger("SOURCE", 2);

    Context::PropertyIndex_t pi;
    a.checkNull("unresolvable name", ctx->lookup("WHATEVER", pi));
}

// Successful unary call with null argument: 'conf->Entry(EMPTY)'
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Entry:call-null", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();
    config->setOption("thename", "value", ConfigurationOption::User);

    std::auto_ptr<IndexableValue> idx(getEntryProperty(a, env, config));

    Segment seg;
    Arguments args(seg, 0, 1);
    verifyNewNull("result must be null", idx->get(args));
}

// Successful unary call for nonexistant option: 'conf->Entry("WHAT")'
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Entry:call-nonexistant", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();

    std::auto_ptr<IndexableValue> idx(getEntryProperty(a, env, config));

    Segment seg;
    seg.pushBackString("WHAT");
    Arguments args(seg, 0, 1);
    verifyNewNull("result must be null", idx->get(args));
}

// Iteration of empty
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Entry:iterate:empty", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();

    std::auto_ptr<IndexableValue> idx(getEntryProperty(a, env, config));
    verifyNewNull("Iteration must be empty", idx->makeFirstContext());
}

// Iteration of non-empty
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Entry:iterate:nonempty", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();
    config->setOption("first", "firstValue", ConfigurationOption::User);
    config->setOption("second", "secondValue", ConfigurationOption::Game);

    std::auto_ptr<IndexableValue> idx(getEntryProperty(a, env, config));
    std::auto_ptr<Context> ctx(idx->makeFirstContext());
    a.checkNonNull("must have context", ctx.get());

    bool hasFirst = false;
    bool hasSecond = false;
    do {
        ContextVerifier verif(*ctx, a("ContextVerifier"));
        verif.verifyBasics();
        verif.verifyTypes();

        String_t key = verifyNewString(a("NAME must be string"), verif.getValue("NAME"));
        if (key == "first") {
            a.check("'first' must appear only once", !hasFirst);
            verif.verifyString("VALUE", "firstValue");
            verif.verifyInteger("SOURCE", 2);
            hasFirst = true;
        } else if (key == "second") {
            a.check("'second' must appear only once", !hasSecond);
            verif.verifyString("VALUE", "secondValue");
            verif.verifyInteger("SOURCE", 3);
            hasSecond = true;
        } else {
            a.fail("unexpected key " + key);
        }
    } while (ctx->next());
    a.check("must have 'first'", hasFirst);
    a.check("must have 'second'", hasSecond);
}

// Error: attempt to assign 'conf->Entry("thename") := ...'
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Entry:error:assign", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();
    config->setOption("thename", "value", ConfigurationOption::User);

    std::auto_ptr<IndexableValue> idx(getEntryProperty(a, env, config));

    Segment seg;
    seg.pushBackString("TheName");
    Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, idx->set(args, 0), Error);
}

// Error: invocation with wrong number of argumements: 'conf->Entry()'
AFL_TEST("game.interface.ConfigurationContext:IFConfiguration_Entry:error:arity", a)
{
    Environment env;
    Ref<Configuration> config = Configuration::create();
    config->setOption("thename", "value", ConfigurationOption::User);

    std::auto_ptr<IndexableValue> idx(getEntryProperty(a, env, config));

    Segment seg;
    Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, idx->get(args), Error);
}
