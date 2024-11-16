/**
  *  \file game/nu/turnloader.cpp
  *  \brief Class game::nu::TurnLoader
  */

#include "game/nu/turnloader.hpp"

#include "afl/string/string.hpp"
#include "game/game.hpp"
#include "game/nu/loader.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"

using afl::data::Access;
using afl::string::ConstStringMemory_t;
using afl::string::Translator;
using afl::sys::LogListener;

namespace {
    const char LOG_NAME[] = "game.nu";
}

game::nu::TurnLoader::TurnLoader(afl::base::Ref<GameState> gameState,
                                 util::ProfileDirectory& profile,
                                 afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory)
    : m_gameState(gameState),
      m_profile(profile),
      m_defaultSpecificationDirectory(defaultSpecificationDirectory)
{ }

game::nu::TurnLoader::~TurnLoader()
{ }

game::TurnLoader::PlayerStatusSet_t
game::nu::TurnLoader::getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const
{
    PlayerStatusSet_t result;
    Access entry = m_gameState->loadGameListEntryPreAuthenticated();
    if (player == entry("player")("id").toInteger()) {
        result += Available;
        result += Playable;
        result += Primary;
        switch (entry("player")("turnstatus").toInteger()) {
         case 1:
            extra = tx("Turn viewed");
            break;
         case 2:
            extra = tx("Turn submitted");
            break;
         default:
            extra = tx("Result file available");
            break;
        }
    } else {
        extra.clear();
    }
    return result;
}

std::auto_ptr<game::Task_t>
game::nu::TurnLoader::loadCurrentTurn(Turn& turn, Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    // FIXME: validate player
    class Task : public Task_t {
     public:
        Task(TurnLoader& parent, Turn& turn, Game& game, int player, Root& root, afl::sys::LogListener& log, Translator& tx, std::auto_ptr<StatusTask_t>& then)
            : m_parent(parent), m_turn(turn), m_game(game), m_player(player), m_root(root), m_log(log), m_translator(tx), m_then(then)
            { }

        virtual void call()
            {
                m_log.write(LogListener::Trace, LOG_NAME, "Task: loadCurrentTurn");
                try {
                    m_parent.doLoadCurrentTurn(m_turn, m_game, m_player, m_root, m_log, m_translator);
                    m_then->call(true);
                }
                catch (std::exception& e) {
                    m_log.write(LogListener::Error, LOG_NAME, String_t(), e);
                    m_then->call(false);
                }
            }
     private:
        TurnLoader& m_parent;
        Turn& m_turn;
        Game& m_game;
        int m_player;
        Root& m_root;
        afl::sys::LogListener& m_log;
        Translator& m_translator;
        std::auto_ptr<StatusTask_t> m_then;
    };
    return m_gameState->login(std::auto_ptr<Task_t>(new Task(*this, turn, game, player, root, session.log(), session.translator(), then)));
}

std::auto_ptr<game::Task_t>
game::nu::TurnLoader::saveCurrentTurn(const Turn& turn, const Game& game, PlayerSet_t players, SaveOptions_t opts, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    // FIXME
    (void) turn;
    (void) players;
    (void) opts;
    (void) root;

    game.expressionLists().saveRecentFiles(m_profile, session.log(), session.translator());

    return makeConfirmationTask(true, then);
}

void
game::nu::TurnLoader::getHistoryStatus(int /*player*/, int turn, afl::base::Memory<HistoryStatus> status, const Root& /*root*/)
{
    /*
     *  Basic idea: be optimistic (WeaklyPositive) that we have a history result for each turn before the current one.
     *  FIXME: when we download a history result, cache it locally (using the regular Backup mechanism we use for v3)
     *  so we can give StronglyPositive answers and avoid network traffic later on.
     */

    // Fetch the result. This should not produce a network access, we already have it.
    Access rst(m_gameState->loadResultPreAuthenticated());
    if (rst.isNull() || rst("success").toInteger() == 0) {
        // Bad result
        status.fill(Negative);
    } else {
        // OK, fill it
        int currentTurn = rst("rst")("game")("turn").toInteger();
        while (HistoryStatus* p = status.eat()) {
            if (turn >= 0 && turn < currentTurn) {
                *p = WeaklyPositive;
            } else {
                *p = Negative;
            }
            ++turn;
        }
    }
}

std::auto_ptr<game::Task_t>
game::nu::TurnLoader::loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    (void) turn;
    (void) game;
    (void) player;
    (void) turnNumber;
    (void) root;

    session.log().write(LogListener::Error, LOG_NAME, "!FIXME: loadHistoryTurn not implemented");
    return makeConfirmationTask(false, then);
}

std::auto_ptr<game::Task_t>
game::nu::TurnLoader::saveConfiguration(const Root& root, afl::sys::LogListener& log, afl::string::Translator& tx, std::auto_ptr<Task_t> then)
{
    return defaultSaveConfiguration(root, &m_profile, log, tx, then);
}

String_t
game::nu::TurnLoader::getProperty(Property p)
{
    switch (p) {
     case LocalFileFormatProperty:
     case RemoteFileFormatProperty:
        // igpFileFormatLocal:
        return "Nu";

     case RootDirectoryProperty:
        // igpRootDirectory:
        return m_defaultSpecificationDirectory->getDirectoryName();
    }
    return String_t();
}

void
game::nu::TurnLoader::doLoadCurrentTurn(Turn& turn, Game& game, int player, Root& /*root*/, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // Load result
    Access rst(m_gameState->loadResultPreAuthenticated());
    if (rst.isNull() || rst("success").toInteger() == 0) {
        throw std::runtime_error(tx("Unable to download result file"));
    }

    // FIXME: loadCurrentDatabases()
    // must create all planets/ships before.

    // Expression lists
    game.expressionLists().loadRecentFiles(m_profile, log, tx);
    game.expressionLists().loadPredefinedFiles(m_profile, *m_defaultSpecificationDirectory, log, tx);

    Loader(tx, log).loadTurn(turn, PlayerSet_t(player), rst);
}
