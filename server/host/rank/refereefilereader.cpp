/**
  *  \file server/host/rank/refereefilereader.cpp
  *  \brief Class server::host::rank::RefereeFileReader
  */

#include "server/host/rank/refereefilereader.hpp"
#include "afl/string/parse.hpp"
#include "afl/string/string.hpp"

// Default constructor.
server::host::rank::RefereeFileReader::RefereeFileReader()
    : FileParser("#;"),
      m_end(false)
{
    // ex RefereeFileReader::RefereeFileReader
    initRanks(m_ranks, 0x7FFFFFFF);
}

void
server::host::rank::RefereeFileReader::handleLine(const String_t& /*fileName*/, int /*lineNr*/, String_t line)
{
    // ex RefereeFileReader::processLine
    String_t::size_type n = line.find_first_not_of(" \t");
    if (n >= line.size()) {
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
    int32_t player, val;
    if (key == "END" && afl::string::strToInteger(value, val)) {
        m_end = (val != 0);
    } else if (key.size() > 4
               && key.compare(0, 4, "RANK", 4) == 0
               && afl::string::strToInteger(key.substr(4), player)
               && player > 0
               && player <= Game::NUM_PLAYERS)
    {
        if (afl::string::strToInteger(value, val)) {
            m_ranks[player-1] = val;
        }
    } else {
        // FIXME: "turns"
        // FIXME: warning?
    }
}

void
server::host::rank::RefereeFileReader::handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
{
    // ignore
}

// Check for game end.
bool
server::host::rank::RefereeFileReader::isEnd() const
{
    return m_end;
}

// Get ranks.
const server::host::rank::Rank_t&
server::host::rank::RefereeFileReader::getRanks() const
{
    return m_ranks;
}
