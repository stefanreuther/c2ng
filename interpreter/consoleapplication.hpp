/**
  *  \file interpreter/consoleapplication.hpp
  */
#ifndef C2NG_INTERPRETER_CONSOLEAPPLICATION_HPP
#define C2NG_INTERPRETER_CONSOLEAPPLICATION_HPP

#include "util/application.hpp"

namespace interpreter {

    class ConsoleApplication : public util::Application {
     public:
        ConsoleApplication(afl::sys::Environment& env, afl::io::FileSystem& fs);

        virtual void appMain();

     private:
        void help();
    };

}

#endif
