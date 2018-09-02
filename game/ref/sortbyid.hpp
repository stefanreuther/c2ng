/**
  *  \file game/ref/sortbyid.hpp
  */
#ifndef C2NG_GAME_REF_SORTBYID_HPP
#define C2NG_GAME_REF_SORTBYID_HPP

#include "game/ref/sortpredicate.hpp"

namespace game { namespace ref {

    class SortById : public SortPredicate {
     public:
        virtual int compare(const Reference& a, const Reference& b) const;

        virtual String_t getClass(const Reference& a) const;
    };

} }

#endif
