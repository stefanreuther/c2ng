/**
  *  \file game/config/bitsetvalueparser.hpp
  *  \brief Class game::config::BitsetValueParser
  */
#ifndef C2NG_GAME_CONFIG_BITSETVALUEPARSER_HPP
#define C2NG_GAME_CONFIG_BITSETVALUEPARSER_HPP

#include "game/config/valueparser.hpp"

namespace game { namespace config {

    /** ValueParser implementation for a bit-set.

        Parses strings of the form "name,name,name" into a set of bits, and vice versa.
        Each name corresponds to a bit.
        The names of the bits are given as a string at construction time.

        BitsetValueParser objects are intended to be statically-allocated (long-lived). */
    class BitsetValueParser : public ValueParser {
     public:
        /** Constructor.
            @param tpl Definition of the bit names; string of the form "bit0,bit1,bit2,..." */
        explicit BitsetValueParser(const char* tpl);
        ~BitsetValueParser();

        // ValueParser:
        virtual int32_t parse(String_t value) const;
        virtual String_t toString(int32_t value) const;

     private:
        const char*const m_template;
    };

} }

#endif
