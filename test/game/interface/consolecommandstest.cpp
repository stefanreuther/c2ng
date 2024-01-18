/**
  *  \file test/game/interface/consolecommandstest.cpp
  *  \brief Test for game::interface::ConsoleCommands
  */

#include "game/interface/consolecommands.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/io/nulltextwriter.hpp"
#include "afl/io/textreader.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "util/unicodechars.hpp"

/**
  *  \file u/t_game_interface_consolecommands.cpp
  *  \brief Test for game::interface::ConsoleCommands
  */



namespace {
    class TextReader : public afl::io::TextReader {
     public:
        TextReader()
            : m_list(), m_pos()
            { }
        void add(const String_t& str)
            { m_list.push_back(str); }

     protected:
        virtual bool doReadLine(String_t& out)
            {
                if (m_pos < m_list.size()) {
                    out = m_list[m_pos++];
                    return true;
                } else {
                    return false;
                }
            }
     private:
        std::vector<String_t> m_list;
        size_t m_pos;
    };


    struct Environment {
        afl::io::NullFileSystem fs;
        afl::string::NullTranslator tx;
        game::Session session;
        interpreter::Process proc;
        afl::base::Ref<TextReader> input;

        Environment()
            : fs(), tx(), session(tx, fs), proc(session.world(), "tester", 777),
              input(*new TextReader())
            {
                game::interface::registerConsoleCommands(session, input, *new afl::io::NullTextWriter());

                proc.pushFrame(interpreter::BytecodeObject::create(true), false)
                    .localNames.add("UI.RESULT");
            }
    };

    void run(Environment& env, afl::test::Assert a, afl::data::Segment& seg)
    {
        // Value must exist
        interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(env.session.world().getGlobalValue("UI.INPUT"));
        a.check("cv != 0", cv != 0);

        // Verify that value is sensible
        interpreter::test::ValueVerifier vv(*cv, a);
        vv.verifyBasics();

        // Invoke it
        cv->call(env.proc, seg, false);
    }
}

// Normal case
AFL_TEST("game.interface.ConsoleCommands:UI.Input:normal", a)
{
    Environment env;
    env.input->add("hi");

    afl::data::Segment seg;
    seg.pushBackString("prompt");
    run(env, a, seg);

    interpreter::test::verifyNewString(a, env.proc.getVariable("UI.RESULT").release(), "hi");
}

// EOF
AFL_TEST("game.interface.ConsoleCommands:UI.Input:eof", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("prompt");
    run(env, a, seg);

    interpreter::test::verifyNewNull(a, env.proc.getVariable("UI.RESULT").release());
}

// Controls refused by default
AFL_TEST("game.interface.ConsoleCommands:UI.Input:reject:controls", a)
{
    Environment env;
    env.input->add("foo\bar");
    env.input->add("ok");

    afl::data::Segment seg;
    seg.pushBackString("prompt");
    run(env, a, seg);

    interpreter::test::verifyNewString(a, env.proc.getVariable("UI.RESULT").release(), "ok");
}

// Numeric, with failure
AFL_TEST("game.interface.ConsoleCommands:UI.Input:reject:non-numeric", a)
{
    Environment env;
    env.input->add("fail");
    env.input->add("125");

    afl::data::Segment seg;
    seg.pushBackString("prompt");
    seg.pushBackString("title");
    seg.pushBackInteger(20);
    seg.pushBackString("n");
    run(env, a, seg);

    interpreter::test::verifyNewString(a, env.proc.getVariable("UI.RESULT").release(), "125");
}

// No high-ascii, with failure
AFL_TEST("game.interface.ConsoleCommands:UI.Input:reject:high-ascii", a)
{
    Environment env;
    env.input->add(UTF_LEFT_ARROW);
    env.input->add("fine");

    afl::data::Segment seg;
    seg.pushBackString("prompt");
    seg.pushBackString("title");
    seg.pushBackInteger(20);
    seg.pushBackString("h");
    run(env, a, seg);

    interpreter::test::verifyNewString(a, env.proc.getVariable("UI.RESULT").release(), "fine");
}

// Length overflow: input is truncated
AFL_TEST("game.interface.ConsoleCommands:UI.Input:length-limit", a)
{
    Environment env;
    env.input->add("excess");

    afl::data::Segment seg;
    seg.pushBackString("prompt");
    seg.pushBackString("title");
    seg.pushBackInteger(4);
    run(env, a, seg);

    interpreter::test::verifyNewString(a, env.proc.getVariable("UI.RESULT").release(), "exce");
}

// Null prompt
AFL_TEST("game.interface.ConsoleCommands:UI.Input:null-prompt", a)
{
    Environment env;
    env.input->add("not read");

    afl::data::Segment seg;
    seg.pushBackNew(0);
    run(env, a, seg);

    interpreter::test::verifyNewNull(a, env.proc.getVariable("UI.RESULT").release());
}

// Arity error
AFL_TEST("game.interface.ConsoleCommands:UI.Input:error:arity", a)
{
    Environment env;

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, run(env, a, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.ConsoleCommands:UI.Input:error:type", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("prompt");
    seg.pushBackString("title");
    seg.pushBackString("err");
    AFL_CHECK_THROWS(a, run(env, a, seg), interpreter::Error);
}

// Flag error
AFL_TEST("game.interface.ConsoleCommands:UI.Input:error:flag", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("prompt");
    seg.pushBackString("title");
    seg.pushBackInteger(20);
    seg.pushBackString("xyzzy");
    AFL_CHECK_THROWS(a, run(env, a, seg), interpreter::Error);
}
