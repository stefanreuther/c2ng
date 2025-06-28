/**
  *  \file game/v3/directoryloader.hpp
  *  \brief Class game::v3::DirectoryLoader
  */
#ifndef C2NG_GAME_V3_DIRECTORYLOADER_HPP
#define C2NG_GAME_V3_DIRECTORYLOADER_HPP

#include <memory>
#include "afl/base/ref.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/browser/usercallback.hpp"
#include "game/map/universe.hpp"
#include "game/playerarray.hpp"
#include "game/turnloader.hpp"
#include "game/v3/directoryscanner.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace v3 {

    class ControlFile;

    /** TurnLoader for unpacked game directory. */
    class DirectoryLoader : public TurnLoader {
     public:
        /** Constructor.
            \param specificationDirectory Specification directory (union of game directory, default specification directory)
            \param defaultSpecificationDirectory Default specification directory (share/specs)
            \param charset Game character set
            \param scanner Directory scanner (for initialisation)
            \param fs File System instance
            \param pProfile Profile directory (optional)
            \param pCallback User callback (optional; if not given, passwords are not checked) */
        DirectoryLoader(afl::base::Ref<afl::io::Directory> specificationDirectory,
                        afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                        std::auto_ptr<afl::charset::Charset> charset,
                        const DirectoryScanner& scanner,
                        afl::io::FileSystem& fs,
                        util::ProfileDirectory* pProfile,
                        game::browser::UserCallback* pCallback);

        virtual PlayerStatusSet_t getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const;
        virtual std::auto_ptr<Task_t> loadCurrentTurn(Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual std::auto_ptr<Task_t> saveCurrentTurn(const Game& game, PlayerSet_t player, SaveOptions_t opts, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual void getHistoryStatus(int player, int turn, afl::base::Memory<HistoryStatus> status, const Root& root);
        virtual std::auto_ptr<Task_t> loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual std::auto_ptr<Task_t> saveConfiguration(const Root& root, afl::sys::LogListener& log, afl::string::Translator& tx, std::auto_ptr<Task_t> then);
        virtual String_t getProperty(Property p);

     private:
        /*
         *  Integration (constructor parameters)
         */
        afl::base::Ref<afl::io::Directory> m_specificationDirectory;
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;
        std::auto_ptr<afl::charset::Charset> m_charset;
        afl::io::FileSystem& m_fileSystem;
        util::ProfileDirectory* m_pProfile;
        game::browser::UserCallback* m_pCallback;

        /*
         *  State
         */

        /** Player flags. */
        PlayerArray<DirectoryScanner::PlayerFlags_t> m_playerFlags;

        /** Outbox file format status.
            We track what file format we loaded a game from, so we can re-write it in the same format.
            The relevant format is just the outbox format where we distinguish between DOS (3.0) and Windows (3.5).
            Let Windows be the default, so this is the set of DOS format files.
            Another DOS/Windows switch is the control.dat file, which is handled internally by ControlFile. */
        PlayerSet_t m_playersWithDosOutbox;

        /** Implementation of loadCurrentTurn.
            Can throw on error.
            \param game Game
            \param player Player
            \param root Root
            \param session Session */
        void doLoadCurrentTurn(Game& game, int player, Root& root, Session& session);

        /** Implementation of loadHistoryTurn.
            Can throw on error.
            \param turn Turn
            \param game Game
            \param player Player
            \param turnNumber Turn number to load
            \param root Root
            \param log Logger
            \param tx Translator */
        void doLoadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Implementation of saveCurrentTurn.
            Can throw on error.
            \param game Game
            \param players Players
            \param root Root
            \param session Session */
        void doSaveCurrentTurn(const Game& game, PlayerSet_t players, const Root& root, Session& session);

        /** Load KORE file.
            \param file File
            \param turn Target turn
            \param player Player number
            \param log Logger
            \param tx Translator */
        void loadKore(afl::io::Stream& file, Turn& turn, int player, afl::sys::LogListener& log, afl::string::Translator& tx) const;

        /** Load SKORE file.
            \param file File
            \param turn Target turn
            \param log Logger
            \param tx Translator */
        void loadSkore(afl::io::Stream& file, Turn& turn, afl::sys::LogListener& log, afl::string::Translator& tx) const;

        /** Save ships.
            Writes the count and the ships, but not the signature.
            \param file File, freshly-created
            \param univ Universe to save.
            \param player Player number
            \param control ControlFile for this game directory
            \param remapExplore true to remap the Explore mission
            \return Checksum over data written */
        uint32_t saveShips(afl::io::Stream& file, const game::map::Universe& univ, int player, ControlFile& control, bool remapExplore) const;

        /** Save planets.
            Writes the count and the planets, but not the signature.
            \param file File, freshly-created
            \param univ Universe to save.
            \param player Player number
            \param control ControlFile for this game directory
            \return Checksum over data written */
        uint32_t savePlanets(afl::io::Stream& file, const game::map::Universe& univ, int player, ControlFile& control) const;

        /** Save starbasess.
            Writes the count and the starbasess, but not the signature.
            \param file File, freshly-created
            \param univ Universe to save.
            \param player Player number
            \param control ControlFile for this game directory
            \return Checksum over data written */
        uint32_t saveBases(afl::io::Stream& file, const game::map::Universe& univ, int player, ControlFile& control) const;
    };

} }

#endif
