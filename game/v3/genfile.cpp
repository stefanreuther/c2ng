/**
  *  \file game/v3/genfile.cpp
  *  \brief Class game::v3::GenFile
  */

#include <cstring>
#include "game/v3/genfile.hpp"

namespace gt = game::v3::structures;

namespace {
    /* "NOPASSWORD".
       Setting the password to this value (cleartext) will inhibit the password prompt in all programs.
       Note that "" (empty string) is a valid password, it will cause a prompt (which users confirm with Enter). */
    const char* NOPASSWORD = "NOPASSWORD";

    const uint8_t NEW_PASSWORD_OFFSET = 50;
    const int16_t NEW_PASSWORD_FLAG = 13;
}

// Default constructor.
game::v3::GenFile::GenFile()
    : m_data()
{
    setSignatures();
}

// Construct from data.
game::v3::GenFile::GenFile(const game::v3::structures::Gen& data)
    : m_data(data)
{
    setSignatures();
}

// Load from GENx.DAT file.
void
game::v3::GenFile::loadFromFile(afl::io::Stream& in)
{
    in.fullRead(afl::base::fromObject(m_data));
    setSignatures();
}

// Load from result file.
void
game::v3::GenFile::loadFromResult(afl::io::Stream& in)
{
    // ex GGen::GGen (sort-of)
    game::v3::structures::ResultGen data;
    in.fullRead(afl::base::fromObject(data));

    std::memcpy(m_data.timestamp, data.timestamp, sizeof(m_data.timestamp));
    std::memcpy(m_data.scores,    data.scores,    sizeof(m_data.scores));
    m_data.playerId          = data.playerId;
    std::memcpy(m_data.password,  data.password,  sizeof(data.password));
    m_data.zero              = 0;
    std::memcpy(&m_data.checksums, &data.checksums, sizeof(m_data.checksums));
    m_data.newPasswordFlag   = 0;
    std::memset(m_data.newPassword, 0, sizeof(m_data.newPassword));
    m_data.turnNumber        = data.turnNumber;
    m_data.timestampChecksum = data.timestampChecksum;

    setSignatures();
}

// Get turn number as contained in the file.
int
game::v3::GenFile::getTurnNumber() const
{
    return m_data.turnNumber;
}

// Get player number as contained in the file.
int
game::v3::GenFile::getPlayerId() const
{
    // ex GGen::getPlayerId
    return m_data.playerId;
}

// Get timestamp.
game::Timestamp
game::v3::GenFile::getTimestamp() const
{
    return Timestamp(m_data.timestamp);
}

// Get score.
int
game::v3::GenFile::getScore(int player, Score what) const
{
    if (player > 0 && player <= gt::NUM_PLAYERS) {
        const gt::GenScore& score = m_data.scores[player-1];
        switch (what) {
         case NumPlanets:      return score.numPlanets;
         case NumCapitalShips: return score.numCapitalShips;
         case NumFreighters:   return score.numFreighters;
         case NumBases:        return score.numBases;
        }
    }
    return -1;
}

// Check password.
bool
game::v3::GenFile::isPassword(const String_t& pass) const
{
    // ex GGen::getPassword, GGen::isPassword
    // Determine password
    const size_t LEN = 10;
    static_assert(sizeof(m_data.newPassword) == LEN, "sizeof newPassword");
    static_assert(sizeof(m_data.password) == 2*LEN, "sizeof password");
    char requiredPassword[LEN];
    if (m_data.newPasswordFlag == NEW_PASSWORD_FLAG) {
        for (size_t i = 0; i < LEN; ++i) {
            requiredPassword[i] = static_cast<char>(m_data.newPassword[i] - NEW_PASSWORD_OFFSET);
        }
    } else {
        for (size_t i = 0; i < LEN; ++i) {
            uint8_t k = static_cast<uint8_t>(m_data.password[i] - m_data.password[2*LEN-1-i] + 32);
            if (k < 32 || k > 127) {
                requiredPassword[i] = ' ';
            } else {
                requiredPassword[i] = static_cast<char>(k);
            }
        }
    }

    // Determine password length
    size_t i = LEN;
    while (i > 0 && requiredPassword[i-1] <= ' ') {
        --i;
    }

    // Compare
    return pass.size() == i
        && pass.compare(0, i, requiredPassword, i) == 0;
}

