/**
  *  \file server/console/pipeterminal.hpp
  *  \brief Class server::console::PipeTerminal
  */
#ifndef C2NG_SERVER_CONSOLE_PIPETERMINAL_HPP
#define C2NG_SERVER_CONSOLE_PIPETERMINAL_HPP

#include "afl/io/textwriter.hpp"
#include "server/console/terminal.hpp"

namespace server { namespace console {

    /** Terminal for using console in a pipe.
        Produces minimal output (no banner/prompt, plain-text for everything else). */
    class PipeTerminal : public Terminal {
     public:
        /** Constructor.
            \param out Standard Output
            \param err Error Output */
        explicit PipeTerminal(afl::io::TextWriter& out, afl::io::TextWriter& err);

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
server::console::PipeTerminal::PipeTerminal(afl::io::TextWriter& out, afl::io::TextWriter& err)
    : m_outputStream(out), m_errorStream(err)
{ }

#endif
