/**
  *  \file game/interface/scriptapplication.hpp
  *  \brief Class game::interface::ScriptApplication
  */
#ifndef C2NG_GAME_INTERFACE_SCRIPTAPPLICATION_HPP
#define C2NG_GAME_INTERFACE_SCRIPTAPPLICATION_HPP

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "util/application.hpp"

namespace game { namespace interface {

    /** Interpreter console application.
        Implements the main application of the c2script application
        that can execute script code in a game context. */
    class ScriptApplication : public util::Application {
     public:
        struct Parameters;

        /** Constructor.
            @param env Environment
            @param fs  File System */
        ScriptApplication(afl::sys::Environment& env, afl::io::FileSystem& fs);

        // Application:
        virtual void appMain();

     private:
        void parseParameters(Parameters& params);
        void help();
    };

} }

#endif
