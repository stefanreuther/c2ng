/**
  *  \file u/t_server_console_pipeterminal.cpp
  *  \brief Test for server::console::PipeTerminal
  */

#include "server/console/pipeterminal.hpp"

#include "t_server_console.hpp"
#include "afl/io/internaltextwriter.hpp"

void
TestServerConsolePipeTerminal::testIt()
{
    using afl::io::InternalTextWriter;

    // printBanner (none)
    {
        InternalTextWriter out, err;
        server::console::PipeTerminal(out, err).printBanner();
        TS_ASSERT(out.getContent().empty());
        TS_ASSERT(err.getContent().empty());
    }

    // printPrimaryPrompt (empty)
    {
        InternalTextWriter out, err;
        server::console::ContextStack_t stack;
        server::console::PipeTerminal(out, err).printPrimaryPrompt(stack);
        TS_ASSERT(out.getContent().empty());
        TS_ASSERT(err.getContent().empty());
    }

    // printSecondaryPrompt (empty)
    {
        InternalTextWriter out, err;
        server::console::PipeTerminal(out, err).printSecondaryPrompt();
        TS_ASSERT(out.getContent().empty());
        TS_ASSERT(err.getContent().empty());
    }

    // printError (goes to err)
    {
        InternalTextWriter out, err;
        server::console::PipeTerminal(out, err).printError("boom");
        TS_ASSERT(out.getContent().empty());
        TS_ASSERT(err.getContent().size() > 0);
    }

    // printResultPrefix/printResultSuffix (goes to out)
    {
        InternalTextWriter out, err;
        server::console::PipeTerminal(out, err).printResultPrefix();
        server::console::PipeTerminal(out, err).printResultSuffix();
        TS_ASSERT(out.getContent().size() > 0);
        TS_ASSERT(err.getContent().empty());
    }

    // printMessage (goes to out)
    {
        InternalTextWriter out, err;
        server::console::PipeTerminal(out, err).printMessage("hi");
        TS_ASSERT(out.getContent().size() > 0);
        TS_ASSERT(err.getContent().empty());
    }
}

