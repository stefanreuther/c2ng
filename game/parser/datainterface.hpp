/**
  *  \file game/parser/datainterface.hpp
  *  \brief Interface game::parser::DataInterface
  */
#ifndef C2NG_GAME_PARSER_DATAINTERFACE_HPP
#define C2NG_GAME_PARSER_DATAINTERFACE_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace game { namespace parser {

    /** Interface connecting message parser to game data. */
    class DataInterface : public afl::base::Deletable {
     public:
        enum Name {
            ShortRaceName,
            LongRaceName,
            AdjectiveRaceName,
            HullName
        };

        /** Get current player number.
            Used to attribute incoming data (via the "values = player" assignment).
            \return player number */
        virtual int getPlayerNumber() const = 0;

        /** Parse a name into a number.
            Parsing should not be case sensitive.
            Parsing should anticipate trailing/leading spaces in original data,
            because the message parser will have stripped those from \c name.

            \param which Type of mapping to parse
            \param name  Name to parse
            \return player/hull number if found, otherwise 0 */
        virtual int parseName(Name which, const String_t& name) const = 0;

        /** Expand race names in a template.
            Should be the same as PlayerList::expandNames(tpl, true).

            This is used to identify lines such as "Tholians prefer deserts" in hconfig,
            and should use host names.

            \param tpl String with placeholders
            \return Expanded string */
        virtual String_t expandRaceNames(String_t tpl) const = 0;
    };

} }

#endif
