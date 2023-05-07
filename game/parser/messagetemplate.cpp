/**
  *  \file game/parser/messagetemplate.cpp
  *  \brief Class game::parser::MessageTemplate
  */

#include "game/parser/messagetemplate.hpp"
#include "afl/string/format.hpp"
#include "util/string.hpp"
#include "afl/string/parse.hpp"
#include "game/parser/datainterface.hpp"

using afl::string::strUCase;
using afl::string::strTrim;
using afl::string::strCaseCompare;


namespace {
    /** Format a number into a string.
        We cannot use numToString, because that honors user configuration
        and might mangle the values to be not parseable. */
    String_t formatNumber(int32_t i)
    {
        return afl::string::Format("%d", i);
    }

    /** Parse integer value according to a type modifier.
        \param value Value from message
        \param type  Type modifier from template
        \param iface Data interface
        \return transformed string (usually, a stringified integer)

        \todo Implement sloppy compare. Text from messages can have Unicode
        characters replaced by spaces by host. It might also have been
        truncated. The same goes for hull names.

        \todo It would make sense to detect when the player's files do not
        match host's. However, we must be really sure about this: in a game
        with fewer than 11 players, the FreeFighters template will try to
        interpret config strings like "max mine radius 150" as name/count
        pairs. */
    String_t prepareValue(String_t value, String_t type, const game::parser::DataInterface& iface)
    {
        if (type.size() > 7 && type.compare(type.size()-7, 7, "+ALLIES", 7) == 0) {
            /* Value includes an alliance marker, but we just want the plain name.
               The message line reads "Bird Man ! :    0".
                 !     => this race has offered something to us
                 +     => we have offered something
                 :     => since HOST sometimes drops the colon, we tell the matching engine
                          to include it in the name, which means we must strip it here
               Parsing of the actual alliances is done in msgparse.cc, generateFlagAllies(). */
            String_t::size_type n = value.size();
            while (n > 0 && (value[n-1] == '+' || value[n-1] == '!' || value[n-1] == ':' || value[n-1] == ' ')) {
                --n;
            }
            value.erase(n);
            type.erase(type.size()-7);
        }

        value = strTrim(value);
        if (type == "RACE") {
            /* Full race name */
            if (int i = iface.parseName(iface.LongRaceName, value)) {
                return formatNumber(i);
            } else {
                return String_t();
            }
        } else if (type == "RACE.SHORT") {
            /* Short race name */
            if (int i = iface.parseName(iface.ShortRaceName, value)) {
                return formatNumber(i);
            } else {
                return String_t();
            }
        } else if (type == "RACE.ADJ") {
            /* Race adjective name */
            if (int i = iface.parseName(iface.AdjectiveRaceName, value)) {
                return formatNumber(i);
            } else {
                return String_t();
            }
        } else if (type == "HULL") {
            /* Hull name */
            if (int i = iface.parseName(iface.HullName, value)) {
                return formatNumber(i);
            } else {
                return String_t();
            }
        } else if (type == "X10" || type == "X100" || type == "X1000") {
            /* Scaled integer */
            String_t::size_type start = (value.empty() || (value[0] != '+' && value[0] != '-')) ? 0 : 1;
            String_t::size_type n = value.find_first_not_of("0123456789", start);
            if (n == String_t::npos) {
                /* completely numeric */
                return value + type.substr(2);
            } else if (value[n] == '.') {
                /* includes decimal point */
                value.erase(n, 1);
                String_t::size_type i = 0;
                while (n+i < value.size() && i+2 < type.size() && value[n+i] >= '0' && value[n+i] <= '9') {
                    ++i;
                }
                value.erase(n+i);
                while (i+2 < type.size()) {
                    value += '0', ++i;
                }
                return value;
            } else {
                /* no decimal point */
                return value.substr(0, n) + type.substr(2);
            }
        } else {
            String_t::size_type n = type.find('/');
            if (n != type.npos) {
                /* Enum */
                String_t::size_type p = 0;
                int i = 0;
                while (n != type.npos) {
                    if (strCaseCompare(value, strTrim(String_t(type, p, n-p))) == 0) {
                        return formatNumber(i);
                    }
                    ++i;
                    p = n+1;
                    n = type.find('/', p);
                }
                if (strCaseCompare(value, strTrim(String_t(type, p))) == 0) {
                    return formatNumber(i);
                }
                return String_t();
            } else {
                /* Unparsed */
                return value;
            }
        }
    }
}

