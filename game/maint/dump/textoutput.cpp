/**
  *  \file game/maint/dump/textoutput.cpp
  *  \brief Class game::maint::dump::TextOutput
  */

#include "game/maint/dump/textoutput.hpp"
#include "afl/string/format.hpp"

using afl::string::Format;

game::maint::dump::TextOutput::TextOutput(afl::io::TextWriter& out)
    : m_output(out),
      m_level(0)
{ }

void
game::maint::dump::TextOutput::startRecord(String_t header)
{
    // DumpSimpleOutputReceiver::startRecord(string_t header)
    m_output.writeLine(Format("%s%s:", String_t(2*m_level, ' '), header));
    ++m_level;
}

void
game::maint::dump::TextOutput::addField(String_t name, String_t formattedValue)
{
    // DumpSimpleOutputReceiver::addField(string_t name, string_t value)
    m_output.writeLine(Format("%s%-30s = %s", String_t(2*m_level, ' '), name, formattedValue));
}

void
game::maint::dump::TextOutput::addUnparsedData(String_t formattedValue)
{
    // DumpSimpleOutputReceiver::addUnparsedData(string_t formatted)
    m_output.writeLine(Format("%sUnparsed = %s", String_t(2*m_level, ' '), formattedValue));
}

void
game::maint::dump::TextOutput::endRecord()
{
    // DumpSimpleOutputReceiver::endRecord()
    m_output.writeLine();
    if (m_level > 0) {
        --m_level;
    }
}
