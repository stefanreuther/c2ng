/**
  *  \file game/maint/dump/application.hpp
  *  \brief Class game::maint::dump::Application
  */
#ifndef C2NG_GAME_MAINT_DUMP_APPLICATION_HPP
#define C2NG_GAME_MAINT_DUMP_APPLICATION_HPP

#include "util/application.hpp"

namespace game { namespace maint { namespace dump {

    /** Dumper console application.
        Main entry point to the c2dump utility. */
    class Application : public util::Application {
     public:
        /** Constructor.
            @param env Environment
            @param fs  File System */
        Application(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : util::Application(env, fs)
            { }

        void appMain();

     private:
        void help(afl::io::TextWriter& out);
        void reportError(String_t msg);
    };

} } }

#endif
