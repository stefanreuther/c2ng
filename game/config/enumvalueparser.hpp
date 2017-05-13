/**
  *  \file game/config/enumvalueparser.hpp
  *  \brief Class game::config::EnumValueParser
  */
#ifndef C2NG_GAME_CONFIG_ENUMVALUEPARSER_HPP
#define C2NG_GAME_CONFIG_ENUMVALUEPARSER_HPP

#include "game/config/valueparser.hpp"

namespace game { namespace config {

    /** Value parser for enums.
        Parses a value from a list of possible words, each mapping to an integer.

        The enumeration is defined by a template that is a comma-separated list of words (e.g. "a,b,c").
        Each word maps to its position in the list (0,1,2,...), and is accepted in either case.
        Formatting (toString()) produces the word in its original case in the list. */
    class EnumValueParser : public ValueParser {
     public:
        /** Constructor.
            \param tpl Template. */
        explicit EnumValueParser(const char* tpl);
        ~EnumValueParser();
        virtual int32_t parse(String_t value) const;
        virtual String_t toString(int32_t value) const;

     private:
        const char*const m_template;
    };

} }

#endif