// Constructor.
game::parser::MessageTemplate::MessageTemplate(MessageInformation::Type type, String_t name)
    : m_messageType(type),
      m_name(name),
      m_continueFlag(false),
      m_instructions(),
      m_strings(),
      m_variables(),
      m_types()
{ }

// Destructor.
game::parser::MessageTemplate::~MessageTemplate()
{ }

// Add "match" instruction.
void
game::parser::MessageTemplate::addMatchInstruction(uint8_t opcode, uint16_t value)
{
    // ex GMessageTemplate::addMatchInstruction
    m_instructions.push_back(Instruction(opcode, 0, value, 0));
}

// Add "value" instruction.
void
game::parser::MessageTemplate::addValueInstruction(uint8_t opcode, String_t value)
{
    // ex GMessageTemplate::addValueInstruction
    String_t::size_type p = 0, n;
    for (n = value.find(','); n != String_t::npos; p = n+1, n = value.find(',', p)) {
        m_instructions.push_back(Instruction(opcode, 0, uint16_t(m_strings.size()), 0));
        m_strings.push_back(strTrim(String_t(value, p, n-p)));
    }
    m_instructions.push_back(Instruction(opcode, 0, uint16_t(m_strings.size()), 0));
    m_strings.push_back(strTrim(String_t(value, p)));
}

// Add "check" instruction.
void
game::parser::MessageTemplate::addCheckInstruction(uint8_t opcode, int8_t offset, String_t value)
{
    // ex GMessageTemplate::addCheckInstruction
    m_instructions.push_back(Instruction(opcode, offset, uint16_t(m_strings.size()), 0));
    if ((opcode & iMask) == iParse || (opcode & iMask) == iArray) {
        String_t::size_type p = 0, n;
        for (n = value.find('$'); n != String_t::npos; p = n+1, n = value.find('$', p)) {
            m_strings.push_back(String_t(value, p, n-p));
            ++m_instructions.back().count;
        }
        m_strings.push_back(String_t(value, p));
    } else {
        m_strings.push_back(value);
    }
}

// Add a single variable.
void
game::parser::MessageTemplate::addVariable(String_t name)
{
    // ex GMessageTemplate::addVariable
    String_t type;
    afl::string::strSplit(name, name, type, ":");
    m_variables.push_back(strTrim(strUCase(name)));
    m_types.push_back(strTrim(strUCase(type)));
}

// Add list of variables.
void
game::parser::MessageTemplate::addVariables(String_t names)
{
    // ex GMessageTemplate::addVariables
    String_t::size_type p = 0, n;
    for (n = names.find(','); n != String_t::npos; p = n+1, n = names.find(',', p)) {
        addVariable(String_t(names, p, n-p));
    }
    addVariable(String_t(names, p));
}

// Set continuation flag.
void
game::parser::MessageTemplate::setContinueFlag(bool flag)
{
    // ex GMessageTemplate::setContinueFlag
    m_continueFlag = flag;
}

// Get number of variables.
size_t
game::parser::MessageTemplate::getNumVariables() const
{
    // ex GMessageTemplate::getNumVariables
    return m_variables.size();
}

// Get number of wildcards.
size_t
game::parser::MessageTemplate::getNumWildcards() const
{
    // ex GMessageTemplate::getNumWildcards
    size_t total = 0;
    for (std::vector<Instruction>::const_iterator i = m_instructions.begin(); i != m_instructions.end(); ++i) {
        if (i->opcode == iValue || (i->opcode & iMask) == iFind) {
            ++total;
        } else if ((i->opcode & iMask) == iParse || (i->opcode & iMask) == iArray) {
            total += i->count;
        }
    }
    return total;
}

