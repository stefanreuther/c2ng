/**
  *  \file interpreter/memorycommandsource.cpp
  *  \brief Class interpreter::MemoryCommandSource
  */

#include "interpreter/memorycommandsource.hpp"

// Constructor.
interpreter::MemoryCommandSource::MemoryCommandSource()
    : m_lines(),
      m_index(0)
{
    // ex IntMemoryCommandSource::IntMemoryCommandSource
}

// Constructor.
interpreter::MemoryCommandSource::MemoryCommandSource(String_t line)
    : m_lines(1, line),
      m_index(0)
{
    // ex IntMemoryCommandSource::IntMemoryCommandSource
}

// Add line to this command source.
void
interpreter::MemoryCommandSource::addLine(const String_t& line)
{
    // ex IntMemoryCommandSource::addLine
    m_lines.push_back(line);
}


void
interpreter::MemoryCommandSource::readNextLine()
{
    // ex IntMemoryCommandSource::readNextLine
    if (m_index < m_lines.size()) {
        setNextLine(m_lines[m_index++]);
    } else {
        setEOF();
    }
}

bool
interpreter::MemoryCommandSource::setCharsetNew(afl::charset::Charset* cs)
{
    // ex IntMemoryCommandSource::setEncoding
    delete cs;
    return false;
}

void
interpreter::MemoryCommandSource::addTraceTo(Error& /*e*/, afl::string::Translator& /*tx*/)
{ }
