/**
  *  \file game/config/integervalueparser.cpp
  */

#include <stdexcept>
#include "game/config/integervalueparser.hpp"
#include "afl/string/parse.hpp"
#include "afl/string/format.hpp"
#include "util/translation.hpp"

// Global instance.
game::config::IntegerValueParser game::config::IntegerValueParser::instance;

// Constructor.
game::config::IntegerValueParser::IntegerValueParser()
{
    // ex ValueIntParser::ValueIntParser
}

// Destructor.
game::config::IntegerValueParser::~IntegerValueParser()
{ }

// Parse integer.
int32_t
game::config::IntegerValueParser::parse(String_t value) const
{
    // ex ValueIntParser::parse
    int32_t result;
    String_t::size_type pos;
    if (!afl::string::strToInteger(value, result, pos) && !afl::string::strToInteger(value.substr(0, pos), result)) {
        throw std::range_error(_("Invalid number"));
    }
    return result;
}

// Format integer.
String_t
game::config::IntegerValueParser::toString(int32_t value) const
{
    // ex ValueIntParser::toString
    return afl::string::Format("%d", value);
}
