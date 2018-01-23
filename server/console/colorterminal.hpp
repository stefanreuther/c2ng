/**
  *  \file server/console/colorterminal.hpp
  *  \brief Class server::console::ColorTerminal
  */
#ifndef C2NG_SERVER_CONSOLE_COLORTERMINAL_HPP
#define C2NG_SERVER_CONSOLE_COLORTERMINAL_HPP

#include "afl/io/textwriter.hpp"
#include "server/console/terminal.hpp"

namespace server { namespace console {

    /** Color terminal.
        Produces colored text output for an interactive application, using ANSI color escapes. */
    class ColorTerminal : public Terminal {
     public:
        /** Constructor.
            \param out Standard Output
            \param err Error Output */
        explicit ColorTerminal(afl::io::TextWriter& out, afl::io::TextWriter& err);

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
server::console::ColorTerminal::ColorTerminal(afl::io::TextWriter& out, afl::io::TextWriter& err)
    : m_outputStream(out), m_errorStream(err)
{ }

#endif
