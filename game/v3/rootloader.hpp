/**
  *  \file game/v3/rootloader.hpp
  */
#ifndef C2NG_GAME_V3_ROOTLOADER_HPP
#define C2NG_GAME_V3_ROOTLOADER_HPP

#include <memory>
#include "afl/base/ptr.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/browser/usercallback.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/root.hpp"
#include "game/v3/directoryscanner.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace v3 {

    // Basic idea: a RootLoader is
    // - stateful (=can cache the DirectoryScanner, not re-entrant)
    // - multi-use (=can be used multiple times to re-scan)
    class RootLoader {
     public:
        RootLoader(afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                   util::ProfileDirectory* pProfile,
                   game::browser::UserCallback* pCallback,
                   afl::string::Translator& tx,
                   afl::sys::LogListener& log,
                   afl::io::FileSystem& fs);

        afl::base::Ptr<Root> load(afl::base::Ref<afl::io::Directory> gameDirectory,
                                  afl::charset::Charset& charset,
                                  const game::config::UserConfiguration& config,
                                  bool forceEmpty);

     private:
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;
        util::ProfileDirectory* m_pProfile;
        game::browser::UserCallback* m_pCallback;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
        afl::io::FileSystem& m_fileSystem;

        DirectoryScanner m_scanner;

        void loadConfiguration(Root& root, afl::io::Directory& dir, afl::charset::Charset& charset);
    };

} }

#endif
