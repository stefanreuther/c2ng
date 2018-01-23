/**
  *  \file server/console/nullterminal.hpp
  *  \brief Class server::console::NullTerminal
  */
#ifndef C2NG_SERVER_CONSOLE_NULLTERMINAL_HPP
#define C2NG_SERVER_CONSOLE_NULLTERMINAL_HPP

#include "server/console/terminal.hpp"

namespace server { namespace console {

    /** Null terminal.
        This implements Terminal as a null operation, mainly for testing. */
    class NullTerminal : public Terminal {
     public:
        virtual void printBanner();
        virtual void printPrimaryPrompt(const ContextStack_t& st);
        virtual void printSecondaryPrompt();
        virtual void printError(String_t msg);
        virtual void printResultPrefix();
        virtual void printResultSuffix();
        virtual void printMessage(String_t s);
    };

} }

#endif
