/**
  *  \file game/v3/trn/orfilter.hpp
  */
#ifndef C2NG_GAME_V3_TRN_ORFILTER_HPP
#define C2NG_GAME_V3_TRN_ORFILTER_HPP

#include "game/v3/trn/filter.hpp"

namespace game { namespace v3 { namespace trn {

    class OrFilter : public Filter {
     public:
        /** Create.
            \param lhs,rhs sub-expressions. Both must be false that we return false. */
        OrFilter(const Filter& lhs, const Filter& rhs);

        virtual bool accept(const TurnFile& trn, size_t index) const;

     private:
        const Filter& m_lhs;
        const Filter& m_rhs;
    };

} } }

#endif
