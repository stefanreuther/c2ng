/**
  *  \file server/console/terminal.hpp
  *  \brief Interface server::console::Terminal
  */
#ifndef C2NG_SERVER_CONSOLE_TERMINAL_HPP
#define C2NG_SERVER_CONSOLE_TERMINAL_HPP

#include "afl/base/deletable.hpp"
#include "server/console/context.hpp"
#include "util/application.hpp"

namespace server { namespace console {

    /** Terminal output for a console.
        Provides functions to write (partly hardcoded) strings to the terminal the console is running on.
        This is used to distinguish between different modes the console runs in. */
    class Terminal : public afl::base::Deletable {
     public:
        /** Print welcome banner. */
        virtual void printBanner() = 0;

        /** Print primary prompt (ask user to provide a new command).
            \param st Current context stack */
        virtual void printPrimaryPrompt(const ContextStack_t& st) = 0;

        /** Print secondary prompt (ask user to continue a partial command). */
        virtual void printSecondaryPrompt() = 0;

        /** Print error message.
            \param msg Message */
        virtual void printError(String_t msg) = 0;

        /** Print result prefix.
            This call is followed by the output of a result, followed by printResultSuffix(). */
        virtual void printResultPrefix() = 0;

        /** Print result suffix. */
        virtual void printResultSuffix() = 0;

        /** Print a normal progress message.
            \param s message */
        virtual void printMessage(String_t s) = 0;

        /** Convert a ContextStack_t into a string to use as a prompt.
            \param st Current context stack
            \return Prompt */
        static String_t packContextStack(const ContextStack_t& st);
    };

} }

#endif
