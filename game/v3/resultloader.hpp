/**
  *  \file game/v3/resultloader.hpp
  *  \brief Class game::v3::ResultLoader
  */
#ifndef C2NG_GAME_V3_RESULTLOADER_HPP
#define C2NG_GAME_V3_RESULTLOADER_HPP

#include <memory>
#include "afl/charset/charset.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/map/universe.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "game/turnloader.hpp"
#include "game/v3/directoryscanner.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace v3 {

    /** TurnLoader for result/turn files. */
    class ResultLoader : public TurnLoader {
     public:
        /** Constructor.
            \param specificationDirectory Specification directory (union of game directory, default specification directory)
            \param defaultSpecificationDirectory Default specification directory (share/specs)
            \param charset Game character set
            \param tx Translator
            \param log Logger
            \param scanner Directory scanner (for initialisation)
            \param fs File System instance
            \param pProfile Profile directory (optional) */
        ResultLoader(afl::base::Ref<afl::io::Directory> specificationDirectory,
                     afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                     std::auto_ptr<afl::charset::Charset> charset,
                     afl::string::Translator& tx,
                     afl::sys::LogListener& log,
                     const DirectoryScanner& scanner,
                     afl::io::FileSystem& fs,
                     util::ProfileDirectory* pProfile);

        virtual PlayerStatusSet_t getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const;
        virtual std::auto_ptr<Task_t> loadCurrentTurn(Turn& turn, Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual std::auto_ptr<Task_t> saveCurrentTurn(const Turn& turn, const Game& game, PlayerSet_t players, SaveOptions_t opts, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual void getHistoryStatus(int player, int turn, afl::base::Memory<HistoryStatus> status, const Root& root);
        virtual std::auto_ptr<Task_t> loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, std::auto_ptr<StatusTask_t> then);
        virtual String_t getProperty(Property p);

        void loadTurnfile(Turn& trn, const Root& root, afl::io::Stream& file, int player) const;

     private:
        afl::base::Ref<afl::io::Directory> m_specificationDirectory;
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;
        std::auto_ptr<afl::charset::Charset> m_charset;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
        afl::io::FileSystem& m_fileSystem;
        util::ProfileDirectory* m_pProfile;

        PlayerArray<DirectoryScanner::PlayerFlags_t> m_playerFlags;

        void doLoadCurrentTurn(Turn& turn, Game& game, int player, game::Root& root, Session& session);
        void doLoadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root);
        void doSaveCurrentTurn(const Turn& turn, const Game& game, PlayerSet_t players, const Root& root, Session& session);
    };

} }

#endif
