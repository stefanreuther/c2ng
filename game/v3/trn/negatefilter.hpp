/**
  *  \file game/v3/trn/negatefilter.hpp
  */
#ifndef C2NG_GAME_V3_TRN_NEGATEFILTER_HPP
#define C2NG_GAME_V3_TRN_NEGATEFILTER_HPP

#include "game/v3/trn/filter.hpp"

namespace game { namespace v3 { namespace trn {

    class NegateFilter : public Filter {
     public:
        /** Create.
            \param other sub-expression. We return true iff it returns false. */
        explicit NegateFilter(const Filter& other);

        virtual bool accept(const TurnFile& trn, size_t index) const;

     private:
        const Filter& m_other;
    };

} } }

#endif
