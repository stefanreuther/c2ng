/**
  *  \file game/spec/basichullfunctionlist.cpp
  *  \brief Class game::spec::BasicHullFunctionList
  */

#include "game/spec/basichullfunctionlist.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/string/string.hpp"
#include "game/spec/hull.hpp"
#include "util/fileparser.hpp"
#include "util/string.hpp"

namespace {
    /** Parser for 'hullfunc.cc'. */
    class BasicHullFunctionReader : public util::FileParser {
     public:
        BasicHullFunctionReader(game::spec::BasicHullFunctionList& list, afl::string::Translator& tx, afl::sys::LogListener& log)
            : util::FileParser(";#"),
              m_list(list),
              m_translator(tx),
              m_log(log),
              m_lastFunction(0),
              m_lastFunctionWasBogus(false)
            { }

        virtual void handleLine(const String_t& fileName, int lineNr, String_t line);
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line);

     private:
        /** Reference to data store. */
        game::spec::BasicHullFunctionList& m_list;

        /** Translator. */
        afl::string::Translator& m_translator;

        /** Log target. */
        afl::sys::LogListener& m_log;

        /** Last seen hull function. Null if none yet. */
        game::spec::BasicHullFunction* m_lastFunction;

        /** If set, the last hull function line was bogus, and we're ignoring text until the next one.
            This is used to avoid generating excessive error messages. */
        bool m_lastFunctionWasBogus;

        void handleError(const String_t& fileName, int lineNr, String_t message);
    };
}

void
BasicHullFunctionReader::handleLine(const String_t& fileName, int lineNr, String_t line)
{
    // ex GHullFunctionReader::process
    line = afl::string::strTrim(line);

    String_t::size_type pos = line.find_first_of("=,");
    if (pos == String_t::npos) {
        handleError(fileName, lineNr, m_translator.translateString("Syntax error"));
        return;
    }

    if (line[pos] == ',') {
        // It's a new function
        String_t flags, name;
        String_t number = afl::string::strRTrim(line.substr(0, pos));
        int      parsedNumber;

        line.erase(0, pos+1);
        if (!afl::string::strSplit(line, flags, name, ",")) {
            handleError(fileName, lineNr, m_translator.translateString("Syntax error"));
            return;
        }
        name = afl::string::strTrim(name);

        if (!afl::string::strToInteger(number, parsedNumber) || parsedNumber < 0 || parsedNumber > 0x7FFF) {
            m_lastFunctionWasBogus = true;
            handleError(fileName, lineNr, afl::string::Format(m_translator.translateString("Invalid device number for \"%s\"").c_str(), name));
            return;
        }

        // avoid dupes
        if (m_list.getFunctionByName(name, false) != 0) {
            m_lastFunctionWasBogus = true;
            handleError(fileName, lineNr, afl::string::Format(m_translator.translateString("Duplicate definition for hull function with name \"%s\"").c_str(), name));
            return;
        }
        if (m_list.getFunctionById(parsedNumber) != 0) {
            m_lastFunctionWasBogus = true;
            handleError(fileName, lineNr, afl::string::Format(m_translator.translateString("Duplicate definition for hull function #%d").c_str(), parsedNumber));
            return;
        }
        m_lastFunction = m_list.addFunction(parsedNumber, name);
        m_lastFunctionWasBogus = false;
    } else {
        // it's an assignment
        if (m_lastFunctionWasBogus) {
            return;
        }
        if (m_lastFunction == 0) {
            handleError(fileName, lineNr, m_translator.translateString("Expected function definition"));
            return;
        }

        String_t name  = afl::string::strTrim(line.substr(0, pos));
        String_t value = afl::string::strTrim(line.substr(pos+1));
        if (util::stringMatch("Implies", name)) {
            // 'i' takes device number or name
            int number;
            trimComments(value);
            if (!afl::string::strToInteger(value, number)) {
                if (const game::spec::BasicHullFunction* hf = m_list.getFunctionByName(value, false)) {
                    number = hf->getId();
                } else {
                    handleError(fileName, lineNr, afl::string::Format(m_translator.translateString("Unknown hull function \"%s\"").c_str(), value));
                    return;
                }
            }
            if (number != m_lastFunction->getId()) {
                m_lastFunction->setImpliedFunctionId(number);
            }
        } else if (util::stringMatch("Description", name)) {
            // 'd' takes string
            m_lastFunction->setDescription(value);
        } else if (util::stringMatch("Explanation", name)) {
            // 'e' takes string
            m_lastFunction->addToExplanation(value);
        } else if (util::stringMatch("Picture", name)) {
            // 'p' takes string
            m_lastFunction->setPictureName(value);
        } else if (util::stringMatch("Standard", name)) {
            // 's' takes list of hull numbers
            trimComments(value);
            do {
                String_t lhs = afl::string::strTrim(afl::string::strFirst(value, ","));
                int num;
                if (! afl::string::strToInteger(lhs, num) || num <= 0) {
                    handleError(fileName, lineNr, afl::string::Format(m_translator.translateString("Invalid hull number \"%s\"").c_str(), lhs));
                    break;
                }
                m_list.addDefaultAssignment(num, m_lastFunction->getId());
            } while (afl::string::strRemove(value, ","));
        } else {
            // ignore
        }
    }
}

