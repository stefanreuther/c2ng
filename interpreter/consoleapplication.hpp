/**
  *  \file interpreter/consoleapplication.hpp
  *  \brief Class interpreter::ConsoleApplication
  */
#ifndef C2NG_INTERPRETER_CONSOLEAPPLICATION_HPP
#define C2NG_INTERPRETER_CONSOLEAPPLICATION_HPP

#include "util/application.hpp"

namespace interpreter {

    /** Interpreter console application.
        Implements the main application of the c2script application
        that can compile, disassemble, and execute script code. */
    class ConsoleApplication : public util::Application {
     public:
        struct Parameters;

        /** Constructor.
            @param env Environment
            @param fs  File System */
        ConsoleApplication(afl::sys::Environment& env, afl::io::FileSystem& fs);

        // Application:
        virtual void appMain();

     private:
        void parseParameters(Parameters& params);
        void help();
    };

}

#endif
