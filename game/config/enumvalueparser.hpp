/**
  *  \file game/config/enumvalueparser.hpp
  */
#ifndef C2NG_GAME_CONFIG_ENUMVALUEPARSER_HPP
#define C2NG_GAME_CONFIG_ENUMVALUEPARSER_HPP

#include "game/config/valueparser.hpp"

namespace game { namespace config {

    class EnumValueParser : public ValueParser {
     public:
        EnumValueParser(const char* tpl);
        ~EnumValueParser();
        virtual int32_t parse(String_t value) const;
        virtual String_t toString(int32_t value) const;

     private:
        const char*const m_template;
    };

} }

#endif
