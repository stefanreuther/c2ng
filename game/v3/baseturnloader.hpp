/**
  *  \file game/v3/baseturnloader.hpp
  *  \brief Class game::v3::BaseTurnLoader
  */
#ifndef C2NG_GAME_V3_BASETURNLOADER_HPP
#define C2NG_GAME_V3_BASETURNLOADER_HPP

#include "afl/charset/charset.hpp"
#include "game/turnloader.hpp"

namespace game { namespace v3 {

    /** Common base for v3 turn loaders.
        This implements:
        - history turn loading (getHistoryStatus(), loadHistoryTurn())
        - config access (saveConfiguration())
        - utilities for implementing loadCurrentTurn(): loadExpressionLists(), loadExpressionLists() */
    class BaseTurnLoader : public TurnLoader {
     public:
        /** Constructor.
            @param specDir    Specification directory (game + default)
            @param charset    Game character set
            @param fs         File system (for backups)
            @param pProfile   Profile directory (optional, for config files) */
        BaseTurnLoader(const afl::base::Ref<afl::io::Directory>& specDir, std::auto_ptr<afl::charset::Charset> charset, afl::io::FileSystem& fs, util::ProfileDirectory* pProfile);

        /** Implementation of TurnLoader::getHistoryStatus.
            Checks the backups for saved result files.
            @param player Player
            @param turn   Turn number
            @param status Room for status. Return as many turns as will fit.
            @param root   Root */
        void getHistoryStatus(int player, int turn, afl::base::Memory<HistoryStatus> status, const Root& root);

        /** Implementation of TurnLoader::loadHistoryTurn.
            Returns a task to load the history turn from a backed-up result.
            @param turn       Turn
            @param game       Game
            @param player     Player number
            @param turnNumber Turn number
            @param root       Root
            @param then       Task to execute after loading the turn
            @return newly-allocated task */
        std::auto_ptr<Task_t> loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, Session& session, std::auto_ptr<StatusTask_t> then);

        /** Implementation of TurnLoader::saveConfiguration.
            Returns a task that saves configuration.
            @param root       Root
            @param log        Logger
            @param tx         Translator
            @param then       Task to execute after saving configuration */
        std::auto_ptr<Task_t> saveConfiguration(const Root& root, afl::sys::LogListener& log, afl::string::Translator& tx, std::auto_ptr<Task_t> then);

        /** Load expression lists (lru.ini, expr.ini).
            @param game Game
            @param log  Logger
            @param tx   Translator
            @see game::config::ExpressionLists */
        void loadExpressionLists(Game& game, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Save expression lists (lru.ini).
            @param game Game
            @param log  Logger
            @param tx   Translator
            @see game::config::ExpressionLists */
        void saveExpressionLists(const Game& game, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Load extra files.
            Call after loading turn data.
            @param game    Game
            @param turn    Turn
            @param root    Root
            @param session Session */
        void loadExtraFiles(Game& game, Turn& turn, int player, Root& root, Session& session);

        /** Access file system.
            @return file system */
        afl::io::FileSystem& fileSystem() const
            { return m_fileSystem; }

        /** Access character set.
            @return character set */
        afl::charset::Charset& charset() const
            { return *m_charset; }

        /** Access specification directory.
            @return specification directory */
        afl::io::Directory& specificationDirectory() const
            { return *m_specificationDirectory; }

     private:
        afl::io::FileSystem& m_fileSystem;
        const std::auto_ptr<afl::charset::Charset> m_charset;
        const afl::base::Ref<afl::io::Directory> m_specificationDirectory;
        util::ProfileDirectory*const m_pProfile;

        void doLoadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, afl::sys::LogListener& log, afl::string::Translator& tx);
    };

} }

// Constructor.
inline
game::v3::BaseTurnLoader::BaseTurnLoader(const afl::base::Ref<afl::io::Directory>& specDir, std::auto_ptr<afl::charset::Charset> charset, afl::io::FileSystem& fs, util::ProfileDirectory* pProfile)
    : m_fileSystem(fs),
      m_charset(charset),
      m_specificationDirectory(specDir),
      m_pProfile(pProfile)
{ }

#endif
