/**
  *  \file server/host/rank/scorefilereader.cpp
  *  \brief Class server::host::rank::ScoreFileReader
  */

#include "server/host/rank/scorefilereader.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/pack.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/string/parse.hpp"

// Constructor.
server::host::rank::ScoreFileReader::ScoreFileReader(afl::net::redis::HashKey scoreKey, afl::net::redis::HashKey descriptionKey)
    : FileParser("#;"),
      m_scoreKey(scoreKey),
      m_descriptionKey(descriptionKey),
      m_name(),
      m_description()
{
    // ex ScoreReader::ScoreReader
    /* need not initialize values, because we're not going to use them anyway
       until we see a delimiter */
}

void
server::host::rank::ScoreFileReader::handleLine(const String_t& /*fileName*/, int /*lineNr*/, String_t line)
{
    // ex ScoreReader::processLine
    /* Comment or blank? */
    String_t::size_type n = line.find_first_not_of(" \t");
    if (n >= line.size()) {
        return;
    }

    /* Section delimiter? */
    if (line[n] == '%') {
        flush();
        m_name = afl::string::strTrim(line.substr(n+1));
        return;
    }

    /* Must be an assignment */
    String_t::size_type p = line.find('=');
    if (p == line.npos) {
        // FIXME: log it?
        return;
    }

    String_t key(afl::string::strUCase(afl::string::strTrim(line.substr(n, p-n))));
    String_t value(afl::string::strTrim(line.substr(p+1)));
    int32_t player, score;
    if (key == "DESCRIPTION") {
        m_description = value;
    } else if (key.size() > 5
               && key.compare(0, 5, "SCORE", 5) == 0
               && afl::string::strToInteger(key.substr(5), player)
               && player > 0 && player <= Game::NUM_PLAYERS)
    {
        if (afl::string::strToInteger(value, score)) {
            m_values[player-1] = score;
        }
    } else {
        // FIXME?
    }
}

void
server::host::rank::ScoreFileReader::handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
{ }

void
server::host::rank::ScoreFileReader::flush()
{
    // ex ScoreReader::flush
    if (!m_name.empty()) {
        m_scoreKey.stringField(m_name).set(packScore(m_values));
        m_descriptionKey.stringField(m_name).set(m_description);
    }
    for (int i = 0; i < Game::NUM_PLAYERS; ++i) {
        m_values[i] = -1;
    }
    m_name.clear();
    m_description.clear();
}


String_t
server::host::rank::packScore(const Score_t& score)
{
    uint8_t packed[Game::NUM_PLAYERS * 4];
    afl::bits::packArray<afl::bits::Int32LE>(packed, score);
    return afl::string::fromBytes(packed);
}