void
BasicHullFunctionReader::handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
{ }


// /** Shortcut function to display errors. */
void
BasicHullFunctionReader::handleError(const String_t& fileName, int lineNr, String_t message)
{
    // ex GHullFunctionReader::error
    m_log.write(m_log.Error, "game.spec.hullfunc", fileName, lineNr, message);
}

/************************* BasicHullFunctionList *************************/

// Constructor.
game::spec::BasicHullFunctionList::BasicHullFunctionList()
    : m_functions()
{ }

// Destructor.
game::spec::BasicHullFunctionList::~BasicHullFunctionList()
{ }

// Clear hull functions definitions.
void
game::spec::BasicHullFunctionList::clear()
{
    // ex GHullFunctionData::clear (part)
    m_functions.clear();
}

// Load from file.
void
game::spec::BasicHullFunctionList::load(afl::io::Stream& in, afl::string::Translator& tx, afl::sys::LogListener& log)
{
    // ex GHullFunctionData::loadBasicFunctionDefinitions (part)
    BasicHullFunctionReader(*this, tx, log).parseFile(in);
}

// Get definition of a basic function.
const game::spec::BasicHullFunction*
game::spec::BasicHullFunctionList::getFunctionById(int id) const
{
    // ex GHullFunctionData::getBasicFunctionById
    for (size_t i = 0, n = m_functions.size(); i < n; ++i) {
        if (m_functions[i]->getId() == id) {
            return m_functions[i];
        }
    }
    return 0;
}

// Get definition of a basic function by name.
const game::spec::BasicHullFunction*
game::spec::BasicHullFunctionList::getFunctionByName(String_t name, bool acceptPartialMatch) const
{
    // ex GHullFunctionData::getBasicFunctionByName, hullfunc.pas:GetBasicFunctionByName
    BasicHullFunction* result = 0;

    name = afl::string::strLCase(name);
    for (size_t i = 0, n = m_functions.size(); i < n; ++i) {
        const String_t foundName = afl::string::strLCase(m_functions[i]->getName());
        if (foundName == name) {
            // exact match
            result = m_functions[i];
            break;
        }
        if (acceptPartialMatch && foundName.length() > name.length() && foundName.compare(0, name.length(), name) == 0) {
            // partial match, only to be surpassed by an exact match
            result = m_functions[i];
            acceptPartialMatch = false;
        }
    }
    return result;
}

// Add basic function definition.
game::spec::BasicHullFunction*
game::spec::BasicHullFunctionList::addFunction(int id, String_t name)
{
    // ex GHullFunctionData::addBasicFunction
    return m_functions.pushBackNew(new BasicHullFunction(id, name));
}

// Check whether two basic hull function identifiers match.
bool
game::spec::BasicHullFunctionList::matchFunction(int requestedFunctionId, int foundFunctionId) const
{
    // ex GHullFunctionData::matchBasicFunction
    size_t loopLimit = m_functions.size();
    while (foundFunctionId != requestedFunctionId) {
        const BasicHullFunction* foundFunction = getFunctionById(foundFunctionId);
        if (foundFunction == 0) {
            // this function does not exist
            return false;
        }

        foundFunctionId = foundFunction->getImpliedFunctionId();
        if (foundFunctionId < 0) {
            // this function doesn't imply anything
            return false;
        }

        if (loopLimit == 0) {
            // loop detected
            return false;
        }
        --loopLimit;
    }

    // found match
    return true;
}

// Add a default assignment.
void
game::spec::BasicHullFunctionList::addDefaultAssignment(int hullId, int basicFunctionId)
{
    m_defaultAssignments.push_back(std::make_pair(hullId, basicFunctionId));
}

// Perform default assignments on a set of hulls.
void
game::spec::BasicHullFunctionList::performDefaultAssignments(ComponentVector<Hull>& hulls) const
{
    // ex GHullFunctionData::performDefaultAssignments, hullfunc.pas:LoadDefaultHullfuncs
    // FIXME: having this function here makes it hard to test due to cyclic dependency Hull<>BasicHullFunctionList
    for (size_t i = 0, n = m_defaultAssignments.size(); i < n; ++i) {
        if (Hull* h = hulls.get(m_defaultAssignments[i].first)) {
            h->changeHullFunction(ModifiedHullFunctionList::Function_t(m_defaultAssignments[i].second),
                                  PlayerSet_t::allUpTo(MAX_PLAYERS),
                                  PlayerSet_t(),
                                  true /* assign to hull */);
        }
    }
}

// Get number of functions.
size_t
game::spec::BasicHullFunctionList::getNumFunctions() const
{
    return m_functions.size();
}

// Get function by index.
const game::spec::BasicHullFunction*
game::spec::BasicHullFunctionList::getFunctionByIndex(size_t index) const
{
    if (index < m_functions.size()) {
        return m_functions[index];
    } else {
        return 0;
    }
}
