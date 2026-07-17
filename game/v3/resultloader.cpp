/**
  *  \file game/v3/resultloader.cpp
  *  \brief Class game::v3::ResultLoader
  */

#include "game/v3/resultloader.hpp"
#include "afl/base/ptr.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/db/fleetloader.hpp"
#include "game/game.hpp"
#include "game/player.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "game/v3/loader.hpp"
#include "game/v3/parser.hpp"
#include "game/v3/passwordchecker.hpp"
#include "game/v3/trn/fileset.hpp"
#include "game/v3/turnfile.hpp"
#include "util/backupfile.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::Stream;
using afl::string::Format;
using afl::string::Translator;
using afl::sys::LogListener;
using game::config::UserConfiguration;

namespace {
    const char LOG_NAME[] = "game.v3.resultloader";
}

game::v3::ResultLoader::ResultLoader(afl::base::Ref<afl::io::Directory> specificationDirectory,
                                     afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                     std::auto_ptr<afl::charset::Charset> charset,
                                     const DirectoryScanner& scanner,
                                     afl::io::FileSystem& fs,
                                     util::ProfileDirectory* pProfile,
                                     game::browser::UserCallback* pCallback)
    : BaseTurnLoader(specificationDirectory, charset, fs, pProfile),
      m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_pCallback(pCallback)
{
    for (int i = 1; i <= DirectoryScanner::NUM_PLAYERS; ++i) {
        m_playerFlags.set(i, scanner.getPlayerFlags(i));
    }
}

game::v3::ResultLoader::PlayerStatusSet_t
game::v3::ResultLoader::getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const
{
    PlayerStatusSet_t result;
    DirectoryScanner::PlayerFlags_t flags = m_playerFlags.get(player);
    if (flags.contains(DirectoryScanner::HaveResult)) {
        if (flags.contains(DirectoryScanner::HaveTurn)) {
            extra = tx("RST + TRN");
        } else {
            extra = tx("RST");
        }
        result += Available;
        result += Playable;
        result += Primary;
    } else {
        extra.clear();
    }
    return result;
}

std::auto_ptr<game::Task_t>
game::v3::ResultLoader::loadCurrentTurn(Game& game, int player, game::Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    // ex game/load.h:loadCommon
    class Task : public Task_t {
     public:
        Task(ResultLoader& parent, Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t>& then)
            : m_parent(parent), m_game(game), m_player(player), m_root(root), m_session(session), m_then(then),
              m_checker(game.currentTurn(), parent.m_pCallback, session.log(), session.translator())
            { }

        virtual void call()
            {
                m_session.log().write(LogListener::Trace, LOG_NAME, "Task: loadCurrentTurn");
                try {
                    m_parent.doLoadCurrentTurn(m_game, m_player, m_root, m_session);
                    m_checker.checkPassword(m_player, m_session.authCache(), m_then);
                }
                catch (std::exception& e) {
                    m_session.log().write(LogListener::Error, LOG_NAME, String_t(), e);
                    m_then->call(false);
                }
            }
     private:
        ResultLoader& m_parent;
        Game& m_game;
        int m_player;
        Root& m_root;
        Session& m_session;
        std::auto_ptr<StatusTask_t> m_then;
        PasswordChecker m_checker;
    };
    return std::auto_ptr<Task_t>(new Task(*this, game, player, root, session, then));
}

std::auto_ptr<game::Task_t>
game::v3::ResultLoader::saveCurrentTurn(const Game& game, PlayerSet_t players, SaveOptions_t /*opts*/, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    // ex saveTurns
    try {
        doSaveCurrentTurn(game, players, root, session);
        return makeConfirmationTask(true, then);
    }
    catch (std::exception& e) {
        session.log().write(LogListener::Error, LOG_NAME, session.translator()("Unable to save game"), e);
        return makeConfirmationTask(false, then);
    }
}

