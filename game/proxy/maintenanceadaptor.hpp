/**
  *  \file game/proxy/maintenanceadaptor.hpp
  *  \brief Interface game::proxy::MaintenanceAdaptor
  */
#ifndef C2NG_GAME_PROXY_MAINTENANCEADAPTOR_HPP
#define C2NG_GAME_PROXY_MAINTENANCEADAPTOR_HPP

#include "afl/base/deletable.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/translator.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/playerlist.hpp"

namespace game { namespace proxy {

    /** Adaptor for MaintenanceProxy.
        Provides the objects required to perform directory maintenance operations. */
    class MaintenanceAdaptor : public afl::base::Deletable {
     public:
        /** Access target directory.
            This is the game directory to receive the unpack result,
            provide the maketurn source, or the files to be swept.
            @return directory (must internally be managed using Ref) */
        virtual afl::io::Directory& targetDirectory() = 0;

        /** Access translator.
            @return translator */
        virtual afl::string::Translator& translator() = 0;

        /** Access game character set.
            This character set is used for encoding/decoding game files.
            @return character set */
        virtual afl::charset::Charset& charset() = 0;

        /** Access player list.
            Provides the set of all players, and their names.
            @return player list */
        virtual const PlayerList& playerList() = 0;

        /** Access file system.
            The file system is used for resolving backup file names.
            @return file system */
        virtual afl::io::FileSystem& fileSystem() = 0;

        /** Access user configuration.
            @return user configuration */
        virtual game::config::UserConfiguration& userConfiguration() = 0;
    };

} }

#endif
