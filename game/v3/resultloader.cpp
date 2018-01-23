/**
  *  \file game/v3/resultloader.cpp
  */

#include "game/v3/resultloader.hpp"
#include "afl/base/ptr.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/format.hpp"
#include "game/config/stringoption.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/player.hpp"
#include "game/root.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "game/v3/commandextra.hpp"
#include "game/v3/loader.hpp"
#include "game/v3/packer.hpp"
#include "game/v3/resultfile.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/turnfile.hpp"
#include "util/backupfile.hpp"
#include "game/v3/inboxfile.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/reverter.hpp"

using afl::string::Format;
using afl::except::FileFormatException;

namespace {
    const char LOG_NAME[] = "game.v3.resultloader";

    // FIXME: where to define this? ex opt_BackupResult
    const game::config::StringOptionDescriptor opt_BackupResult = { "Backup.Result" };

    struct KoreTargetHeader {
        char sig[4];
        game::v3::structures::UInt32_t num;
    };

    void seekToSection(afl::io::Stream& file,
                       game::v3::ResultFile& result,
                       game::v3::ResultFile::Section section,
                       afl::string::Translator& tx)
    {
        afl::io::Stream::FileSize_t pos = 0;
        if (!result.getSectionOffset(section, pos)) {
            // With a normal ResultFile, this cannot happen
            throw FileFormatException(file, tx.translateString("File is missing required section"));
        }
        file.setPos(pos);
    }

    afl::io::Stream::FileSize_t getShipXYSize(const game::v3::ResultFile& result)
    {
        afl::io::Stream::FileSize_t start, end;
        if (result.getSectionOffset(result.ShipXYSection, start) && result.getSectionOffset(result.GenSection, end) && end > start) {
            return end - start;
        } else {
            return 0;
        }
    }

    void mergeScores(game::score::TurnScoreList& scores, const game::v3::structures::ResultGen& gen)
    {
        // ex GStatFile::mergeGen
        game::score::TurnScore& r = scores.addTurn(gen.turnNumber, gen.timestamp);

        const game::score::TurnScore::Slot_t pi = scores.addSlot(game::score::ScoreId_Planets);
        const game::score::TurnScore::Slot_t ci = scores.addSlot(game::score::ScoreId_Capital);
        const game::score::TurnScore::Slot_t fi = scores.addSlot(game::score::ScoreId_Freighters);
        const game::score::TurnScore::Slot_t bi = scores.addSlot(game::score::ScoreId_Bases);

        for (int i = 1; i <= game::v3::structures::NUM_PLAYERS; ++i) {
            // FIXME: implement some sensible merging here.
            // - when we know a score blanker is in use, treat 0 as unknown
            // - do not overwrite a nonzero value with 0 [but what about rehosts?]
            const game::v3::structures::GenScore& gs = gen.scores[i-1];
            r.set(pi, i, int(gs.numPlanets));
            r.set(ci, i, int(gs.numCapitalShips));
            r.set(fi, i, int(gs.numFreighters));
            r.set(bi, i, int(gs.numBases));
        }
    }

    // /** Extract commands from a message. This figures out the PHost
    //     commands from a message a player sent to himself.
    //     \param trn     Game turn object
    //     \param text    Decoded message text
    //     \param player  Player number
    //     \returns Message text without commands. Might be empty if the
    //     message consists entirely of commands. */
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
            if (game::v3::Command* cmd = game::v3::Command::parseCommand(now, false)) {
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

game::v3::ResultLoader::ResultLoader(afl::charset::Charset& charset,
                                     afl::string::Translator& tx,
                                     afl::sys::LogListener& log,
                                     const DirectoryScanner& scanner,
                                     afl::io::FileSystem& fs)
    : m_charset(charset),  // FIXME: lifetime!
      m_translator(tx),
      m_log(log),
      m_fileSystem(fs)
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
            extra = tx.translateString("RST + TRN");
        } else {
            extra = tx.translateString("RST");
        }
        result += Available;
        result += Playable;
        result += Primary;
    } else {
        extra.clear();
    }
    return result;
}

