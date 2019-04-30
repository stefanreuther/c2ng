/**
  *  \file game/spec/mission.cpp
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

// /** Construct blank mission definition. */
game::spec::Mission::Mission()
    : m_number(0),
      m_raceMask(),              // FIXME?
      m_flags(),
      name(),
      short_name(),
      exp_condition(),
      exp_warning(),
      exp_label(),
      cmd_onset(),
      hotkey(0)
{
    // ex GMission::GMission()
    m_parameterTypes[InterceptParameter] = m_parameterTypes[TowParameter] = NoParameter;
}

// /** Construct mission definition from mission.cc line. */
game::spec::Mission::Mission(int number, String_t descriptionLine)
    : m_number(number),
      m_raceMask(PlayerSet_t::allUpTo(MAX_RACES)),
      m_flags(),
      name(),
      short_name(),
      exp_condition(),
      exp_warning(),
      exp_label(),
      cmd_onset(),
      hotkey(0)
{
    // ex GMission::GMission(int number, string_t desc_line)
    m_parameterTypes[InterceptParameter] = m_parameterTypes[TowParameter] = NoParameter;
    parseDescription(descriptionLine);
}

game::spec::Mission::~Mission()
{ }

/** Get mission number. */
int
game::spec::Mission::getNumber() const
{
    // ex GMission::getNumber
    return m_number;
}

/** Get races which can do this mission.
    (This is indeed races, not players!).
    \return set of race numbers */
game::PlayerSet_t
game::spec::Mission::getRaceMask() const
{
    // ex GMission::getRaceMask
    return m_raceMask;
}

/** Set races which can do this mission. */
void
game::spec::Mission::setRaceMask(PlayerSet_t mask)
{
    // ex GMission::setRaceMask
    m_raceMask = mask;
}

/** Get mission flags. \return mf_XXX */
game::spec::Mission::FlagSet_t
game::spec::Mission::getFlags() const
{
    // ex GMission::getFlags
    return m_flags;
}

/** Check mission flag.
    \param i mf_XXX */
bool
game::spec::Mission::hasFlag(Flag flag) const
{
    // ex GMission::hasFlag
    return m_flags.contains(flag);
}

/** Set mission flags.
    \param fl Set of mf_XXX */
void
game::spec::Mission::setFlags(FlagSet_t flags)
{
    // ex GMission::setFlags
    m_flags = flags;
}

/** Get mission name. This name should be displayed in selection lists. */
String_t
game::spec::Mission::getName() const
{
    // ex GMission::getName
    return name;
}

/** Set mission name. This name is displayed in selection lists. */
void
game::spec::Mission::setName(String_t name)
{
    // ex GMission::setName
    this->name = name;
}

/** Get short mission name. This name is used when space is tight. */
String_t
game::spec::Mission::getShortName() const
{
    // ex GMission::getShortName
    return short_name;
}

/** Set short mission name. This name is used when space is tight. */
void
game::spec::Mission::setShortName(String_t short_name)
{
    // ex GMission::setShortName
    this->short_name = short_name;
}

/** Get assigned hot-key.
    The hot-key is an ASCII code (7-bit) for a key to select this mission. */
char
game::spec::Mission::getHotkey() const
{
    // ex GMission::getHotkey
    return hotkey;
}

/** Assign hot-key.
    The hot-key is an ASCII code (7-bit) for a key to select this mission. */
void
game::spec::Mission::setHotkey(char c)
{
    // ex GMission::setHotkey
    hotkey = c;
}

/** Get parameter definition.
    \param id which parameter you want to know about
    \return ma_XXX, mat_XXX. Zero if parameter is not required. */
game::spec::Mission::ParameterType
game::spec::Mission::getParameterType(MissionParameter p) const
{
    // ex GMission::getParameterType (sort-of)
    return m_parameterTypes[p];
}

/** Set parameter type.
    \param id which parameter
    \param spec parameter type, combination of ma_XXX, mat_XXX */
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

