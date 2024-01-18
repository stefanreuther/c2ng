/**
  *  \file test/interpreter/commandsourcetest.cpp
  *  \brief Test for interpreter::CommandSource
  */

#include "interpreter/commandsource.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST("interpreter.CommandSource", a)
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
    a.check("01. isEOF", t.isEOF());                      // reports EOF before first line!
    a.checkEqual("02. getLineNumber", t.getLineNumber(), 0);

    // Set a new line
    t.setNextLine("a");
    a.check("11. isEOF", !t.isEOF());
    a.checkEqual("12. getLineNumber", t.getLineNumber(), 1);
    a.checkEqual("13. getCurrentToken", t.tokenizer().getCurrentToken(), interpreter::Tokenizer::tIdentifier);
    a.checkEqual("14. getCurrentString", t.tokenizer().getCurrentString(), "A");

    // Set another line
    t.setNextLine("1+2+3");
    a.check("21. isEOF", !t.isEOF());
    a.checkEqual("22. getLineNumber", t.getLineNumber(), 2);

    // Set end of file
    t.setEOF();
    a.check("31. isEOF", t.isEOF());
    a.checkEqual("32. getLineNumber", t.getLineNumber(), 2);
}
