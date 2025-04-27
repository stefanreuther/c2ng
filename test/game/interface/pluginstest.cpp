/**
  *  \file test/game/interface/pluginstest.cpp
  *  \brief Test for game::interface::Plugins
  */

#include "game/interface/plugins.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/loglistener.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "interpreter/simpleprocedure.hpp"
#include "interpreter/values.hpp"

using util::plugin::Plugin;

namespace {
    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::InternalFileSystem fs;
        game::Session session;

        Environment()
            : tx(), fs(), session(tx, fs)
            { }
    };

    void IFSaveString(String_t& out, interpreter::Process& /*proc*/, interpreter::Arguments& args)
    {
        args.checkArgumentCount(1);
        out = interpreter::toString(args.getNext(), false);
    }

    typedef interpreter::SimpleProcedure<String_t&> StringProcedure_t;
}

/** Test LoadResource / Plugin::ResourceFile.
    A: create an environment containing a plugin with a ResourceFile. Create a plugin loader.
    E: LoadResource is called. */
AFL_TEST("game.interface.Plugins:createPluginLoader:ResourceFile", a)
{
    Environment env;

    // Define a plugin
    Plugin* plug = new util::plugin::Plugin("T");
    plug->addItem(Plugin::ResourceFile, "thefile.res");
    env.session.plugins().addNewPlugin(plug);

    // Capture the LoadResource call
    String_t savedString;
    env.session.world().setNewGlobalValue("LOADRESOURCE", new StringProcedure_t(savedString, IFSaveString));

    // Test code
    interpreter::Process proc(env.session.world(), "testLoadResource", 99);
    proc.pushFrame(game::interface::createPluginLoader(*plug), false);
    proc.run(0);

    // Verify
    a.checkEqual("01. getState", proc.getState(), interpreter::Process::Ended);
    a.checkEqual("02. savedString", savedString, "thefile.res");
}

/** Test LoadResource / Plugin::HelpFile.
    A: create an environment containing a plugin with a HelpFile. Create a plugin loader.
    E: LoadHelpFile is called. */
AFL_TEST("game.interface.Plugins:createPluginLoader:HelpFile", a)
{
    Environment env;

    // Define a plugin
    Plugin* plug = new util::plugin::Plugin("H");
    plug->addItem(Plugin::HelpFile, "helpme.xml");
    env.session.plugins().addNewPlugin(plug);

    // Capture the LoadResource call
    String_t savedString;
    env.session.world().setNewGlobalValue("LOADHELPFILE", new StringProcedure_t(savedString, IFSaveString));

    // Test code
    interpreter::Process proc(env.session.world(), "testLoadHelpFile", 99);
    proc.pushFrame(game::interface::createPluginLoader(*plug), false);
    proc.run(0);

    // Verify
    a.checkEqual("01. getState", proc.getState(), interpreter::Process::Ended);
    a.checkEqual("02. savedString", savedString, "helpme.xml");
}

/** Test Load / Plugin::ScriptFile.
    A: create an environment containing a plugin with a ScriptFile. Create a plugin loader.
    E: Script is loaded and executed. */
AFL_TEST("game.interface.Plugins:createPluginLoader:ScriptFile", a)
{
    Environment env;

    // Define a plugin
    Plugin* plug = new util::plugin::Plugin("Q");
    plug->addItem(Plugin::ScriptFile, "sf.q");
    plug->setBaseDirectory("qd");
    env.session.plugins().addNewPlugin(plug);

    // Create the script file
    env.fs.createDirectory("qd");
    env.fs.openFile("qd/sf.q", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("remember 'text'\n"));

    // Capture the script execution
    String_t savedString;
    env.session.world().setNewGlobalValue("REMEMBER", new StringProcedure_t(savedString, IFSaveString));

    // Test code
    interpreter::Process proc(env.session.world(), "testLoadScript", 99);
    proc.pushFrame(game::interface::createPluginLoader(*plug), false);
    proc.run(0);

    // Verify
    a.checkEqual("01. getState", proc.getState(), interpreter::Process::Ended);
    a.checkEqual("02. savedString", savedString, "text");
}

/** Test Eval / Plugin::Command.
    A: create an environment containing a plugin with a Command. Create a plugin loader.
    E: Command is executed. */
AFL_TEST("game.interface.Plugins:createPluginLoader:Command", a)
{
    Environment env;

    // Define a plugin
    Plugin* plug = new util::plugin::Plugin("C");
    plug->addItem(Plugin::Command, "remember 'this'");
    env.session.plugins().addNewPlugin(plug);

    // Capture the script execution
    String_t savedString;
    env.session.world().setNewGlobalValue("REMEMBER", new StringProcedure_t(savedString, IFSaveString));

    // Test code
    interpreter::Process proc(env.session.world(), "testExecScript", 99);
    proc.pushFrame(game::interface::createPluginLoader(*plug), false);
    proc.run(0);

    // Verify
    a.checkEqual("01. getState", proc.getState(), interpreter::Process::Ended);
    a.checkEqual("02. savedString", savedString, "this");
}

