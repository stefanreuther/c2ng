/**
  *  \file util/configurationfile.cpp
  *  \brief Class util::ConfigurationFile
  *
  *  This class adds new functionality over PCC2, and replaces the ConfigFileUpdater class.
  */

#include "util/configurationfile.hpp"
#include "afl/string/char.hpp"
#include "util/stringparser.hpp"

using afl::string::charIsSpace;
using afl::string::strUCase;

namespace {
    bool charIsNotSpace(char c)
    {
        return !charIsSpace(c);
    }
    bool charIsNotSpaceOrEqual(char c)
    {
        return !charIsSpace(c) && c != '=';
    }
}

// Constructor.
util::ConfigurationFile::ConfigurationFile()
    : m_elements(),
      m_whitespaceIsSignificant(false)
{ }

// Destructor.
util::ConfigurationFile::~ConfigurationFile()
{ }

// Set significance of whitespace in values.
void
util::ConfigurationFile::setWhitespaceIsSignificant(bool flag)
{
    m_whitespaceIsSignificant = flag;
}

// Load from file.
void
util::ConfigurationFile::load(afl::io::TextReader& in)
{
    String_t line;
    String_t prefix;
    String_t tmp;
    String_t sectionPrefix;
    while (in.readLine(line)) {
        // Skip initial space
        StringParser p(line);
        p.parseWhile(charIsSpace, tmp);

        // Identify line
        if (p.parseEnd() || p.parseCharacter('#') || p.parseCharacter(';')) {
            // Comment or blank line
            prefix += line;
            prefix += '\n';
        } else if (p.parseCharacter('%')) {
            // '%foo' section delimiter
            p.parseWhile(charIsSpace, tmp);
            p.parseWhile(charIsNotSpace, tmp);
            Element* pElem = m_elements.pushBackNew(new Element(Section, strUCase(tmp), prefix + line, String_t()));
            sectionPrefix = pElem->key + ".";
            prefix.clear();
        } else if (p.parseCharacter('[')) {
            // '[foo]' section delimiter
            p.parseDelim("]", tmp);
            Element* pElem = m_elements.pushBackNew(new Element(Section, strUCase(tmp), prefix + line, String_t()));
            sectionPrefix = pElem->key + ".";
            prefix.clear();
        } else {
            // Check for assignment
            String_t key;
            p.parseWhile(charIsNotSpaceOrEqual, key);
            p.parseWhile(charIsSpace, tmp);
            if (!key.empty() && p.parseCharacter('=')) {
                if (!m_whitespaceIsSignificant) {
                    p.parseWhile(charIsSpace, tmp);
                }
                m_elements.pushBackNew(new Element(Assignment, sectionPrefix + strUCase(key), prefix + line.substr(0, p.getPosition()), p.getRemainder()));
            } else {
                m_elements.pushBackNew(new Element(Generic, String_t(), prefix + line, String_t()));
            }
            prefix.clear();
        }
    }

    // Final part
    if (!prefix.empty()) {
        m_elements.pushBackNew(new Element(Generic, String_t(), prefix, String_t()));
    }
}

// Save to file.
void
util::ConfigurationFile::save(afl::io::TextWriter& out) const
{
    for (size_t i = 0, n = m_elements.size(); i < n; ++i) {
        Element& e = *m_elements[i];
        out.writeLine(e.prefix + e.value);
    }
}

// Merge from another ConfigurationFile.
void
util::ConfigurationFile::merge(const ConfigurationFile& other)
{
    const Element* sectionHeader = 0;
    for (size_t i = 0, n = other.getNumElements(); i < n; ++i) {
        if (const Element* e = other.getElementByIndex(i)) {
            switch (e->type) {
             case Generic:
                break;

             case Section:
                sectionHeader = e;
                break;

             case Assignment:
                if (const Element* existing = findElement(Assignment, e->key)) {
                    // Replacing an existing value
                    const_cast<Element*>(existing)->value = e->value;
                } else if (sectionHeader != 0) {
                    // Adding a value to a section
                    size_t existingSection;
                    if (findIndex(Section, sectionHeader->key).get(existingSection)) {
                        // Adding to existing section
                        size_t insertPosition = findSectionEnd(existingSection + 1);
                        m_elements.insertNew(m_elements.begin() + insertPosition, new Element(*e));
                    } else {
                        // Creating a new section: section header, element
                        m_elements.pushBackNew(new Element(*sectionHeader));
                        m_elements.pushBackNew(new Element(*e));
                    }
                } else {
                    String_t::size_type dot = e->key.find('.');
                    if (dot == String_t::npos) {
                        // Adding a value to unnamed section
                        size_t insertPosition = findSectionEnd(0);
                        m_elements.insertNew(m_elements.begin() + insertPosition, new Element(*e));
                    } else {
                        // Dotted value, that is, input file contains "SEC.KEY = VALUE" and that thing does not yet exist.
                        // The prefix here contains something we cannot use ("SEC." must be removed).
                        // Therefore, we need to generate the whole assignment anew, which is the same as the 'set' operation.
                        set(e->key.substr(0, dot), e->key.substr(dot+1), e->value);
                    }
                }
                break;
            }
        }
    }
}

// Set single value.
void
util::ConfigurationFile::set(String_t key, String_t value)
{
    String_t::size_type dot = key.find('.');
    if (dot != String_t::npos) {
        set(key.substr(0, dot), key.substr(dot+1), value);
    } else {
        set(String_t(), key, value);
    }
}

