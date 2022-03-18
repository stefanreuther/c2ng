/**
  *  \file game/v3/trn/namefilter.hpp
  */
#ifndef C2NG_GAME_V3_TRN_NAMEFILTER_HPP
#define C2NG_GAME_V3_TRN_NAMEFILTER_HPP

#include "game/v3/trn/filter.hpp"
#include "afl/string/string.hpp"

namespace game { namespace v3 { namespace trn {

    class NameFilter : public Filter {
     public:
        /** Create.
            \param str name we want to match
            \param wildcard true to match all commands beginning with that name, false to match exact names */
        NameFilter(String_t name, bool wildcard);

        virtual bool accept(const TurnFile& trn, size_t index) const;

     private:
        const String_t m_name;
        bool m_wildcard;
    };

} } }

#endif
