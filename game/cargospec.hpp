/**
  *  \file game/cargospec.hpp
  *  \brief Class game::CargoSpec
  */
#ifndef C2NG_GAME_CARGOSPEC_HPP
#define C2NG_GAME_CARGOSPEC_HPP

#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "game/spec/cost.hpp"

namespace game {

    /** Cargo specification. A cargo specification contains an amount of
        - N/T/D/M
        - money, supplies
        - colonists
        - fighters
        - torpedoes of unspecified type

        This is used by the script interface.
        It is closely related to spec::Cost, which stores a cost consisting of T/D/M/sup/mc only.
        Having both separate frees Cost users from having to deal with the other types. */
    class CargoSpec {
     public:
        enum Type {
            Neutronium,
            Tritanium,
            Duranium,
            Molybdenum,
            Fighters,
            Colonists,
            Supplies,
            Money,
            Torpedoes
        };
        static const size_t LIMIT = Torpedoes+1;

        /** Construct blank cargospec. */
        CargoSpec();

        /** Construct from Cost.
            \param cost value */
        CargoSpec(const spec::Cost& cost);

        /** Construct from CargoSpec or PHost string.
            \see parse
            \param str string to parse
            \param acceptMax accept "max" as value */
        CargoSpec(const String_t& str, bool acceptMax);

        /** Add.
            \param other Value to add to this one.
            \return *this */
        CargoSpec& operator+=(const CargoSpec& other);

        /** Subtract.
            \param other Value to subtract from this one
            \return *this */
        CargoSpec& operator-=(const CargoSpec& other);

        /** Multiply in-place.
            \param n Factor to multiply this with.
            \return *this */
        CargoSpec& operator*=(int32_t n);

        /** Multiply.
            \param n Factor to multiply with
            \return scaled cost */
        CargoSpec  operator*(int32_t n) const;

        /** Compare for equality.
            \param other other value
            \return true iff these two CargoSpec are equal */
        bool  operator==(const CargoSpec& other) const;

        /** Compare for inequality.
            \param other other value
            \return true iff these two CargoSpec are different */
        bool  operator!=(const CargoSpec& other) const;

        /** Convert to PHost string.
            Note that this returns a valid string accepted by PHost for a "cost" option only
            if its content is restricted to positive amounts of T/D/M/S/$;
            "beamup" accepts everything but W (Torpedoes) and F (Fighters). */
        String_t toPHostString() const;

        /** Convert to CCScript-style string.
            Generates a list of words of the form "nnnT", with T identifying the resource (may be more than one), nnn being the amount.
            If this cost is valid, the resultant string will be a valid parameter for CCScript cargo operations.
            \return string, possibly blank */
        String_t toCargoSpecString() const;

        /** Convert to cost.
            This discards everything not accepted by Cost.
            \return cost */
        spec::Cost toCost() const;

        /** Parse cargo specification.
            Accepts both PHost ("Tnnn") and cargospec ("nnnT") format.
            The parse result will replace the content of this CargoSpec.
            \param str String to parse
            \param acceptMax true to accept "Tmax" format used by PHost. "max" will be treated as 10000.
            \return true on success */
        bool parse(const String_t& str, bool acceptMax);

        /** Set component value.
            \param type component
            \param n amount */
        void set(Type type, int32_t n);

        /** Get component value.
            \param type component
            \return amount */
        int32_t get(Type type) const;

        /** Add component.
            \param type component
            \param n amount to add */
        void add(Type type, int32_t n);

        /** Clear.
            Sets all components to zero. */
        void clear();

        /** Check whether this cargospec contains at least as much as required for \c other.
            Supply sale is taken into account.
            \param other price of the item
            \return true iff this CargoSpec contains the same or more than \c other. Supply sale is handled. */
        bool isEnoughFor(const CargoSpec& other) const;

        /** Check validity.
            Invalid CargoSpec have values below zero.
            \return true iff all components are non-negative */
        bool isNonNegative() const;

        /** Check whether this CargoSpec is empty.
            \return true iff all components are zero */
        bool isZero() const;

        /** Perform supply sale.
            If this contains a negative amount of money, try to make it positive by selling supplies. */
        void sellSuppliesIfNeeded();

        /** In-place divide by integer.
            Splits this CargoSpec evenly into n parts, such that (resultValue*=n).isEnoughFor(originalValue).
            \retval true on success
            \retval false if this divides by zero */
        bool divide(int32_t n);

        /** Divide by CargoSpec.
            Computes how many times \c other fits into \c *this, such that (other*=result).isEnoughFor(*this).
            Alternatively, if this is the amount of cargo available, and \c other is a part
            cost, computes the number of parts that can be built.
            Supply sale is taken into account.
            \param other [in] Part cost
            \param result [out] Part count
            \retval true on success
            \retval false if this divides by zero (other is empty) */
        bool divide(const CargoSpec& other, int32_t& result) const;

        /** Convert character into cargo type.
            \param c [in] character
            \param type [out] cargo type
            \retval true conversion succeeded
            \retval false character didn't match any known cargo type */
        static bool charToType(char c, Type& type);

     private:
        int32_t m_amounts[LIMIT];
    };

    /** makePrintable for testing.
        \param ref Reference */
    inline String_t makePrintable(const CargoSpec& c)
    {
        return c.toCargoSpecString();
    }

}

// Compare for inequality.
inline bool
game::CargoSpec::operator!=(const CargoSpec& other) const
{
    return !operator==(other);
}

// Set component value.
inline void
game::CargoSpec::set(Type type, int32_t n)
{
    m_amounts[type] = n;
}

// Get component value.
inline int32_t
game::CargoSpec::get(Type type) const
{
    return m_amounts[type];
}

// Add component.
inline void
game::CargoSpec::add(Type type, int32_t n)
{
    m_amounts[type] += n;
}

#endif
