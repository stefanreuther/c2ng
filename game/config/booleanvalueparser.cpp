/**
  *  \file game/config/booleanvalueparser.cpp
  */

#include "game/config/booleanvalueparser.hpp"
#include "util/string.hpp"
#include "afl/string/parse.hpp"

game::config::BooleanValueParser game::config::BooleanValueParser::instance;

game::config::BooleanValueParser::BooleanValueParser()
{
    // ex ValueBoolParser::ValueBoolParser
}

game::config::BooleanValueParser::~BooleanValueParser()
{ }

int32_t
game::config::BooleanValueParser::parse(String_t value) const
{
    // ex ValueBoolParser::parse
    using util::stringMatch;
    if (stringMatch("No", value) || stringMatch("False", value)) {
        return 0;
    } else if (stringMatch("Yes", value) || stringMatch("True", value)) {
        return 1;
    } else if (stringMatch("Allies", value)) {
        return 2;
    } else if (stringMatch("External", value)) {
        return 3;
    } else {
        int32_t tmp;
        if (afl::string::strToInteger(value, tmp)) {
            return tmp;
        } else {
            return 1; /* arbitrary */
        }
    }
}

String_t
game::config::BooleanValueParser::toString(int32_t value) const
{
    // ex ValueBoolParser::toString
    switch (value) {
     case 0:
        return "No";
     case 1:
     default: /* arbitrary */
        return "Yes";
     case 2:
        return "Allies";
     case 3:
        return "External";
    }
}
