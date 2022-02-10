/**
  *  \file game/v3/resultloader.cpp
  *  \brief Class game::v3::ResultLoader
  */

#include "game/v3/resultloader.hpp"
#include "afl/base/inlinememory.hpp"
#include "afl/base/ptr.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/config/stringoption.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/db/fleetloader.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/player.hpp"
#include "game/root.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"
#include "game/v3/genfile.hpp"
#include "game/v3/inboxfile.hpp"
#include "game/v3/loader.hpp"
#include "game/v3/packer.hpp"
#include "game/v3/parser.hpp"
#include "game/v3/registrationkey.hpp"
#include "game/v3/resultfile.hpp"
#include "game/v3/reverter.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/trn/fileset.hpp"
#include "game/v3/trn/turnprocessor.hpp"
#include "game/v3/turnfile.hpp"
#include "util/backupfile.hpp"
#include "util/translation.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::except::FileFormatException;
using afl::except::checkAssertion;
using afl::io::Stream;
using afl::string::Format;
using game::config::UserConfiguration;

namespace {
    const char LOG_NAME[] = "game.v3.resultloader";

    /* Extract commands from a message.
       This figures out the PHost commands from a message a player sent to himself.
       \param trn     Game turn object
       \param text    Decoded message text
       \param player  Player number
       \returns Message text without commands. Might be empty if the message consists entirely of commands. */
    String_t extractCommands(game::Turn& trn, String_t text, int player)
    {
        String_t::size_type n = text.find_first_not_of(' ');
        if (n != text.npos && text[n] == '<') {
            // it's a genuine message. Don't parse.
            return text;
        }

        String_t s;
        do {
            String_t now = afl::string::strFirst(text, "\n");
            if (game::v3::Command::isMessageIntroducer(now)) {
                // the rest is a message.
                return s + text;
            }
            if (game::v3::Command* cmd = game::v3::Command::parseCommand(now, false, false)) {
                game::v3::CommandExtra::create(trn).create(player).addNewCommand(cmd);
            } else {
                s += now;
                s += '\n';
            }
        } while (afl::string::strRemove(text, "\n"));

        String_t::size_type x = s.length();
        while (x > 0 && s[x-1] == '\n') {
            --x;
        }
        if (x < s.length()) {
            s.erase(x);
        }
        return s;
    }
}

game::v3::ResultLoader::ResultLoader(afl::base::Ref<afl::io::Directory> specificationDirectory,
                                     afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                     std::auto_ptr<afl::charset::Charset> charset,
                                     afl::string::Translator& tx,
                                     afl::sys::LogListener& log,
                                     const DirectoryScanner& scanner,
                                     afl::io::FileSystem& fs,
                                     util::ProfileDirectory* pProfile)
    : m_specificationDirectory(specificationDirectory),
      m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_charset(charset),
      m_translator(tx),
      m_log(log),
      m_fileSystem(fs),
      m_pProfile(pProfile)
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
            : m_parent(parent), m_turn(turn), m_game(game), m_player(player), m_root(root), m_session(session), m_then(then)
            { }

        virtual void call()
            {
                m_parent.m_log.write(afl::sys::LogListener::Trace, LOG_NAME, "Task: loadCurrentTurn");
                try {
                    m_parent.doLoadCurrentTurn(m_turn, m_game, m_player, m_root, m_session);
                    m_then->call(true);
                }
                catch (std::exception& e) {
                    m_session.log().write(afl::sys::LogListener::Error, LOG_NAME, String_t(), e);
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
    };
    return std::auto_ptr<Task_t>(new Task(*this, turn, game, player, root, session, then));
}

