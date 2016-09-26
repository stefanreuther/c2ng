/**
  *  \file game/spec/friendlycode.cpp
  */

#include <cstring>
#include <stdexcept>
#include "game/spec/friendlycode.hpp"
#include "game/limits.hpp"
#include "util/translation.hpp"
#include "afl/string/format.hpp"
#include "util/string.hpp"


// /** Construct empty GFCode object. */
game::spec::FriendlyCode::FriendlyCode()
    : m_code(),
      m_description(),
      m_races(),
      m_flags()
{ }

// /** Construct from fcodes.cc file line. This one handles a line
//     that originally contained `c + "," + desc_line'.
//     \param c          friendly code
//     \param desc_line  description line */
game::spec::FriendlyCode::FriendlyCode(String_t code, String_t descriptionLine)
    : m_code(code),
      m_description(),
      m_races(),
      m_flags()
{
    // ex GFCode::GFCode
    initFromString(descriptionLine);
}

game::spec::FriendlyCode::~FriendlyCode()
{ }

const String_t&
game::spec::FriendlyCode::getCode() const
{
    // ex GFCode::getFCode
    return m_code;
}

String_t
game::spec::FriendlyCode::getDescription(const PlayerList& playerList) const
{
    // ex GFCode::getDescription
    return playerList.expandNames(m_description);
}

game::spec::FriendlyCode::FlagSet_t
game::spec::FriendlyCode::getFlags() const
{
    return m_flags;
}

game::PlayerSet_t
game::spec::FriendlyCode::getRaces() const
{
    return m_races;
}




/** Check whether code works on specified object.
    \param o a ship or planet */
bool
game::spec::FriendlyCode::worksOn(const game::map::Object& o, game::config::HostConfiguration& config) const
{
    // ex GFCode::worksOn
    // FIXME: incomplete
    /*if (const GShip* s = dynamic_cast<const GShip*>(&o))
        return worksOn(*s);
        else*/
    if (const game::map::Planet* p = dynamic_cast<const game::map::Planet*>(&o)) {
        return worksOn(*p, config);
    } else {
        return false;
    }
}

/** Check whether code works on planet.
    \param p a planet */
bool
game::spec::FriendlyCode::worksOn(const game::map::Planet& p, game::config::HostConfiguration& config) const
{
    // ex GFCode::worksOn
    // FIXME: consider moving this out of FriendlyCode. Instead, make separate filter classes. It's only used to build filtered lists.
    if (!p.isPlayable(p.ReadOnly)) {
        return false;
    }
    int owner = 0;
    if (!p.getOwner(owner)) {
        return false;
    }
    if (!m_races.contains(config.getPlayerRaceNumber(owner))) {
        return false;
    }
    if (m_flags.contains(PlanetCode) || (m_flags.contains(StarbaseCode) && p.hasBase())) {
        return true;
    }
    return false;
}


// /** Parse a flag list.
//     \param s       string we got from user
//     \param data    allowed options, in caps (e.g., for fcodes, "SPBRAU")
//     \param value   pointer to integer variable with result (first option
//                    has value 1, second has value 2, third has 4, etc.)
//     \param races   pointer to player set. Can be NULL, then
//                    we'll not accept races. *races can be preinitialized
//                    to a default value (*value can not). Note that the result
//                    may contain bits outside 1..11.
//     \param accept_bad_flags if set, accept unknown flags.
//     \returns true on success, false on error */
bool
game::spec::FriendlyCode::parseFlags(const String_t& s, const char* data, FlagSet_t& flags, PlayerSet_t& races)
{
    // ex util/misc.cc:parseFlags
    // FIXME: rewrite using Memory<>
    String_t::size_type t = 0;
    flags = FlagSet_t();
    while (t < s.length()) {
        char c = s[t];
        if (c == '+' || c == '-') {
            bool negate = (c == '-');
            races = PlayerSet_t();
            while (++t < s.length()) {
                int raceNr = 0;
                if (!util::parsePlayerCharacter(s[t], raceNr) || raceNr <= 0 || raceNr > MAX_RACES) {
                    return false;
                }
                races += raceNr;
            }
            if (negate) {
                races ^= PlayerSet_t::allUpTo(MAX_RACES);
            }
            break;
        } else {
            const char* p = strchr(data, std::toupper(uint8_t(c)));
            if (p) {
                flags += Flag(p - data);
            }
            ++t;
        }
    }
    return true;
}

void
game::spec::FriendlyCode::initFromString(const String_t& descriptionLine)
{
    // ex GFCode::initFromString
    String_t flagStr;
    if (!afl::string::strSplit(descriptionLine, flagStr, m_description, ",")) {
        // FIXME: exception?
        throw std::range_error(afl::string::Format(_("Friendly code \"%s\" lacking description").c_str(), m_code));
    }
    m_description = afl::string::strTrim(m_description);

    m_races = PlayerSet_t::allUpTo(MAX_RACES);
    if (!parseFlags(flagStr, "SPBCARU", m_flags, m_races)) {
        throw std::range_error(afl::string::Format(_("Malformed flags for friendly code \"%s\"").c_str(), m_code));
    }
}