/** Get name for parameter. */
String_t
game::spec::Mission::getParameterName(MissionParameter id) const
{
    // ex GMission::getParameterName
    // Note that PCC2 does not translate these words, so neither do we.
    if (arg_names[id].empty()) {
        if (id == TowParameter) {
            return "Tow";
        } else {
            return "Intercept";
        }
    } else {
        return arg_names[id];
    }
}

/** Set parameter name.
    This name is used when PCC2 prompts for a name.
    It can be blank to invoke the default. */
void
game::spec::Mission::setParameterName(MissionParameter id, String_t name)
{
    // ex GMission::setParameterName
    arg_names[id] = name;
}

/** Get condition. */
String_t
game::spec::Mission::getConditionExpression() const
{
    // ex GMission::getConditionExpression
    return exp_condition;
}

/** Set condition.
    This condition verifies whether the mission is allowed to be set (hard condition).
    \see worksOn(GShip&) */
void
game::spec::Mission::setConditionExpression(String_t cond)
{
    // ex GMission::setConditionExpression
    exp_condition = cond;
}

/** Get warning condition. */
String_t
game::spec::Mission::getWarningExpression() const
{
    // ex GMission::getWarningExpression
    return exp_warning;
}

/** Set warning condition.
    This condition verifies whether the mission will work (soft condition).
    \see isWarning(GShip&) */
void
game::spec::Mission::setWarningExpression(String_t warning)
{
    // ex GMission::setWarningExpression
    exp_warning = warning;
}

/** Get label expression. */
String_t
game::spec::Mission::getLabelExpression() const
{
    // ex GMission::getLabelExpression
    return exp_label;
}

/** Set label expression.
    This expression produces a string to display when the mission is set on a ship.
    \see getLabel(GShip&) */
void
game::spec::Mission::setLabelExpression(String_t label)
{
    // ex GMission::setLabelExpression
    exp_label = label;
}

/** Get "on set" command. */
String_t
game::spec::Mission::getSetCommand() const
{
    // ex GMission::getSetCommand
    return cmd_onset;
}

/** Set "on set" command. */
void
game::spec::Mission::setSetCommand(String_t cmd)
{
    // ex GMission::setSetCommand
    cmd_onset = cmd;
}

// /** True iff mission (might) work on the specified ship. Actually,
//     this should return true if we want the user to be able to set the
//     mission on the specified ship. */
bool
game::spec::Mission::worksOn(const game::map::Ship& ship, const game::config::HostConfiguration& config, const HostVersion& host, const RegistrationKey& key) const
{
    // ex GMission::worksOn
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

    // FIXME/@change: we do not check the expression condition
    // // Check mission.cc condition
    // IntBCORef bco = compileExpression(exp_condition, IntCompilationContext());
    // if (bco) {
    //     // Run it
    //     IntExecutionContext exec("Temporary: Mission");
    //     exec.pushNewContext(new IntShipContext(ship.getId()));
    //     exec.pushNewFrame(new IntExecutionFrame(bco));
    //     if (runTemporaryProcess(exec)) {
    //         // Evaluated successfully
    //         if (getBoolValue(exec.getResult()) <= 0) {
    //             return false;
    //         }
    //     } else {
    //         // Mission is not permitted if expression fails (same as in PCC 1.x)
    //         return false;
    //     }
    // }

    return true;
}

/** Parse mission.cc description line.
    This will set all affected members according to that definition.

    \param desc_line line from mission.cc, sans mission number (i.e. "!h#,Tow a ship") */
void
game::spec::Mission::parseDescription(const String_t& descriptionLine)
{
    // ex GMission::parseDescription
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
        name = descriptionLine.substr(n+1);

        /* assign hot-key if so desired */
        String_t::size_type p = name.find('~');
        if (p != name.npos && p+1 < name.length()) {
            /* "~x" assigns a hot-key. Only standard keys so far. */
            char key = afl::string::charToLower(name[p+1]);
            if (key > ' ' && key < 127) {
                hotkey = key;
            }
            name.erase(p, 1);
        }

        /* assign standard short-name */
        short_name.assign(name, 0, 7);
    }
}
