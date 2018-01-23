/**
  *  \file u/t_server_console_terminal.cpp
  *  \brief Test for server::console::Terminal
  */

#include "server/console/terminal.hpp"

#include "t_server_console.hpp"

/** Interface test. */
void
TestServerConsoleTerminal::testInterface()
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
void
TestServerConsoleTerminal::testPack()
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
        TS_ASSERT_EQUALS(server::console::Terminal::packContextStack(st), "");
    }

    // Single
    {
        server::console::ContextStack_t st;
        st.pushBackNew(new NullContext("n"));
        TS_ASSERT_EQUALS(server::console::Terminal::packContextStack(st), "n");
    }

    // Two
    {
        server::console::ContextStack_t st;
        st.pushBackNew(new NullContext("n"));
        st.pushBackNew(new NullContext("qq"));
        TS_ASSERT_EQUALS(server::console::Terminal::packContextStack(st), "n qq");
    }
}
