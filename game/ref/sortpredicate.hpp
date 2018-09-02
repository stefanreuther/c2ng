/**
  *  \file game/ref/sortpredicate.hpp
  */
#ifndef C2NG_GAME_REF_SORTPREDICATE_HPP
#define C2NG_GAME_REF_SORTPREDICATE_HPP

#include "game/reference.hpp"
#include "afl/string/string.hpp"
#include "afl/base/deletable.hpp"

namespace game { namespace ref {

    class SortPredicate : public afl::base::Deletable {
     public:
        class CombinedPredicate;

        virtual int compare(const Reference& a, const Reference& b) const = 0;

        virtual String_t getClass(const Reference& a) const = 0;

        CombinedPredicate then(const SortPredicate& other) const;
    };

} }

class game::ref::SortPredicate::CombinedPredicate : public SortPredicate {
 public:
    CombinedPredicate(const SortPredicate& first, const SortPredicate& second);

    virtual int compare(const Reference& a, const Reference& b) const;

    virtual String_t getClass(const Reference& a) const;
 private:
    const SortPredicate& m_first;
    const SortPredicate& m_second;
};

#endif
