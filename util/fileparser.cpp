/**
  *  \file util/fileparser.cpp
  *  \brief Class util::FileParser
  */

#include <cstring>
#include "util/fileparser.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/char.hpp"


// Constructor.
util::FileParser::FileParser(const char* commentCharacters)
    : m_commentCharacters(commentCharacters),
      m_charset()
{ }

// Destructor.
util::FileParser::~FileParser()
{ }

// Parse a file.
void
util::FileParser::parseFile(afl::io::Stream& s)
{
    afl::io::TextFile textFile(s);
    configureTextFile(textFile);

    String_t name = s.getName();
    String_t line;
    while (textFile.readLine(line)) {
        String_t::size_type n = 0;
        while (n < line.size() && afl::string::charIsSpace(line[n])) {
            ++n;
        }
        if (n >= line.size() || (m_commentCharacters != 0 && std::strchr(m_commentCharacters, line[n]) != 0)) {
            handleIgnoredLine(name, textFile.getLineNumber(), line);
        } else {
            handleLine(name, textFile.getLineNumber(), line);
        }
    }
}

// Parse an optional file.
bool
util::FileParser::parseOptionalFile(afl::io::Directory& dir, String_t fileName)
{
    afl::base::Ptr<afl::io::Stream> s = dir.openFileNT(fileName, afl::io::FileSystem::OpenRead);
    if (s.get() != 0) {
        parseFile(*s);
        return true;
    } else {
        return false;
    }
}

// Trim comments.
void
util::FileParser::trimComments(String_t& line)
{
    if (m_commentCharacters != 0) {
        String_t::size_type n = line.find_first_of(m_commentCharacters);
        if (n != String_t::npos) {
            while (n > 0 && afl::string::charIsSpace(line[n-1])) {
                --n;
            }
            line.erase(n);
        }
    }
}

// Set character set.
void
util::FileParser::setCharsetNew(afl::charset::Charset* cs)
{
    // ex FileParser::setCharacterSet(CharacterSet cs)
    m_charset.reset(cs);
}

// Configure a text file.
void
util::FileParser::configureTextFile(afl::io::TextFile& textFile)
{
    // ex FileParser::configureTextFile
    // If we are configured for a fixed charset, use that.
    // Otherwise, the textfile can do whatever it pleases.
    if (m_charset.get() != 0) {
        textFile.setCharsetNew(m_charset->clone());
    }
}
