/**
  *  \file u/t_server_console_terminal.hpp
  */
#ifndef C2NG_U_T_SERVER_CONSOLE_TERMINAL_HPP
#define C2NG_U_T_SERVER_CONSOLE_TERMINAL_HPP

#include "afl/io/internaltextwriter.hpp"

/** Common function to verify an interactive terminal instance.
    \tparam T type to verify
    \param msg Name (to use in failure messages) */
template<typename T>
void verifyInteractiveTerminal(const char* msg)
{
    using afl::io::InternalTextWriter;

    // printBanner (goes to out)
    {
        InternalTextWriter out, err;
        T(out, err).printBanner();
        TSM_ASSERT(msg, out.getContent().size() > 0);
        TSM_ASSERT(msg, err.getContent().empty());
    }

    // printPrimaryPrompt (goes to out)
    {
        InternalTextWriter out, err;
        server::console::ContextStack_t stack;
        T(out, err).printPrimaryPrompt(stack);
        TSM_ASSERT(msg, out.getContent().size() > 0);
        TSM_ASSERT(msg, err.getContent().empty());
    }

    // printSecondaryPrompt (goes to out)
    {
        InternalTextWriter out, err;
        T(out, err).printSecondaryPrompt();
        TSM_ASSERT(msg, out.getContent().size() > 0);
        TSM_ASSERT(msg, err.getContent().empty());
    }

    // printError (goes to err)
    {
        InternalTextWriter out, err;
        T(out, err).printError("boom");
        TSM_ASSERT(msg, out.getContent().empty());
        TSM_ASSERT(msg, err.getContent().size() > 0);
    }

    // printResultPrefix/printResultSuffix (goes to out)
    {
        InternalTextWriter out, err;
        T(out, err).printResultPrefix();
        T(out, err).printResultSuffix();
        TSM_ASSERT(msg, out.getContent().size() > 0);
        TSM_ASSERT(msg, err.getContent().empty());
    }

    // printMessage (goes to out)
    {
        InternalTextWriter out, err;
        T(out, err).printMessage("hi");
        TSM_ASSERT(msg, out.getContent().size() > 0);
        TSM_ASSERT(msg, err.getContent().empty());
    }
}

#endif