void
game::v3::ResultLoader::loadCurrentTurn(Turn& turn, Game& game, int player, game::Root& root, Session& session)
{
    // ex game/load.h:loadCommon
    // Initialize planets and bases
    Loader ldr(m_charset, m_translator, m_log);
    ldr.prepareUniverse(turn.universe());

    // Reverter
    turn.universe().setNewReverter(new Reverter(turn.universe(), session));

    // Create CommandExtra. This allows further code to deal with PHost commands.
    CommandExtra::create(turn);

    // xyplan.dat
    afl::base::Ptr<afl::io::Stream> file = root.gameDirectory().openFileNT(Format("xyplan%d.dat", player), afl::io::FileSystem::OpenRead);
    if (file.get() == 0) {
        file = root.specificationDirectory().openFile("xyplan.dat", afl::io::FileSystem::OpenRead).asPtr();
    }
    ldr.loadPlanetCoordinates(turn.universe(), *file);
    file = 0;

    // planet.nm
    file = root.specificationDirectory().openFile("planet.nm", afl::io::FileSystem::OpenRead).asPtr();
    ldr.loadPlanetNames(turn.universe(), *file);
    file = 0;

    // storm.nm
    file = root.specificationDirectory().openFile("storm.nm", afl::io::FileSystem::OpenRead).asPtr();
    ldr.loadIonStormNames(turn.universe(), *file);

    // load database
    loadCurrentDatabases(turn, game, player, root, session, m_charset);

    // ex GGameResultStorage::load(GGameTurn& trn)
    file = root.gameDirectory().openFile(Format("player%d.rst", player), afl::io::FileSystem::OpenRead).asPtr();
    if (const game::Player* p = root.playerList().get(player)) {
        m_log.write(m_log.Info, LOG_NAME, Format(m_translator.translateString("Loading %s RST file...").c_str(), p->getName(Player::AdjectiveName)));
    }
    loadResult(turn, root, game, *file, player, true);
    file = 0;

    if (m_playerFlags.get(player).contains(DirectoryScanner::HaveTurn)) {
        file = root.gameDirectory().openFile(Format("player%d.trn", player), afl::io::FileSystem::OpenRead).asPtr();
        loadTurnfile(turn, root, *file, player);
        file = 0;
    }

    // loadFleets(trn, *game_file_dir, player);

    // /* Util */
    // /* FIXME: it is ugly to do this here; loadDirectory does it itself */
    // if (Ptr<Stream> s = game_file_dir->openFileNT(format("util%d.dat", player), Stream::C_READ))
    //     (GUtilParser(trn, player)).read(*s);

    // /* FLAK */
    // maybeLoadFlakVcrs(trn, *game_file_dir, player);
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
        if (tpl.hasFile(m_fileSystem, root.userConfiguration()[opt_BackupResult]())) {
            *p = StronglyPositive;
        } else {
            *p = Negative;
        }

        ++turn;
    }
}

