/**
  *  \file interpreter/exporter/consoleapplication.hpp
  */
#ifndef C2NG_INTERPRETER_EXPORTER_CONSOLEAPPLICATION_HPP
#define C2NG_INTERPRETER_EXPORTER_CONSOLEAPPLICATION_HPP

#include "util/application.hpp"
#include "interpreter/world.hpp"
#include "interpreter/context.hpp"

namespace interpreter { namespace exporter {

    class ConsoleApplication : public util::Application {
     public:
        ConsoleApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : Application(env, fs)
            { }

        void appMain();

     private:
        void help();
        Context* findArray(const String_t& name, World& world);
    };

} }

#endif
