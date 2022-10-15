/**
  *  \file game/v3/resultloader.cpp
  *  \brief Class game::v3::ResultLoader
  */

#include "game/v3/resultloader.hpp"
#include "afl/base/ptr.hpp"
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
using afl::sys::LogListener;
using game::config::UserConfiguration;

namespace {
    const char LOG_NAME[] = "game.v3.resultloader";
}

game::v3::ResultLoader::ResultLoader(afl::base::Ref<afl::io::Directory> specificationDirectory,
                                     afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                     std::auto_ptr<afl::charset::Charset> charset,
                                     afl::string::Translator& tx,
                                     afl::sys::LogListener& log,
                                     const DirectoryScanner& scanner,
                                     afl::io::FileSystem& fs,
                                     util::ProfileDirectory* pProfile,
                                     game::browser::UserCallback* pCallback)
    : m_specificationDirectory(specificationDirectory),
      m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_charset(charset),
      m_translator(tx),
      m_log(log),
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
game::v3::ResultLoader::loadCurrentTurn(Turn& turn, Game& game, int player, game::Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    // ex game/load.h:loadCommon
    class Task : public Task_t {
     public:
        Task(ResultLoader& parent, Turn& turn, Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t>& then)
            : m_parent(parent), m_turn(turn), m_game(game), m_player(player), m_root(root), m_session(session), m_then(then),
              m_checker(turn, parent.m_pCallback, parent.m_log, parent.m_translator)
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
        ResultLoader& m_parent;
        Turn& m_turn;
        Game& m_game;
        int m_player;
        Root& m_root;
        Session& m_session;
        std::auto_ptr<StatusTask_t> m_then;
        PasswordChecker m_checker;
    };
    return std::auto_ptr<Task_t>(new Task(*this, turn, game, player, root, session, then));
}

