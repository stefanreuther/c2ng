/**
  *  \file game/pcc/turnloader.cpp
  *  \brief Class game::pcc::TurnLoader
  */

#include "game/pcc/turnloader.hpp"
#include "afl/base/inlinememory.hpp"
#include "afl/base/ptr.hpp"
#include "afl/data/access.hpp"
#include "afl/data/value.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/multidirectory.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/db/fleetloader.hpp"
#include "game/pcc/browserhandler.hpp"
#include "game/player.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "game/v3/loader.hpp"
#include "game/v3/parser.hpp"
#include "game/v3/passwordchecker.hpp"
#include "game/v3/trn/fileset.hpp"
#include "game/v3/turnfile.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::Stream;
using afl::string::Format;
using afl::sys::LogListener;
using game::v3::Loader;
using game::v3::Parser;
using game::v3::PasswordChecker;
using game::v3::TurnFile;

namespace {
    const char LOG_NAME[] = "game.pcc";
}

game::pcc::TurnLoader::TurnLoader(afl::base::Ref<ServerTransport> serverTransport,
                                  afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                  afl::base::Ref<util::ServerDirectory> serverDirectory,
                                  std::auto_ptr<afl::charset::Charset> charset,
                                  afl::sys::LogListener& log,
                                  PlayerSet_t availablePlayers,
                                  util::ProfileDirectory& profile)
    : m_serverTransport(serverTransport),
      m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_serverDirectory(serverDirectory),
      m_charset(charset),
      m_log(log),
      m_profile(profile),
      m_availablePlayers(availablePlayers)
{ }

game::pcc::TurnLoader::PlayerStatusSet_t
game::pcc::TurnLoader::getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const
{
    PlayerStatusSet_t result;
    if (m_availablePlayers.contains(player)) {
        // FIXME: show "+ TRN"? "temporary?"
        extra = tx("RST");
        result += Available;
        result += Playable;
        result += Primary;
    } else {
        extra.clear();
    }
    return result;
}

std::auto_ptr<game::Task_t>
game::pcc::TurnLoader::loadCurrentTurn(Turn& turn, Game& game, int player, game::Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    // ex game/load.h:loadCommon
    class Task : public Task_t {
     public:
        Task(TurnLoader& parent, Turn& turn, Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t>& then)
            : m_parent(parent), m_turn(turn), m_game(game), m_player(player), m_root(root), m_session(session), m_then(then),
              m_checker(turn, &parent.m_serverTransport->handler().callback(), parent.m_log, session.translator())
            { }

        virtual void call()
            {
                m_parent.m_log.write(LogListener::Trace, LOG_NAME, "Task: loadCurrentTurn");
                try {
                    m_parent.doLoadCurrentTurn(m_turn, m_game, m_player, m_root, m_session);
                    m_checker.checkPassword(m_player, m_session.authCache(), m_then);
                }
                catch (std::exception& e) {
                    m_session.log().write(LogListener::Error, LOG_NAME, String_t(), e);
                    m_then->call(false);
                }
            }
     private:
        TurnLoader& m_parent;
        Turn& m_turn;
        Game& m_game;
        int m_player;
        Root& m_root;
        Session& m_session;
        std::auto_ptr<StatusTask_t> m_then;
        PasswordChecker m_checker;
    };
    return m_serverTransport->handler().login(m_serverTransport->account(),
                                              std::auto_ptr<Task_t>(new Task(*this, turn, game, player, root, session, then)));
}

