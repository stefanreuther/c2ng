/**
  *  \file game/config/booleanvalueparser.hpp
  */
#ifndef C2NG_GAME_CONFIG_BOOLEANVALUEPARSER_HPP
#define C2NG_GAME_CONFIG_BOOLEANVALUEPARSER_HPP

#include "game/config/valueparser.hpp"

namespace game { namespace config {

    /** Value parser for booleans. This parses PHost-style bools (Yes/No/Allies/External). */
    class BooleanValueParser : public ValueParser {
     public:
        BooleanValueParser();
        ~BooleanValueParser();
        virtual int32_t parse(String_t value) const;
        virtual String_t toString(int32_t value) const;

        static BooleanValueParser instance;
    };

} }

#endif
