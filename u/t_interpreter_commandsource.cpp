/**
  *  \file u/t_interpreter_commandsource.cpp
  *  \brief Test for interpreter::CommandSource
  */

#include "interpreter/commandsource.hpp"

#include "t_interpreter.hpp"

/** Interface test. */
void
TestInterpreterCommandSource::testIt()
{
    class Tester : public interpreter::CommandSource {
     public:
        virtual void readNextLine()
            { }
        virtual bool setCharsetNew(afl::charset::Charset* /*cs*/)
            { return false; }
        virtual void addTraceTo(interpreter::Error& /*e*/, afl::string::Translator& /*tx*/)
            { }

        using CommandSource::setNextLine;
        using CommandSource::setEOF;
    };
    Tester t;

    // Initial state
    TS_ASSERT(t.isEOF());                      // reports EOF before first line!
    TS_ASSERT_EQUALS(t.getLineNumber(), 0);

    // Set a new line
    t.setNextLine("a");
    TS_ASSERT(!t.isEOF());
    TS_ASSERT_EQUALS(t.getLineNumber(), 1);
    TS_ASSERT_EQUALS(t.tokenizer().getCurrentToken(), interpreter::Tokenizer::tIdentifier);
    TS_ASSERT_EQUALS(t.tokenizer().getCurrentString(), "A");

    // Set another line
    t.setNextLine("1+2+3");
    TS_ASSERT(!t.isEOF());
    TS_ASSERT_EQUALS(t.getLineNumber(), 2);

    // Set end of file
    t.setEOF();
    TS_ASSERT(t.isEOF());
    TS_ASSERT_EQUALS(t.getLineNumber(), 2);
}
