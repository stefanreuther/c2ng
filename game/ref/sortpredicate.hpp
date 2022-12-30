/**
  *  \file game/ref/sortpredicate.hpp
  *  \brief Class game::ref::SortPredicate
  */
#ifndef C2NG_GAME_REF_SORTPREDICATE_HPP
#define C2NG_GAME_REF_SORTPREDICATE_HPP

#include "game/reference.hpp"
#include "afl/string/string.hpp"
#include "afl/base/deletable.hpp"

namespace game { namespace ref {

    /** Base class for sorting/grouping predicates for lists of references.

        A predicate is implemented as a class derived from this.
        Functions taking a SortPredicate take it by const-reference,
        allowing use of temporary objects in code such as "sort(SortBy::Name())".
        This also enforces that predicates shall be stateless,
        and not rely on a particular call order. */
    class SortPredicate : public afl::base::Deletable {
     public:
        /** Internal class to implement then(). */
        class CombinedPredicate;

        /** 3-way comparison (for sorting).
            This function must implement a proper weak ordering,
            i.e. symmetric and transitive equality, transitive less/greater relations.

            \param a first reference
            \param b second reference

            \return 0 if a=b, negative if a<b, positive if a>b. */
        virtual int compare(const Reference& a, const Reference& b) const = 0;

        /** Get class name (for grouping).
            This can be used to generate subheadings in a sorted list.
            Class names and sort order should match such that identical class names are sorted together.

            \param a reference

            \return class name (can be empty) */
        virtual String_t getClass(const Reference& a) const = 0;

        /** Build combined predicate.
            Items that sort identical using this sort will be further sorted by the other predicate.
            The result will be a temporary object, allowing use in calls such as "sort(SortByA().then(SortByB()))".
            Both predicates (this and other) must have sufficient lifetime.

            \param other Other predicate

            \return combined predicate */
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
