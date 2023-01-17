/**
  *  \file game/v3/directoryloader.cpp
  *  \brief Class game::v3::DirectoryLoader
  */

#include "game/v3/directoryloader.hpp"
#include "afl/checksums/bytesum.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/except/filetooshortexception.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"
#include "game/v3/controlfile.hpp"
#include "game/v3/fizzfile.hpp"
#include "game/v3/genextra.hpp"
#include "game/v3/genfile.hpp"
#include "game/v3/loader.hpp"
#include "game/v3/outboxreader.hpp"
#include "game/v3/packer.hpp"
#include "game/v3/parser.hpp"
#include "game/v3/passwordchecker.hpp"
#include "game/v3/registry.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/writer.hpp"
#include "util/backupfile.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::except::checkAssertion;
using afl::io::Directory;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::string::Format;
using afl::sys::LogListener;
using game::config::UserConfiguration;
using game::v3::CommandContainer;

namespace gt = game::v3::structures;

namespace {
    const char*const LOG_NAME = "game.v3.dirloader";

    class LocalOutboxReader : public game::v3::OutboxReader {
     public:
        LocalOutboxReader(game::msg::Outbox& outbox, int sender)
            : m_outbox(outbox), m_sender(sender)
            { }

        virtual void addMessage(String_t text, game::PlayerSet_t receivers)
            { m_outbox.addMessageFromFile(m_sender, text, receivers); }

     private:
        game::msg::Outbox& m_outbox;
        int m_sender;
    };

    /* Compute checksum over a file. */
    uint32_t computeFileChecksum(Directory& dir, const String_t& fileName)
    {
        // ex computeShipChecksum, computePlanetChecksum, computeBaseChecksum
        // @change Instead of trying to reconstruct the checksum from data in memory
        // (running into all sorts of problems for example with non 1:1 charset mappings),
        // just compute them over the actual files.
        Ref<Stream> file = dir.openFile(fileName, FileSystem::OpenRead);
        uint8_t buffer[4096];
        uint32_t result = 0;
        while (size_t n = file->read(buffer)) {
            result = afl::checksums::ByteSum().add(afl::base::ConstBytes_t(buffer).trim(n), result);
        }
        return result;
    }

    /* Compute checksum over an array of bytes, shortcut. */
    uint32_t computeChecksum(const afl::base::ConstBytes_t bytes)
    {
        // ex control.pas:Checksum (sort-of)
        return afl::checksums::ByteSum().add(bytes, 0);
    }

    /* Send commands.
       This appends commands to be sent through the message file to the specified Outbox object,
       and stores the others into the cmdX.txt file in the given directory. */
    void saveCommands(Directory& dir, const CommandContainer& cc, game::msg::Outbox& out, int player, afl::string::Translator& tx, const game::Timestamp& ts)
    {
        // ex phost.pas:MailPHostCommands, SavePHostCommandsI, SavePHostCommands, FreePHostCommands
        String_t fileAccum;
        String_t messageAccum;
        for (CommandContainer::ConstIterator_t i = cc.begin(); i != cc.end(); ++i) {
            if (game::v3::Command* pCommand = *i) {
                String_t commandText = pCommand->getCommandText();
                if (commandText.empty()) {
                    // ignore
                } else if (commandText[0] != '$') {
                    // Send through message file
                    if (messageAccum.size() + commandText.size() > 500) {
                        if (!messageAccum.empty()) {
                            out.addMessage(player, messageAccum, game::PlayerSet_t(player));
                        }
                        messageAccum.clear();
                    }
                    if (!messageAccum.empty()) {
                        messageAccum += '\n';
                    }
                    messageAccum += commandText;
                } else {
                    // Send through command file
                    fileAccum += commandText;
                    fileAccum += '\n';
                }
            }
        }

        if (!messageAccum.empty()) {
            out.addMessage(player, messageAccum, game::PlayerSet_t(player));
        }

        String_t fileName = Format("cmd%d.txt", player);
        if (!fileAccum.empty()) {
            Ref<Stream> file = dir.openFile(fileName, FileSystem::Create);
            afl::io::TextFile tf(*file);
            tf.writeLine(tx.translateString("# Additional commands"));
            tf.writeLine(Format("$time %s", ts.getTimestampAsString()));
            tf.writeText(fileAccum);
            tf.flush();
        } else {
            dir.eraseNT(fileName);
        }
    }
}