// Check presence of password.
bool
game::v3::GenFile::hasPassword() const
{
    // ex GGen::hasPassword
    return !isPassword(NOPASSWORD);
}

// Change password.
void
game::v3::GenFile::setPassword(const String_t& pass)
{
    // ex GGen::setPassword
    for (size_t i = 0; i < sizeof(m_data.newPassword); ++i) {
        if (i < pass.size()) {
            m_data.newPassword[i] = static_cast<uint8_t>(pass[i] + NEW_PASSWORD_OFFSET);
        } else {
            m_data.newPassword[i] = NEW_PASSWORD_OFFSET;
        }
    }
    m_data.newPasswordFlag = NEW_PASSWORD_FLAG;
}

// Set password (from TRN file).
void
game::v3::GenFile::setNewPasswordData(afl::base::ConstBytes_t pass)
{
    // ex GGen::setPasswordFromTurn
    afl::base::Bytes_t(m_data.newPassword).copyFrom(pass);
    m_data.newPasswordFlag = NEW_PASSWORD_FLAG;
}

// Get password (for TRN file).
afl::base::ConstBytes_t
game::v3::GenFile::getNewPasswordData() const
{
    if (m_data.newPasswordFlag == NEW_PASSWORD_FLAG) {
        return afl::base::ConstBytes_t(m_data.newPassword);
    } else {
        return afl::base::Nothing;
    }
}

// Get data.
void
game::v3::GenFile::getData(game::v3::structures::Gen& data)
{
    // ex GGen::getGenRecord (sort-of)
    data = m_data;
}

// Get signature 1 (*.dis files).
const game::v3::GenFile::Signature_t&
game::v3::GenFile::getSignature1() const
{
    return m_signature1;
}

// Get signature 2 (*.dat files).
const game::v3::GenFile::Signature_t&
game::v3::GenFile::getSignature2() const
{
    return m_signature2;
}

// Get section checksum.
uint32_t
game::v3::GenFile::getSectionChecksum(Section_t sec) const
{
    // ex GGen::getChecksum
    return m_data.checksums[sec];
}

// Set section checksum.
void
game::v3::GenFile::setSectionChecksum(Section_t sec, uint32_t value)
{
    // ex GGen::setChecksum
    m_data.checksums[sec] = value;
}

// Copy scores to TurnScoreList object.
void
game::v3::GenFile::copyScoresTo(game::score::TurnScoreList& scores) const
{
    // ex GStatFile::mergeGen
    namespace gs = game::score;
    gs::TurnScore& r = scores.addTurn(getTurnNumber(), getTimestamp());

    const gs::TurnScore::Slot_t pi = scores.addSlot(gs::ScoreId_Planets);
    const gs::TurnScore::Slot_t ci = scores.addSlot(gs::ScoreId_Capital);
    const gs::TurnScore::Slot_t fi = scores.addSlot(gs::ScoreId_Freighters);
    const gs::TurnScore::Slot_t bi = scores.addSlot(gs::ScoreId_Bases);

    for (int i = 1; i <= gt::NUM_PLAYERS; ++i) {
        // FIXME: implement some sensible merging here.
        // - when we know a score blanker is in use, treat 0 as unknown
        // - do not overwrite a nonzero value with 0 [but what about rehosts?]
        const gt::GenScore& gs = m_data.scores[i-1];
        r.set(pi, i, int(gs.numPlanets));
        r.set(ci, i, int(gs.numCapitalShips));
        r.set(fi, i, int(gs.numFreighters));
        r.set(bi, i, int(gs.numBases));
    }
}


void
game::v3::GenFile::setSignatures()
{
    // ex GGen::genSig
    for (size_t i = 0; i < sizeof(Signature_t); ++i) {
        m_signature1[i] = uint8_t(m_data.password[10+i]);
        m_signature2[i] = uint8_t(m_data.password[10+i] + (i+1));
    }
}
