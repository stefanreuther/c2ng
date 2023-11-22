/**
  *  \file util/resourcefileapplication.hpp
  *  \brief Class util::ResourceFileApplication
  */
#ifndef C2NG_UTIL_RESOURCEFILEAPPLICATION_HPP
#define C2NG_UTIL_RESOURCEFILEAPPLICATION_HPP

#include "util/application.hpp"
#include "afl/sys/commandlineparser.hpp"

namespace util {

    /** Console application for manipulating resource files (c2restool).
        */
    class ResourceFileApplication : public Application {
     public:
        /** Constructor.
            @param env Environment instance
            @param fs File system instance */
        ResourceFileApplication(afl::sys::Environment& env, afl::io::FileSystem& fs);

        // Main entry point
        virtual void appMain();

        void help();

     private:

        void doCreate(afl::sys::CommandLineParser& cmdl);
        void doList(afl::sys::CommandLineParser& cmdl);
        void doExtract(afl::sys::CommandLineParser& cmdl);
        void doExtractAll(afl::sys::CommandLineParser& cmdl);
    };

}

#endif
