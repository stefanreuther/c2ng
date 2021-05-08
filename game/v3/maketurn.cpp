/**
  *  \file game/v3/maketurn.cpp
  *  \brief Class game::v3::Maketurn
  */

#include "game/v3/maketurn.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "game/msg/outbox.hpp"
#include "game/v3/genfile.hpp"
#include "game/v3/outboxreader.hpp"
#include "game/v3/registrationkey.hpp"
#include "game/v3/structures.hpp"
#include "util/string.hpp"

namespace gt = game::v3::structures;

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::string::Format;

namespace {
    const char*const LOG_NAME = "game.v3.maketurn";

    /*
     *  Generic type switches for the generateTurnCommands function
     */

    struct ObjectInfo {
        int shipId;
        gt::String3_t friendlyCode;
    };

    int getId(const gt::Ship& sh)   { return sh.shipId;   }
    int getId(const gt::Planet& pl) { return pl.planetId; }
    int getId(const gt::Base& b)    { return b.baseId;    }

    void makeTurnCommands(game::v3::TurnFile& turn, const gt::Ship& oldShip, const gt::Ship& newShip)
        { turn.makeShipCommands(oldShip.shipId, oldShip, newShip); }
    void makeTurnCommands(game::v3::TurnFile& turn, const gt::Planet& oldPlanet, const gt::Planet& newPlanet)
        { turn.makePlanetCommands(oldPlanet.planetId, oldPlanet, newPlanet); }
    void makeTurnCommands(game::v3::TurnFile& turn, const gt::Base& oldBase, const gt::Base& newBase)
        { turn.makeBaseCommands(oldBase.baseId, oldBase, newBase); }

    void checkObject(ObjectInfo& oi, const gt::Ship& ship)
        {
            if (oi.shipId == 0) {
                oi.shipId = ship.shipId;
                oi.friendlyCode = ship.friendlyCode;
            }
        }
    void checkObject(ObjectInfo&, const gt::Planet&)
        { }
    void checkObject(ObjectInfo&, const gt::Base&)
        { }

    /** Generate turn commands for one data file pair.
        \tparam T Content type
        \param dir        [in] directory to work in
        \param baseName   [in] basename of file ("ship")
        \param player     [in] player Id
        \param turn       [out] commands are generated here
        \param oi         [in/out]  information of first ship is reported here */
    template<typename T>
    void generateTurnCommands(afl::io::Directory& dir,
                              const char* baseName,
                              const int player,
                              game::v3::TurnFile& turn,
                              ObjectInfo& oi,
                              afl::string::Translator& tx)
    {
        T datBuffer, disBuffer;

        // Open files
        Ref<Stream> datFile = dir.openFile(Format("%s%d.dat", baseName, player), FileSystem::OpenRead);
        Ref<Stream> disFile = dir.openFile(Format("%s%d.dis", baseName, player), FileSystem::OpenRead);

        // Read and compare counts
        gt::Int16_t datCount, disCount;
        datFile->fullRead(datCount.m_bytes);
        disFile->fullRead(disCount.m_bytes);
        const int16_t count = datCount;
        if (count != disCount) {
            throw afl::except::FileFormatException(*datFile, tx("Object count mismatch"));
        }

        // Read and process content
        for (int i = 0; i < count; ++i) {
            datFile->fullRead(afl::base::fromObject(datBuffer));
            disFile->fullRead(afl::base::fromObject(disBuffer));
            const int datId = getId(datBuffer);
            const int disId = getId(disBuffer);
            if (datId != disId) {
                throw afl::except::FileFormatException(*datFile, tx("Object Id mismatch"));
            }
            makeTurnCommands(turn, disBuffer, datBuffer);
            checkObject(oi, datBuffer);
        }
    }


    /*
     *  Messages
     */

    class MessageSender : public game::v3::OutboxReader {
     public:
        MessageSender(game::v3::TurnFile& trn, afl::string::Translator& tx, const game::PlayerList& players, afl::charset::Charset& cs)
            : m_turnFile(trn),
              m_translator(tx),
              m_playerList(players),
              m_charset(cs)
            { }
        virtual void addMessage(String_t text, game::PlayerSet_t receivers)
            {
                // FIXME: this is not very efficient, and it mangles the message through our whole normalisation process.
                // It would be more efficient to send messages directly, but that would need a way to deal with recipient headers.
                game::msg::Outbox out;
                out.addMessage(m_turnFile.getPlayer(), text, receivers);
                m_turnFile.sendOutbox(out, m_turnFile.getPlayer(), m_translator, m_playerList, m_charset);
            }
     private:
        game::v3::TurnFile& m_turnFile;
        afl::string::Translator& m_translator;
        const game::PlayerList& m_playerList;
        afl::charset::Charset& m_charset;
    };

