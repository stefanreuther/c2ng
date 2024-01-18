/**
  *  \file test/server/console/terminaltest.cpp
  *  \brief Test for server::console::Terminal
  */

#include "server/console/terminal.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.console.Terminal:interface")
{
    class Tester : public server::console::Terminal {
     public:
        virtual void printBanner()
            { }
        virtual void printPrimaryPrompt(const server::console::ContextStack_t& /*st*/)
            { }
        virtual void printSecondaryPrompt()
            { }
        virtual void printError(String_t /*msg*/)
            { }
        virtual void printResultPrefix()
            { }
        virtual void printResultSuffix()
            { }
        virtual void printMessage(String_t /*s*/)
            { }
    };
    Tester t;
}

/** Test packContextStack(). */
AFL_TEST("server.console.Terminal:packContextStack", a)
{
    class NullContext : public server::console::Context {
     public:
        NullContext(String_t name)
            : m_name(name)
            { }
        virtual bool call(const String_t& /*cmd*/, interpreter::Arguments /*args*/, server::console::Parser& /*parser*/, std::auto_ptr<afl::data::Value>& /*result*/)
            { return false; }
        virtual String_t getName()
            { return m_name; }
     private:
        String_t m_name;
    };

    // Empty
    {
        server::console::ContextStack_t st;
        a.checkEqual("01. empty", server::console::Terminal::packContextStack(st), "");
    }

    // Single
    {
        server::console::ContextStack_t st;
        st.pushBackNew(new NullContext("n"));
        a.checkEqual("11. singla", server::console::Terminal::packContextStack(st), "n");
    }

    // Two
    {
        server::console::ContextStack_t st;
        st.pushBackNew(new NullContext("n"));
        st.pushBackNew(new NullContext("qq"));
        a.checkEqual("21. two", server::console::Terminal::packContextStack(st), "n qq");
    }
}
