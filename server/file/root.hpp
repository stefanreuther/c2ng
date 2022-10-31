/**
  *  \file server/file/root.hpp
  */
#ifndef C2NG_SERVER_FILE_ROOT_HPP
#define C2NG_SERVER_FILE_ROOT_HPP

#include "afl/sys/log.hpp"
#include "afl/io/stream.hpp"
#include "afl/charset/charset.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "game/v3/directoryscanner.hpp"
#include "afl/io/directory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/playerarray.hpp"
#include "server/common/racenames.hpp"

namespace server { namespace file {

    class DirectoryItem;

    class Root {
     public:
        Root(DirectoryItem& rootDirectory, afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory);

        ~Root();

        /** Access root directory.
            \return root directory */
        DirectoryItem& rootDirectory();

        /** Access logger.
            Attach a listener to receive log messages.
            \return logger */
        afl::sys::Log& log();

        afl::charset::Charset& defaultCharacterSet();

        const server::common::RaceNames& defaultRaceNames() const;

        game::v3::DirectoryScanner& directoryScanner();

        afl::io::Stream::FileSize_t getMaxFileSize() const;
        void setMaxFileSize(afl::io::Stream::FileSize_t limit);

     private:
        afl::sys::Log m_log;

        DirectoryItem& m_rootDirectory;

        afl::io::Stream::FileSize_t m_maxFileSize;

        afl::charset::CodepageCharset m_defaultCharset;

        server::common::RaceNames m_defaultRaceNames;

        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;

        afl::string::NullTranslator m_translator;

        game::v3::DirectoryScanner m_scanner;

        void loadRaceNames();
    };

} }

#endif
