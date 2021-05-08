/**
  *  \file game/v3/directoryscanner.cpp
  *  \brief Class game::v3::DirectoryScanner
  */

#include <cassert>
#include "game/v3/directoryscanner.hpp"
#include "afl/checksums/bytesum.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/format.hpp"
#include "afl/string/translator.hpp"
#include "game/hostversion.hpp"
#include "game/timestamp.hpp"
#include "game/v3/resultfile.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/turnfile.hpp"
#include "game/v3/inboxfile.hpp"
#include "game/parser/messagevalue.hpp"
#include "game/parser/datainterface.hpp"
#include "game/parser/messageinformation.hpp"

namespace gt = game::v3::structures;

namespace {
    const char LOG_NAME[] = "game.v3.scan";

    bool parseNumber(const String_t& text, String_t::size_type& pos, int& val)
    {
        val = 0;
        bool ok = false;
        while (pos < text.size() && text[pos] >= '0' && text[pos] <= '9') {
            val = 10*val + (text[pos] - '0');
            ok = true;
            ++pos;
        }
        return ok;
    }

    int32_t parseHostVersion(const String_t& text, bool host)
    {
        // ex game/storage/overview.cc:parseHostVersion, readmsg.pas::ParseHostVersion
        // FIXME: use util::StringParser?
        String_t::size_type pos = 0;
        while (pos < text.size() && (text[pos] == ' ' || text[pos] == 'v')) {
            ++pos;
        }

        // Major number
        int val;
        if (!parseNumber(text, pos, val)) {
            return 0;
        }
        int32_t result = int32_t(100000) * val;

        // Minor number
        if (pos < text.size() && text[pos] == '.') {
            ++pos;
            if (!parseNumber(text, pos, val)) {
                return 0;
            }
            // THost: 3.0, 3.1, 3.14, 3.2, 3.21
            // PHost: 2.7, 2.8, 2.9, 2.10, ...
            if (host && val < 10) {
                val *= 10;
            }
            result += 1000L * val;
        }

        // Patchlevel
        if (pos < text.size()) {
            if (text[pos] == '.') {
                ++pos;
                if (parseNumber(text, pos, val)) {
                    result += val;
                }
            } else if (text[pos] >= 'a' && text[pos] <= 'z') {
                result += (text[pos] - 'a' + 1);
            }
        }
        return result;
    }
}

// Construct empty overview.
game::v3::DirectoryScanner::DirectoryScanner(afl::io::Directory& specificationDirectory, afl::string::Translator& tx, afl::sys::LogListener& log)
    : m_translator(tx),
      m_log(log),
      m_messageParser()
{
    // ex GDirectoryOverview::GDirectoryOverview
    clear();
    initMessageParser(specificationDirectory);
}

