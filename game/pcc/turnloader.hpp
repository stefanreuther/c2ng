/**
  *  \file game/pcc/turnloader.hpp
  *  \brief Class game::pcc::TurnLoader
  */
#ifndef C2NG_GAME_PCC_TURNLOADER_HPP
#define C2NG_GAME_PCC_TURNLOADER_HPP

#include <memory>
#include "afl/charset/charset.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/map/universe.hpp"
#include "game/pcc/serverdirectory.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "game/turnloader.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace pcc {

    /** TurnLoader for uploaded game directories.

        As of 20220406:
        - load RST, TRN and specs from server directory
        - upload TRN to filer or host
        - history, fleets etc. stored locally if an appropriate directory is provided
        - no backups and history turns */
    class TurnLoader : public game::TurnLoader {
     public:
        /** Constructor.
            @param localDirectory                  Local directory (chart, fleet, etc.; used as Root::gameDirectory())
            @param defaultSpecificationDirectory   Default specification directory (share/specs)
            @param serverDirectory                 Server directory (RST, TRN, specs). Also provides access to BrowserHandler/Account.
            @param hostGameNumber                  Game number for hosted game (0 if not a hosted game)
            @param charset                         Game character set
            @param log                             Logger
            @param availablePlayers                Available players
            @param profile                         Profile directory (default configs) */
        TurnLoader(afl::base::Ref<afl::io::Directory> localDirectory,
                   afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                   afl::base::Ref<ServerDirectory> serverDirectory,
                   int32_t hostGameNumber,
                   std::auto_ptr<afl::charset::Charset> charset,
                   afl::sys::LogListener& log,
                   PlayerSet_t availablePlayers,
                   util::ProfileDirectory& profile);

        // TurnLoader:
        virtual PlayerStatusSet_t getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const;
        virtual std::auto_ptr<Task_t> loadCurrentTurn(Turn& turn, Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual std::auto_ptr<Task_t> saveCurrentTurn(const Turn& turn, const Game& game, PlayerSet_t player, SaveOptions_t opts, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual void getHistoryStatus(int player, int turn, afl::base::Memory<HistoryStatus> status, const Root& root);
        virtual std::auto_ptr<Task_t> loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual std::auto_ptr<Task_t> saveConfiguration(const Root& root, afl::sys::LogListener& log, afl::string::Translator& tx, std::auto_ptr<Task_t> then);
        virtual String_t getProperty(Property p);

     private:
        afl::base::Ref<afl::io::Directory> m_localDirectory;
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;
        afl::base::Ref<ServerDirectory> m_serverDirectory;
        int32_t m_hostGameNumber;
        std::auto_ptr<afl::charset::Charset> m_charset;
        afl::sys::LogListener& m_log;
        util::ProfileDirectory& m_profile;

        PlayerSet_t m_availablePlayers;

        void doLoadCurrentTurn(Turn& turn, Game& game, int player, game::Root& root, Session& session);
        void doSaveCurrentTurn(const Turn& turn, const Game& game, PlayerSet_t players, SaveOptions_t opts, const Root& root, Session& session);
    };

} }

#endif
