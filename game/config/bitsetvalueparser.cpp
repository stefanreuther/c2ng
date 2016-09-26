/**
  *  \file game/config/bitsetvalueparser.cpp
  */

#include "game/config/bitsetvalueparser.hpp"
#include "afl/string/string.hpp"
#include "afl/string/parse.hpp"
#include "util/string.hpp"

game::config::BitsetValueParser::BitsetValueParser(const char* tpl)
    : m_template(tpl)
{ }
game::config::BitsetValueParser::~BitsetValueParser()
{ }

int32_t
game::config::BitsetValueParser::parse(String_t value) const
{
    // ex ValueBitsetParser::parse
    int32_t result = 0;
    do {
        String_t item = afl::string::strTrim(afl::string::strFirst(value, ","));
        int32_t tmp;
        if (item.size() == 0) {
            // blank field
        } else if (afl::string::strToInteger(item, tmp)) {
            // numeric field
            result |= tmp;
        } else {
            // word field
            const char* tpl = m_template;
            String_t ele;
            int32_t bit = 1;
            while (util::eatWord(tpl, ele)) {
                if (afl::string::strCaseCompare(ele, item) == 0) {
                    result |= bit;
                    break;
                }
                bit <<= 1;
            }
        }
    } while (afl::string::strRemove(value, ","));
    return result;
}

String_t
game::config::BitsetValueParser::toString(int32_t value) const
{
    // ex ValueBitsetParser::toString
    String_t result;
    const char* tpl = m_template;
    String_t ele;
    int32_t bit = 1;
    while (util::eatWord(tpl, ele)) {
        if ((value & bit) != 0) {
            if (result.size()) {
                result.append(",");
            }
            result.append(ele);
        }
        bit <<= 1;
    }
    return result;
}