// Scan for files.
void
game::v3::DirectoryScanner::scan(afl::io::Directory& dir, afl::charset::Charset& charset, bool resultOnly)
{
    // ex GDirectoryOverview::scan
    Timestamp times[NUM_PLAYERS];
    int turnNumbers[NUM_PLAYERS];
    int numPlayers = 0;

    // Reset
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        turnNumbers[i-1] = 0;
        m_playerFlags[i-1] = PlayerFlags_t();
    }

    // do we have unpacked game data?
    if (!resultOnly) {
        for (int i = 1; i <= NUM_PLAYERS; ++i) {
            String_t fileName = afl::string::Format("gen%d.dat", i);
            try {
                afl::base::Ptr<afl::io::Stream> file = dir.openFileNT(fileName, afl::io::FileSystem::OpenRead);
                if (file.get() != 0) {
                    gt::Gen gen;
                    if (file->read(afl::base::fromObject(gen)) == sizeof(gen) && gen.playerId == i && gen.turnNumber > 0) {
                        // OK, save it
                        turnNumbers[i-1] = gen.turnNumber;
                        m_playerFlags[i-1] += HaveUnpacked;
                        times[i-1] = gen.timestamp;
                        ++numPlayers;

                        // Check for matching result
                        gt::ResultGen rgen;
                        if (checkResult(dir, charset, i, rgen, 0)) {
                            if (rgen.turnNumber == gen.turnNumber && Timestamp(rgen.timestamp) == Timestamp(gen.timestamp)) {
                                // same turn, ok
                                m_playerFlags[i-1] += HaveResult;
                            } else if (rgen.turnNumber >= gen.turnNumber && Timestamp(gen.timestamp).isEarlierThan(Timestamp(rgen.timestamp))) {
                                // newer turn, ok. PCC 1.x only checks the timestamp.
                                // Accept '>=' turns, for rehosts.
                                m_playerFlags[i-1] += HaveNewResult;
                            } else {
                                // we cannot make sense of this RST.
                                // But do not prevent the user from unpacking in case he's trying something clever.
                                m_playerFlags[i-1] |= HaveOtherResult;
                            }
                        }
                    } else {
                        m_log.write(m_log.Warn, LOG_NAME, file->getName(), 0, m_translator.translateString("File exists but is invalid and has been ignored"));
                    }

                    file = dir.openFileNT(afl::string::Format("mdata%d.dat", i), afl::io::FileSystem::OpenRead);
                    if (file.get() != 0) {
                        checkHostVersion(*file, charset, m_hostVersions[i-1]);
                    }
                }
            }
            catch (afl::except::FileProblemException& e) {
                m_log.write(m_log.Warn, LOG_NAME, e.getFileName(), 0, e.what());
            }
            catch (std::exception& e) {
                m_log.write(m_log.Warn, LOG_NAME, fileName, 0, e.what());
            }
        }
    }

    if (numPlayers == 0) {
        // we don't have an unpacked game directory. Do we have a RST?
        for (int i = 1; i <= NUM_PLAYERS; ++i) {
            gt::ResultGen rgen;
            if (checkResult(dir, charset, i, rgen, &m_hostVersions[i-1])) {
                turnNumbers[i-1] = rgen.turnNumber;
                times[i-1] = rgen.timestamp;
                m_playerFlags[i-1] += HaveResult;
                ++numPlayers;
            }
        }
    }

    if (numPlayers == 0) {
        // directory is empty, punt.
        return;
    }

    // We have some data. Figure out which is current.
    int currentTurn = 0;
    int currentPlayer = 0;
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        if (turnNumbers[i-1] > currentTurn) {
            currentTurn = turnNumbers[i-1];
            currentPlayer = i;
        }
    }
    assert(currentTurn != 0);

    // Check for conflicts
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        if (turnNumbers[i-1] != 0 && (turnNumbers[i-1] != currentTurn || times[i-1] != times[currentPlayer-1])) {
            m_playerFlags[i-1] |= HaveConflict;
        }
    }

    // Last step: look for TRN files.
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        String_t fileName = afl::string::Format("player%d.trn", i);
        try {
            afl::base::Ptr<afl::io::Stream> file = dir.openFileNT(fileName, afl::io::FileSystem::OpenRead);
            if (file.get() != 0) {
                TurnFile trn(charset, m_translator, *file, false);
                if (trn.getPlayer() == i) {
                    // Matching turn found, check whether it makes sense.
                    // We ignore the turn if it is empty or stale.
                    // Note that we need to check the raw turn header here to determine emptiness;
                    // we told TurnFile to not read the turn body so it will report no commands using the regular way.
                    if (trn.getTurnHeader().numCommands > 0 && times[i-1] == trn.getTimestamp()) {
                        m_playerFlags[i-1] += HaveTurn;
                    }
                } else {
                    m_log.write(m_log.Warn, LOG_NAME, file->getName(), 0, m_translator("File exists but is invalid and has been ignored"));
                }
            }
        }
        catch (afl::except::FileProblemException& e) {
            m_log.write(m_log.Warn, LOG_NAME, e.getFileName(), 0, e.what());
        }
        catch (std::exception& e) {
            m_log.write(m_log.Warn, LOG_NAME, fileName, 0, e.what());
        }
    }
}

// Clear stored state.
void
game::v3::DirectoryScanner::clear()
{
    // ex GDirectoryOverview::clear
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        m_playerFlags[i-1] = PlayerFlags_t();
        m_hostVersions[i-1] = HostVersion();
    }
}

// Get flags for one player.
game::v3::DirectoryScanner::PlayerFlags_t
game::v3::DirectoryScanner::getPlayerFlags(int playerId) const
{
    // ex GDirectoryOverview::getPlayerFlags
    if (playerId > 0 && playerId <= NUM_PLAYERS) {
        return m_playerFlags[playerId-1];
    } else {
        return PlayerFlags_t();
    }
}

// Get directory flags.
game::v3::DirectoryScanner::PlayerFlags_t
game::v3::DirectoryScanner::getDirectoryFlags() const
{
    // ex GDirectoryOverview::getGlobalFlags
    PlayerFlags_t result;
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        result += m_playerFlags[i-1];
    }
    return result;
}

// Get players that have a particular flag set.
game::PlayerSet_t
game::v3::DirectoryScanner::getPlayersWhere(PlayerFlags_t flags) const
{
    // ex GDirectoryOverview::getPlayersWhere
    PlayerSet_t result;
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        if (m_playerFlags[i-1].containsAnyOf(flags)) {
            result += i;
        }
    }
    return result;
}

// Get host version.
game::HostVersion
game::v3::DirectoryScanner::getDirectoryHostVersion() const
{
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        if (!m_playerFlags[i-1].empty() && !m_playerFlags[i-1].contains(HaveConflict)) {
            return m_hostVersions[i-1];
        }
    }
    return HostVersion();
}

// Get default player.
int
game::v3::DirectoryScanner::getDefaultPlayer() const
{
    // ex GDirectoryOverview::getDefaultPlayer
    int result = 0;
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        if (!m_playerFlags[i-1].empty()) {
            if (result == 0) {
                result = i;
            } else {
                return 0;
            }
        }
    }
    return result;
}


/*
 *  Privates
 */

/** Check result file.
    \param dir Directory to look in
    \param playerId Player Id to look for
    \param rgen [out] Result turn file information
    \param pVersion [out,optional] Host version
    \retval true result file found and valid
    \retval false result file invalid or not found */
