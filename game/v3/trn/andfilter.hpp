/**
  *  \file game/v3/trn/andfilter.hpp
  */
#ifndef C2NG_GAME_V3_TRN_ANDFILTER_HPP
#define C2NG_GAME_V3_TRN_ANDFILTER_HPP

#include "game/v3/trn/filter.hpp"

namespace game { namespace v3 { namespace trn {

    class AndFilter : public Filter {
     public:
        /** Create.
            \param lhs,rhs sub-expressions. Both must be true that we return true. */
        AndFilter(const Filter& lhs, const Filter& rhs);

        virtual bool accept(const TurnFile& trn, size_t index) const;

     private:
        const Filter& m_lhs;
        const Filter& m_rhs;
    };

} } }

#endif
