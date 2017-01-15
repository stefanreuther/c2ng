/**
  *  \file game/v3/rootloader.hpp
  */
#ifndef C2NG_GAME_V3_ROOTLOADER_HPP
#define C2NG_GAME_V3_ROOTLOADER_HPP

#include <memory>
#include "afl/base/ptr.hpp"
#include "afl/io/directory.hpp"
#include "game/root.hpp"
#include "game/v3/directoryscanner.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "game/playerlist.hpp"
#include "util/profiledirectory.hpp"
#include "afl/io/filesystem.hpp"

namespace game { namespace v3 {

    // Basic idea: a RootLoader is
    // - stateful (=can cache the DirectoryScanner, not re-entrant)
    // - multi-use (=can be used multiple times to re-scan)
    class RootLoader {
     public:
        RootLoader(afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                   util::ProfileDirectory& profile,
                   afl::string::Translator& tx,
                   afl::sys::LogListener& log,
                   afl::io::FileSystem& fs);

        void setCharsetNew(afl::charset::Charset* p);

        afl::base::Ptr<Root> load(afl::base::Ref<afl::io::Directory> gameDirectory, bool forceEmpty);

     private:
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;
        util::ProfileDirectory& m_profile;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
        afl::io::FileSystem& m_fileSystem;

        DirectoryScanner m_scanner;

        std::auto_ptr<afl::charset::Charset> m_charset;

        void loadConfiguration(Root& root);
        void loadPConfig(Root& root, afl::base::Ptr<afl::io::Stream> pconfig, afl::base::Ptr<afl::io::Stream> shiplist, game::config::ConfigurationOption::Source source);
        void loadHConfig(Root& root, afl::io::Stream& hconfig, game::config::ConfigurationOption::Source source);
        void loadRaceMapping(Root& root, afl::io::Stream& file, game::config::ConfigurationOption::Source source);

        void loadRaceNames(PlayerList& list, afl::io::Directory& dir);
    };

} }

#endif
