/**
  *  \file game/v3/trn/stringfilter.hpp
  */
#ifndef C2NG_GAME_V3_TRN_STRINGFILTER_HPP
#define C2NG_GAME_V3_TRN_STRINGFILTER_HPP

#include "game/v3/trn/filter.hpp"

namespace game { namespace v3 { namespace trn {

    class StringFilter : public Filter {
     public:
        /** Create.
            \param str string to match (case-insensitive) */
        explicit StringFilter(const String_t& str);

        virtual bool accept(const TurnFile& trn, size_t index) const;

     private:
        const String_t m_string;
    };

} } }

#endif
