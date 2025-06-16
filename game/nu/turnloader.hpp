/**
  *  \file game/nu/turnloader.hpp
  *  \brief Class game::nu::TurnLoader
  */
#ifndef C2NG_GAME_NU_TURNLOADER_HPP
#define C2NG_GAME_NU_TURNLOADER_HPP

#include "afl/base/ptr.hpp"
#include "afl/data/access.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/map/universe.hpp"
#include "game/nu/gamestate.hpp"
#include "game/turnloader.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace nu {

    /** TurnLoader implementation for planets.nu. */
    class TurnLoader : public game::TurnLoader {
     public:
        /** Constructor.
            @param gameState   Shared game state (for RST access)
            @param profile     Profile directory (for configuration)
            @param defaultSpecificationDirectory  Local specification directory (for local/default spec files) */
        TurnLoader(afl::base::Ref<GameState> gameState,
                   util::ProfileDirectory& profile,
                   afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory);

        /** Destructor. */
        ~TurnLoader();

        // TurnLoader:
        virtual PlayerStatusSet_t getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const;
        virtual std::auto_ptr<Task_t> loadCurrentTurn(Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual std::auto_ptr<Task_t> saveCurrentTurn(const Game& game, PlayerSet_t player, SaveOptions_t opts, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual void getHistoryStatus(int player, int turn, afl::base::Memory<HistoryStatus> status, const Root& root);
        virtual std::auto_ptr<Task_t> loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, Session& session, std::auto_ptr<StatusTask_t> then);
        virtual std::auto_ptr<Task_t> saveConfiguration(const Root& root, afl::sys::LogListener& log, afl::string::Translator& tx, std::auto_ptr<Task_t> then);
        virtual String_t getProperty(Property p);

     private:
        afl::base::Ref<GameState> m_gameState;
        util::ProfileDirectory& m_profile;
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;

        void doLoadCurrentTurn(Game& game, int player, Root& root, afl::sys::LogListener& log, afl::string::Translator& tx);
    };

} }

#endif