std::auto_ptr<game::Task_t>
game::pcc::TurnLoader::saveCurrentTurn(const Turn& turn, const Game& game, PlayerSet_t players, SaveOptions_t opts, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    // ex saveTurns
    class Task : public Task_t {
     public:
        Task(TurnLoader& parent, const Turn& turn, const Game& game, PlayerSet_t players, SaveOptions_t opts, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
            : m_parent(parent), m_turn(turn), m_game(game), m_players(players), m_options(opts), m_root(root), m_session(session), m_then(then)
            { }

        virtual void call()
            {
                m_parent.m_log.write(LogListener::Trace, LOG_NAME, "Task: saveCurrentTurn");
                try {
                    m_parent.doSaveCurrentTurn(m_turn, m_game, m_players, m_options, m_root, m_session);
                    m_then->call(true);
                }
                catch (std::exception& e) {
                    m_session.log().write(LogListener::Error, LOG_NAME, m_session.translator()("Unable to save game"), e);
                    m_then->call(false);
                }
            }
     private:
        TurnLoader& m_parent;
        const Turn& m_turn;
        const Game& m_game;
        const PlayerSet_t m_players;
        const SaveOptions_t m_options;
        const Root& m_root;
        Session& m_session;
        std::auto_ptr<StatusTask_t> m_then;
    };
    return m_serverTransport->handler().login(m_serverTransport->account(),
                                              std::auto_ptr<Task_t>(new Task(*this, turn, game, players, opts, root, session, then)));
}

void
game::pcc::TurnLoader::getHistoryStatus(int /*player*/, int /*turn*/, afl::base::Memory<HistoryStatus> status, const Root& /*root*/)
{
    // FIXME: load history from server for hosted games
    // FIXME: implement local history
    status.fill(Negative);
}

std::auto_ptr<game::Task_t>
game::pcc::TurnLoader::loadHistoryTurn(Turn& /*turn*/, Game& /*game*/, int /*player*/, int /*turnNumber*/, Root& /*root*/, Session& /*session*/, std::auto_ptr<StatusTask_t> then)
{
    // FIXME: implement
    return makeConfirmationTask(false, then);
}

std::auto_ptr<game::Task_t>
game::pcc::TurnLoader::saveConfiguration(const Root& root, afl::sys::LogListener& log, afl::string::Translator& tx, std::auto_ptr<Task_t> then)
{
    return defaultSaveConfiguration(root, &m_profile, log, tx, then);
}

String_t
game::pcc::TurnLoader::getProperty(Property p)
{
    switch (p) {
     case LocalFileFormatProperty:
        // igpFileFormatLocal: DOS, Windows
        return "RST";

     case RemoteFileFormatProperty:
        // igpFileFormatRemote: turn file format
        return "Windows";

     case RootDirectoryProperty:
        // igpRootDirectory:
        return m_defaultSpecificationDirectory->getDirectoryName();
    }
    return String_t();
}

void
game::pcc::TurnLoader::doLoadCurrentTurn(Turn& turn, Game& game, int player, game::Root& root, Session& session)
{
    // Initialize
    afl::string::Translator& tx = session.translator();
    Loader ldr(*m_charset, tx, m_log);
    ldr.prepareUniverse(turn.universe());
    ldr.prepareTurn(turn, root, session, player);

    // Merged spec directory
    Ref<afl::io::MultiDirectory> specDir = afl::io::MultiDirectory::create();
    specDir->addDirectory(m_serverDirectory);
    specDir->addDirectory(m_defaultSpecificationDirectory);

    // Load common files
    ldr.loadCommonFiles(*m_serverDirectory, *specDir, turn.universe(), player);

    // Load database
    loadCurrentDatabases(turn, game, player, root, session);

    // Load expression lists from profile
    game.expressionLists().loadRecentFiles(m_profile, m_log, tx);
    game.expressionLists().loadPredefinedFiles(m_profile, *specDir, m_log, tx);

    // Load result file from remote
    {
        Ref<Stream> file = m_serverDirectory->openFile(Format("player%d.rst", player), afl::io::FileSystem::OpenRead);
        m_log.write(LogListener::Info, LOG_NAME, Format(tx("Loading %s RST file..."), root.playerList().getPlayerName(player, Player::AdjectiveName, tx)));
        ldr.loadResult(turn, root, game, *file, player);
    }

    // Try to load turn from remote
    try {
        Ptr<Stream> file = m_serverDirectory->openFileNT(Format("player%d.trn", player), afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            m_log.write(LogListener::Info, LOG_NAME, Format(tx("Loading %s TRN file..."), root.playerList().getPlayerName(player, Player::AdjectiveName, tx)));
            Loader(*m_charset, tx, m_log).loadTurnfile(turn, root, *file, player);
        }
    }
    catch (afl::except::FileProblemException& e) {
        m_log.write(LogListener::Warn, LOG_NAME, tx("File has been ignored"), e);
    }

    // Load fleets
    // Must be after loading the result/turn because it requires shipsource flags
    try {
        game::db::FleetLoader(*m_charset, tx).load(root.gameDirectory(), turn.universe(), player);
    }
    catch (afl::except::FileProblemException& e) {
        m_log.write(LogListener::Warn, LOG_NAME, tx("File has been ignored"), e);
    }

    // Load FLAK from remote
    ldr.loadFlakBattles(turn, *m_serverDirectory, player);

    // Load util from remote
    Parser mp(tx, m_log, game, player, root, game::actions::mustHaveShipList(session), session.world().atomTable());
    {
        Ptr<Stream> file = m_serverDirectory->openFileNT(Format("util%d.dat", player), afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            mp.loadUtilData(*file, *m_charset);
        } else {
            mp.handleNoUtilData();
        }
    }

    // Message parser
    {
        Ptr<Stream> file = specDir->openFileNT("msgparse.ini", afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            mp.parseMessages(*file, turn.inbox(), *m_charset);
        }
    }
}

void
game::pcc::TurnLoader::doSaveCurrentTurn(const Turn& turn, const Game& game, PlayerSet_t players, SaveOptions_t opts, const Root& root, Session& session)
{
    afl::string::Translator& tx = session.translator();
    if (turn.getCommandPlayers().containsAnyOf(players)) {
        // Request temporary turns if desired
        m_serverTransport->setTemporaryTurn(opts.contains(MarkTurnTemporary));

        // Build all the turns
        m_log.write(LogListener::Info, LOG_NAME, tx("Generating turn commands..."));
        game::v3::trn::FileSet turns(*m_serverDirectory, *m_charset);
        for (int player = 1; player <= MAX_PLAYERS; ++player) {
            if (players.contains(player)) {
                TurnFile& thisTurn = turns.create(player, turn.getTimestamp(), turn.getTurnNumber());
                Loader(*m_charset, tx, m_log).saveTurnFile(thisTurn, turn, player, root);
            }
        }

        // Generate turns
        turns.updateTrailers();

        // Upload all files
        turns.saveAll(session.log(), root.playerList(), tx);
    }

    for (int player = 1; player <= MAX_PLAYERS; ++player) {
        if (players.contains(player)) {
            if (turn.getLocalDataPlayers().contains(player)) {
                // chart.cc
                saveCurrentDatabases(turn, game, player, root, session, *m_charset);

                // Fleets
                game::db::FleetLoader(*m_charset, tx).save(root.gameDirectory(), turn.universe(), player);
            }
        }
    }

    game.expressionLists().saveRecentFiles(m_profile, m_log, tx);

    m_log.write(LogListener::Info, LOG_NAME, tx("Uploading data..."));
    m_serverDirectory->flush();
}
