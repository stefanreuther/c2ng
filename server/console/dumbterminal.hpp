/**
  *  \file server/console/dumbterminal.hpp
  *  \brief Class server::console::DumbTerminal
  */
#ifndef C2NG_SERVER_CONSOLE_DUMBTERMINAL_HPP
#define C2NG_SERVER_CONSOLE_DUMBTERMINAL_HPP

#include "afl/io/textwriter.hpp"
#include "server/console/terminal.hpp"

namespace server { namespace console {

    /** Dumb terminal.
        Produces plain text output for an interactive application. */
    class DumbTerminal : public Terminal {
     public:
        /** Constructor.
            \param out Standard Output
            \param err Error Output */
        explicit DumbTerminal(afl::io::TextWriter& out, afl::io::TextWriter& err);

        virtual void printBanner();
        virtual void printPrimaryPrompt(const ContextStack_t& st);
        virtual void printSecondaryPrompt();
        virtual void printError(String_t msg);
        virtual void printResultPrefix();
        virtual void printResultSuffix();
        virtual void printMessage(String_t s);

     private:
        afl::io::TextWriter& m_outputStream;
        afl::io::TextWriter& m_errorStream;
    };

} }

inline
server::console::DumbTerminal::DumbTerminal(afl::io::TextWriter& out, afl::io::TextWriter& err)
    : m_outputStream(out), m_errorStream(err)
{ }

#endif
