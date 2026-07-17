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
#include "game/browser/usercallback.hpp"
#include "game/map/universe.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "game/v3/baseturnloader.hpp"
#include "game/v3/directoryscanner.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace v3 {

    /** TurnLoader for result/turn files. */
    class ResultLoader : public BaseTurnLoader {
     public:
        /** Constructor.
            \param specificationDirectory Specification directory (union of game directory, default specification directory)
            \param defaultSpecificationDirectory Default specification directory (share/specs)
            \param charset Game character set
            \param scanner Directory scanner (for initialisation)
            \param fs File System instance
            \param pProfile Profile directory (optional)
            \param pCallback User callback (optional; if not given, passwords are not checked) */
        ResultLoader(afl::base::Ref<afl::io::Directory> specificationDirectory,
                     afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                     std::auto_ptr<afl::charset::Charset> charset,
                     const DirectoryScanner& scanner,
                     afl::io::FileSystem& fs,
                     util::ProfileDirectory* pProfile,
                     game::browser::UserCallback* pCallback);

        virtual PlayerStatusSet_t getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const;
        virtual std::auto_ptr<Task_t> loadCurrentTurn(Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual std::auto_ptr<Task_t> saveCurrentTurn(const Game& game, PlayerSet_t players, SaveOptions_t opts, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual String_t getProperty(Property p);

        void loadTurnfile(Turn& trn, const Root& root, afl::io::Stream& file, int player, afl::sys::LogListener& log, afl::string::Translator& tx) const;

     private:
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;
        game::browser::UserCallback* m_pCallback;

        PlayerArray<DirectoryScanner::PlayerFlags_t> m_playerFlags;

        void doLoadCurrentTurn(Game& game, int player, game::Root& root, Session& session);
        void doSaveCurrentTurn(const Game& game, PlayerSet_t players, const Root& root, Session& session);
    };

} }

#endif
