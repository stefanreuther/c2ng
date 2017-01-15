/**
  *  \file game/v3/trn/indexfilter.hpp
  */
#ifndef C2NG_GAME_V3_TRN_INDEXFILTER_HPP
#define C2NG_GAME_V3_TRN_INDEXFILTER_HPP

#include "game/v3/trn/filter.hpp"

namespace game { namespace v3 { namespace trn {

    class IndexFilter : public Filter {
     public:
        /** Create.
            \param from,to index range, 1-based(!)(FIXME?), boundaries inclusive. */
        IndexFilter(size_t from, size_t to);

        virtual bool accept(const TurnFile& trn, size_t index) const;

     private:
        size_t m_from;
        size_t m_to;
    };

} } }

#endif
