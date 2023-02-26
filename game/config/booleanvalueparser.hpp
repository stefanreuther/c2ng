/**
  *  \file game/config/booleanvalueparser.hpp
  *  \brief Class game::config::BooleanValueParser
  */
#ifndef C2NG_GAME_CONFIG_BOOLEANVALUEPARSER_HPP
#define C2NG_GAME_CONFIG_BOOLEANVALUEPARSER_HPP

#include "game/config/valueparser.hpp"

namespace game { namespace config {

    /** Value parser for booleans. This parses PHost-style bools (Yes/No/Allies/External).
        A BooleanValueParser is stateless; therefore, you usually use the provided static instance. */
    class BooleanValueParser : public ValueParser {
     public:
        /** Constructor. */
        BooleanValueParser();
        ~BooleanValueParser();

        // ValueParser:
        virtual int32_t parse(String_t value) const;
        virtual String_t toString(int32_t value) const;

        /** Static instance. */
        static const BooleanValueParser instance;
    };

} }

#endif
