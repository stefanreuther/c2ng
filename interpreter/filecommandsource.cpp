/**
  *  \file interpreter/filecommandsource.cpp
  */

#include "interpreter/filecommandsource.hpp"
#include "afl/string/format.hpp"
#include "interpreter/error.hpp"

interpreter::FileCommandSource::FileCommandSource(afl::io::TextFile& tf)
    : m_textFile(tf)
{
    // ex IntFileCommandSource::IntFileCommandSource
}

void
interpreter::FileCommandSource::readNextLine()
{
    // ex IntFileCommandSource::readNextLine
    String_t line;
    if (m_textFile.readLine(line)) {
        setNextLine(line);
    } else {
        setEOF();
    }
}

bool
interpreter::FileCommandSource::setCharsetNew(afl::charset::Charset* cs)
{
    // ex IntFileCommandSource::setEncoding
    m_textFile.setCharsetNew(cs);
    return true;
}

void
interpreter::FileCommandSource::addTraceTo(Error& e, afl::string::Translator& tx)
{
    // FIXME: PCC2 uses basename of file here
    e.addTrace(afl::string::Format(tx.translateString("in file '%s', line %d").c_str(), m_textFile.getName(), getLineNumber()));
}
