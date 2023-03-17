/**
  *  \file game/ref/nullpredicate.hpp
  *  \brief Class game::ref::NullPredicate
  */
#ifndef C2NG_GAME_REF_NULLPREDICATE_HPP
#define C2NG_GAME_REF_NULLPREDICATE_HPP

#include "game/ref/sortpredicate.hpp"

namespace game { namespace ref {

    /** Null predicate.
        Does not sort anything (=falls back to default sort),
        and does not generate any dividers. */
    class NullPredicate : public SortPredicate {
     public:
        // SortPredicate:
        virtual int compare(const Reference& a, const Reference& b) const;
        virtual String_t getClass(const Reference& a) const;
    };

} }

#endif