bool
game::v3::DirectoryScanner::checkResult(afl::io::Directory& dir, afl::charset::Charset& charset, int playerId, game::v3::structures::ResultGen& rgen, HostVersion* pVersion)
{
    // ex game/storage/overview.cc:checkResult
    // Open file
    const String_t fileName = afl::string::Format("player%d.rst", playerId);
    const afl::base::Ptr<afl::io::Stream> file = dir.openFileNT(fileName, afl::io::FileSystem::OpenRead);
    if (file.get() == 0) {
        return false;
    }

    // Check GEN block for validity
    try {
        ResultFile rst(*file, m_translator);
        afl::io::Stream::FileSize_t offset;
        if (rst.getSectionOffset(rst.GenSection, offset)) {
            file->setPos(offset);
            if (file->read(afl::base::fromObject(rgen)) == sizeof(rgen)
                && rgen.playerId == playerId
                && rgen.turnNumber > 0
                && rgen.timestampChecksum == int16_t(afl::checksums::ByteSum().add(rgen.timestamp, 0)))
            {
                // ok
                if (pVersion) {
                    if (rst.getSectionOffset(rst.MessageSection, offset)) {
                        file->setPos(offset);
                        checkHostVersion(*file, charset, *pVersion);
                    }
                }
                return true;
            } else {
                // error
                m_log.write(m_log.Warn, LOG_NAME, file->getName(), 0, m_translator.translateString("File exists but is invalid and has been ignored"));
                return false;
            }
        } else {
            m_log.write(m_log.Warn, LOG_NAME, file->getName(), 0, m_translator.translateString("File exists but is invalid and has been ignored"));
            return false;
        }
    }
    catch (afl::except::FileProblemException& e) {
        m_log.write(m_log.Warn, LOG_NAME, e.getFileName(), 0, e.what());
        return false;
    }
    catch (std::exception& e) {
        m_log.write(m_log.Warn, LOG_NAME, fileName, 0, e.what());
        return false;
    }
}

void
game::v3::DirectoryScanner::checkHostVersion(afl::io::Stream& stream, afl::charset::Charset& charset, game::HostVersion& version)
{
    // ex game/storage/overview.cc:checkHostVersion

    // Imports
    using game::parser::MessageInformation;
    using game::parser::MessageConfigurationValue_t;

    // Integration for the message parser.
    // We do not need any integration with game data, so give it some mocks.
    class NullDataInterface : public game::parser::DataInterface {
     public:
        virtual int getPlayerNumber() const
            { return 0; }
        virtual int parseName(Name /*which*/, const String_t& /*name*/) const
            { return 0; }
        virtual String_t expandRaceNames(String_t name) const
            { return name; }
    };
    NullDataInterface iface;

    // Turn number. We do not need the turn number for this parsing.
    const int turnNr = 1;

    String_t hostType;
    String_t hostVersion;

    /* Check the file */
    try {
        // Load inbox file header
        InboxFile file(stream, charset, m_translator);

        // Load the messages, starting with the last one, looking for a match.
        for (size_t i = file.getNumMessages(); i > 0; --i) {
            // Parse
            afl::container::PtrVector<MessageInformation> info;
            m_messageParser.parseMessage(file.loadMessage(i-1), iface, turnNr, info, m_translator, m_log);

            // Check
            for (afl::container::PtrVector<MessageInformation>::iterator i = info.begin(); i != info.end(); ++i) {
                for (MessageInformation::Iterator_t j = (*i)->begin(); j != (*i)->end(); ++j) {
                    if (MessageConfigurationValue_t* val = dynamic_cast<MessageConfigurationValue_t*>(*j)) {
                        if (val->getIndex() == "HOSTTYPE") {
                            hostType = afl::string::strLCase(val->getValue());
                        }
                        if (val->getIndex() == "HOSTVERSION") {
                            hostVersion = afl::string::strLCase(val->getValue());
                        }
                    }
                }
            }
            if (!hostType.empty() && !hostVersion.empty()) {
                break;
            }
        }
    }
    catch (std::exception& e) {
        // Ignore
    }

    // Evaluate the result
    if (!hostVersion.empty()) {
         if (hostType == "host") {
             version.set(game::HostVersion::Host, parseHostVersion(hostVersion, true));
         }
         if (hostType == "srace") {
             version.set(game::HostVersion::SRace, parseHostVersion(hostVersion, true));
         }
         if (hostType == "phost") {
             version.set(game::HostVersion::PHost, parseHostVersion(hostVersion, false));
         }
    }
}

void
game::v3::DirectoryScanner::initMessageParser(afl::io::Directory& dir)
{
    // ex loadMessageParser, sort of
    static const char fileName[] = "hostver.ini";
    try {
        afl::base::Ref<afl::io::Stream> file = dir.openFile(fileName, afl::io::FileSystem::OpenRead);
        m_messageParser.load(*file, m_translator, m_log);
    }
    catch (afl::except::FileProblemException& e) {
        m_log.write(m_log.Warn, LOG_NAME, e.getFileName(), 0, e.what());
    }
    catch (std::exception& e) {
        m_log.write(m_log.Warn, LOG_NAME, fileName, 0, e.what());
    }
}