void
game::v3::ResultLoader::loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root)
{
    // Initialize planets and bases
    Loader ldr(m_charset, m_translator, m_log);
    ldr.prepareUniverse(turn.universe());

    // xyplan.dat
    // FIXME: backup this?
    afl::base::Ptr<afl::io::Stream> file = root.gameDirectory().openFileNT(Format("xyplan%d.dat", player), afl::io::FileSystem::OpenRead);
    if (file.get() == 0) {
        file = root.specificationDirectory().openFile("xyplan.dat", afl::io::FileSystem::OpenRead).asPtr();
    }
    ldr.loadPlanetCoordinates(turn.universe(), *file);
    file = 0;

    // planet.nm
    file = root.specificationDirectory().openFile("planet.nm", afl::io::FileSystem::OpenRead).asPtr();
    ldr.loadPlanetNames(turn.universe(), *file);
    file = 0;

    // storm.nm
    file = root.specificationDirectory().openFile("storm.nm", afl::io::FileSystem::OpenRead).asPtr();
    ldr.loadIonStormNames(turn.universe(), *file);

    // load turn file backup
    util::BackupFile tpl;
    tpl.setGameDirectoryName(root.gameDirectory().getDirectoryName());
    tpl.setPlayerNumber(player);
    tpl.setTurnNumber(turnNumber);

    file = tpl.openFile(m_fileSystem, root.userConfiguration()[opt_BackupResult]()).asPtr();
    if (const game::Player* p = root.playerList().get(player)) {
        m_log.write(m_log.Info, LOG_NAME, Format(m_translator.translateString("Loading %s backup file...").c_str(), p->getName(Player::AdjectiveName)));
    }
    loadResult(turn, root, game, *file, player, false);
    file = 0;
    // if (have_trn) {
    //     Ptr<Stream> trnfile = game_file_dir->openFile(format("player%d.trn", player), Stream::C_READ);
    //     loadTurn(trn, *trnfile, player);
    // }

    // // // // // loadFleets(trn, *game_file_dir, player);

    // // // // // /* Util */
    // // // // // /* FIXME: it is ugly to do this here; loadDirectory does it itself */
    // // // // // if (Ptr<Stream> s = game_file_dir->openFileNT(format("util%d.dat", player), Stream::C_READ))
    // // // // //     (GUtilParser(trn, player)).read(*s);

    // // // // // /* FLAK */
    // // // // // maybeLoadFlakVcrs(trn, *game_file_dir, player);
}


// /** Load a RST file. */
void
game::v3::ResultLoader::loadResult(Turn& turn, Root& root, Game& game, afl::io::Stream& file, int player, bool withBackup)
{
    // ex game/load-rst.cc:loadResult
    structures::Int16_t n;

    ResultFile result(file, m_translator);
    PlayerSet_t source(player);
    Loader loader(m_charset, m_translator, m_log);

    // Gen
    structures::ResultGen gen;
    seekToSection(file, result, ResultFile::GenSection, m_translator);
    file.fullRead(afl::base::fromObject(gen));
    if (gen.playerId != player) {
        throw FileFormatException(file, Format(m_translator.translate("File is owned by player %d, should be %d").c_str(), int(gen.playerId), player));
    }
//     trn.setHaveData(player);
//     trn.setGen(player, GGen(gen));
    mergeScores(game.scores(), gen);
    turn.setTurnNumber(gen.turnNumber);
    turn.setTimestamp(gen.timestamp);

    // Backup
    if (withBackup) {
        // FIXME: this can be moved into the caller
        try {
            file.setPos(0);
            util::BackupFile tpl;
            tpl.setPlayerNumber(player);
            tpl.setTurnNumber(gen.turnNumber);
            tpl.setGameDirectoryName(root.gameDirectory().getDirectoryName());
            tpl.copyFile(m_fileSystem, root.userConfiguration()[opt_BackupResult](), file);
        }
        catch (std::exception& e) {
            m_log.write(m_log.Warn, LOG_NAME, m_translator.translate("Unable to create backup file"), e);
        }
    }

    // Ships
    seekToSection(file, result, ResultFile::ShipSection, m_translator);
    file.fullRead(afl::base::fromObject(n));
    loader.loadShips(turn.universe(), file, n, Loader::LoadBoth, root.hostVersion().getKind() == HostVersion::SRace, source);

    // Targets
    seekToSection(file, result, ResultFile::TargetSection, m_translator);
    file.fullRead(afl::base::fromObject(n));
    loader.loadTargets(turn.universe(), file, n, loader.TargetPlaintext, source, turn.getTurnNumber());

    // Planets
    seekToSection(file, result, ResultFile::PlanetSection, m_translator);
    file.fullRead(afl::base::fromObject(n));
    loader.loadPlanets(turn.universe(), file, n, Loader::LoadBoth, source);

    // Starbases
    seekToSection(file, result, ResultFile::BaseSection, m_translator);
    file.fullRead(afl::base::fromObject(n));
    loader.loadBases(turn.universe(), file, n, Loader::LoadBoth, source);

    // Messages
    seekToSection(file, result, ResultFile::MessageSection, m_translator);
    loader.loadInbox(turn.inbox(), file, gen.turnNumber);

    // SHIPXY (must be after SHIP) <-- FIXME: why this comment (from PCC2)?
    seekToSection(file, result, ResultFile::ShipXYSection, m_translator);
    loader.loadShipXY(turn.universe(), file, getShipXYSize(result), Loader::LoadBoth, source, PlayerSet_t());

    // VCRs
    seekToSection(file, result, ResultFile::VcrSection, m_translator);
    loader.loadBattles(turn, file, root.hostConfiguration());

    // Windows part of RST
    afl::io::Stream::FileSize_t pos;
    if (result.getSectionOffset(ResultFile::KoreSection, pos)) {
        // KORE
        file.setPos(pos);
        loader.loadKoreMinefields(turn.universe(), file, 500, player, turn.getTurnNumber());
        loader.loadKoreIonStorms(turn.universe(), file, 50);
        loader.loadKoreExplosions(turn.universe(), file, 50);
//         player_racenames.load(s); /* FIXME: configurable? */
//         host_racenames = player_racenames;
        file.setPos(pos + 500*8+600+50*4+682);
        loader.loadUfos(turn.universe(), file, 1, 100);

        file.setPos(pos + 500*8+600+50*4+682+7800);
        KoreTargetHeader kth;
        if (file.read(afl::base::fromObject(kth)) == sizeof(kth) && std::memcmp(kth.sig, "1120", 4) == 0) {
            uint32_t n = kth.num;
            if (n > uint32_t(game::v3::structures::NUM_SHIPS)) {
                throw FileFormatException(file, m_translator.translate("Unbelievable number of visual contacts"));
            }
            loader.loadTargets(turn.universe(), file, n, loader.TargetEncrypted, source, turn.getTurnNumber());
        }
    }

    if (result.getSectionOffset(ResultFile::SkoreSection, pos)) {
        // SKORE
        file.setPos(pos);
        if (file.read(afl::base::fromObject(n)) == sizeof(n) && n > 100) {
            loader.loadUfos(turn.universe(), file, 101, n - 100);
        }
    }
}