    /** Send command messages.
        \param turn  [in/out] Turn to send them on
        \param s     [in] cmdX.txt file
        \param oi    [in] Object information gathered when generating commands, for THost allies
        \param log   [in/out] Logger */
    void sendCommands(game::v3::TurnFile& turn, Stream& s, const ObjectInfo& oi, afl::sys::LogListener& log, afl::string::Translator& tx)
    {
        // ex generateCommands
        afl::io::TextFile tf(s);
        tf.setCharsetNew(turn.charset().clone());
        String_t line;
        String_t accum;
        while (tf.readLine(line)) {
            line = afl::string::strTrim(line);
            if (line.empty() || line[0] == '#') {
                // ignore
            } else {
                String_t verb = afl::string::strNthWord(line, 0);
                if (util::stringMatch("$TIMEstamp", verb)) {
                    // check timestamp
                    if (afl::string::strNthWord(line, 1) != turn.getTimestamp().getTimestampAsString()) {
                        log.write(afl::sys::LogListener::Warn, LOG_NAME, Format(tx("File '%s' does not match current turn; skipping").c_str(), s.getName()));
                        break;
                    }
                } else if (afl::string::strCaseCompare("$THOST-ALLIES", verb) == 0) {
                    // send THost alliances
                    if (oi.shipId == 0) {
                        log.write(afl::sys::LogListener::Warn, LOG_NAME, Format(tx("Player %d has no ship; alliance changes not transmitted").c_str(), turn.getPlayer()));
                    } else {
                        turn.sendTHostAllies(afl::string::strNthWordRest(line, 1), oi.shipId, afl::string::fromBytes(oi.friendlyCode));
                    }
                } else if (verb.size() == 0 || verb[0] == '$') {
                    // meta-verb or invalid
                } else {
                    // command
                    if (accum.size() != 0 && accum.size() + line.size() > 500) {
                        turn.sendMessage(turn.getPlayer(), turn.getPlayer(), accum, turn.charset());
                        accum.clear();
                    }
                    accum += line;
                    accum += '\n';
                }
            }
        }

        if (!accum.empty()) {
            turn.sendMessage(turn.getPlayer(), turn.getPlayer(), accum, turn.charset());
        }
    }
}


// Constructor.
game::v3::Maketurn::Maketurn(afl::io::Directory& dir, const PlayerList& players, afl::charset::Charset& charset, afl::string::Translator& tx)
    : m_directory(dir),
      m_playerList(players),
      m_charset(charset),
      m_translator(tx),
      m_turns(dir, charset)
{
    // ex GMaketurn::GMaketurn
}

// Destructor.
game::v3::Maketurn::~Maketurn()
{ }

// Generate turn for a player.
size_t
game::v3::Maketurn::makeTurn(int playerNr, afl::sys::LogListener& log)
{
    // ex GMaketurn::makeTurn, makeProtoTurn
    // Read gen.dat file
    GenFile gen;
    gen.loadFromFile(*m_directory.openFile(Format("gen%d.dat", playerNr), FileSystem::OpenRead));

    TurnFile& thisTurn = m_turns.create(playerNr, gen.getTimestamp(), gen.getTurnNumber());

    // Load key
    std::auto_ptr<afl::charset::Charset> keyCS(m_charset.clone());
    RegistrationKey key(keyCS);
    key.initFromDirectory(m_directory, log, m_translator);

    // Configure
    thisTurn.setFeatures(TurnFile::FeatureSet_t() + TurnFile::WinplanFeature);
    thisTurn.setRegistrationKey(key, gen.getTurnNumber());

    // Generate bulk turn commands
    ObjectInfo oi = { 0, {0,0,0} };
    generateTurnCommands<gt::Ship>  (m_directory, "ship",  playerNr, thisTurn, oi, m_translator);
    generateTurnCommands<gt::Planet>(m_directory, "pdata", playerNr, thisTurn, oi, m_translator);
    generateTurnCommands<gt::Base>  (m_directory, "bdata", playerNr, thisTurn, oi, m_translator);

    // Messages
    {
        Ptr<Stream> s = m_directory.openFileNT(Format("mess35%d.dat", playerNr), FileSystem::OpenRead);
        if (s.get() != 0) {
            MessageSender(thisTurn, m_translator, m_playerList, m_charset).loadOutbox35(*s, m_charset, m_translator);
        } else {
            s = m_directory.openFileNT(Format("mess%d.dat", playerNr), FileSystem::OpenRead);
            if (s.get() != 0) {
                MessageSender(thisTurn, m_translator, m_playerList, m_charset).loadOutbox(*s, m_charset, m_translator);
            }
        }
    }

    // Commands
    {
        Ptr<Stream> s = m_directory.openFileNT(Format("cmd%d.txt", playerNr), FileSystem::OpenRead);
        if (s.get() != 0) {
            sendCommands(thisTurn, *s, oi, log, m_translator);
        }
    }

    // Password
    afl::base::ConstBytes_t newPassword = gen.getNewPasswordData();
    if (!newPassword.empty()) {
        thisTurn.addCommand(tcm_ChangePassword, 0, newPassword);
    }

    // Generate turn
    thisTurn.update();        // FIXME: in FileSet?

    return thisTurn.getNumCommands();
}

// Finish and write out turn files.
void
game::v3::Maketurn::saveAll(afl::sys::LogListener& log, afl::io::FileSystem& fs, const game::config::UserConfiguration& config)
{
    // ex GMaketurn::writeTurns
    m_turns.updateTrailers();
    m_turns.saveAll(log, m_playerList, fs, config, m_translator);
}

// Get number of prepared turns in this object.
size_t
game::v3::Maketurn::getNumFiles() const
{
    // ex GMaketurn::getNumTurns
    return m_turns.getNumFiles();
}
