/**
  *  \file game/v3/trn/fileset.hpp
  *  \brief Class game::v3::trn::FileSet
  */
#ifndef C2NG_GAME_V3_TRN_FILESET_HPP
#define C2NG_GAME_V3_TRN_FILESET_HPP

#include "afl/base/ptr.hpp"
#include "afl/charset/charset.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/playerlist.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"

namespace game { namespace v3 { namespace trn {

    /** Set of turn files.
        Turn files always have to be written as a group, so they "know each other" via their trailers.
        This implements the basic functions for that.

        The actual turn file creation logic is in classes Maketurn or ResultLoader.
        - call create() and populate the TurnFile instances
        - call updateTrailers()
        - call saveAll()

        FileSet does not further validate the turn files.
        Normally, you should not mix files from different turns.
        When given a set of files from different turns, FileSet will still produce files that match among others,
        but if the directory contains further mixed files, may not match them all even if theoretically possible. */
    class FileSet {
     public:
        static const int NUM_PLAYERS = 11;

        /** Constructor.
            \param dir Target directory
            \param charset Character set */
        FileSet(afl::io::Directory& dir, afl::charset::Charset& charset);

        /** Destructor. */
        ~FileSet();

        /** Create a turn file in memory.
            \param playerNr Player number. Each number should be passed only once here.
            \param timestamp Timestamp.
            \param turnNumber Turn number.
            \return newly-created TurnFile instance */
        TurnFile& create(int playerNr, const Timestamp& timestamp, int turnNumber);

        /** Update turn file trailers.
            This will check the target directory whether a trailer can be re-used,
            or build a new trailer, and apply that to all files. */
        void updateTrailers();

        /** Save all turn files.
            This will save the files and possible backup copies.
            \param log Logger
            \param players Player list (for logging)
            \param fs File system (for backups)
            \param config User configuration file (for backups)
            \param tx Translator (for logging) */
        void saveAll(afl::sys::LogListener& log, const PlayerList& players, afl::io::FileSystem& fs, const game::config::UserConfiguration& config, afl::string::Translator& tx);

        /** Save all turn files, no backup.
            This will save the files but not create backup copies.
            \param log Logger
            \param players Player list (for logging)
            \param tx Translator (for logging) */
        void saveAll(afl::sys::LogListener& log, const PlayerList& players, afl::string::Translator& tx);

        /** Get number of turn files.
            \return number of turn files */
        size_t getNumFiles() const;

     private:
        afl::io::Directory& m_directory;
        afl::charset::Charset& m_charset;

        afl::container::PtrVector<TurnFile> m_turnFiles;
        std::vector<int> m_turnNumbers;
    };

} } }

#endif
