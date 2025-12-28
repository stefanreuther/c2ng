/**
  *  \file game/maint/dump/input.cpp
  *  \brief Class game::maint::dump::Input
  */

#include "game/maint/dump/input.hpp"
#include "afl/bits/fixedstring.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/string/format.hpp"
#include "game/maint/dump/output.hpp"

using afl::string::Format;

game::maint::dump::Input::Input(Input& other, size_t size)
    : m_input(other.m_input.split(size)), m_charset(other.m_charset)
{
    // DumpBlockInput::DumpBlockInput(DumpBlockInput& other, std::size_t rsize)
}

String_t
game::maint::dump::Input::readByte()
{
    // DumpBlockInput::readByte()
    if (const uint8_t* p = m_input.eat()) {
        return Format("%d", *p);
    } else {
        return "<missing>";
    }
}

String_t
game::maint::dump::Input::readWord()
{
    // DumpBlockInput::readWord()
    if (const afl::bits::Int16LE::Bytes_t* p = m_input.eatN<sizeof(*p)>()) {
        int16_t value = afl::bits::Int16LE::unpack(*p);
        return Format("%d", value);
    } else {
        return "<missing>";
    }
}

String_t
game::maint::dump::Input::readLong()
{
    // DumpBlockInput::readLong()
    if (const afl::bits::Int32LE::Bytes_t* p = m_input.eatN<sizeof(*p)>()) {
        int32_t value = afl::bits::Int32LE::unpack(*p);
        return Format("%d", value);
    } else {
        return "<missing>";
    }
}

String_t
game::maint::dump::Input::readCoordinate()
{
    // DumpBlockInput::readCoordinate()
    if (getRemainingSize() >= 4) {
        String_t x = readWord();
        String_t y = readWord();
        return "(" + x + "," + y + ")";
    } else {
        return "<missing>";
    }
}

String_t
game::maint::dump::Input::readString(size_t length)
{
    // DumpBlockInput::readString(std::size_t length)
    if (getRemainingSize() >= length) {
        String_t s = m_charset.decode(afl::bits::unpackFixedString(m_input.split(length)));
        return "'" + s + "'";
    } else {
        return "<missing>";
    }
}

String_t
game::maint::dump::Input::readPascalString()
{
    // DumpBlockInput::readPascalString()
    uint8_t length[1];
    if (read(length) == 1) {
        return readString(length[0]);
    } else {
        return "<missing>";
    }
}

size_t
game::maint::dump::Input::getRemainingSize() const
{
    // DumpBlockInput::getRemainingSize() const
    return m_input.size();
}

String_t
game::maint::dump::Input::readUnparsed(size_t limit)
{
    // DumpBlockInput::readUnparsed(std::size_t limit)
    size_t n = getRemainingSize();
    if (n > limit) {
        n = limit;
    }

    String_t result;
    for (size_t i = 0; i < n; ++i) {
        if (i != 0) {
            result += ' ';
        }
        result += Format("%02X", *m_input.eat());
    }
    return result;
}

size_t
game::maint::dump::Input::peek(afl::base::Bytes_t mem)
{
    // DumpBlockInput::peek(void* buffer, size_t size)
    return mem.copyFrom(m_input).size();
}

size_t
game::maint::dump::Input::read(afl::base::Bytes_t mem)
{
    // DumpBlockInput::read(void* buffer, size_t size)
    size_t got = peek(mem);
    return skip(got);
}

size_t
game::maint::dump::Input::skip(size_t size)
{
    // DumpBlockInput::skip(size_t size)
    return m_input.split(size).size();
}

void
game::maint::dump::Input::dumpRemainder(Output& out)
{
    // dumpRemainder(DumpBlockInput& bi, DumpOutputReceiver& r)
    while (getRemainingSize() > 0) {
        out.addUnparsedData(readUnparsed(16));
    }
}
