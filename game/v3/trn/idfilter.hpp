/**
  *  \file game/v3/trn/idfilter.hpp
  */
#ifndef C2NG_GAME_V3_TRN_IDFILTER_HPP
#define C2NG_GAME_V3_TRN_IDFILTER_HPP

#include "game/types.hpp"
#include "game/v3/trn/filter.hpp"

namespace game { namespace v3 { namespace trn {

    class IdFilter : public Filter {
     public:
        /** Create.
            \param lower,upper range to match, boundaries inclusive. */
        IdFilter(Id_t lower, Id_t upper);

        virtual bool accept(const TurnFile& trn, size_t index) const;

     private:
        const Id_t m_lower;
        const Id_t m_upper;
    };

} } }

#endif