// Constructor.
game::v3::DirectoryLoader::DirectoryLoader(afl::base::Ref<afl::io::Directory> specificationDirectory,
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
      m_pCallback(pCallback),
      m_playerFlags(),
      m_playersWithDosOutbox()
{
    for (int i = 1; i <= DirectoryScanner::NUM_PLAYERS; ++i) {
        m_playerFlags.set(i, scanner.getPlayerFlags(i));
    }
}

game::v3::DirectoryLoader::PlayerStatusSet_t
game::v3::DirectoryLoader::getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const
{
    PlayerStatusSet_t result;
    DirectoryScanner::PlayerFlags_t flags = m_playerFlags.get(player);
    if (flags.contains(DirectoryScanner::HaveUnpacked)) {
        if (flags.contains(DirectoryScanner::HaveNewResult)) {
            extra = tx.translateString("new RST");
        } else {
            extra = tx.translateString("unpacked");
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
game::v3::DirectoryLoader::loadCurrentTurn(Turn& turn, Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    class Task : public Task_t {
     public:
        Task(DirectoryLoader& parent, Turn& turn, Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t>& then)
            : m_parent(parent), m_turn(turn), m_game(game), m_player(player), m_root(root), m_session(session), m_then(then),
              m_checker(turn, parent.m_pCallback, session.log(), session.translator())
            { }

        virtual void call()
            {
                m_session.log().write(LogListener::Trace, LOG_NAME, "Task: loadCurrentTurn");
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
        DirectoryLoader& m_parent;
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
game::v3::DirectoryLoader::saveCurrentTurn(const Turn& turn, const Game& game, PlayerSet_t players, SaveOptions_t /*opts*/, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    // ex saveDirectory
    try {
        doSaveCurrentTurn(turn, game, players, root);
        return makeConfirmationTask(true, then);
    }
    catch (std::exception& e) {
        session.log().write(LogListener::Error, LOG_NAME, session.translator().translateString("Unable to save game"), e);
        return makeConfirmationTask(false, then);
    }
}

void
game::v3::DirectoryLoader::getHistoryStatus(int player, int turn, afl::base::Memory<HistoryStatus> status, const Root& root)
{
    // FIXME: same as ResultLoader?
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
game::v3::DirectoryLoader::loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, std::auto_ptr<StatusTask_t> then)
{
    class Task : public Task_t {
     public:
        Task(DirectoryLoader& parent, Turn& turn, Game& game, int player, int turnNumber, Root& root, std::auto_ptr<StatusTask_t>& then)
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
        DirectoryLoader& m_parent;
        Turn& m_turn;
        Game& m_game;
        int m_player;
        int m_turnNumber;
        Root& m_root;
        std::auto_ptr<StatusTask_t> m_then;
    };
    return std::auto_ptr<Task_t>(new Task(*this, turn, game, player, turnNumber, root, then));
}

std::auto_ptr<game::Task_t>
game::v3::DirectoryLoader::saveConfiguration(const Root& root, std::auto_ptr<Task_t> then)
{
    return defaultSaveConfiguration(root, m_pProfile, m_log, m_translator, then);
}

String_t
game::v3::DirectoryLoader::getProperty(Property p)
{
    switch (p) {
     case LocalFileFormatProperty:
        // igpFileFormatLocal: DOS, Windows
        if (m_playersWithDosOutbox.empty()) {
            return "Windows";
        } else {
            return "DOS";
        }

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
game::v3::DirectoryLoader::doLoadCurrentTurn(Turn& turn, Game& game, int player, Root& root, Session& session)
{
    // ex game/load.h:loadDirectory
    m_log.write(m_log.Info, LOG_NAME, Format(m_translator("Loading %s data...").c_str(), root.playerList().getPlayerName(player, Player::AdjectiveName, m_translator)));

    // gen.dat
    Directory& dir = root.gameDirectory();
    GenFile gen;
    {
        Ref<Stream> file = dir.openFile(Format("gen%d.dat", player), FileSystem::OpenRead);
        gen.loadFromFile(*file);
        if (gen.getPlayerId() != player) {
            throw afl::except::FileProblemException(*file, Format(m_translator("File is owned by player %d, should be %d").c_str(), gen.getPlayerId(), player));
        }
    }
    GenExtra::create(turn).create(player) = gen;

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

    // FIXME->trn.setHaveData(player);
    gen.copyScoresTo(game.scores());
    turn.setTurnNumber(gen.getTurnNumber());
    turn.setTimestamp(gen.getTimestamp());

    // Configure
    const PlayerSet_t source(player);
    const bool remapExplore = !root.hostVersion().isMissionAllowed(1);

    gt::Int16_t rawCount;

    // Ships
    {
        Ref<Stream> s = dir.openFile(Format("ship%d.dat", player), FileSystem::OpenRead);
        s->fullRead(rawCount.m_bytes);
        ldr.loadShips(turn.universe(), *s, rawCount, Loader::LoadCurrent, remapExplore, source);
    }
    {
        Ref<Stream> s = dir.openFile(Format("ship%d.dis", player), FileSystem::OpenRead);
        s->fullRead(rawCount.m_bytes);
        ldr.loadShips(turn.universe(), *s, rawCount, Loader::LoadPrevious, remapExplore, source);
    }

    // Targets
    {
        Ref<Stream> s = dir.openFile(Format("target%d.dat", player), FileSystem::OpenRead);
        s->fullRead(rawCount.m_bytes);
        ldr.loadTargets(turn.universe(), *s, rawCount, Loader::TargetPlaintext, source, gen.getTurnNumber());
    }
    {
        Ptr<Stream> s = dir.openFileNT(Format("target%d.ext", player), FileSystem::OpenRead);
        if (s.get() != 0) {
            s->fullRead(rawCount.m_bytes);
            ldr.loadTargets(turn.universe(), *s, rawCount, Loader::TargetPlaintext, source, gen.getTurnNumber());
        }
    }

    // Planets
    {
        Ref<Stream> s = dir.openFile(Format("pdata%d.dat", player), FileSystem::OpenRead);
        s->fullRead(rawCount.m_bytes);
        ldr.loadPlanets(turn.universe(), *s, rawCount, Loader::LoadCurrent, source);
    }
    {
        Ref<Stream> s = dir.openFile(Format("pdata%d.dis", player), FileSystem::OpenRead);
        s->fullRead(rawCount.m_bytes);
        ldr.loadPlanets(turn.universe(), *s, rawCount, Loader::LoadPrevious, source);
    }

    // Starbases
    {
        Ref<Stream> s = dir.openFile(Format("bdata%d.dat", player), FileSystem::OpenRead);
        s->fullRead(rawCount.m_bytes);
        ldr.loadBases(turn.universe(), *s, rawCount, Loader::LoadCurrent, source);
    }
    {
        Ref<Stream> s = dir.openFile(Format("bdata%d.dis", player), FileSystem::OpenRead);
        s->fullRead(rawCount.m_bytes);
        ldr.loadBases(turn.universe(), *s, rawCount, Loader::LoadPrevious, source);
    }

    // Messages
    {
        Ref<Stream> s = dir.openFile(Format("mdata%d.dat", player), FileSystem::OpenRead);
        ldr.loadInbox(turn.inbox(), *s, gen.getTurnNumber());
    }

    // ShipXY
    {
        Ref<Stream> s = dir.openFile(Format("shipxy%d.dat", player), FileSystem::OpenRead);
        ldr.loadShipXY(turn.universe(), *s, s->getSize(), Loader::LoadBoth, source, PlayerSet_t());
    }

    // VCRs
    {
        Ref<Stream> s = dir.openFile(Format("vcr%d.dat", player), FileSystem::OpenRead);
        ldr.loadBattles(turn, *s, root.hostConfiguration());
    }

    // Outbox
    {
        afl::base::Ptr<Stream> s = dir.openFileNT(Format("mess35%d.dat", player), FileSystem::OpenRead);
        if (s.get() != 0) {
            LocalOutboxReader(turn.outbox(), player).loadOutbox35(*s, *m_charset, m_translator);
            m_playersWithDosOutbox -= player;
        } else {
            s = dir.openFileNT(Format("mess%d.dat", player), FileSystem::OpenRead);
            if (s.get() != 0) {
                LocalOutboxReader(turn.outbox(), player).loadOutbox(*s, *m_charset, m_translator);
                m_playersWithDosOutbox += player;
            }
        }
    }

    // Commands
    // ex phost.pas:LoadPHostCommands
    {
        afl::base::Ptr<Stream> s = dir.openFileNT(Format("cmd%d.txt", player), FileSystem::OpenRead);
        if (s.get() != 0) {
            CommandExtra::create(turn).create(player).loadCommandFile(*s, gen.getTimestamp());
        }
    }

    // Kore
    {
        afl::base::Ptr<Stream> s = dir.openFileNT(Format("kore%d.dat", player), FileSystem::OpenRead);
        if (s.get() != 0) {
            loadKore(*s, turn, player);
        }
    }

    // Skore
    {
        afl::base::Ptr<Stream> s = dir.openFileNT(Format("skore%d.dat", player), FileSystem::OpenRead);
        if (s.get() != 0) {
            loadSkore(*s, turn);
        }
    }

    // FLAK
    ldr.loadFlakBattles(turn, dir, player);

    // Util
    Parser mp(m_translator, m_log, game, player, root, game::actions::mustHaveShipList(session), session.world().atomTable());
    {
        afl::base::Ptr<Stream> s = dir.openFileNT(Format("util%d.dat", player), FileSystem::OpenRead);
        if (s.get() != 0) {
            mp.loadUtilData(*s, *m_charset);
        } else {
            mp.handleNoUtilData();
        }
    }

    // Message parser
    {
        afl::base::Ptr<Stream> file = m_specificationDirectory->openFileNT("msgparse.ini", afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            mp.parseMessages(*file, turn.inbox(), *m_charset);
        }
    }
}

void
game::v3::DirectoryLoader::doLoadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root)
{
    // FIXME: same as ResultLoader?
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

    // if (have_trn) {
    //     Ptr<Stream> trnfile = game_file_dir->openFile(Format("player%d.trn", player), Stream::C_READ);
    //     loadTurn(trn, *trnfile, player);
    // }

    // FIXME: history fleets not loaded here
    // FIXME: alliances not loaded until here; would need message/util.dat parsing
    // FIXME: load FLAK
}

void
game::v3::DirectoryLoader::doSaveCurrentTurn(const Turn& turn, const Game& game, PlayerSet_t players, const Root& root)
{
    Directory& dir = root.gameDirectory();

    FizzFile fizz;
    fizz.load(dir);
    if (!fizz.isValid()) {
        m_log.write(m_log.Warn, LOG_NAME, m_translator("File \"fizz.bin\" not found. Game data may not be usable with other programs."));
    }

    for (int player = 1; player <= MAX_PLAYERS; ++player) {
        if (players.contains(player)) {
            m_log.write(m_log.Info, LOG_NAME, Format(m_translator("Writing %s data..."), root.playerList().getPlayerName(player, Player::AdjectiveName, m_translator)));
            ControlFile control;
            control.load(dir, player, m_translator, m_log);

            // Load GenFile
            GenFile gen;
            if (const GenFile* p = GenExtra::get(turn, player)) {
                gen = *p;
            } else {
                gen.loadFromFile(*dir.openFile(Format("gen%d.dat", player), FileSystem::OpenRead));
            }

            const uint32_t sigCheck = computeChecksum(gen.getSignature2());
            uint32_t shipChecksum = sigCheck, planetChecksum = sigCheck, baseChecksum = sigCheck;

            // Ships
            {
                Ref<Stream> s = dir.openFile(Format("ship%d.dat", player), FileSystem::Create);
                shipChecksum += saveShips(*s, turn.universe(), player, control, !root.hostVersion().isMissionAllowed(1));
                s->fullWrite(gen.getSignature2());
            }

            // Planets
            {
                Ref<Stream> s = dir.openFile(Format("pdata%d.dat", player), FileSystem::Create);
                planetChecksum += savePlanets(*s, turn.universe(), player, control);
                s->fullWrite(gen.getSignature2());
            }

            // Bases
            {
                Ref<Stream> s = dir.openFile(Format("bdata%d.dat", player), FileSystem::Create);
                baseChecksum += saveBases(*s, turn.universe(), player, control);
                s->fullWrite(gen.getSignature2());
            }

            // Messages and commands. We add the commands to the message box, save that, and remove them again.
            // FIXME: can we do without this modifying operation?
            game::msg::Outbox& out = const_cast<game::msg::Outbox&>(turn.outbox());
            size_t marker = out.getNumMessages();
            try {
                // Commands
                if (const CommandContainer* cc = CommandExtra::get(turn, player)) {
                    saveCommands(dir, *cc, out, player, m_translator, turn.getTimestamp());
                }

                // Messages
                if (!m_playersWithDosOutbox.contains(player)) {
                    // Windows
                    Ref<Stream> file = dir.openFile(Format("mess35%d.dat", player), FileSystem::Create);
                    Writer(*m_charset, m_translator, m_log).saveOutbox35(out, player, *file);
                } else {
                    // DOS
                    Ref<Stream> file = dir.openFile(Format("mess%d.dat", player), FileSystem::Create);
                    Writer(*m_charset, m_translator, m_log).saveOutbox(out, player, root.playerList(), *file);
                }
                out.deleteMessagesAfter(marker);
            }
            catch (...) {
                out.deleteMessagesAfter(marker);
                throw;
            }

            // Add DIS checksums
            shipChecksum   += computeFileChecksum(dir, Format("ship%d.dis",  player));
            planetChecksum += computeFileChecksum(dir, Format("pdata%d.dis", player));
            baseChecksum   += computeFileChecksum(dir, Format("bdata%d.dis", player));

            gen.setSectionChecksum(gt::ShipSection,   shipChecksum);
            gen.setSectionChecksum(gt::PlanetSection, planetChecksum);
            gen.setSectionChecksum(gt::BaseSection,   baseChecksum);
            fizz.set(gt::ShipSection,   player, shipChecksum);
            fizz.set(gt::PlanetSection, player, planetChecksum);
            fizz.set(gt::BaseSection,   player, baseChecksum);

            // Save GenFile
            gt::Gen genData;
            gen.getData(genData);
            dir.openFile(Format("gen%d.dat", player), FileSystem::Create)
                ->fullWrite(afl::base::fromObject(genData));

            control.save(dir, m_translator, m_log);
        }
    }

    updateGameRegistry(dir, turn.getTimestamp());
    fizz.save(dir);

    // Recent
    if (m_pProfile != 0) {
        game.expressionLists().saveRecentFiles(*m_pProfile, m_log, m_translator);
    }
}

void
game::v3::DirectoryLoader::loadKore(afl::io::Stream& file, Turn& turn, int player) const
{
    // ex game/load-dir.cc:loadKore, ccmain.pas:LoadObjects (part)
    // Header
    gt::KoreHeader header;
    size_t headerSize = file.read(afl::base::fromObject(header));
    if (headerSize != sizeof(header)) {
        // Some programs generate 0-length koreX.dat files. Ignore those.
        if (headerSize == 0) {
            m_log.write(m_log.Info, LOG_NAME, Format(m_translator.translateString("File \"%s\" is empty and will be ignored.").c_str(), file.getName()));
            return;
        } else {
            throw afl::except::FileTooShortException(file);
        }
    }

    if (header.turnNumber != turn.getTurnNumber()) {
        m_log.write(m_log.Warn, LOG_NAME, Format(m_translator.translateString("File \"%s\" is stale and will be ignored.").c_str(), file.getName()));
        return;
    }

    Loader ldr(*m_charset, m_translator, m_log);

    // Minefields
    ldr.loadKoreMinefields(turn.universe(), file, 500, player, turn.getTurnNumber());

    // Ion storms
    ldr.loadKoreIonStorms(turn.universe(), file, 50);

    // Explosions
    ldr.loadKoreExplosions(turn.universe(), file, 50);

    // Ufos
    ldr.loadUfos(turn.universe(), file, 1, 100);

    // Visual contacts
    gt::Int32_t buffer[6];
    static_assert(sizeof(buffer) == 24, "sizeof buffer");
    if (file.read(afl::base::fromObject(buffer)) == sizeof(buffer) && std::memcmp(buffer, "1120", 4) == 0) {
        int32_t count = buffer[5];
        if (count < 0 || count > gt::NUM_SHIPS) {
            throw afl::except::FileFormatException(file, m_translator.translateString("Unbelievable number of visual contacts"));
        }
        ldr.loadTargets(turn.universe(), file, count, Loader::TargetEncrypted, PlayerSet_t(player), turn.getTurnNumber());
    }
}

void
game::v3::DirectoryLoader::loadSkore(afl::io::Stream& file, Turn& turn) const
{
    // ex game/load-dir.cc:loadSkore, ccmain.pas:LoadObjects (part)

    // Read header. It is not fatal if we cannot read it; some programs (Winplan?) generate blank SKORE.DAT files.
    gt::SkoreHeader header;
    if (file.read(afl::base::fromObject(header)) != sizeof(header)) {
        return;
    }

    // Do we have extended Ufos?
    if (std::memcmp(header.signature, "yAmsz", 5) == 0 && header.resultVersion > 0 && header.numUfos > 100) {
        Loader(*m_charset, m_translator, m_log).loadUfos(turn.universe(), file, 101, header.numUfos - 100);
    }
}

uint32_t
game::v3::DirectoryLoader::saveShips(afl::io::Stream& file, const game::map::Universe& univ, int player, ControlFile& control, bool remapExplore) const
{
    // ex game/save.cc:saveShips

    // Pass 1: count
    uint16_t count = 0;
    for (Id_t i = 1, n = univ.ships().size(); i <= n; ++i) {
        if (const game::map::Ship* pShip = univ.ships().get(i)) {
            if (pShip->getShipSource().contains(player)) {
                ++count;
            }
        }
    }

    gt::Int16_t rawCount;
    rawCount = count;
    file.fullWrite(rawCount.m_bytes);
    uint32_t checksum = computeChecksum(rawCount.m_bytes);

    // Pass 2: data
    for (Id_t i = 1, n = univ.ships().size(); i <= n; ++i) {
        if (const game::map::Ship* pShip = univ.ships().get(i)) {
            if (pShip->getShipSource().contains(player)) {
                // Get ship data
                game::map::ShipData shipData;
                pShip->getCurrentShipData(shipData);

                // Serialize it
                gt::Ship rawShip;
                Packer(*m_charset).packShip(rawShip, i, shipData, remapExplore);
                file.fullWrite(afl::base::fromObject(rawShip));

                // Checksums
                const uint32_t thisChecksum = computeChecksum(afl::base::fromObject(rawShip));
                checksum += thisChecksum;
                control.set(gt::ShipSection, i, thisChecksum);
            }
        }
    }
    return checksum;
}

uint32_t
game::v3::DirectoryLoader::savePlanets(afl::io::Stream& file, const game::map::Universe& univ, int player, ControlFile& control) const
{
    // ex game/save.cc:savePlanets

    // Pass 1: count
    uint16_t count = 0;
    for (Id_t i = 1, n = univ.planets().size(); i <= n; ++i) {
        if (const game::map::Planet* pPlanet = univ.planets().get(i)) {
            if (pPlanet->getPlanetSource().contains(player)) {
                ++count;
            }
        }
    }

    gt::Int16_t rawCount;
    rawCount = count;
    file.fullWrite(rawCount.m_bytes);
    uint32_t checksum = computeChecksum(rawCount.m_bytes);

    // Pass 2: data
    for (Id_t i = 1, n = univ.planets().size(); i <= n; ++i) {
        if (const game::map::Planet* pPlanet = univ.planets().get(i)) {
            if (pPlanet->getPlanetSource().contains(player)) {
                // Get planet data
                game::map::PlanetData planetData;
                pPlanet->getCurrentPlanetData(planetData);

                // Serialize it
                gt::Planet rawPlanet;
                Packer(*m_charset).packPlanet(rawPlanet, i, planetData);
                file.fullWrite(afl::base::fromObject(rawPlanet));

                // Checksums
                const uint32_t thisChecksum = computeChecksum(afl::base::fromObject(rawPlanet));
                checksum += thisChecksum;
                control.set(gt::PlanetSection, i, thisChecksum);
            }
        }
    }
    return checksum;
}

uint32_t
game::v3::DirectoryLoader::saveBases(afl::io::Stream& file, const game::map::Universe& univ, int player, ControlFile& control) const
{
    // ex game/save.cc:saveBases

    // Pass 1: count
    uint16_t count = 0;
    for (Id_t i = 1, n = univ.planets().size(); i <= n; ++i) {
        if (const game::map::Planet* pPlanet = univ.planets().get(i)) {
            if (pPlanet->getBaseSource().contains(player)) {
                ++count;
            }
        }
    }

    gt::Int16_t rawCount;
    rawCount = count;
    file.fullWrite(rawCount.m_bytes);
    uint32_t checksum = computeChecksum(rawCount.m_bytes);

    // Pass 2: data
    for (Id_t i = 1, n = univ.planets().size(); i <= n; ++i) {
        if (const game::map::Planet* pPlanet = univ.planets().get(i)) {
            if (pPlanet->getBaseSource().contains(player)) {
                // Get base data
                game::map::BaseData baseData;
                pPlanet->getCurrentBaseData(baseData);

                // Owner
                const int baseOwner = pPlanet->getOwner().orElse(player);

                // Serialize it
                gt::Base rawBase;
                Packer(*m_charset).packBase(rawBase, i, baseData, baseOwner);
                file.fullWrite(afl::base::fromObject(rawBase));

                // Checksums
                const uint32_t thisChecksum = computeChecksum(afl::base::fromObject(rawBase));
                checksum += thisChecksum;
                control.set(gt::BaseSection, i, thisChecksum);
            }
        }
    }
    return checksum;
}
