/**
  *  \file game/spec/friendlycode.cpp
  *  \brief Class game::spec::FriendlyCode
  */

#include <cstring>
#include <stdexcept>
#include "game/spec/friendlycode.hpp"
#include "afl/string/format.hpp"
#include "game/limits.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/registrationkey.hpp"
#include "game/spec/hullfunction.hpp"
#include "util/string.hpp"
#include "util/translation.hpp"

using game::spec::HullFunction;

// Default constructor.
game::spec::FriendlyCode::FriendlyCode()
    : m_code(),
      m_description(),
      m_races(),
      m_flags()
{ }

// Construct from definition.
game::spec::FriendlyCode::FriendlyCode(String_t code, String_t descriptionLine)
    : m_code(code),
      m_description(),
      m_races(),
      m_flags()
{
    // ex GFCode::GFCode
    initFromString(descriptionLine);
}

// Destructor.
game::spec::FriendlyCode::~FriendlyCode()
{ }

// Get friendly code.
const String_t&
game::spec::FriendlyCode::getCode() const
{
    // ex GFCode::getFCode
    return m_code;
}

// Get description.
String_t
game::spec::FriendlyCode::getDescription(const PlayerList& playerList) const
{
    // ex GFCode::getDescription
    return playerList.expandNames(m_description, false);
}

// Get flags.
game::spec::FriendlyCode::FlagSet_t
game::spec::FriendlyCode::getFlags() const
{
    return m_flags;
}

// Get set of races who can use this friendly code.
game::PlayerSet_t
game::spec::FriendlyCode::getRaces() const
{
    return m_races;
}

// Check whether this friendly code works on an object.
bool
game::spec::FriendlyCode::worksOn(const game::map::Object& o,
                                  const UnitScoreDefinitionList& scoreDefinitions,
                                  const game::spec::ShipList& shipList,
                                  const game::config::HostConfiguration& config) const
{
    // ex GFCode::worksOn
    if (const game::map::Ship* s = dynamic_cast<const game::map::Ship*>(&o)) {
        return worksOn(*s, scoreDefinitions, shipList, config);
    } else if (const game::map::Planet* p = dynamic_cast<const game::map::Planet*>(&o)) {
        return worksOn(*p, config);
    } else {
        return false;
    }
}

bool
game::spec::FriendlyCode::worksOn(const game::map::Ship& s,
                                  const UnitScoreDefinitionList& scoreDefinitions,
                                  const game::spec::ShipList& shipList,
                                  const game::config::HostConfiguration& config) const
{
    // ex GFCode::worksOn(const GShip& s)
    if (!s.isPlayable(s.ReadOnly)) {
        return false;
    }
    int owner = 0;
    if (!s.getRealOwner().get(owner)) {
        return false;
    }
    if (!m_races.contains(config.getPlayerRaceNumber(owner))) {
        return false;
    }
    if (!m_flags.contains(ShipCode)) {
        return false;
    }
    if (m_flags.contains(CapitalShipCode)
        && s.getNumBeams().orElse(0) == 0
        && s.getNumLaunchers().orElse(0) == 0
        && s.getNumBeams().orElse(0) == 0)
    {
        return false;
    }
    if (m_flags.contains(AlchemyShipCode)
        && !s.hasSpecialFunction(HullFunction::MerlinAlchemy,     scoreDefinitions, shipList, config)
        && !s.hasSpecialFunction(HullFunction::NeutronicRefinery, scoreDefinitions, shipList, config)
        && !s.hasSpecialFunction(HullFunction::AriesRefinery,     scoreDefinitions, shipList, config))
    {
        return false;
    }
    return true;
}

// Check whether this friendly code works on a planet.
bool
game::spec::FriendlyCode::worksOn(const game::map::Planet& p, const game::config::HostConfiguration& config) const
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

bool
game::spec::FriendlyCode::isPermitted(const RegistrationKey& key) const
{
    return !m_flags.contains(RegisteredCode)
        || key.getStatus() != RegistrationKey::Unregistered;
}

/** Parse a flag list.
    \param s       [in] string we got from user
    \param data    [in] allowed options, in caps (e.g., for fcodes, "SPBRAU")
    \param value   [out] flag result
    \param races   [out] player set result
    \returns true on success, false on error */
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

/** Initialize from description string.
    \param descriptionLine Description */
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
