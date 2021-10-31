/**
  *  \file util/application.hpp
  *  \brief Class util::Application
  */
#ifndef C2NG_UTIL_APPLICATION_HPP
#define C2NG_UTIL_APPLICATION_HPP

#include "afl/base/deletable.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/environment.hpp"
#include "consolelogger.hpp"

namespace util {

    /** Base class for console application.
        This aggregates a few common objects:
        - afl::sys::Environment instance
        - afl::io::FileSystem instance
        - afl::string::Translator instance
        - ConsoleLogger
        - standard output/error streams

        You derive from Application and implement appMain().
        Your main() function looks like
        <code>
          return MyApplication(env, fs).run();
        </code>

        This will also catch and log all exceptions your appMain() throws. */
    class Application : public afl::base::Deletable {
     public:
        /** Constructor.
            \param env Environment instance
            \param fs File system instance */
        Application(afl::sys::Environment& env, afl::io::FileSystem& fs);

        /** Destructor. */
        ~Application();

        /** Run the application.
            Invokes appMain() with exception protection.
            \return return code (exit code) */
        int run();

        /** Exit the application.
            \param n return code (exit code).

            Note that this function is implemented by throwing an exception.
            It will only work from the thread that called run().
            It will not work if called inside a block that catches all exceptions (catch(...)). */
        void exit(int n);

        /** Exit the application, producing an error message.
            \param str Message */
        void errorExit(String_t str);

        /** Access environment instance.
            \return instance */
        afl::sys::Environment& environment();

        /** Access file system instance.
            \return instance */
        afl::io::FileSystem& fileSystem();

        /** Access translator instance.
            \return instance */
        afl::string::Translator& translator();

        /** Access general logger instance.
            Use for logging/pass to components that need a loggeer.
            \return instance */
        afl::sys::LogListener& log();

        /** Access ConsoleLogger instance.
            Returns the same object as log(), but as type ConsoleLogger so it can be configured.
            \return instance */
        ConsoleLogger& consoleLogger();

        /** Return standard error stream.
            \return instance */
        afl::io::TextWriter& errorOutput();

        /** Return standard output stream.
            \return instance */
        afl::io::TextWriter& standardOutput();

     protected:
        /** Application.
            This contains what normally would be your main() function.
            It can exit
            - normally, producing return code 0.
            - by calling exit(n), producing return code n.
            - by calling errorExit(s), producing a log message and return code 1.
            - by throwing an exception, producing a log message and return code 1. */
        virtual void appMain() = 0;

     private:
        afl::sys::Environment& m_environment;
        afl::io::FileSystem& m_fileSystem;
        afl::string::NullTranslator m_translator;
        ConsoleLogger m_logger;
        afl::base::Ref<afl::io::TextWriter> m_errorOutput;
        afl::base::Ref<afl::io::TextWriter> m_standardOutput;

        void reportError(String_t str);
    };

}

#endif
