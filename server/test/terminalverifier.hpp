/**
  *  \file server/test/terminalverifier.hpp
  *  \brief Verification of Terminal implementations
  */
#ifndef C2NG_SERVER_TEST_TERMINALVERIFIER_HPP
#define C2NG_SERVER_TEST_TERMINALVERIFIER_HPP

#include "afl/io/internaltextwriter.hpp"
#include "afl/test/assert.hpp"

namespace server { namespace test {

    /** Common function to verify an interactive terminal instance.
        \tparam T type to verify
        \param a Asserter (to use in failure messages) */
    template<typename T>
    void verifyInteractiveTerminal(afl::test::Assert a)
    {
        using afl::io::InternalTextWriter;

        // printBanner (goes to out)
        {
            InternalTextWriter out, err;
            T(out, err).printBanner();
            a.check("printBanner out", out.getContent().size() > 0);
            a.check("printBanner err", err.getContent().empty());
        }

        // printPrimaryPrompt (goes to out)
        {
            InternalTextWriter out, err;
            server::console::ContextStack_t stack;
            T(out, err).printPrimaryPrompt(stack);
            a.check("printPrimaryPrompt out", out.getContent().size() > 0);
            a.check("printPrimaryPrompt err", err.getContent().empty());
        }

        // printSecondaryPrompt (goes to out)
        {
            InternalTextWriter out, err;
            T(out, err).printSecondaryPrompt();
            a.check("printSecondaryPrompt out", out.getContent().size() > 0);
            a.check("printSecondaryPrompt err", err.getContent().empty());
        }

        // printError (goes to err)
        {
            InternalTextWriter out, err;
            T(out, err).printError("boom");
            a.check("printError out", out.getContent().empty());
            a.check("printError err", err.getContent().size() > 0);
        }

        // printResultPrefix/printResultSuffix (goes to out)
        {
            InternalTextWriter out, err;
            T(out, err).printResultPrefix();
            T(out, err).printResultSuffix();
            a.check("printResult out", out.getContent().size() > 0);
            a.check("printResult err", err.getContent().empty());
        }

        // printMessage (goes to out)
        {
            InternalTextWriter out, err;
            T(out, err).printMessage("hi");
            a.check("printMessage out", out.getContent().size() > 0);
            a.check("printMessage err", err.getContent().empty());
        }
    }

} }

#endif
