/**
  *  \file game/spec/missionlist.cpp
  */

#include "game/spec/missionlist.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/parse.hpp"
#include "util/string.hpp"
#include "game/limits.hpp"
#include "interpreter/values.hpp"
#include "util/translation.hpp"
#include "game/v3/structures.hpp"

namespace {
    const char LOG_NAME[] = "game.spec.missionlist";

    bool compareMissions(const game::spec::Mission& a, const game::spec::Mission& b)
    {
        // ex game/mission.cc:missionCompare
        if (a.getNumber() < b.getNumber())
            return true;
        if (a.getNumber() > b.getNumber())
            return false;
        return a.getRaceMask().toInteger() < b.getRaceMask().toInteger();
    }

    /** Extract mission parameter from string.
        Updates the parameter name in-place.
        \param msn   [in/out] mission to update
        \param line  [in] mission.ini line
        \param tpos  [in] position of this parameter's token ("I:") in the line
        \param opos  [in] position of other parameter's token ("T:") in the line
        \param limit [in] position after parameters (closing paren) */
    void extractMissionParameter(game::spec::Mission& msn,
                                 game::MissionParameter which,
                                 const String_t& line,
                                 const String_t::size_type tpos,
                                 const String_t::size_type opos,
                                 const String_t::size_type limit)
    {
        if (tpos != line.npos) {
            String_t::size_type tend = limit-1;
            if (opos != line.npos && opos > tpos)
                tend = opos;
            while (line[tend-1] == ' ' || line[tend-1] == ',')
                --tend;
            msn.setParameterName(which, line.substr(tpos+2, tend-tpos-2));
        }
    }
}

/** Construct empty mission list. */
game::spec::MissionList::MissionList()
    : m_data(),
      m_usedLetters(0)
{ }

game::spec::MissionList::~MissionList()
{ }

/*
 *  Container interface
 */

size_t
game::spec::MissionList::size() const
{
    return m_data.size();
}

game::spec::MissionList::Iterator_t
game::spec::MissionList::begin() const
{
    return m_data.begin();
}

game::spec::MissionList::Iterator_t
game::spec::MissionList::end() const
{
    return m_data.end();
}

const game::spec::Mission*
game::spec::MissionList::at(size_t i) const
{
    return i < m_data.size()
         ? &m_data[i]
         : 0;
}

// /** Add mission to list. This will add a (copy of) /msn/ to the
//     mission list. If the mission was already defined, the call is
//     ignored.
//     \return true iff mission was added, false if call was ignored */
bool
game::spec::MissionList::addMission(const Mission& msn)
{
    // ex GMissionList::addMission

    /* do not add if it's already there. A mission is considered
       already there if its race mask is not disjoint to ours. (This
       might sound counter-intuitive, but it's used to refuse
       replacing a MISSION.CC definition by a MISSION.INI one;
       MISSION.CC generally has better race masks and checking for
       subsets would make MISSION.INI always override those.) */
    for (Iterator_t i = begin(); i != end(); ++i) {
        if (i->getNumber() != msn.getNumber())
            continue;
        if (!msn.getRaceMask().containsAnyOf(i->getRaceMask()))
            continue;
        return false;
    }

    /* assign hot-key. Do this without modifying /msn/. */
    char c = msn.getHotkey();
    if (c == 0) {
        if (msn.getNumber() >= 0 && msn.getNumber() < 10) {
            c = '0' + msn.getNumber();
        } else {
            for (int i = 0; i < 26; ++i) {
                if (!(m_usedLetters & (1 << i))) {
                    c = 'a' + i;  // FIXME: assumes ASCII
                    break;
                }
            }
            if (c == 0) {
                /* all letters used up */
                m_usedLetters = 0;
                c = 'a';
            }
        }
    }
    if (c >= 'a' && c <= 'z') {
        m_usedLetters |= 1 << (c - 'a');
    }

    m_data.push_back(msn);
    m_data.back().setHotkey(c);
    return true;
}

/** Sort mission list. Brings list into numerical mission order, which
    will then be used on the mission screen. */
void
game::spec::MissionList::sort()
{
    // ex GMissionList::sort
    std::sort(m_data.begin(), m_data.end(), compareMissions);
}

/** Clear this mission list instance. */
void
game::spec::MissionList::clear()
{
    // ex GMissionList::clear
    m_usedLetters = 0;
    m_data.clear();
}

/** Get mission by number/player.
    \param id mission number
    \param race_mask races (not players!) for which you want the mission. */
const game::spec::Mission*
game::spec::MissionList::getMissionByNumber(int id, PlayerSet_t raceMask) const
{
    // ex GMissionList::getMissionByNumber
    size_t slot;
    if (getIndexByNumber(id, raceMask, slot)) {
        return at(slot);
    } else {
        return 0;
    }
}

bool
game::spec::MissionList::getIndexByNumber(int id, PlayerSet_t raceMask, size_t& index) const
{
    for (size_t i = 0, n = m_data.size(); i < n; ++i) {
        const Mission& msn = m_data[i];
        if (msn.getNumber() == id && msn.getRaceMask().containsAnyOf(raceMask)) {
            index = i;
            return true;
        }
    }
    return false;
}


/** Load mission.cc file.
    \param in      stream */
