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
    : m_specificationDirectory(specificationDirectory),
      m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_charset(charset),
      m_fileSystem(fs),
      m_pProfile(pProfile),
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

void
game::v3::ResultLoader::getHistoryStatus(int player, int turn, afl::base::Memory<HistoryStatus> status, const Root& root)
{
    // FIXME: validate turn number? If turn number is >= current turn, report Negative.
    while (HistoryStatus* p = status.eat()) {
        // Prepare template
        util::BackupFile tpl;
        tpl.setGameDirectoryName(root.gameDirectory().getDirectoryName());
        tpl.setPlayerNumber(player);
        tpl.setTurnNumber(turn);

        // Do we have a history file?
        if (tpl.hasFile(m_fileSystem, root.userConfiguration()[UserConfiguration::Backup_Result]())) {
            *p = StronglyPositive;
        } else {
            *p = Negative;
        }

        ++turn;
    }
}

std::auto_ptr<game::Task_t>
game::v3::ResultLoader::loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    class Task : public Task_t {
     public:
        Task(ResultLoader& parent, Turn& turn, Game& game, int player, int turnNumber, Root& root, LogListener& log, Translator& tx, std::auto_ptr<StatusTask_t>& then)
            : m_parent(parent), m_turn(turn), m_game(game), m_player(player), m_turnNumber(turnNumber), m_root(root), m_log(log), m_translator(tx), m_then(then)
            { }
        virtual void call()
            {
                m_log.write(LogListener::Trace, LOG_NAME, "Task: loadHistoryTurn");
                try {
                    m_parent.doLoadHistoryTurn(m_turn, m_game, m_player, m_turnNumber, m_root, m_log, m_translator);
                    m_then->call(true);
                }
                catch (std::exception& e) {
                    m_log.write(LogListener::Error, LOG_NAME, String_t(), e);
                    m_then->call(false);
                }
            }
     private:
        ResultLoader& m_parent;
        Turn& m_turn;
        Game& m_game;
        int m_player;
        int m_turnNumber;
        Root& m_root;
        LogListener& m_log;
        Translator& m_translator;
        std::auto_ptr<StatusTask_t> m_then;
    };
    return std::auto_ptr<Task_t>(new Task(*this, turn, game, player, turnNumber, root, session.log(), session.translator(), then));
}

std::auto_ptr<game::Task_t>
game::v3::ResultLoader::saveConfiguration(const Root& root, afl::sys::LogListener& log, afl::string::Translator& tx, std::auto_ptr<Task_t> then)
{
    return defaultSaveConfiguration(root, m_pProfile, log, tx, then);
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
    Loader(*m_charset, tx, log).loadTurnfile(trn, root, file, player);
}

void
game::v3::ResultLoader::doLoadCurrentTurn(Game& game, int player, game::Root& root, Session& session)
{
    // Initialize
    Turn& turn = game.currentTurn();
    Translator& tx = session.translator();
    LogListener& log = session.log();
    Loader ldr(*m_charset, tx, log);
    ldr.prepareUniverse(turn.universe());
    ldr.prepareTurn(turn, root, session, player);

    // Load common files
    ldr.loadCommonFiles(root.gameDirectory(), *m_specificationDirectory, turn.universe(), player);

    // load database
    loadCurrentDatabases(game, player, root, session);

    // expression lists
    if (m_pProfile != 0) {
        game.expressionLists().loadRecentFiles(*m_pProfile, log, tx);
        game.expressionLists().loadPredefinedFiles(*m_pProfile, *m_specificationDirectory, log, tx);
    }

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
            tpl.copyFile(m_fileSystem, root.userConfiguration()[UserConfiguration::Backup_Result](), *file);
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
    // Must be after loading the result/turn because it requires shipsource flags
    try {
        game::db::FleetLoader(*m_charset, tx).load(root.gameDirectory(), turn.universe(), player);
    }
    catch (afl::except::FileProblemException& e) {
        log.write(LogListener::Warn, LOG_NAME, tx("File has been ignored"), e);
    }

    // FLAK
    ldr.loadFlakBattles(turn, root.gameDirectory(), player);

    // Util
    Parser mp(tx, log, game, player, root, game::actions::mustHaveShipList(session), session.world().atomTable());
    {
        Ptr<Stream> file = root.gameDirectory().openFileNT(Format("util%d.dat", player), afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            mp.loadUtilData(*file, *m_charset);
        } else {
            mp.handleNoUtilData();
        }
    }

    // Message parser
    {
        Ptr<Stream> file = m_specificationDirectory->openFileNT("msgparse.ini", afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            mp.parseMessages(*file, turn.inbox(), *m_charset);
        }
    }
}

void
game::v3::ResultLoader::doLoadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // Initialize planets and bases
    Loader ldr(*m_charset, tx, log);
    ldr.prepareUniverse(turn.universe());

    // FIXME: backup these files?
    ldr.loadCommonFiles(root.gameDirectory(), *m_specificationDirectory, turn.universe(), player);

    // load turn file backup
    util::BackupFile tpl;
    tpl.setGameDirectoryName(root.gameDirectory().getDirectoryName());
    tpl.setPlayerNumber(player);
    tpl.setTurnNumber(turnNumber);

    {
        Ref<Stream> file = tpl.openFile(m_fileSystem, root.userConfiguration()[UserConfiguration::Backup_Result](), tx);
        log.write(LogListener::Info, LOG_NAME, Format(tx("Loading %s backup file..."), root.playerList().getPlayerName(player, Player::AdjectiveName, tx)));
        ldr.loadResult(turn, root, game, *file, player);
    }

    // FIXME: load turn
    // if (have_trn) {
    //     Ptr<Stream> trnfile = game_file_dir->openFile(format("player%d.trn", player), Stream::C_READ);
    //     loadTurn(trn, *trnfile, player);
    // }

    // FIXME: history fleets not loaded here
    // loadFleets(trn, *game_file_dir, player);
    // FIXME: alliances not loaded until here; would need message/util.dat parsing
    // FIXME: load FLAK
}

void
game::v3::ResultLoader::doSaveCurrentTurn(const Game& game, PlayerSet_t players, const Root& root, Session& session)
{
    const Turn& turn = game.currentTurn();
    Translator& tx = session.translator();
    LogListener& log = session.log();
    if (turn.getCommandPlayers().containsAnyOf(players)) {
        game::v3::trn::FileSet turns(root.gameDirectory(), *m_charset);
        log.write(LogListener::Info, LOG_NAME, tx("Generating turn commands..."));

        // Create turn files
        for (int player = 1; player <= MAX_PLAYERS; ++player) {
            if (players.contains(player)) {
                TurnFile& thisTurn = turns.create(player, turn.getTimestamp(), turn.getTurnNumber());
                Loader(*m_charset, tx, log).saveTurnFile(thisTurn, turn, player, root);
            }
        }

        // Generate turn
        turns.updateTrailers();
        turns.saveAll(log, root.playerList(), m_fileSystem, root.userConfiguration(), tx);
    }

    for (int player = 1; player <= MAX_PLAYERS; ++player) {
        if (players.contains(player)) {
            if (turn.getLocalDataPlayers().contains(player)) {
                // chart.cc
                saveCurrentDatabases(game, player, root, session, *m_charset);

                // Fleets
                game::db::FleetLoader(*m_charset, tx).save(root.gameDirectory(), turn.universe(), player);
            }
        }
    }

    if (m_pProfile != 0) {
        game.expressionLists().saveRecentFiles(*m_pProfile, log, tx);
    }
}