std::auto_ptr<game::Task_t>
game::v3::ResultLoader::saveCurrentTurn(const Turn& turn, const Game& game, PlayerSet_t players, SaveOptions_t /*opts*/, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    // ex saveTurns
    try {
        doSaveCurrentTurn(turn, game, players, root, session);
        return makeConfirmationTask(true, then);
    }
    catch (std::exception& e) {
        session.log().write(LogListener::Error, LOG_NAME, m_translator("Unable to save game"), e);
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
game::v3::ResultLoader::loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, std::auto_ptr<StatusTask_t> then)
{
    class Task : public Task_t {
     public:
        Task(ResultLoader& parent, Turn& turn, Game& game, int player, int turnNumber, Root& root, std::auto_ptr<StatusTask_t>& then)
            : m_parent(parent), m_turn(turn), m_game(game), m_player(player), m_turnNumber(turnNumber), m_root(root), m_then(then)
            { }
        virtual void call()
            {
                m_parent.m_log.write(LogListener::Trace, LOG_NAME, "Task: loadHistoryTurn");
                try {
                    m_parent.doLoadHistoryTurn(m_turn, m_game, m_player, m_turnNumber, m_root);
                    m_then->call(true);
                }
                catch (std::exception& e) {
                    m_parent.m_log.write(LogListener::Error, LOG_NAME, String_t(), e);
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
        std::auto_ptr<StatusTask_t> m_then;
    };
    return std::auto_ptr<Task_t>(new Task(*this, turn, game, player, turnNumber, root, then));
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
game::v3::ResultLoader::loadTurnfile(Turn& trn, const Root& root, afl::io::Stream& file, int player) const
{
    // ex game/load-trn.cc:loadTurn
    m_log.write(m_log.Info, LOG_NAME, Format(m_translator("Loading %s TRN file..."), root.playerList().getPlayerName(player, Player::AdjectiveName, m_translator)));
    Loader(*m_charset, m_translator, m_log).loadTurnfile(trn, root, file, player);
}

void
game::v3::ResultLoader::doLoadCurrentTurn(Turn& turn, Game& game, int player, game::Root& root, Session& session)
{
    // Initialize
    Loader ldr(*m_charset, m_translator, m_log);
    ldr.prepareUniverse(turn.universe());
    ldr.prepareTurn(turn, root, session, player);

    // Load common files
    ldr.loadCommonFiles(root.gameDirectory(), *m_specificationDirectory, turn.universe(), player);

    // load database
    loadCurrentDatabases(turn, game, player, root, session);

    // expression lists
    if (m_pProfile != 0) {
        game.expressionLists().loadRecentFiles(*m_pProfile, m_log, m_translator);
        game.expressionLists().loadPredefinedFiles(*m_pProfile, *m_specificationDirectory, m_log, m_translator);
    }

    // ex GGameResultStorage::load(GGameTurn& trn)
    {
        Ref<Stream> file = root.gameDirectory().openFile(Format("player%d.rst", player), afl::io::FileSystem::OpenRead);
        m_log.write(m_log.Info, LOG_NAME, Format(m_translator("Loading %s RST file..."), root.playerList().getPlayerName(player, Player::AdjectiveName, m_translator)));
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
            m_log.write(m_log.Warn, LOG_NAME, m_translator("Unable to create backup file"), e);
        }
    }

    if (m_playerFlags.get(player).contains(DirectoryScanner::HaveTurn)) {
        Ref<Stream> file = root.gameDirectory().openFile(Format("player%d.trn", player), afl::io::FileSystem::OpenRead);
        loadTurnfile(turn, root, *file, player);
    }

    // Backup

    // Load fleets.
    // Must be after loading the result/turn because it requires shipsource flags
    game::db::FleetLoader(*m_charset, m_translator).load(root.gameDirectory(), turn.universe(), player);

    // Util
    Parser mp(m_translator, m_log, game, player, root, game::actions::mustHaveShipList(session), session.world().atomTable());
    {
        Ptr<Stream> file = root.gameDirectory().openFileNT(Format("util%d.dat", player), afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            mp.loadUtilData(*file, *m_charset);
        }
    }

    // Message parser
    {
        Ptr<Stream> file = m_specificationDirectory->openFileNT("msgparse.ini", afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            mp.parseMessages(*file, turn.inbox());
        }
    }

    // FLAK
    ldr.loadFlakBattles(turn, root.gameDirectory(), player);
}

void
game::v3::ResultLoader::doLoadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root)
{
    // Initialize planets and bases
    Loader ldr(*m_charset, m_translator, m_log);
    ldr.prepareUniverse(turn.universe());

    // FIXME: backup these files?
    ldr.loadCommonFiles(root.gameDirectory(), *m_specificationDirectory, turn.universe(), player);

    // load turn file backup
    util::BackupFile tpl;
    tpl.setGameDirectoryName(root.gameDirectory().getDirectoryName());
    tpl.setPlayerNumber(player);
    tpl.setTurnNumber(turnNumber);

    {
        Ref<Stream> file = tpl.openFile(m_fileSystem, root.userConfiguration()[UserConfiguration::Backup_Result](), m_translator);
        m_log.write(m_log.Info, LOG_NAME, Format(m_translator("Loading %s backup file..."), root.playerList().getPlayerName(player, Player::AdjectiveName, m_translator)));
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
game::v3::ResultLoader::doSaveCurrentTurn(const Turn& turn, const Game& game, PlayerSet_t players, const Root& root, Session& session)
{
    if (session.getEditableAreas().contains(Session::CommandArea)) {
        game::v3::trn::FileSet turns(root.gameDirectory(), *m_charset);
        m_log.write(m_log.Info, LOG_NAME, m_translator("Generating turn commands..."));

        // Create turn files
        for (int player = 1; player <= MAX_PLAYERS; ++player) {
            if (players.contains(player)) {
                TurnFile& thisTurn = turns.create(player, turn.getTimestamp(), turn.getTurnNumber());
                Loader(*m_charset, m_translator, m_log).saveTurnFile(thisTurn, turn, player, root);
            }
        }

        // Generate turn
        turns.updateTrailers();
        turns.saveAll(m_log, root.playerList(), m_fileSystem, root.userConfiguration(), m_translator);
    }

    if (session.getEditableAreas().contains(Session::LocalDataArea)) {
        for (int player = 1; player <= MAX_PLAYERS; ++player) {
            if (players.contains(player)) {
                // chart.cc
                saveCurrentDatabases(turn, game, player, root, session, *m_charset);

                // Fleets
                game::db::FleetLoader(*m_charset, m_translator).save(root.gameDirectory(), turn.universe(), player);
            }
        }
    }

    if (m_pProfile != 0) {
        game.expressionLists().saveRecentFiles(*m_pProfile, m_log, m_translator);
    }
}
