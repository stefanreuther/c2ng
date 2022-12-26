/**
  *  \file game/pcc/turnloader.cpp
  *  \brief Class game::pcc::TurnLoader
  */

#include "game/pcc/turnloader.hpp"
#include "afl/base/inlinememory.hpp"
#include "afl/base/ptr.hpp"
#include "afl/data/access.hpp"
#include "afl/data/value.hpp"
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
#include "server/interface/hostturn.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::Stream;
using afl::string::Format;
using afl::sys::LogListener;
using game::v3::Loader;
using game::v3::Parser;
using game::v3::PasswordChecker;
using game::v3::TurnFile;
using server::interface::HostTurn;

namespace {
    const char LOG_NAME[] = "game.pcc";

    String_t formatTurnStatus(int32_t result, afl::string::Translator& tx)
    {
        // MissingTurn, NeedlessTurn cannot be results of a turn submission.
        switch (result & ~HostTurn::TemporaryTurnFlag) {
         case HostTurn::GreenTurn:  return Format(tx("Turn was accepted"));
         case HostTurn::YellowTurn: return Format(tx("Turn was accepted with warnings (yellow)"));
         case HostTurn::RedTurn:    return Format(tx("Turn was rejected (red)"));
         case HostTurn::BadTurn:    return Format(tx("Turn was rejected (invalid)"));
         case HostTurn::StaleTurn:  return Format(tx("Turn was stale"));
         default:                   return Format(tx("Unknown turn status (%d)"), result);
        }
    }
}