// Get number of restrictions.
size_t
game::parser::MessageTemplate::getNumRestrictions() const
{
    // ex GMessageTemplate::getNumRestrictions
    size_t total = 0;
    for (std::vector<Instruction>::const_iterator i = m_instructions.begin(); i != m_instructions.end(); ++i) {
        if (i->opcode != iValue) {
            ++total;
        }
    }
    return total;
}

// Find variable slot by name.
afl::base::Optional<size_t>
game::parser::MessageTemplate::getVariableSlotByName(const String_t name) const
{
    // ex GMessageTemplate::getVariableSlotByName
    for (size_t i = 0; i < m_variables.size(); ++i) {
        if (name == m_variables[i]) {
            return i;
        }
    }
    return afl::base::Nothing;
}

// Get variable name by index.
String_t
game::parser::MessageTemplate::getVariableName(size_t index) const
{
    // ex GMessageTemplate::getVariableName
    if (index < m_variables.size()) {
        return m_variables[index];
    } else {
        return String_t();
    }
}

// Get name of template (as set in constructor).
String_t
game::parser::MessageTemplate::getTemplateName() const
{
    // ex GMessageTemplate::getTemplateName
    return m_name;
}

// Match message against this template.
bool
game::parser::MessageTemplate::match(const MessageLines_t& message, const DataInterface& iface, std::vector<String_t>& values) const
{
    // ex GMessageTemplate::match
    size_t line = 0;
    for (std::vector<Instruction>::const_iterator it = m_instructions.begin(); it != m_instructions.end(); ++it) {
        uint8_t opcode = it->opcode;
        uint8_t group  = opcode & iMask;
        if (opcode == iMatchKind) {
            /* Match MsgHdrKind */
            if (getMessageHeaderInformation(message, MsgHdrKind) != it->index) {
                return false;
            }
        } else if (opcode == iMatchSubId) {
            /* Match MsgHdrSubId */
            if (getMessageHeaderInformation(message, MsgHdrSubId) != it->index) {
                return false;
            }
        } else if (opcode == iMatchBigId) {
            /* Match MsgHdrBigId */
            if (getMessageHeaderInformation(message, MsgHdrBigId) != it->index) {
                return false;
            }
        } else if (opcode == iValue) {
            /* Unconditionally produce a value */
            const String_t& s = m_strings[it->index];
            if (strCaseCompare(s, "player") == 0) {
                /* Player has been given by the caller */
                values.push_back(formatNumber(iface.getPlayerNumber()));
            } else if (strCaseCompare(s, "id") == 0) {
                /* Id is an integer */
                values.push_back(formatNumber(getMessageHeaderInformation(message, MsgHdrId)));
            } else if (strCaseCompare(s, "bigid") == 0) {
                /* BigId is an integer */
                values.push_back(formatNumber(getMessageHeaderInformation(message, MsgHdrBigId)));
            } else if (strCaseCompare(s, "subid") == 0) {
                /* The message header information is a character.
                   This is intended for the case where the character is a race number (0-9, a, b), and we want an integer. */
                // Change from PCC2: this will produce c->12 for colonists.
                int nr;
                if (!util::parsePlayerCharacter(static_cast<char>(getMessageHeaderInformation(message, MsgHdrSubId)), nr)) {
                    nr = 0;
                }
                values.push_back(formatNumber(nr));
            } else {
                values.push_back(s);
            }
        } else if (group == iCheck) {
            /* Fail if string not found */
            if (!check(message, line, *it, iface)) {
                return false;
            }
        } else if (group == iFail) {
            /* Fail if string found */
            if (check(message, line, *it, iface)) {
                return false;
            }
        } else if (group == iFind) {
            /* Remember found status */
            if (check(message, line, *it, iface)) {
                values.push_back("1");
            } else {
                values.push_back("0");
            }
        } else if (group == iParse || group == iArray) {
            /* Parse or Array: start by locating the match */
            size_t typeIndex = values.size();
            if ((opcode & ~iMask) == sAny) {
                for (MessageLines_t::size_type i = 0; i < message.size(); ++i) {
                    if (matchLine(message[i], it->index, it->count, typeIndex, values, iface)) {
                        line = i;
                        goto ok;
                    }
                }
                return false;
             ok:;
            } else {
                if ((opcode & ~iMask) == sRelative) {
                    line += it->offset;
                } else {
                    line = it->offset - 1;
                }
                if (line >= message.size()) {
                    return false;
                }
                if (!matchLine(message[line], it->index, it->count, typeIndex, values, iface)) {
                    return false;
                }
            }

            /* If it's an array, read the additional lines */
            if (group == iArray) {
                size_t nelems = 1;
                while (int(nelems) < NUM_PLAYERS && line+1U < message.size() && matchLine(message[line+1], it->index, it->count, typeIndex, values, iface)) {
                    ++nelems, ++line;
                }
                consolidateArray(values, it->count, nelems);
            }
        } else {
            return false;
        }
    }
    return true;
}

