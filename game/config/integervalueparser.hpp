/**
  *  \file game/config/integervalueparser.hpp
  *  \brief Class game::config::IntegerValueParser
  */
#ifndef C2NG_GAME_CONFIG_INTEGERVALUEPARSER_HPP
#define C2NG_GAME_CONFIG_INTEGERVALUEPARSER_HPP

#include "game/config/valueparser.hpp"

namespace game { namespace config {

    /** Value parser for integers.
        This parses regular decimal integers. */
    class IntegerValueParser : public ValueParser {
     public:
        IntegerValueParser();
        ~IntegerValueParser();
        virtual int32_t parse(String_t value) const;
        virtual String_t toString(int32_t value) const;

        static IntegerValueParser instance;
    };

} }


#endif
