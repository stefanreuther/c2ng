/**
  *  \file game/maint/dump/typedetector.cpp
  *  \brief Class game::maint::dump::TypeDetector
  */

#include <cstring>
#include "game/maint/dump/typedetector.hpp"

game::maint::dump::TypeDetector::TypeDetector()
    : m_possibleTypes(),
      m_foundParser(),
      m_typeOverride(),
      m_fileBasename()
{
    // DumpTypeDetect::DumpTypeDetect()
}

void
game::maint::dump::TypeDetector::setRequiredType(String_t name)
{
    // DumpTypeDetect::setRequiredType(const char* name)
    m_typeOverride = afl::string::strLCase(name);
}

void
game::maint::dump::TypeDetector::setFileBaseName(String_t base)
{
    // DumpTypeDetect::setFileBaseName(const char* base)
    m_fileBasename = afl::string::strLCase(base);
}

void
game::maint::dump::TypeDetector::start()
{
    // DumpTypeDetect::start()
    m_possibleTypes.clear();
    m_foundParser = 0;
}

void
game::maint::dump::TypeDetector::match(const char* type, const char* base, Parser_t& parser)
{
    // DumpTypeDetect::match(const char* type, const char* base, DumpParser parser)
    // We match this entry if type is same as specified using -t, or -t gave "auto" (or blank)
    if (m_typeOverride.size() != 0 && m_typeOverride != "auto") {
        if (m_typeOverride != type)
            return;
    } else if (base == 0) {
        // No type specified, and no basename, so we cannot accept this
        return;
    } else {
        // Type is "auto", so check basename. If this type has no basename filter,
        // reject it.
        size_t size = std::strlen(base);
        if (size <= m_fileBasename.size() && m_fileBasename.compare(0, size, base, size) != 0)
            return;
        if (m_fileBasename.size() != size
            && m_fileBasename[size] != '.'
            && (m_fileBasename[size] < '0' || m_fileBasename[size] > '9'))
            return;
    }

    // Okay, matches.
    if (parser != m_foundParser) {
        m_foundParser = parser;
        m_possibleTypes.push_back(type);
    }
}
