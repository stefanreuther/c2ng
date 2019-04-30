/**
  *  \file game/maint/sweepapplication.hpp
  */
#ifndef C2NG_GAME_MAINT_SWEEPAPPLICATION_HPP
#define C2NG_GAME_MAINT_SWEEPAPPLICATION_HPP

#include "util/application.hpp"
#include "afl/sys/environment.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/textwriter.hpp"

namespace game { namespace maint {

    class SweepApplication : public util::Application {
     public:
        SweepApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : Application(env, fs)
            { }

        void appMain();

     private:
        void help(afl::io::TextWriter& out);
    };

} }

#endif
