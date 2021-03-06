/**
  *  \file interpreter/commandsource.cpp
  *  \brief Class interpreter::CommandSource
  */

#include "interpreter/commandsource.hpp"

// Default constructor.
interpreter::CommandSource::CommandSource()
    : m_tokenizer(String_t()),
      m_lineNr(0),
      m_eof(true)
{
    // ex IntCommandSource::IntCommandSource
}

// Destructor.
interpreter::CommandSource::~CommandSource()
{ }

// Set next input line.
void
interpreter::CommandSource::setNextLine(String_t s)
{
    // IntCommandSource::setNextLine
    ++m_lineNr;
    m_eof = false;
    m_tokenizer = Tokenizer(s);
}