/** Test createLoaderForUnloadedPlugins().
    A: create an environment with multiple plugins, partly loaded. Call createLoaderForUnloadedPlugins().
    E: Only unloaded plugins are loaded. */
AFL_TEST("game.interface.Plugins:createLoaderForUnloadedPlugins", a)
{
    Environment env;

    // Create some plugins
    // - A is standalone, not loaded
    {
        Plugin* plug = new Plugin("A");
        plug->addItem(Plugin::Command, "RA 'one'");
        plug->setLoaded(false);
        env.session.plugins().addNewPlugin(plug);
    }

    // - B is standalone, loaded
    {
        Plugin* plug = new Plugin("B");
        plug->addItem(Plugin::Command, "RB 'two'");
        plug->setLoaded(true);
        env.session.plugins().addNewPlugin(plug);
    }

    // - C requires D, not loaded
    // Must initialize from file because only that can provide dependencies
    {
        const char*const FILE =
            "exec = rc 'three' & d\n"
            "requires = d\n";
        afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

        Plugin* plug = new Plugin("C");
        plug->initFromPluginFile("d", "c.c2p", ms, env.session.log(), env.tx);
        plug->setLoaded(false);
        env.session.plugins().addNewPlugin(plug);
    }

    // - D is standalone, not loaded
    {
        Plugin* plug = new Plugin("D");
        plug->addItem(Plugin::Command, "RD 'four'");
        plug->addItem(Plugin::Command, "d := 'x'");   // marker for C to recognize that D is loaded
        plug->setLoaded(false);
        env.session.plugins().addNewPlugin(plug);
    }

    // Capture the script execution
    // There is no guarantee for the order of execution, other than that dependencies need to be met.
    String_t savedStrings[4];
    env.session.world().setNewGlobalValue("RA", new StringProcedure_t(savedStrings[0], IFSaveString));
    env.session.world().setNewGlobalValue("RB", new StringProcedure_t(savedStrings[1], IFSaveString));
    env.session.world().setNewGlobalValue("RC", new StringProcedure_t(savedStrings[2], IFSaveString));
    env.session.world().setNewGlobalValue("RD", new StringProcedure_t(savedStrings[3], IFSaveString));

    // Test code
    interpreter::Process proc(env.session.world(), "testUnloaded", 99);
    proc.pushFrame(game::interface::createLoaderForUnloadedPlugins(env.session.plugins()), false);
    proc.run(0);

    // Verify
    a.checkEqual("01. getState", proc.getState(), interpreter::Process::Ended);
    a.checkEqual("02. savedString", savedStrings[0], "one");
    a.checkEqual("03. savedString", savedStrings[1], "");          // did not execute
    a.checkEqual("04. savedString", savedStrings[2], "threex");
    a.checkEqual("05. savedString", savedStrings[3], "four");
}

/** Test createFileLoader(), success case.
    A: create a script file, create a file loader
    E: file loaded and executed correctly */
AFL_TEST("game.interface.Plugins:createFileLoader:success", a)
{
    Environment env;

    // Create a file
    env.fs.openFile("x.q", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("remember 'load'\n"));

    // Capture the script execution
    String_t savedString;
    env.session.world().setNewGlobalValue("REMEMBER", new StringProcedure_t(savedString, IFSaveString));

    // Test code
    interpreter::Process proc(env.session.world(), "testUnloaded", 99);
    proc.pushFrame(game::interface::createFileLoader("x.q", "origin", false), false);
    proc.run(0);

    // Verify
    a.checkEqual("01. getState", proc.getState(), interpreter::Process::Ended);
    a.checkEqual("02. savedString", savedString, "load");
}

/** Test createFileLoader(), failure case.
    A: create a file loader but no script file
    E: load succeeds but an error is reported

    The error message is a regular script message, not flagged as error.
    There are additional messages (process state change), so as of 20250427, this will see getNumMessages()=3.
    Guaranteed is that we have more than zero and, if the file is optional, fewer. */
AFL_TEST("game.interface.Plugins:createFileLoader:failure", a)
{
    size_t baseline;

    // File is required
    {
        Environment env;

        // Capture logs
        afl::test::LogListener c;
        env.session.log().addListener(c);

        // Test code
        interpreter::Process proc(env.session.world(), "testUnloaded", 99);
        proc.pushFrame(game::interface::createFileLoader("x.q", "origin", false), false);
        proc.run(0);

        // Verify
        a.checkEqual("01. getState", proc.getState(), interpreter::Process::Ended);
        a.check("02. getNumMessages", c.getNumMessages() > 0);

        baseline = c.getNumMessages();
    }

    // File is optional. Should generate fewer messages.
    {
        Environment env;

        // Capture logs
        afl::test::LogListener c;
        env.session.log().addListener(c);

        // Test code
        interpreter::Process proc(env.session.world(), "testUnloaded", 99);
        proc.pushFrame(game::interface::createFileLoader("x.q", "origin", true), false);
        proc.run(0);

        // Verify
        a.checkEqual("01. getState", proc.getState(), interpreter::Process::Ended);
        a.check("02. getNumMessages", c.getNumMessages() < baseline);
    }
}
