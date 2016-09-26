/**
  *  \file game/config/valueparser.cpp
  *  \brief Interface game::config::ValueParser
  */

#include "game/config/valueparser.hpp"
#include "afl/string/string.hpp"

// Virtual destructor.
game::config::ValueParser::~ValueParser()
{ }

// Parse comma-separated list into array.
void
game::config::ValueParser::parseArray(String_t value, afl::base::Memory<int32_t> array) const
{
    // ex ValueParser::parseArray
    int32_t lastValue = 0;
    while (int32_t* p = array.eat()) {
        if (!value.empty()) {
            String_t::size_type n = value.find(',');
            if (n != String_t::npos) {
                lastValue = parse(afl::string::strTrim(value.substr(0, n)));
                value.erase(0, n+1);
            } else {
                lastValue = parse(afl::string::strTrim(value));
                value.clear();
            }
        }
        *p = lastValue;
    }
}

// Convert array to string.
String_t
game::config::ValueParser::toStringArray(afl::base::Memory<const int32_t> array) const
{
    // ex ValueParser::toStringArray
    String_t result;
    while (const int32_t* p = array.eat()) {
        if (!result.empty()) {
            result += ",";
        }
        result += toString(*p);
    }
    return result;
}
