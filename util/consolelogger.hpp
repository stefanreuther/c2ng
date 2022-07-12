/**
  *  \file util/consolelogger.hpp
  *  \brief Class util::ConsoleLogger
  */
#ifndef C2NG_UTIL_CONSOLELOGGER_HPP
#define C2NG_UTIL_CONSOLELOGGER_HPP

#include "afl/base/ptr.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/sys/mutex.hpp"
#include "util/messagematcher.hpp"

namespace util {

    /** Console logger.
        This class provides a logger that writes to the console.
        This logger is used by default in all programs.

        By default, this class expects to be connected with the afl::io::TextWriter's provided by the afl::sys::Environment.

        To connect the logger, call attachWriter() after constructing it.
        - call log.attachWriter(_, env.attachTextWriter(_)) if you are a console program and it is an error if the given channel does not exist.
        - call log.attachWriter(_, env.attachTextWriterNT(_)) if you are a GUI where it is not an error if the given channel does not exist;
          GUI programs must internally provide a console view. */
    class ConsoleLogger : public afl::sys::LogListener {
     public:
        /** Default constructor. */
        ConsoleLogger();

        virtual ~ConsoleLogger();
        virtual void handleMessage(const Message& msg);

        /** Attach a TextWriter.
            \param error true: this is the writer for Warn and Error messages (afl::sys::Environment::Error),
                         false: this is the writer for Trace, Debug, and Info messages (afl::sys::Environment::Output).
            \param w TextWriter to use.

            The TextWriter can be 0 to discard the respective message range.
            The TextWriter must be able to be called from any thread, because the ConsoleLogger is called from any thread.
            The ConsoleLogger will however serialize all calls.
            The TextWriter must therefore be "fast enough" because it is called with a mutex held.
            (This is an exception to the "no I/O while holding a critical resource" rule;
            we expect console output to be reasonably fast.) */
        void attachWriter(bool error, afl::base::Ptr<afl::io::TextWriter> w);

        /** Set configuration.
            \param config MessageMatcher configuration. Produces one of
            - "hide", "drop" (=hide)
            - "keep", "show" (=show normally, default)
            - "raw" (=show just text)
            \param tx Translator (for error messages) */
        void setConfiguration(String_t config, afl::string::Translator& tx);

     private:
        afl::sys::Mutex m_mutex;
        afl::base::Ptr<afl::io::TextWriter> m_writers[2];
        MessageMatcher m_matcher;
    };

}

#endif
