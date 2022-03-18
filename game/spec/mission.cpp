/**
  *  \file game/spec/mission.cpp
  *  \brief Class game::spec::Mission
  */

#include "game/spec/mission.hpp"
#include "afl/string/char.hpp"
#include "game/hostversion.hpp"
#include "game/limits.hpp"
#include "game/map/ship.hpp"
#include "game/playerset.hpp"
#include "game/registrationkey.hpp"
#include "util/string.hpp"

namespace {
    /** Parse list of players from list of letters.
        Parses a "123456789ab"-style string up to its end or a ',', and leaves the parse position there.
        Used for parsing mission definitions etc.
        \param text [in] Text to parse
        \param index [in/out] Parse position
        \return player set */
    game::PlayerSet_t parsePlayerList(const String_t& text, String_t::size_type& index)
    {
        // FIXME (PCC2): can we merge this with misc.cc:parseFlags?
        // FIXME (c2ng): semi-duplicate to inline code in class FriendlyCode
        game::PlayerSet_t result;
        while (index < text.size() && text[index] != ',') {
            int raceNr = 0;
            if (util::parsePlayerCharacter(text[index], raceNr) && raceNr > 0 && raceNr <= game::MAX_RACES) {
                result += raceNr;
            }
            ++index;
        }
        return result;
    }
}

game::spec::Mission::Mission()
    : m_number(0),
      m_raceMask(),
      m_flags(),
      m_name(),
      m_shortName(),
      m_conditionExpression(),
      m_warningExpression(),
      m_labelExpression(),
      m_setCommand(),
      m_hotkey(0)
{
    // ex GMission::GMission()
    m_parameterTypes[InterceptParameter] = m_parameterTypes[TowParameter] = NoParameter;
}

game::spec::Mission::Mission(int number, String_t descriptionLine)
    : m_number(number),
      m_raceMask(PlayerSet_t::allUpTo(MAX_RACES)),
      m_flags(),
      m_name(),
      m_shortName(),
      m_conditionExpression(),
      m_warningExpression(),
      m_labelExpression(),
      m_setCommand(),
      m_hotkey(0)
{
    // ex GMission::GMission(int number, string_t desc_line)
    m_parameterTypes[InterceptParameter] = m_parameterTypes[TowParameter] = NoParameter;
    parseDescription(descriptionLine);
}

game::spec::Mission::~Mission()
{ }

int
game::spec::Mission::getNumber() const
{
    // ex GMission::getNumber
    return m_number;
}

game::PlayerSet_t
game::spec::Mission::getRaceMask() const
{
    // ex GMission::getRaceMask
    return m_raceMask;
}

void
game::spec::Mission::setRaceMask(PlayerSet_t mask)
{
    // ex GMission::setRaceMask
    m_raceMask = mask;
}

game::spec::Mission::FlagSet_t
game::spec::Mission::getFlags() const
{
    // ex GMission::getFlags
    return m_flags;
}

bool
game::spec::Mission::hasFlag(Flag flag) const
{
    // ex GMission::hasFlag
    return m_flags.contains(flag);
}

void
game::spec::Mission::setFlags(FlagSet_t flags)
{
    // ex GMission::setFlags
    m_flags = flags;
}

String_t
game::spec::Mission::getName() const
{
    // ex GMission::getName
    return m_name;
}

void
game::spec::Mission::setName(String_t name)
{
    // ex GMission::setName
    m_name = name;
}

String_t
game::spec::Mission::getShortName() const
{
    // ex GMission::getShortName
    return m_shortName;
}

void
game::spec::Mission::setShortName(String_t shortName)
{
    // ex GMission::setShortName
    this->m_shortName = shortName;
}

char
game::spec::Mission::getHotkey() const
{
    // ex GMission::getHotkey
    return m_hotkey;
}

void
game::spec::Mission::setHotkey(char c)
{
    // ex GMission::setHotkey
    m_hotkey = c;
}


/*
 *  Parameter accessors)
 */

game::spec::Mission::ParameterType
game::spec::Mission::getParameterType(MissionParameter p) const
{
    // ex GMission::getParameterType (sort-of)
    return m_parameterTypes[p];
}

void
game::spec::Mission::setParameterType(MissionParameter p, ParameterType type)
{
    // ex GMission::setParameterType (sort-of)
    m_parameterTypes[p] = type;
}

game::spec::Mission::ParameterFlagSet_t
game::spec::Mission::getParameterFlags(MissionParameter p) const
{
    return m_parameterFlags[p];
}

void
game::spec::Mission::setParameterFlags(MissionParameter p, ParameterFlagSet_t flags)
{
    m_parameterFlags[p] = flags;
}

String_t
game::spec::Mission::getParameterName(MissionParameter p) const
{
    // ex GMission::getParameterName
    // Note that PCC2 does not translate these words, so neither do we.
    if (m_parameterNames[p].empty()) {
        if (p == TowParameter) {
            return "Tow";
        } else {
            return "Intercept";
        }
    } else {
        return m_parameterNames[p];
    }
}

