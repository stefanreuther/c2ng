/**
  *  \file test/server/console/pipeterminaltest.cpp
  *  \brief Test for server::console::PipeTerminal
  */

#include "server/console/pipeterminal.hpp"

#include "afl/io/internaltextwriter.hpp"
#include "afl/test/testrunner.hpp"

using afl::io::InternalTextWriter;

// printBanner (none)
AFL_TEST("server.console.PipeTerminal:printBanner", a)
{
    InternalTextWriter out, err;
    server::console::PipeTerminal(out, err).printBanner();
    a.check("out", out.getContent().empty());
    a.check("err", err.getContent().empty());
}

// printPrimaryPrompt (empty)
AFL_TEST("server.console.PipeTerminal:printPrimaryPrompt", a)
{
    InternalTextWriter out, err;
    server::console::ContextStack_t stack;
    server::console::PipeTerminal(out, err).printPrimaryPrompt(stack);
    a.check("out", out.getContent().empty());
    a.check("err", err.getContent().empty());
}

// printSecondaryPrompt (empty)
AFL_TEST("server.console.PipeTerminal:printSecondaryPrompt", a)
{
    InternalTextWriter out, err;
    server::console::PipeTerminal(out, err).printSecondaryPrompt();
    a.check("out", out.getContent().empty());
    a.check("err", err.getContent().empty());
}

// printError (goes to err)
AFL_TEST("server.console.PipeTerminal:printError", a)
{
    InternalTextWriter out, err;
    server::console::PipeTerminal(out, err).printError("boom");
    a.check("out", out.getContent().empty());
    a.check("err", err.getContent().size() > 0);
}

// printResultPrefix/printResultSuffix (goes to out)
AFL_TEST("server.console.PipeTerminal:printResultSuffix", a)
{
    InternalTextWriter out, err;
    server::console::PipeTerminal(out, err).printResultPrefix();
    server::console::PipeTerminal(out, err).printResultSuffix();
    a.check("out", out.getContent().size() > 0);
    a.check("err", err.getContent().empty());
}

// printMessage (goes to out)
AFL_TEST("server.console.PipeTerminal:printMessage", a)
{
    InternalTextWriter out, err;
    server::console::PipeTerminal(out, err).printMessage("hi");
    a.check("out", out.getContent().size() > 0);
    a.check("err", err.getContent().empty());
}