std::auto_ptr<game::Task_t>
game::v3::ResultLoader::saveCurrentTurn(const Turn& turn, const Game& game, int player, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    // ex saveTurns
    // FIXME: saveTurns took a PlayerSet
    try {
        doSaveCurrentTurn(turn, game, player, root, session);
        return makeConfirmationTask(true, then);
    }
    catch (std::exception& e) {
        session.log().write(afl::sys::LogListener::Error, LOG_NAME, session.translator().translateString("Unable to save game"), e);
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
                m_parent.m_log.write(afl::sys::LogListener::Trace, LOG_NAME, "Task: loadHistoryTurn");
                try {
                    m_parent.doLoadHistoryTurn(m_turn, m_game, m_player, m_turnNumber, m_root);
                    m_then->call(true);
                }
                catch (std::exception& e) {
                    m_parent.m_log.write(afl::sys::LogListener::Error, LOG_NAME, String_t(), e);
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
game::v3::ResultLoader::loadTurnfile(Turn& trn, Root& root, afl::io::Stream& file, int player) const
{
    // ex game/load-trn.cc:loadTurn
    m_log.write(m_log.Info, LOG_NAME, Format(m_translator("Loading %s TRN file..."), root.playerList().getPlayerName(player, Player::AdjectiveName)));

    // Load, validate, and log.
    TurnFile f(*m_charset, m_translator, file);
    if (f.getPlayer() != player) {
        throw FileFormatException(file, Format(m_translator("Turn file belongs to player %d"), f.getPlayer()));
    }
    if (f.getFeatures().contains(TurnFile::TaccomFeature)) {
        m_log.write(m_log.Info, LOG_NAME, Format(m_translator("Turn file contains %d attachment%!1{s%}"), f.getNumFiles()));
    }

    // Use TurnProcessor to load the turn file.
    class LocalTurnProcessor : public game::v3::trn::TurnProcessor {
     public:
        LocalTurnProcessor(Turn& turn, Root& root, Stream& file, int player, bool remapExplore, const ResultLoader& parent)
            : m_turn(turn),
              m_root(root),
              m_file(file),
              m_player(player),
              m_remapExplore(remapExplore),
              m_parent(parent)
            { }
        void fail(const char* tpl, int arg)
            { throw FileFormatException(m_file, Format(m_parent.m_translator(tpl), arg)); }
        virtual void handleInvalidCommand(int code)
            { fail(N_("Turn file contains invalid command code %d"), code); }
        virtual void validateShip(int id)
            {
                if (const game::map::Ship* sh = m_turn.universe().ships().get(id)) {
                    if (!sh->getShipSource().contains(m_player)) {
                        fail(N_("Turn file refers to ship %d which is not ours"), id);
                    }
                } else {
                    fail(N_("Turn file refers to non-existant ship %d"), id);
                }
            }
        virtual void validatePlanet(int id)
            {
                if (const game::map::Planet* pl = m_turn.universe().planets().get(id)) {
                    if (!pl->getPlanetSource().contains(m_player)) {
                        fail(N_("Turn file refers to planet %d which is not ours"), id);
                    }
                } else {
                    fail(N_("Turn file refers to non-existant planet %d"), id);
                }
            }
        virtual void validateBase(int id)
            {
                if (const game::map::Planet* pl = m_turn.universe().planets().get(id)) {
                    if (!pl->getBaseSource().contains(m_player)) {
                        fail(N_("Turn file refers to starbase %d which is not ours"), id);
                    }
                } else {
                    fail(N_("Turn file refers to non-existant starbase %d"), id);
                }
            }

        virtual void getShipData(int id, Ship_t& out, afl::charset::Charset& charset)
            {
                if (game::map::Ship* sh = m_turn.universe().ships().get(id)) {
                    game::map::ShipData data;
                    sh->getCurrentShipData(data);
                    Packer(charset).packShip(out, id, data, m_remapExplore);
                }
            }

        virtual void getPlanetData(int id, Planet_t& out, afl::charset::Charset& charset)
            {
                if (game::map::Planet* pl = m_turn.universe().planets().get(id)) {
                    game::map::PlanetData data;
                    pl->getCurrentPlanetData(data);
                    Packer(charset).packPlanet(out, id, data);
                }
            }

        virtual void getBaseData(int id, Base_t& out, afl::charset::Charset& charset)
            {
                if (game::map::Planet* pl = m_turn.universe().planets().get(id)) {
                    game::map::BaseData data;
                    pl->getCurrentBaseData(data);
                    Packer(charset).packBase(out, id, data);
                }
            }

        virtual void storeShipData(int id, const Ship_t& in, afl::charset::Charset& charset)
            {
                if (game::map::Ship* sh = m_turn.universe().ships().get(id)) {
                    game::map::ShipData data;
                    Packer(charset).unpackShip(data, in, m_remapExplore);
                    sh->addCurrentShipData(data, PlayerSet_t(m_player));
                }
            }

        virtual void storePlanetData(int id, const Planet_t& in, afl::charset::Charset& charset)
            {
                if (game::map::Planet* pl = m_turn.universe().planets().get(id)) {
                    game::map::PlanetData data;
                    Packer(charset).unpackPlanet(data, in);
                    pl->addCurrentPlanetData(data, PlayerSet_t(m_player));
                }
            }

        virtual void storeBaseData(int id, const Base_t& in, afl::charset::Charset& charset)
            {
                if (game::map::Planet* pl = m_turn.universe().planets().get(id)) {
                    game::map::BaseData data;
                    Packer(charset).unpackBase(data, in);
                    pl->addCurrentBaseData(data, PlayerSet_t(m_player));
                }
            }

        virtual void addMessage(int to, String_t text)
            {
                if (to > 0 && to <= structures::NUM_OWNERS) {
                    if (to == structures::NUM_OWNERS) {
                        to = 0;
                    }
                    m_parent.addMessage(m_turn, text, m_player, PlayerSet_t(to));
                }
            }

        virtual void addNewPassword(const NewPassword_t& pass)
            {
                // FIXME
                (void) pass;
            }

        virtual void addAllianceCommand(String_t text)
            { CommandExtra::create(m_turn).create(m_player).addCommand(Command::TAlliance, 0, text); }
     private:
        Turn& m_turn;
        Root& m_root;
        Stream& m_file;
        const int m_player;
        bool m_remapExplore;
        const ResultLoader& m_parent;
    };

    const bool remapExplore = !root.hostVersion().isMissionAllowed(1);
    LocalTurnProcessor(trn, root, file, player, remapExplore, *this).handleTurnFile(f, *m_charset);
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
        m_log.write(m_log.Info, LOG_NAME, Format(m_translator("Loading %s RST file..."), root.playerList().getPlayerName(player, Player::AdjectiveName)));
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
    game::db::FleetLoader(*m_charset).load(root.gameDirectory(), turn.universe(), player);

    // Util
    Parser mp(m_translator, m_log, game, player, root, game::actions::mustHaveShipList(session));
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
        Ref<Stream> file = tpl.openFile(m_fileSystem, root.userConfiguration()[UserConfiguration::Backup_Result]());
        m_log.write(m_log.Info, LOG_NAME, Format(m_translator("Loading %s backup file..."), root.playerList().getPlayerName(player, Player::AdjectiveName)));
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
game::v3::ResultLoader::doSaveCurrentTurn(const Turn& turn, const Game& game, int player, const Root& root, Session& session)
{
    const char*const LOCATION = "ResultLoader::saveCurrentTurn";
    if (session.getEditableAreas().contains(Session::CommandArea)) {
        game::v3::trn::FileSet turns(root.gameDirectory(), *m_charset);
        m_log.write(m_log.Info, LOG_NAME, m_translator("Generating turn commands..."));

        // Create turn file
        TurnFile& thisTurn = turns.create(player, turn.getTimestamp(), turn.getTurnNumber());

        // Obtain reverter
        const game::map::Universe& u = turn.universe();
        Reverter* rev = dynamic_cast<Reverter*>(u.getReverter());
        checkAssertion(rev != 0, "Reverter exists", LOCATION);

        // Obtain key
        const RegistrationKey* key = dynamic_cast<const RegistrationKey*>(&root.registrationKey());
        checkAssertion(key != 0, "Key exists", LOCATION);

        thisTurn.setFeatures(TurnFile::FeatureSet_t(TurnFile::WinplanFeature));
        thisTurn.setRegistrationKey(*key, turn.getTurnNumber());

        // Make commands
        const game::map::Ship* pAllianceShip = 0;
        Packer pack(*m_charset);
        const bool remapExplore = !root.hostVersion().isMissionAllowed(1);
        for (int i = 1; i <= structures::NUM_SHIPS; ++i) {
            const game::map::Ship* pShip = u.ships().get(i);
            const game::map::ShipData* pOldShip = rev->getShipData(i);
            if (pShip != 0 && pOldShip != 0 && pShip->getShipSource().contains(player)) {
                if (pAllianceShip == 0) {
                    pAllianceShip = pShip;
                }

                // Get ship data
                game::map::ShipData newShip;
                pShip->getCurrentShipData(newShip);

                // Convert into blobs
                structures::Ship rawOldShip, rawNewShip;
                pack.packShip(rawOldShip, i, *pOldShip, remapExplore);
                pack.packShip(rawNewShip, i,   newShip, remapExplore);

                // Make commands
                thisTurn.makeShipCommands(i, rawOldShip, rawNewShip);
            }
        }
        for (int i = 1; i <= structures::NUM_PLANETS; ++i) {
            const game::map::Planet* pPlanet = u.planets().get(i);
            const game::map::PlanetData* pOldPlanet = rev->getPlanetData(i);
            if (pPlanet != 0 && pOldPlanet != 0 && pPlanet->getPlanetSource().contains(player)) {
                // Get planet data
                game::map::PlanetData newPlanet;
                pPlanet->getCurrentPlanetData(newPlanet);

                // Convert into blobs
                structures::Planet rawOldPlanet, rawNewPlanet;
                pack.packPlanet(rawOldPlanet, i, *pOldPlanet);
                pack.packPlanet(rawNewPlanet, i,   newPlanet);

                // Make commands
                thisTurn.makePlanetCommands(i, rawOldPlanet, rawNewPlanet);
            }
        }
        for (int i = 1; i <= structures::NUM_PLANETS; ++i) {
            const game::map::Planet* pPlanet = u.planets().get(i);
            const game::map::BaseData* pOldBase = rev->getBaseData(i);
            if (pPlanet != 0 && pOldBase != 0 && pPlanet->getBaseSource().contains(player)) {
                // Get starbase data
                game::map::BaseData newBase;
                pPlanet->getCurrentBaseData(newBase);

                // Convert into blobs
                structures::Base rawOldBase, rawNewBase;
                pack.packBase(rawOldBase, i, *pOldBase);
                pack.packBase(rawNewBase, i,   newBase);

                // Make commands
                thisTurn.makeBaseCommands(i, rawOldBase, rawNewBase);
            }
        }

        // Messages
        thisTurn.sendOutbox(turn.outbox(), player, m_translator, root.playerList(), *m_charset);

        // Command messages
        if (const CommandExtra* cx = CommandExtra::get(turn)) {
            if (const CommandContainer* cc = cx->get(player)) {
                String_t accum;
                for (CommandContainer::ConstIterator_t i = cc->begin(); i != cc->end(); ++i) {
                    if (const Command* pc = *i) {
                        if (pc->getCommand() == Command::TAlliance) {
                            if (pAllianceShip == 0) {
                                m_log.write(m_log.Warn, LOG_NAME, Format(m_translator("Player %d has no ship; alliance changes not transmitted"), player));
                            } else {
                                thisTurn.sendTHostAllies(pc->getArg(), pAllianceShip->getId(), pAllianceShip->getFriendlyCode().orElse(""));
                            }
                        } else {
                            const String_t text = pc->getCommandText();
                            if (!text.empty() && text[0] != '$') {
                                if (accum.size() + text.size() > 500) {
                                    thisTurn.sendMessage(player, player, accum, *m_charset);
                                    accum.clear();
                                }
                                accum += text;
                                accum += '\n';
                            }
                        }
                    }
                }
                if (!accum.empty()) {
                    thisTurn.sendMessage(player, player, accum, *m_charset);
                }
            }
        }

        // FIXME: load password
        // char new_password[10];
        // if (gen.getNewPasswordData(new_password)) {
        //     trns[pid-1]->addCommand(tcm_ChangePassword, 0, new_password, sizeof(new_password));
        // }

        // Generate turn
        thisTurn.update();        // FIXME: in FileSet?
        turns.updateTrailers();
        turns.saveAll(m_log, root.playerList(), m_fileSystem, root.userConfiguration(), m_translator);
    }

    if (session.getEditableAreas().contains(Session::LocalDataArea)) {
        // chart.cc
        saveCurrentDatabases(turn, game, player, root, session, *m_charset);

        // Fleets
        game::db::FleetLoader(*m_charset).save(root.gameDirectory(), turn.universe(), player);
    }

    if (m_pProfile != 0) {
        game.expressionLists().saveRecentFiles(*m_pProfile, m_log, m_translator);
    }
}

/** Add message from message file.
    This decides whether the message is a command message or a normal message,
    and places it in the appropriate part of the game turn object (outbox, command list).

    \param trn         Turn
    \param text        Message text (decoded)
    \param sender      Sender of message
    \param receivers   Receivers of message

    \note This only recognizes messages to one receiver as command messages.
    It is possible (but unlikely) that someone sends a message to theirselves and somone else.
    Our Maketurn will make sure that the message comes out as a real text message.
    However, with Winplan's maketurn, the message will be interpreted by PHost.  */
void
game::v3::ResultLoader::addMessage(Turn& trn, String_t text, int sender, PlayerSet_t receiver) const
{
    // ex game/load.cc:addMessageFromFile
    // FIXME: this function may have to be moved (Loader?)
    if (receiver == PlayerSet_t(sender)) {
        // It's a message to us. Is it a command message?
        text = extractCommands(trn, text, sender);
        if (text.empty()) {
            return;
        }
    }
    trn.outbox().addMessageFromFile(sender, text, receiver);
}