void
game::spec::MissionList::loadFromFile(afl::io::Stream& in, afl::sys::LogListener& log)
{
    // ex GMissionList::loadFromFile
    afl::io::TextFile tf(in);
    String_t line;
    bool have_mission = false;

    while (tf.readLine(line)) {
        line = afl::string::strTrim(line);
        if (line.empty() || line[0] == ';')
            continue;

        String_t::size_type p = line.find_first_of("=,");
        if (p == String_t::npos) {
            log.write(log.Error, LOG_NAME, in.getName(), tf.getLineNumber(), _("missing delimiter"));
        } else if (line[p] == ',') {
            int mnum;
            if (!afl::string::strToInteger(afl::string::strTrim(line.substr(0, p)), mnum) || mnum < 0 || mnum > 10000) {
                log.write(log.Error, LOG_NAME, in.getName(), tf.getLineNumber(), _("invalid mission number"));
                have_mission = false;
                continue;       // with next mission
            }

            line.erase(0, p+1);
            try {
                have_mission = addMission(Mission(mnum, line));
            }
            // FIXME: was: catch (UserException& e) {
            catch (std::exception& e) {
                log.write(log.Error, LOG_NAME, in.getName(), tf.getLineNumber(), e.what());
            }
        } else {
            /* additional assignments. */
            if (!have_mission)
                continue;

            String_t lhs = afl::string::strTrim(line.substr(0, p));
            line = afl::string::strTrim(line.substr(p+1));
            if (util::stringMatch("I", lhs)) {
                // Intercept name
                m_data.back().setParameterName(InterceptParameter, line);
            } else if (util::stringMatch("J", lhs)) {
                // Tow name
                m_data.back().setParameterName(TowParameter, line);
            } else if (util::stringMatch("Shortname", lhs)) {
                // short mission name
                m_data.back().setShortName(line);
            } else if (util::stringMatch("Condition", lhs)) {
                // condition which says whether this mission is allowed to be set on this ship
                m_data.back().setConditionExpression(line);
            } else if (util::stringMatch("Text", lhs)) {
                // Text to show if mission is set
                m_data.back().setLabelExpression(line);
            } else if (util::stringMatch("Willwork", lhs)) {
                // condition which says whether the mission will work, to warn the user when he sets cloaking on a damaged ship.
                m_data.back().setWarningExpression(line);
            } else if (util::stringMatch("Onset", lhs)) {
                // OnSet hook: command to be called when mission is set
                m_data.back().setSetCommand(line);
            }
        }
    }
}

/** Load from MISSION.INI file. */
void
game::spec::MissionList::loadFromIniFile(afl::io::Stream& in, afl::charset::Charset& cs)
{
    // ex GMissionList::loadFromIniFile
    afl::io::TextFile tf(in);
    tf.setCharsetNew(cs.clone());
    String_t line;
    while (tf.readLine(line)) {
        line = afl::string::strTrim(line);

        String_t::size_type index = line.find(' ');
        if (index == String_t::npos)
            continue;

        int number;
        if (!afl::string::strToInteger(line.substr(0, index), number) || number < 10)
            continue;

        /* Start building the mission */
        Mission new_mission(number, String_t());

        /* "/123" race flags */
        line.erase(0, index+1);
        index = line.rfind('/');
        if (index != line.npos) {
            PlayerSet_t player_restriction;
            String_t::size_type num = 1;
            while (index+num < line.size()) {
                // mission.ini is a v3 thing, so we limit to v3 races
                int raceNr = 0;
                if (util::parsePlayerCharacter(line[index+num], raceNr) && raceNr > 0 && raceNr <= int(game::v3::structures::NUM_PLAYERS)) {
                    player_restriction += raceNr;
                    ++num;
                } else {
                    break;
                }
            }
            if (!player_restriction.empty()) {
                new_mission.setRaceMask(player_restriction);
                line.erase(index, num);
            }
        }

        /* "*#" parameter flags */
        index = line.size();
        while (index > 0) {
            if (line[index-1] == '*') {
                new_mission.setParameterType(InterceptParameter, Mission::IntegerParameter);
                --index;
            } else if (line[index-1] == '#') {
                new_mission.setParameterType(TowParameter, Mission::IntegerParameter);
                --index;
            } else if (line[index-1] == ' ') {
                --index;
            } else if (line[index-1] == ')') {
                /* parenthesized expression: "(I:Foo T:Bar)" */
                String_t::size_type beg = line.rfind('(');
                if (beg == line.npos)
                    break;

                /* Find "I:" and "T:" tokens */
                String_t::size_type ipos = afl::string::strUCase(line).find("I:", beg);
                String_t::size_type tpos = afl::string::strUCase(line).find("T:", beg);

                /* Extract "I:" and "T:" */
                extractMissionParameter(new_mission, InterceptParameter, line, ipos, tpos, index);
                extractMissionParameter(new_mission, TowParameter,       line, tpos, ipos, index);

                /* Trim whitespace and exit */
                index = beg;
                while (index > 0 && line[index-1] == ' ')
                    --index;
                break;
            } else {
                break;
            }
        }
        line.erase(index);

        /* Synthesize "active" expression. */
        bool hasIntercept = new_mission.getParameterType(InterceptParameter) != Mission::NoParameter;
        bool hasTow = new_mission.getParameterType(TowParameter) != Mission::NoParameter;
        if (hasIntercept || hasTow) {
            String_t expr = interpreter::quoteString(line + " (");
            if (hasIntercept) {
                expr += "&'I:'&Mission.Intercept";
            }
            if (hasTow) {
                if (hasIntercept) {
                    expr += "&' T:'";
                } else {
                    expr += "&'T:'";
                }
                expr += "&Mission.Tow";
            }
            expr += "&')'";
            new_mission.setLabelExpression(expr);
        }

        /* Add it */
        new_mission.setName(line);
        addMission(new_mission);
    }
}