void
game::v3::ResultLoader::loadTurnfile(Turn& trn, Root& root, afl::io::Stream& file, int player)
{
    // ex game/load-trn.cc:loadTurn
    if (const game::Player* p = root.playerList().get(player)) {
        m_log.write(m_log.Info, LOG_NAME, Format(m_translator.translateString("Loading %s TRN file...").c_str(), p->getName(Player::AdjectiveName)));
    }

    TurnFile f(m_charset, file);
    if (f.getFeatures().contains(TurnFile::TaccomFeature)) {
        m_log.write(m_log.Info, LOG_NAME, Format(m_translator.translateString("Turn file contains %d attachment%!1{s%}").c_str(), f.getNumFiles()));
    }
    f.sortCommands();

    // Pass 1: verify commands
    for (size_t i = 0, n = f.getNumCommands(); i < n; ++i) {
        int cmdId;
        TurnFile::CommandCode_t cmdCode;
        TurnFile::CommandType cmdType;
        if (f.getCommandId(i, cmdId) && f.getCommandCode(i, cmdCode) && f.getCommandType(i, cmdType)) {
            switch (cmdType) {
             case TurnFile::UndefinedCommand:
                throw FileFormatException(file, Format(m_translator.translateString("Turn file contains invalid command code %d").c_str(), cmdCode));

             case TurnFile::ShipCommand:
                if (const game::map::Ship* sh = trn.universe().ships().get(cmdId)) {
                    if (!sh->getShipSource().contains(player)) {
                        throw FileFormatException(file, Format(m_translator.translateString("Turn file refers to ship %d which is not ours").c_str(), cmdId));
                    }
                } else {
                    throw FileFormatException(file, Format(m_translator.translateString("Turn file refers to non-existant ship %d").c_str(), cmdId));
                }
                break;

             case TurnFile::PlanetCommand:
                if (const game::map::Planet* pl = trn.universe().planets().get(cmdId)) {
                    if (!pl->getPlanetSource().contains(player)) {
                        throw FileFormatException(file, Format(m_translator.translateString("Turn file refers to planet %d which is not ours").c_str(), cmdId));
                    }
                } else {
                    throw FileFormatException(file, Format(m_translator.translateString("Turn file refers to non-existant planet %d").c_str(), cmdId));
                }
                break;

             case TurnFile::BaseCommand:
                if (const game::map::Planet* pl = trn.universe().planets().get(cmdId)) {
                    if (!pl->getBaseSource().contains(player)) {
                        throw FileFormatException(file, Format(m_translator.translateString("Turn file refers to starbase %d which is not ours").c_str(), cmdId));
                    }
                } else {
                    throw FileFormatException(file, Format(m_translator.translateString("Turn file refers to non-existant starbase %d").c_str(), cmdId));
                }
                break;

             case TurnFile::OtherCommand:
                break;
            }
        }
    }

    // Pass 2: process commands.
    String_t timAllies;
    const bool remapExplore = !root.hostVersion().isMissionAllowed(1);
    m_log.write(m_log.Debug, LOG_NAME, Format(m_translator.translateString("Loading %d command%!1{s%}...").c_str(), f.getNumCommands()));
    size_t i = 0;
    while (i < f.getNumCommands()) {
        // Get this command's Id and class
        // (These calls will not fail; we have verified above.)
        int cmdId = 0;
        TurnFile::CommandType cmdType = TurnFile::UndefinedCommand;
        f.getCommandId(i, cmdId);
        f.getCommandType(i, cmdType);

        switch (cmdType) {
         case TurnFile::ShipCommand:
            if (game::map::Ship* sh = trn.universe().ships().get(cmdId)) {
                size_t n = f.findCommandRunLength(i);

                game::map::ShipData data;
                sh->getCurrentShipData(data);

                structures::Ship rawData;
                Packer(m_charset).packShip(rawData, cmdId, data, remapExplore);

                while (n > 0) {
                    TurnFile::CommandCode_t cmdCode;
                    int cmdLength;
                    if (f.getCommandCode(i, cmdCode) && f.getCommandLength(i, cmdLength)) {
                        size_t offset = TurnFile::getCommandCodeRecordIndex(cmdCode);
                        afl::base::ConstBytes_t cmdData = f.getCommandData(i);
                        afl::base::fromObject(rawData).subrange(offset, cmdLength).copyFrom(cmdData);

                        TurnFile::CommandCode_t nextCode;
                        if (cmdCode == tcm_ShipChangeFc && n >= 2 && f.getCommandCode(i+1, nextCode) && nextCode == tcm_ShipChangeFc && cmdData.size() >= 3) {
                            // might be THost alliance command
                            char out[3];
                            afl::base::fromObject(out).copyFrom(cmdData);
                            if ((out[0] == 'f' && out[1] == 'f') || (out[0] == 'F' && out[1] == 'F') || (out[0] == 'e' && out[1] == 'e')) {
                                timAllies.append(out, sizeof(out));
                            }
                        }
                    }
                    ++i, --n;
                }

                Packer(m_charset).unpackShip(data, rawData, remapExplore);
                sh->addCurrentShipData(data, PlayerSet_t(player));
            } else {
                // cannot happen
                ++i;
            }
            break;

         case TurnFile::PlanetCommand:
            if (game::map::Planet* pl = trn.universe().planets().get(cmdId)) {
                size_t n = f.findCommandRunLength(i);

                game::map::PlanetData data;
                pl->getCurrentPlanetData(data);

                structures::Planet rawData;
                Packer(m_charset).packPlanet(rawData, cmdId, data);

                while (n > 0) {
                    TurnFile::CommandCode_t cmdCode;
                    int cmdLength;
                    if (f.getCommandCode(i, cmdCode) && f.getCommandLength(i, cmdLength)) {
                        if (cmdCode == tcm_PlanetBuildBase) {
                            rawData.buildBaseFlag = 1;
                        } else {
                            size_t offset = TurnFile::getCommandCodeRecordIndex(cmdCode);
                            afl::base::ConstBytes_t cmdData = f.getCommandData(i);
                            afl::base::fromObject(rawData).subrange(offset, cmdLength).copyFrom(cmdData);
                        }
                    }
                    ++i, --n;
                }

                Packer(m_charset).unpackPlanet(data, rawData);
                pl->addCurrentPlanetData(data, PlayerSet_t(player));
            } else {
                // cannot happen
                ++i;
            }
            break;

         case TurnFile::BaseCommand:
            if (game::map::Planet* pl = trn.universe().planets().get(cmdId)) {
                size_t n = f.findCommandRunLength(i);

                game::map::BaseData data;
                pl->getCurrentBaseData(data);

                structures::Base rawData;
                Packer(m_charset).packBase(rawData, cmdId, data);

                while (n > 0) {
                    TurnFile::CommandCode_t cmdCode;
                    int cmdLength;
                    if (f.getCommandCode(i, cmdCode) && f.getCommandLength(i, cmdLength)) {
                        size_t offset = TurnFile::getCommandCodeRecordIndex(cmdCode);
                        afl::base::ConstBytes_t cmdData = f.getCommandData(i);
                        afl::base::fromObject(rawData).subrange(offset, cmdLength).copyFrom(cmdData);
                    }
                    ++i, --n;
                }

                Packer(m_charset).unpackBase(data, rawData);
                pl->addCurrentBaseData(data, PlayerSet_t(player));
            } else {
                // cannot happen
                ++i;
            }
            break;

         case TurnFile::OtherCommand: {
            TurnFile::CommandCode_t cmdCode = 0;
            f.getCommandCode(i, cmdCode);
            if (cmdCode == tcm_SendMessage) {
                afl::base::ConstBytes_t cmdData = f.getCommandData(i);
                /* format of message is:
                      id     = length
                      data+0 = sender
                      data+2 = receiver
                      data+4 = text */
                structures::Int16_t to;
                to = 0;
                afl::base::fromObject(to).copyFrom(cmdData.subrange(2, 2));
                int size = cmdId;
                if (to > 0 && to <= structures::NUM_OWNERS) {
                    addMessage(trn, decodeMessage(cmdData.subrange(4, size), m_charset, false), player, PlayerSet_t(to));
                }
            } else if (cmdCode == tcm_ChangePassword) {
                // FIXME -> trn.getGen(player).setPasswordFromTurn(f.getCommandData(i));
            } else {
                // unknown
            }
            ++i;
            break;
         }

         case TurnFile::UndefinedCommand:
            // Cannot happen, but avoid lossage if it does anyway
            ++i;
            break;
        }
    }

    if (!timAllies.empty()) {
        CommandExtra::create(trn).create(player).addCommand(Command::phc_TAlliance, 0, m_charset.decode(afl::string::toBytes(timAllies)));
    }
}

// /** Add message from message file. This decides whether the message is
//     a command message or a normal message, and places it in the appropriate
//     part of the game turn object (outbox, command list).
//     \param trn    Game turn object
//     \param text   Message text
//     \param sender Sender of message
//     \param recv   Receivers of message
//     \note This only recognizes messages to one receiver as command
//     messages. It is possible (but unlikely) that someone sends a
//     message to theirselves and somone else. Our Maketurn will make
//     sure that the message comes out as a real text message. However,
//     with Winplan's maketurn, the message will be interpreted by
//     PHost.  */
void
game::v3::ResultLoader::addMessage(Turn& trn, String_t text, int sender, PlayerSet_t receiver)
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
