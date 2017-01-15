/**
  *  \file game/v3/trn/constantfilter.hpp
  */
#ifndef C2NG_GAME_V3_TRN_CONSTANTFILTER_HPP
#define C2NG_GAME_V3_TRN_CONSTANTFILTER_HPP

#include "game/v3/trn/filter.hpp"

namespace game { namespace v3 { namespace trn {

    class ConstantFilter : public Filter {
     public:
        /** Create.
            \param value result to return */
        explicit ConstantFilter(bool value);

        virtual bool accept(const TurnFile& trn, size_t index) const;

     private:
        const bool m_value;
    };

} } }

#endif
