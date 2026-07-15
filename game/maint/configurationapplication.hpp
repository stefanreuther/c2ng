/**
  *  \file game/maint/configurationapplication.hpp
  *  \brief Class game::maint::ConfigurationApplication
  */
#ifndef C2NG_GAME_MAINT_CONFIGURATIONAPPLICATION_HPP
#define C2NG_GAME_MAINT_CONFIGURATIONAPPLICATION_HPP

#include "afl/io/stream.hpp"
#include "util/application.hpp"
#include "util/configurationfile.hpp"

namespace game { namespace maint {

    class ConfigurationApplication : public util::Application {
     public:
        ConfigurationApplication(afl::sys::Environment& env, afl::io::FileSystem& fs);

        virtual void appMain();

     private:
        void showHelp();
        void loadHConfig(util::ConfigurationFile& out, afl::io::Stream& in);
        void saveHConfig(const util::ConfigurationFile& in, afl::io::Stream& out);
        void loadTruehull(util::ConfigurationFile& out, afl::io::Stream& in);
        void saveTruehull(const util::ConfigurationFile& in, afl::io::Stream& out);
        void loadRaceNames(util::ConfigurationFile& out, afl::io::Stream& in);
        void saveRaceNames(const util::ConfigurationFile& in, afl::io::Stream& out);
        void shuffle(util::ConfigurationFile& config, const String_t& perm);
    };

} }

#endif
