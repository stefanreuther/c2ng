/**
  *  \file game/config/bitsetvalueparser.hpp
  */
#ifndef C2NG_GAME_CONFIG_BITSETVALUEPARSER_HPP
#define C2NG_GAME_CONFIG_BITSETVALUEPARSER_HPP

#include "game/config/valueparser.hpp"

namespace game { namespace config {

    class BitsetValueParser : public ValueParser {
     public:
        BitsetValueParser(const char* tpl);
        ~BitsetValueParser();
        virtual int32_t parse(String_t value) const;
        virtual String_t toString(int32_t value) const;

     private:
        const char*const m_template;
    };

} }

#endif
