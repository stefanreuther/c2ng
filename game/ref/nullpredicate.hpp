/**
  *  \file game/ref/nullpredicate.hpp
  */
#ifndef C2NG_GAME_REF_NULLPREDICATE_HPP
#define C2NG_GAME_REF_NULLPREDICATE_HPP

#include "game/ref/sortpredicate.hpp"

namespace game { namespace ref {

    class NullPredicate : public SortPredicate {
     public:
        virtual int compare(const Reference& a, const Reference& b) const;

        virtual String_t getClass(const Reference& a) const;
    };

} }

#endif
