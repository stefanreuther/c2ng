/**
  *  \file test/game/interface/commandtasktest.cpp
  *  \brief Test for game::interface::CommandTask
  */

#include "game/interface/commandtask.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/subroutinevalue.hpp"

using game::interface::CommandTask;
using game::interface::ContextProvider;

namespace {
    /* Collect log messages/channels in a string */
    class LogCollector : public afl::sys::LogListener {
     public:
        LogCollector()
            : m_out()
            { }
        virtual void handleMessage(const Message& msg)
            {
                if (msg.m_channel != "interpreter.process") {
                    m_out += msg.m_channel;
                    m_out += ":";
                    m_out += msg.m_message;
                    m_out += "\n";
                }
            }
        const String_t& get()
            { return m_out; }
     private:
        String_t m_out;
    };

    /* Environment */
    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        LogCollector log;

        Environment()
            : tx(), fs(), session(tx, fs), log()
            { session.log().addListener(log); }
    };

    /* Run a CommandTask on an Environment, return log output. */
    String_t runTest(Environment& env, CommandTask& task)
    {
        uint32_t pgid = env.session.processList().allocateProcessGroup();
        task.execute(pgid, env.session);
        env.session.processList().startProcessGroup(pgid);
        env.session.processList().run(0);
        return env.log.get();
    }

    /* Make a function that terminates the process.
       Equivalent to
          Function <name>
            End
          EndFunction */
    afl::data::Value* makeTerminateFunction()
    {
        interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
        bco->addInstruction(interpreter::Opcode::maSpecial, interpreter::Opcode::miSpecialTerminate, 0);
        return new interpreter::SubroutineValue(bco);
    }
}

/*
 *  Test cases
 *
 *  Each scenario is run in verbose and non-verbose mode.
 *  We track output.
 *  CommandTask's responsibility is to provide (most) log messages.
 */

// Command
AFL_TEST("game.interface.CommandTask:command:verbose", a)
{
    CommandTask testee("print 'hi'", true, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    String_t output = runTest(env, testee);
    a.checkEqual("output", output,
                 "script.input:print 'hi'\n"
                 "script:hi\n");
}

AFL_TEST("game.interface.CommandTask:command:silent", a)
{
    CommandTask testee("print 'hi'", false, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    String_t output = runTest(env, testee);
    a.checkEqual("output", output,
                 "script:hi\n");
}

// Expression
AFL_TEST("game.interface.CommandTask:expr:verbose", a)
{
    CommandTask testee("42*23", true, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    String_t output = runTest(env, testee);
    a.checkEqual("output", output,
                 "script.input:42*23\n"
                 "script.result:966\n");
}

AFL_TEST("game.interface.CommandTask:expr:silent", a)
{
    CommandTask testee("42*23", false, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    String_t output = runTest(env, testee);

    a.checkEqual("output", output, "");
}

// Expression that returns null
AFL_TEST("game.interface.CommandTask:null-expr:verbose", a)
{
    CommandTask testee("z(0)", true, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    String_t output = runTest(env, testee);

    a.checkEqual("output", output,
                 "script.input:z(0)\n"
                 "script.empty:Empty\n");
}

AFL_TEST("game.interface.CommandTask:null-expr:silent", a)
{
    CommandTask testee("z(0)", false, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    String_t output = runTest(env, testee);

    a.checkEqual("output", output, "");
}

// Stop (status Suspended)
AFL_TEST("game.interface.CommandTask:stop:verbose", a)
{
    CommandTask testee("stop", true, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    String_t output = runTest(env, testee);

    a.checkEqual("output", output,
                 "script.input:stop\n"
                 "script.state:Suspended.\n");
}

AFL_TEST("game.interface.CommandTask:stop:silent", a)
{
    CommandTask testee("stop", false, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    String_t output = runTest(env, testee);

    a.checkEqual("output", output, "");
}

// End (status Terminated)
AFL_TEST("game.interface.CommandTask:terminated:verbose", a)
{
    CommandTask testee("terminateSelf()", true, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    env.session.world().setNewGlobalValue("TERMINATESELF", makeTerminateFunction());
    String_t output = runTest(env, testee);

    a.checkEqual("output", output,
                 "script.input:terminateSelf()\n"
                 "script.state:Terminated.\n");
}

AFL_TEST("game.interface.CommandTask:terminated:silent", a)
{
    CommandTask testee("terminateSelf()", false, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    env.session.world().setNewGlobalValue("TERMINATESELF", makeTerminateFunction());
    String_t output = runTest(env, testee);

    a.checkEqual("output", output, "");
}

// Parse error during compilation
AFL_TEST("game.interface.CommandTask:parse-error:verbose", a)
{
    CommandTask testee("a))", true, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    String_t output = runTest(env, testee);

    a.checkEqual("output", output,
                 "script.input:a))\n"
                 "script.error:Expression incorrectly terminated (missing operator?)\n");
}

AFL_TEST("game.interface.CommandTask:parse-error:silent", a)
{
    CommandTask testee("a))", false, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    String_t output = runTest(env, testee);

    a.checkEqual("output", output,
                 "script.error:Expression incorrectly terminated (missing operator?)\n");
}

// Process crashes (Failed)
AFL_TEST("game.interface.CommandTask:failed:verbose", a)
{
    CommandTask testee("abort 'boo'", true, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    String_t output = runTest(env, testee);

    a.checkEqual("output", output,
                 "script.input:abort 'boo'\n"
                 "script.error:boo\n");
}

AFL_TEST("game.interface.CommandTask:failed:silent", a)
{
    CommandTask testee("abort 'boo'", false, "name", std::auto_ptr<ContextProvider>());
    Environment env;
    String_t output = runTest(env, testee);

    a.checkEqual("output", output,
                 "script.error:boo\n");
}