// Check for a string.
bool
game::parser::MessageTemplate::check(const MessageLines_t& message, size_t& line, const Instruction& insn, const DataInterface& iface) const
{
    // ex GMessageTemplate::check
    String_t needle = strUCase(iface.expandRaceNames(m_strings[insn.index]));
    if ((insn.opcode & ~iMask) == sAny) {
        /* Check all lines */
        for (MessageLines_t::size_type i = 0; i < message.size(); ++i) {
            if (strUCase(message[i]).find(needle) != String_t::npos) {
                /* Found it */
                line = i;
                return true;
            }
        }
        return false;
    } else {
        /* Check just one line */
        size_t lineToCheck;
        if ((insn.opcode & ~iMask) == sRelative) {
            lineToCheck = line + insn.offset;
        } else {
            lineToCheck = insn.offset - 1;    /* user gives position 1-based */
        }
        if (lineToCheck < message.size() && strUCase(message[lineToCheck]).find(needle) != String_t::npos) {
            line = lineToCheck;
            return true;
        } else {
            return false;
        }
    }
}

// Match a single line.
bool
game::parser::MessageTemplate::matchLine(const String_t& line, size_t index, size_t nvar, size_t typeIndex, std::vector<String_t>& values, const DataInterface& iface) const
{
    // ex GMessageTemplate::matchLine
    String_t needle = strUCase(iface.expandRaceNames(m_strings[index]));
    String_t::size_type pos = (needle.empty() ? 0 : strUCase(line).find(needle));
    if (pos != String_t::npos) {
        if (matchPart(line, pos + needle.size(), index+1, nvar, values, iface)) {
            /* Postprocess values */
            size_t valueIndex = values.size() - nvar;
            for (size_t i = 0; i < nvar && typeIndex < m_types.size(); ++i, ++typeIndex, ++valueIndex) {
                values[valueIndex] = prepareValue(values[valueIndex], m_types[typeIndex], iface);
            }
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

// Match a partial line.
bool
game::parser::MessageTemplate::matchPart(const String_t& line, size_t startAt, size_t index, size_t nvar, std::vector<String_t>& values, const DataInterface& iface) const
{
    // ex GMessageTemplate::matchPart
    /* Special cases: */
    if (nvar == 0) {
        return true;
    }
    if (nvar == 1 && m_strings[index].empty()) {
        values.push_back(line.substr(startAt));
        return true;
    }

    /* Build initial hypothesis */
    String_t needle = strUCase(iface.expandRaceNames(m_strings[index]));
    String_t::size_type pos = strUCase(line).rfind(needle);
    if (pos <= startAt || pos == String_t::npos) {
        return false;
    }
    values.push_back(String_t(line, startAt, pos - startAt));

    /* Check subsequent patterns, backtrack if needed */
    while (values.back().find_first_not_of(' ') == String_t::npos || !matchPart(line, pos + needle.size(), index+1, nvar-1, values, iface)) {
        if (pos == 0) {
            /* We cannot backtrack */
            values.pop_back();
            return false;
        }

        String_t::size_type npos = strUCase(line).rfind(needle, pos-1);
        if (npos <= startAt || npos >= pos) {
            values.pop_back();
            return false;
        }
        values.back().assign(line, startAt, npos - startAt);
        pos = npos;
    }
    return true;
}

// Consolidate an array.
void
game::parser::MessageTemplate::consolidateArray(std::vector<String_t>& values, size_t nvar, size_t nelems) const
{
    // ex GMessageTemplate::consolidateArray
    const size_t nil = size_t(-1);
    size_t firstIndex = values.size() - nvar*nelems;

    /* Figure out whether there's an element which defines the index. */
    size_t playerIndex = nil;
    for (size_t i = 0; i < nvar && i+firstIndex < m_variables.size(); ++i) {
        if (m_variables[i+firstIndex] == "INDEX") {
            playerIndex = i;
            break;
        }
    }

    /* Build all elements. We always build 11-element arrays. */
    for (size_t var = 0; var < nvar; ++var) {
        /* This one's special */
        if (var == playerIndex) {
            continue;
        }

        /* Parse individual items. Unset items remain blank. */
        String_t data[NUM_PLAYERS];
        for (size_t i = 0; i < nelems; ++i) {
            int slot;
            if (playerIndex == nil) {
                slot = static_cast<int>(i);
            } else {
                slot = parseIntegerValue(values[playerIndex + i*nvar + firstIndex]) - 1;
            }

            if (slot >= 0 && slot < NUM_PLAYERS) {
                data[slot] = strTrim(values[var + i*nvar + firstIndex]);
            }
        }

        /* Build new value */
        for (int i = 1; i < NUM_PLAYERS; ++i) {
            data[0] += ',';
            data[0] += data[i];
        }
        values[var+firstIndex] = data[0];
    }

    /* Clear player index to avoid that anyone uses it */
    if (playerIndex != nil) {
        values[playerIndex+firstIndex].clear();
    }

    /* Remove excess values */
    values.resize(firstIndex + nvar);
}

// Split message into lines.
void
game::parser::splitMessage(MessageLines_t& out, const String_t& in)
{
    // ex game/msgtemplate.cc:splitMessage
    String_t::size_type i = 0;
    for (String_t::size_type n = in.find('\n'); n != in.npos; i = n+1, n = in.find('\n', i)) {
        out.push_back(String_t(in, i, n-i));
    }
    out.push_back(String_t(in, i));
}

// Extract information from message header.
int32_t
game::parser::getMessageHeaderInformation(const MessageLines_t& msg, MessageHeaderInformation what)
{
    // ex game/msgtemplate.cc:getMessageHeaderInformation

    // Must be at least one line, and at least 5 characters "(-x0)"
    if (msg.empty() || msg[0].size() < 5) {
        return 0;
    }

    // Check format
    if (msg[0][0] != '(' || (msg[0][1] != '-' && msg[0][1] != 'o')) {
        return 0;
    }

    // Kind, Age or SubId
    if (what == MsgHdrAge) {
        return msg[0][1] != '-';
    }
    if (what == MsgHdrKind) {
        return msg[0][2];
    }
    if (what == MsgHdrSubId) {
        return msg[0][3];
    }

    // Id or BigId
    int result = 0;
    String_t::size_type start = (what == MsgHdrBigId ? 3 : 4);
    while (start < msg[0].size() && msg[0][start] != ')') {
        char ch = msg[0][start];
        if (ch >= '0' && ch <= '9') {
            result = 10*result + ch - '0';
        }
        ++start;
    }
    return result;
}

// Parse integer value.
int32_t
game::parser::parseIntegerValue(const String_t& value)
{
    // ex game/msgtemplate.cc:getIntValue, game/msgtemplate.cc:parseIntValue, readmsg.pas:Eval
    // No need to parse YES/NO here. This is used in configuration parsing;
    // in PCC2, it goes through MessageConfigurationValue_t, and thus through BooleanValueParser.
    String_t::size_type pos;
    int32_t result;
    if (afl::string::strToInteger(value, result, pos) || afl::string::strToInteger(value.substr(0, pos), result)) {
        return result;
    } else {
        return -1;
    }
}