// Set single value, sectioned.
void
util::ConfigurationFile::set(String_t section, String_t key, String_t value)
{
    String_t ucSection = afl::string::strUCase(section);
    String_t ucKey = afl::string::strUCase(key);

    String_t assignmentKey = ucSection;
    if (!assignmentKey.empty()) {
        assignmentKey += ".";
    }
    assignmentKey += ucKey;
    
    size_t existingSection;
    if (const Element* existing = findElement(Assignment, assignmentKey)) {
        // Element exists
        const_cast<Element*>(existing)->value = value;
    } else if (section.empty()) {
        // Inserting into nameless section
        size_t insertPosition = findSectionEnd(0);
        insertAssignment(insertPosition, assignmentKey, key, value);
    } else {
        if (findIndex(Section, section).get(existingSection)) {
            // Section exists
            size_t insertPosition = findSectionEnd(existingSection + 1);
            insertAssignment(insertPosition, assignmentKey, key, value);
        } else {
            // Section does not exist
            m_elements.pushBackNew(new Element(Section, ucSection, "% " + section, String_t()));
            insertAssignment(m_elements.size(), assignmentKey, key, value);
        }
    }
}

// Add single value.
void
util::ConfigurationFile::add(String_t key, String_t value)
{
    String_t::size_type dot = key.find('.');
    if (dot != String_t::npos) {
        add(key.substr(0, dot), key.substr(dot+1), value);
    } else {
        add(String_t(), key, value);
    }
}

// Set single value, sectioned.
void
util::ConfigurationFile::add(String_t section, String_t key, String_t value)
{
    String_t ucSection = afl::string::strUCase(section);
    String_t ucKey = afl::string::strUCase(key);

    String_t assignmentKey = ucSection;
    if (!assignmentKey.empty()) {
        assignmentKey += ".";
    }
    assignmentKey += ucKey;

    size_t existingPosition = 0;
    if (findIndex(Assignment, assignmentKey).get(existingPosition)) {
        // Value exists, add new value after it
        insertAssignment(existingPosition + 1, assignmentKey, key, value);
    } else if (section.empty()) {
        // Inserting into nameless section
        insertAssignment(findSectionEnd(0), assignmentKey, key, value);
    } else {
        size_t existingSection = 0;
        if (findIndex(Section, section).get(existingSection)) {
            // Section exists
            size_t insertPosition = findSectionEnd(existingSection + 1);
            insertAssignment(insertPosition, assignmentKey, key, value);
        } else {
            // Section does not exist
            m_elements.pushBackNew(new Element(Section, ucSection, "% " + section, String_t()));
            insertAssignment(m_elements.size(), assignmentKey, key, value);
        }
    }
}

// Remove value.
bool
util::ConfigurationFile::remove(String_t key)
{
    size_t index;
    if (findIndex(Assignment, key).get(index)) {
        m_elements.erase(m_elements.begin() + index);
        return true;
    } else {
        return false;
    }
}

// Add header comment.
void
util::ConfigurationFile::addHeaderComment(const String_t& comment, bool force)
{
    if (!m_elements.empty()) {
        Element& firstElement = *m_elements[0];
        String_t::size_type n = firstElement.prefix.rfind('\n');
        if (n == String_t::npos) {
            // No comment present. Add one.
            firstElement.prefix = comment + "\n" + firstElement.prefix;
        } else if (force) {
            // Comment present, replace it
            firstElement.prefix = comment + firstElement.prefix.substr(n);
        } else {
            // Comment present, keep it
        }
    }
}

// Get number of elements.
size_t
util::ConfigurationFile::getNumElements() const
{
    return m_elements.size();
}

// Get element, given an index.
const util::ConfigurationFile::Element*
util::ConfigurationFile::getElementByIndex(size_t index) const
{
    return (index < m_elements.size()
            ? m_elements[index]
            : 0);
}

// Find index of an element.
afl::base::Optional<size_t>
util::ConfigurationFile::findIndex(ElementType type, String_t key) const
{
    const String_t ucKey = afl::string::strUCase(key);
    size_t n = m_elements.size();
    while (n > 0) {
        --n;
        if (m_elements[n]->type == type && m_elements[n]->key == ucKey) {
            return n;
        }
    }
    return afl::base::Nothing;
}

// Find an element.
const util::ConfigurationFile::Element*
util::ConfigurationFile::findElement(ElementType type, String_t key) const
{
    size_t i;
    if (findIndex(type, key).get(i)) {
        return getElementByIndex(i);
    } else {
        return 0;
    }
}

// Find end of section.
size_t
util::ConfigurationFile::findSectionEnd(size_t startIndex) const
{
    while (startIndex < m_elements.size() && m_elements[startIndex]->type != Section) {
        ++startIndex;
    }
    return startIndex;
}

// Helper to insert an assignment
void
util::ConfigurationFile::insertAssignment(size_t insertPosition,
                                          const String_t& assignmentKey,
                                          const String_t& key,
                                          const String_t& value)
{
    // Determine whitespace prefix
    String_t line = "  ";
    if (insertPosition > 0 && m_elements[insertPosition-1]->type == Assignment) {
        const Element& ele = *m_elements[insertPosition-1];
        size_t n = 0;
        while (n < ele.prefix.size() && afl::string::charIsSpace(ele.prefix[n])) {
            ++n;
        }
        line.assign(ele.prefix, 0, n);
    }

    // Key
    line += key;

    // Determine assignment syntax
    if (m_whitespaceIsSignificant) {
        line += "=";
    } else {
        line += " = ";
    }

    // Add it
    m_elements.insertNew(m_elements.begin() + insertPosition, new Element(Assignment, assignmentKey, line, value));
}

// Check for assignments.
bool
util::ConfigurationFile::hasAssignments() const
{
    for (size_t i = 0, n = m_elements.size(); i < n; ++i) {
        if (m_elements[i]->type == Assignment) {
            return true;
        }
    }
    return false;
}
