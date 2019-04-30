/**
  *  \file game/maint/raterapplication.hpp
  */
#ifndef C2NG_GAME_MAINT_RATERAPPLICATION_HPP
#define C2NG_GAME_MAINT_RATERAPPLICATION_HPP

#include "util/application.hpp"
#include "afl/sys/environment.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/textwriter.hpp"

namespace game { namespace maint {

    class RaterApplication : public util::Application {
     public:
        RaterApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : Application(env, fs)
            { }

        void appMain();

     private:
        void help(afl::io::TextWriter& out);
    };

} }

#endif
