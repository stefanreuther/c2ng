/**
  *  \file game/config/enumvalueparser.cpp
  *  \brief Class game::config::EnumValueParser
  */

#include <stdexcept>
#include "game/config/enumvalueparser.hpp"
#include "afl/string/format.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "util/string.hpp"
#include "afl/string/parse.hpp"

game::config::EnumValueParser::EnumValueParser(const char* tpl)
    : m_template(tpl)
{ }
game::config::EnumValueParser::~EnumValueParser()
{ }

int32_t
game::config::EnumValueParser::parse(String_t value) const
{
    // ex ValueEnumParser::parse
    const char* tpl = m_template;
    String_t ele;
    int32_t counter = 0;
    while (util::eatWord(tpl, ele)) {
        if (afl::string::strCaseCompare(value, ele) == 0) {
            return counter;
        }
        ++counter;
    }

    // Must support parsing plain numbers because we can stringify that.
    // This is also required to be able to meaningfully use it in scripts, e.g. CCfg.Boolean.Edit.
    // Pref() will produce integer values that we want to parse.
    int32_t result = 0;
    if (afl::string::strToInteger(value, result)) {
        return result;
    }

    throw std::range_error(afl::string::Translator::getSystemInstance()("Invalid number"));
}

String_t
game::config::EnumValueParser::toString(int32_t value) const
{
    // ex ValueEnumParser::toString
    const char* tpl = m_template;
    String_t ele;
    int32_t counter = 0;
    while (util::eatWord(tpl, ele)) {
        if (counter == value) {
            return ele;
        }
        ++counter;
    }

    // Fallback, just in case
    return afl::string::Format("%d", value);
}
