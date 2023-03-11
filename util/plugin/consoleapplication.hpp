/**
  *  \file util/plugin/consoleapplication.hpp
  *  \brief Class util::plugin::ConsoleApplication
  */
#ifndef C2NG_UTIL_PLUGIN_CONSOLEAPPLICATION_HPP
#define C2NG_UTIL_PLUGIN_CONSOLEAPPLICATION_HPP

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "util/application.hpp"
#include "util/profiledirectory.hpp"

namespace util { namespace plugin {

    /** Main function of a console-based plugin manager application (c2plugin). */
    class ConsoleApplication : public util::Application {
     public:
        /** Constructor.
            @param env Environment
            @param fs  File system */
        ConsoleApplication(afl::sys::Environment& env, afl::io::FileSystem& fs);

        // Application:
        virtual void appMain();

     private:
        ProfileDirectory profile;

        void doList(afl::sys::Environment::CommandLine_t& cmdl);
        void doAdd(afl::sys::Environment::CommandLine_t& cmdl);
        void doRemove(afl::sys::Environment::CommandLine_t& cmdl);
        void doTest(afl::sys::Environment::CommandLine_t& cmdl);
        void doHelp(afl::sys::Environment::CommandLine_t& cmdl);

        struct Command;
        const Command* findCommand(const String_t& name);
    };

} }

#endif