void
game::spec::Mission::setParameterName(MissionParameter p, String_t name)
{
    // ex GMission::setParameterName
    m_parameterNames[p] = name;
}


/*
 *  Script accessors
 */

String_t
game::spec::Mission::getConditionExpression() const
{
    // ex GMission::getConditionExpression
    return m_conditionExpression;
}

void
game::spec::Mission::setConditionExpression(String_t cond)
{
    // ex GMission::setConditionExpression
    m_conditionExpression = cond;
}

String_t
game::spec::Mission::getWarningExpression() const
{
    // ex GMission::getWarningExpression
    return m_warningExpression;
}

void
game::spec::Mission::setWarningExpression(String_t warning)
{
    // ex GMission::setWarningExpression
    m_warningExpression = warning;
}

String_t
game::spec::Mission::getLabelExpression() const
{
    // ex GMission::getLabelExpression
    return m_labelExpression;
}

void
game::spec::Mission::setLabelExpression(String_t label)
{
    // ex GMission::setLabelExpression
    m_labelExpression = label;
}

String_t
game::spec::Mission::getSetCommand() const
{
    // ex GMission::getSetCommand
    return m_setCommand;
}

void
game::spec::Mission::setSetCommand(String_t cmd)
{
    // ex GMission::setSetCommand
    m_setCommand = cmd;
}


/*
 *  Inquiry
 */

bool
game::spec::Mission::worksOn(const game::map::Ship& ship, const game::config::HostConfiguration& config, const HostVersion& host, const RegistrationKey& key) const
{
    // ex GMission::worksOn, mission.pas:CanDoMission
    // @change We do not check the condition; that is done in script code.
    // FIXME: Consider retiring this function.
    // Ship owner needs to be known and valid.
    const int shipOwner = ship.getRealOwner().orElse(0);
    if (shipOwner == 0) {
        return false;
    }

    // Don't allow mission 1 for SRace
    if (!host.isMissionAllowed(m_number)) {
        return false;
    }

    // Check race mask
    if (!m_raceMask.contains(config.getPlayerMissionNumber(shipOwner))) {
        return false;
    }

    // Check registration status
    if (m_flags.contains(RegisteredMission) && key.getStatus() == RegistrationKey::Unregistered) {
        return false;
    }

    // Check waypoint permission
    if (m_flags.contains(WaypointMission) && ship.isFleetMember()) {
        return false;
    }

    return true;
}

/** Parse mission.cc description line.
    This will set all affected members according to that definition.

    \param descriptionLine line from mission.cc, sans mission number (i.e. "!h#,Tow a ship") */
void
game::spec::Mission::parseDescription(const String_t& descriptionLine)
{
    // ex GMission::parseDescription, mission.pas:ReadMissionCC (part)
    String_t::size_type n = 0;

    ParameterType paramType = IntegerParameter;
    ParameterFlagSet_t paramFlags;
    while (n < descriptionLine.length() && descriptionLine[n] != ',') {
        switch (descriptionLine[n++]) {
         case '+': m_raceMask  = parsePlayerList(descriptionLine, n); break;
         case '-': m_raceMask  = parsePlayerList(descriptionLine, n) ^ PlayerSet_t::allUpTo(MAX_RACES); break;
         case 'r': m_flags    += RegisteredMission; break;
         case 'i': m_flags    += WaypointMission; break;
         case 'o': paramFlags += OwnParameter; break;
         case '!': paramFlags += NotThisParameter; break;
         case 'n': paramType  = IntegerParameter; break; // new
         case 'p': paramType  = PlanetParameter; break;
         case 's': paramType  = ShipParameter; break;
         case 'h': paramType  = HereParameter; break;
         case 'b': paramType  = BaseParameter; break;
         case 'y': paramType  = PlayerParameter; break;
         case '*':
            m_parameterTypes[InterceptParameter] = paramType;
            m_parameterFlags[InterceptParameter] = paramFlags;
            break;
         case '#':
            m_parameterTypes[TowParameter] = paramType;
            m_parameterFlags[TowParameter] = paramFlags;
            break;
        }
    }
    if (n < descriptionLine.length()) {
        m_name = descriptionLine.substr(n+1);

        /* assign hot-key if so desired */
        String_t::size_type p = m_name.find('~');
        if (p != m_name.npos && p+1 < m_name.length()) {
            /* "~x" assigns a hot-key. Only standard keys so far. */
            char key = afl::string::charToLower(m_name[p+1]);
            if (key > ' ' && key < 127) {
                m_hotkey = key;
            }
            m_name.erase(p, 1);
        }

        /* assign standard short-name */
        m_shortName.assign(m_name, 0, 7);
    }
}
