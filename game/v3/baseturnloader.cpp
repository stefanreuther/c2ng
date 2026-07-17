/**
  *  \file game/v3/baseturnloader.cpp
  *  \brief Class game::v3::BaseTurnLoader
  */

#include "game/v3/baseturnloader.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/db/fleetloader.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/v3/loader.hpp"
#include "game/v3/parser.hpp"
#include "util/backupfile.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::string::Format;
using afl::string::Translator;
using afl::sys::LogListener;
using game::config::UserConfiguration;
using util::BackupFile;

namespace {
    const char*const LOG_NAME = "game.v3.loader";
}

void
game::v3::BaseTurnLoader::getHistoryStatus(int player, int turn, afl::base::Memory<HistoryStatus> status, const Root& root)
{
    while (HistoryStatus* p = status.eat()) {
        // Prepare template
        BackupFile tpl;
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
game::v3::BaseTurnLoader::loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    class Task : public Task_t {
     public:
        Task(BaseTurnLoader& parent, Turn& turn, Game& game, int player, int turnNumber, Root& root, LogListener& log, Translator& tx, std::auto_ptr<StatusTask_t>& then)
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
        BaseTurnLoader& m_parent;
        Turn& m_turn;
        Game& m_game;
        const int m_player;
        const int m_turnNumber;
        Root& m_root;
        LogListener& m_log;
        Translator& m_translator;
        const std::auto_ptr<StatusTask_t> m_then;
    };
    return std::auto_ptr<Task_t>(new Task(*this, turn, game, player, turnNumber, root, session.log(), session.translator(), then));
}

std::auto_ptr<game::Task_t>
game::v3::BaseTurnLoader::saveConfiguration(const Root& root, afl::sys::LogListener& log, afl::string::Translator& tx, std::auto_ptr<Task_t> then)
{
    return defaultSaveConfiguration(root, m_pProfile, log, tx, then);
}

void
game::v3::BaseTurnLoader::loadExpressionLists(Game& game, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    if (m_pProfile != 0) {
        game.expressionLists().loadRecentFiles(*m_pProfile, log, tx);
        game.expressionLists().loadPredefinedFiles(*m_pProfile, specificationDirectory(), log, tx);
    }
}

void
game::v3::BaseTurnLoader::saveExpressionLists(const Game& game, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    if (m_pProfile != 0) {
        game.expressionLists().saveRecentFiles(*m_pProfile, log, tx);
    }
}

void
game::v3::BaseTurnLoader::loadExtraFiles(Game& game, Turn& turn, int player, Root& root, Session& session)
{
    LogListener& log = session.log();
    Translator& tx = session.translator();
    try {
        game::db::FleetLoader(charset(), tx).load(root.gameDirectory(), turn.universe(), player);
    }
    catch (afl::except::FileProblemException& e) {
        log.write(LogListener::Warn, LOG_NAME, tx("File has been ignored"), e);
    }

    // FLAK
    Loader ldr(charset(), tx, log);
    ldr.loadFlakBattles(turn, root.gameDirectory(), player);

    // Util
    Parser mp(tx, log, game, player, root, game::actions::mustHaveShipList(session), session.world().atomTable());
    {
        Ptr<Stream> file = root.gameDirectory().openFileNT(Format("util%d.dat", player), FileSystem::OpenRead);
        if (file.get() != 0) {
            mp.loadUtilData(*file, charset());
        } else {
            mp.handleNoUtilData();
        }
    }

    // Message parser
    {
        Ptr<Stream> file = specificationDirectory().openFileNT("msgparse.ini", FileSystem::OpenRead);
        if (file.get() != 0) {
            mp.parseMessages(*file, turn.inbox(), charset());
        }
    }
}

/** Implementation of loadHistoryTurn.
    Can throw on error.
    \param turn Turn
    \param game Game
    \param player Player
    \param turnNumber Turn number to load
    \param root Root
    \param log Logger
    \param tx Translator */
void
game::v3::BaseTurnLoader::doLoadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // Initialize planets and bases
    Loader ldr(*m_charset, tx, log);
    ldr.prepareUniverse(turn.universe());

    // FIXME: backup these files?
    ldr.loadCommonFiles(root.gameDirectory(), *m_specificationDirectory, turn.universe(), player);

    // load turn file backup
    BackupFile tpl;
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
