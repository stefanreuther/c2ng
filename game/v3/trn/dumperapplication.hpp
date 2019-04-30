/**
  *  \file game/v3/trn/dumperapplication.hpp
  */
#ifndef C2NG_GAME_V3_TRN_DUMPERAPPLICATION_HPP
#define C2NG_GAME_V3_TRN_DUMPERAPPLICATION_HPP

#include "afl/io/filesystem.hpp"
#include "afl/string/string.hpp"
#include "afl/sys/environment.hpp"
#include "game/v3/turnfile.hpp"
#include "util/application.hpp"

namespace game { namespace v3 { namespace trn {

    class DumperApplication : public util::Application {
     public:
        DumperApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : Application(env, fs)
            { }

        virtual void appMain();

     private:
        void showHelp();
        void showVersion();

        void processEdit(TurnFile& trn, String_t edit);
        void saveTurn(const TurnFile& trn, String_t fileName);
    };

} } }

#endif
