/**
  *  \file game/spec/cost.hpp
  *  \brief Class game::spec::Cost
  */
#ifndef C2NG_GAME_SPEC_COST_HPP
#define C2NG_GAME_SPEC_COST_HPP

#include "afl/base/types.hpp"
#include "afl/string/string.hpp"

namespace game { namespace spec {

    /** Resource amount/cost.
        Represents the resource cost of a unit (weapon, building), or a corresponding resource set,
        and provides operations for manipulating it.
        It supports only a subset of cargo types to free users from having to deal with all combinations.

        This amount can be <em>valid</em> or <em>invalid</em>.
        It becomes invalid if a member is negative, that is,
        if a large value (the price) is subtracted from a small one (available resource).
        Use isNonNegative() to check the status. */
    class Cost {
     public:
        enum Type {
            Tritanium,
            Duranium,
            Molybdenum,
            Money,
            Supplies
        };
        static const size_t LIMIT = 5;

        /** Constructor.
            Construct empty (zero) cost. */
        Cost();

        /** Add costs.
            \param other Value to add to this one.
            \return *this */
        Cost& operator+=(const Cost& other);

        /** Subtract.
            \param other Value to subtract from this one
            \return *this */
        Cost& operator-=(const Cost& other);

        /** Multiply in-place.
            \param n Factor to multiply this with.
            \return *this */
        Cost& operator*=(int32_t n);

        /** Multiply.
            \param n Factor to multiply with
            \return scaled cost */
        Cost  operator*(int32_t n) const;

        /** Compare for equality.
            \param other other value
            \return true iff these two costs are equal */
        bool  operator==(const Cost& other) const;

        /** Compare for inequality.
            \param other other value
            \return true iff these two costs are different */
        bool  operator!=(const Cost& other) const;

        /** Convert to PHost-style string.
            Generates a list of words of the form "Tnnn" with T identifying the resource, nnn the amount.
            If this cost is valid, the resultant string will be a valid parameter for, say, PHost's "beamup" command.
            \return string, never blank */
        String_t toPHostString() const;

        /** Convert to CCScript-style string.
            Generates a list of words of the form "nnnT", with T identifying the resource (may be more than one), nnn being the amount.
            If this cost is valid, the resultant string will be a valid parameter for CCScript cargo operations.
            \return string, possibly blank */
        String_t toCargoSpecString() const;

        /** Limit amount of items to build.
            This object represents the available resources.
            We attempt to build \c amount items that cost \c item each.
            \param orderedAmount Number of items to build
            \param itemCost Cost of each item.
            \return largest n <= orderedAmount such that isEnoughFor(itemCost*n). */
        int32_t getMaxAmount(int32_t orderedAmount, const Cost& itemCost) const;

        /** Set component.
            \param type Component to set.
            \param n new amount */
        void set(Type type, int32_t n);

        /** Get component.
            \param type component to query
            \return amount of component (can be negative if missing) */
        int32_t get(Type type) const;

        /** Add component.
            \param type component to add.
            \param n increment */
        void add(Type type, int32_t n);

        /** Clear this cost.
            \post isZero() */
        void clear();

        /** Check whether this resource amount is large enough to buy an item.
            \param other price of the item
            \return true iff this cost is equal or higher than \c other. Supply sale is handled. */
        bool isEnoughFor(const Cost& other) const;

        /** Check validity.
            Invalid costs have values below zero.
            \return true iff all components are non-negative */
        bool isNonNegative() const;

        /** Check whether this cost is empty.
            \return true iff all components are zero */
        bool isZero() const;

        /** Parse a string into a Cost structure.
            This is intended to be used by the script and config interface.
            It accepts both PCC cargospecs ("nnnT") and PHost cost format ("Tnnn").

            This routine never fails; if it finds an invalid character it returns the cost parsed so far.

            \param value string to parse
            \return parsed cost */
        static Cost fromString(const String_t& value);
     private:
        int32_t m_amounts[LIMIT];
    };

} }

#endif