game::pcc::TurnLoader::TurnLoader(afl::base::Ref<afl::io::Directory> localDirectory,
                                  afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                  afl::base::Ref<ServerDirectory> serverDirectory,
                                  int32_t hostGameNumber,
                                  std::auto_ptr<afl::charset::Charset> charset,
                                  afl::string::Translator& tx,
                                  afl::sys::LogListener& log,
                                  PlayerSet_t availablePlayers,
                                  util::ProfileDirectory& profile)
    : m_localDirectory(localDirectory),
      m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_serverDirectory(serverDirectory),
      m_hostGameNumber(hostGameNumber),
      m_charset(charset),
      m_translator(tx),
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
              m_checker(turn, &parent.m_serverDirectory->handler().callback(), parent.m_log, parent.m_translator)
            { }

        virtual void call()
            {
                m_parent.m_log.write(LogListener::Trace, LOG_NAME, "Task: loadCurrentTurn");
                try {
                    m_parent.doLoadCurrentTurn(m_turn, m_game, m_player, m_root, m_session);
                    m_checker.checkPassword(m_player, m_session.authCache(), m_then);
                }
                catch (std::exception& e) {
                    m_session.log().write(afl::sys::LogListener::Error, LOG_NAME, String_t(), e);
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
    return m_serverDirectory->handler().login(m_serverDirectory->account(),
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
    return m_serverDirectory->handler().login(m_serverDirectory->account(),
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
game::pcc::TurnLoader::loadHistoryTurn(Turn& /*turn*/, Game& /*game*/, int /*player*/, int /*turnNumber*/, Root& /*root*/, std::auto_ptr<StatusTask_t> then)
{
    // FIXME: implement
    return makeConfirmationTask(false, then);
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
    Loader ldr(*m_charset, m_translator, m_log);
    ldr.prepareUniverse(turn.universe());
    ldr.prepareTurn(turn, root, session, player);

    // Merged spec directory
    Ref<afl::io::MultiDirectory> specDir = afl::io::MultiDirectory::create();
    specDir->addDirectory(m_serverDirectory);
    specDir->addDirectory(m_defaultSpecificationDirectory);

    // Load common files
    ldr.loadCommonFiles(*m_serverDirectory, *specDir, turn.universe(), player);

    // Load database from local game directory (=root.gameDirectory())
    loadCurrentDatabases(turn, game, player, root, session);

    // Load expression lists from profile
    game.expressionLists().loadRecentFiles(m_profile, m_log, m_translator);
    game.expressionLists().loadPredefinedFiles(m_profile, *specDir, m_log, m_translator);

    // Load result file from remote
    {
        Ref<Stream> file = m_serverDirectory->openFile(Format("player%d.rst", player), afl::io::FileSystem::OpenRead);
        m_log.write(LogListener::Info, LOG_NAME, Format(m_translator("Loading %s RST file..."), root.playerList().getPlayerName(player, Player::AdjectiveName, m_translator)));
        ldr.loadResult(turn, root, game, *file, player);

        // TODO: backups?
    }

    // Try to load turn from remote
    {
        Ptr<Stream> file = m_serverDirectory->openFileNT(Format("player%d.trn", player), afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            m_log.write(LogListener::Info, LOG_NAME, Format(m_translator("Loading %s TRN file..."), root.playerList().getPlayerName(player, Player::AdjectiveName, m_translator)));
            Loader(*m_charset, m_translator, m_log).loadTurnfile(turn, root, *file, player);
        }
    }

    // Load fleets from local game directory
    // Must be after loading the result/turn because it requires shipsource flags
    game::db::FleetLoader(*m_charset, m_translator).load(root.gameDirectory(), turn.universe(), player);

    // Load FLAK from remote
    ldr.loadFlakBattles(turn, *m_serverDirectory, player);

    // Load util from remote
    Parser mp(m_translator, m_log, game, player, root, game::actions::mustHaveShipList(session), session.world().atomTable());
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
    if (session.getEditableAreas().contains(Session::CommandArea)) {
        game::v3::trn::FileSet turns(*m_serverDirectory, *m_charset);
        m_log.write(LogListener::Info, LOG_NAME, m_translator("Generating turn commands..."));

        // Create turn files
        std::vector<TurnFile*> turnPtrs;
        for (int player = 1; player <= MAX_PLAYERS; ++player) {
            if (players.contains(player)) {
                TurnFile& thisTurn = turns.create(player, turn.getTimestamp(), turn.getTurnNumber());
                Loader(*m_charset, m_translator, m_log).saveTurnFile(thisTurn, turn, player, root);
                turnPtrs.push_back(&thisTurn);
            }
        }

        // Generate turns
        turns.updateTrailers();

        // Upload all files
        for (size_t i = 0; i < turnPtrs.size(); ++i) {
            // Create image
            TurnFile& thisTurn = *turnPtrs[i];
            int player = thisTurn.getPlayer();
            String_t fileName = Format("player%d.trn", player);
            m_log.write(LogListener::Info, LOG_NAME, Format(m_translator("Uploading %s..."), fileName));
            afl::io::InternalStream sink;
            thisTurn.write(sink);

            if (m_hostGameNumber != 0) {
                // Hosted game: submit to host
                BrowserHandler& handler = m_serverDirectory->handler();
                game::browser::Account& account = m_serverDirectory->account();
                std::auto_ptr<afl::data::Value> result(handler.uploadTurnPreAuthenticated(account, m_hostGameNumber, player, sink.getContent()));
                afl::data::Access a(result);
                if (a("result").toInteger()) {
                    // Turn status
                    m_log.write(LogListener::Info, LOG_NAME, formatTurnStatus(a("status").toInteger(), m_translator));

                    // Turn checker output
                    String_t output = a("output").toString();
                    if (!output.empty()) {
                        m_log.write(LogListener::Info, LOG_NAME, m_translator("Turn checker output:"));
                        String_t::size_type p = 0, n;
                        while ((n = output.find('\n', p)) != String_t::npos) {
                            m_log.write(LogListener::Info, LOG_NAME, "> " + output.substr(p, n-p));
                            p = n+1;
                        }
                        if (p < output.size()) {
                            m_log.write(LogListener::Info, LOG_NAME, "> " + output.substr(p));
                        }
                    }

                    // Mark temporary
                    if (a("allowtemp").toInteger() && opts.contains(MarkTurnTemporary)) {
                        handler.markTurnTemporaryPreAuthenticated(account, m_hostGameNumber, player, 1);
                        m_log.write(LogListener::Info, LOG_NAME, m_translator("Turn marked temporary."));
                    }
                } else {
                    m_log.write(LogListener::Error, LOG_NAME, Format(m_translator("Error uploading turn: %s"), a("error").toString()));
                }
            } else {
                // Uploaded game: just upload the file
                m_serverDirectory->putFile(fileName, sink.getContent());
            }
        }
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

    game.expressionLists().saveRecentFiles(m_profile, m_log, m_translator);
}