String_t
game::v3::ResultLoader::getProperty(Property p)
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
game::v3::ResultLoader::loadTurnfile(Turn& trn, const Root& root, afl::io::Stream& file, int player, afl::sys::LogListener& log, afl::string::Translator& tx) const
{
    // ex game/load-trn.cc:loadTurn
    log.write(LogListener::Info, LOG_NAME, Format(tx("Loading %s TRN file..."), root.playerList().getPlayerName(player, Player::AdjectiveName, tx)));
    Loader(charset(), tx, log).loadTurnfile(trn, root, file, player);
}

void
game::v3::ResultLoader::doLoadCurrentTurn(Game& game, int player, game::Root& root, Session& session)
{
    // Initialize
    Turn& turn = game.currentTurn();
    Translator& tx = session.translator();
    LogListener& log = session.log();
    Loader ldr(charset(), tx, log);
    ldr.prepareUniverse(turn.universe());
    ldr.prepareTurn(turn, root, session, player);

    // Load common files
    ldr.loadCommonFiles(root.gameDirectory(), specificationDirectory(), turn.universe(), player);

    // load database
    loadCurrentDatabases(game, player, root, session);

    // expression lists
    loadExpressionLists(game, log, tx);

    // ex GGameResultStorage::load(GGameTurn& trn)
    {
        Ref<Stream> file = root.gameDirectory().openFile(Format("player%d.rst", player), afl::io::FileSystem::OpenRead);
        log.write(LogListener::Info, LOG_NAME, Format(tx("Loading %s RST file..."), root.playerList().getPlayerName(player, Player::AdjectiveName, tx)));
        ldr.loadResult(turn, root, game, *file, player);

        // Backup
        try {
            file->setPos(0);
            util::BackupFile tpl;
            tpl.setPlayerNumber(player);
            tpl.setTurnNumber(turn.getTurnNumber());
            tpl.setGameDirectoryName(root.gameDirectory().getDirectoryName());
            tpl.copyFile(fileSystem(), root.userConfiguration()[UserConfiguration::Backup_Result](), *file);
        }
        catch (std::exception& e) {
            log.write(LogListener::Warn, LOG_NAME, tx("Unable to create backup file"), e);
        }
    }

    if (m_playerFlags.get(player).contains(DirectoryScanner::HaveTurn)) {
        try {
            Ref<Stream> file = root.gameDirectory().openFile(Format("player%d.trn", player), afl::io::FileSystem::OpenRead);
            loadTurnfile(turn, root, *file, player, log, tx);
        }
        catch (afl::except::FileProblemException& e) {
            log.write(LogListener::Warn, LOG_NAME, tx("File has been ignored"), e);
        }
    }

    // Load fleets.
    // Must be after loading main data because it requires shipsource flags
    loadExtraFiles(game, turn, player, root, session);
}

void
game::v3::ResultLoader::doSaveCurrentTurn(const Game& game, PlayerSet_t players, const Root& root, Session& session)
{
    const Turn& turn = game.currentTurn();
    Translator& tx = session.translator();
    LogListener& log = session.log();
    if (turn.getCommandPlayers().containsAnyOf(players)) {
        game::v3::trn::FileSet turns(root.gameDirectory(), charset());
        log.write(LogListener::Info, LOG_NAME, tx("Generating turn commands..."));

        // Create turn files
        for (int player = 1; player <= MAX_PLAYERS; ++player) {
            if (players.contains(player)) {
                TurnFile& thisTurn = turns.create(player, turn.getTimestamp(), turn.getTurnNumber());
                Loader(charset(), tx, log).saveTurnFile(thisTurn, turn, player, root);
            }
        }

        // Generate turn
        turns.updateTrailers();
        turns.saveAll(log, root.playerList(), fileSystem(), root.userConfiguration(), tx);
    }

    for (int player = 1; player <= MAX_PLAYERS; ++player) {
        if (players.contains(player)) {
            if (turn.getLocalDataPlayers().contains(player)) {
                // chart.cc
                saveCurrentDatabases(game, player, root, session, charset());

                // Fleets
                game::db::FleetLoader(charset(), tx).save(root.gameDirectory(), turn.universe(), player);
            }
        }
    }

    saveExpressionLists(game, log, tx);
}
