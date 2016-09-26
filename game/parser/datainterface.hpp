/**
  *  \file game/parser/datainterface.hpp
  */
#ifndef C2NG_GAME_PARSER_DATAINTERFACE_HPP
#define C2NG_GAME_PARSER_DATAINTERFACE_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace game { namespace parser {

    class DataInterface : public afl::base::Deletable {
     public:
        enum Name {
            ShortRaceName,
            FullRaceName,
            AdjectiveRaceName,
            HullName
        };

        // get current player number
        virtual int getPlayerNumber() const = 0;

        // find number such that strUCase(item(nr)) = strUCase(name), otherwise 0
        virtual int parseName(Name which, const String_t& name) const = 0;

        // replaces "host_racenames.expandNames"
        virtual String_t expandRaceNames(String_t name) const = 0;
    };

} }

#endif
