/**
  *  \file game/cargospec.cpp
  *  \brief Class game::CargoSpec
  */

#include <cstring>
#include "game/cargospec.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "game/limits.hpp"

namespace {
    /** Cargo type letters.
        Indexes match CargoSpec::Type. */
    const char CARGO_TYPE_LETTERS[] = "NTDMFCS$W";
    static_assert(sizeof(CARGO_TYPE_LETTERS) == game::CargoSpec::LIMIT+1, "CARGO_TYPE_LETTERS vs. LIMIT");

    /** Parse number from string.
        \param result [out] number
        \param value [in] the string
        \param i [in/out] current parse position
        \return true on success */
    bool eatNumber(int32_t& result, const String_t& value, String_t::size_type& i)
    {
        int32_t amount = 0;
        int32_t sign = 1;
        bool digits = false;

        // Parse sign
        if (i < value.size() && value[i] == '-') {
            sign = -1;
            ++i;
        } else if (i < value.size() && value[i] == '+') {
            ++i;
        }

        // Parse digits
        while (i < value.size() && value[i] >= '0' && value[i] <= '9') {
            amount = 10*amount + value[i] - '0';
            digits = true;
            ++i;
        }
        result = sign*amount;
        return digits;
    }

    /** Parse "max" token.
        \param value [in] string
        \param i [in/out] current parse position
        \retval true "max" token found, \c i has been advanced
        \retval false "max" token not found, \c i unchanged */
    bool eatMax(const String_t& value, String_t::size_type& i)
    {
        String_t::size_type ii = i;
        String_t::size_type j;
        String_t::size_type limit = value.size();
        static const char max[] = "max";
        for (j = 0; j < 3; ++j) {
            if (ii+j >= limit || afl::string::charToLower(value[ii+j]) != max[j]) {
                break;
            }
        }
        if (j == 0 || (ii+j < limit && value[ii+j] != ' ' && value[ii+j] != '\t')) {
            return false;
        }
        i += j;
        return true;
    }
}

// Construct blank cargospec.
game::CargoSpec::CargoSpec()
{
    clear();
}

// Construct from Cost.
game::CargoSpec::CargoSpec(const spec::Cost& cost)
{
    clear();
    set(Tritanium,  cost.get(cost.Tritanium));
    set(Duranium,   cost.get(cost.Duranium));
    set(Molybdenum, cost.get(cost.Molybdenum));
    set(Money,      cost.get(cost.Money));
    set(Supplies,   cost.get(cost.Supplies));
}

// Construct from CargoSpec or PHost string.
game::CargoSpec::CargoSpec(const String_t& str, bool acceptMax)
{
    parse(str, acceptMax);
}

// Add.
game::CargoSpec&
game::CargoSpec::operator+=(const CargoSpec& other)
{
    for (size_t i = 0; i < LIMIT; ++i) {
        m_amounts[i] += other.m_amounts[i];
    }
    return *this;
}

// Subtract.
game::CargoSpec&
game::CargoSpec::operator-=(const CargoSpec& other)
{
    for (size_t i = 0; i < LIMIT; ++i) {
        m_amounts[i] -= other.m_amounts[i];
    }
    return *this;
}

// Multiply in-place.
game::CargoSpec&
game::CargoSpec::operator*=(int32_t n)
{
    for (size_t i = 0; i < LIMIT; ++i) {
        m_amounts[i] *= n;
    }
    return *this;
}

// Multiply.
game::CargoSpec
game::CargoSpec::operator*(int32_t n) const
{
    CargoSpec result(*this);
    result *= n;
    return result;
}

// Compare for equality.
bool
game::CargoSpec::operator==(const CargoSpec& other) const
{
    for (size_t i = 0; i < LIMIT; ++i) {
        if (m_amounts[i] != other.m_amounts[i]) {
            return false;
        }
    }
    return true;
}

// Compare for inequality.
bool
game::CargoSpec::operator!=(const CargoSpec& other) const
{
    return !operator==(other);
}


// Convert to PHost string.
String_t
game::CargoSpec::toPHostString() const
{
    String_t result;
    for (size_t i = 0; i < LIMIT; ++i) {
        if (m_amounts[i] != 0) {
            if (!result.empty()) {
                result += ' ';
            }
            result += CARGO_TYPE_LETTERS[i];
            result += afl::string::Format("%d", m_amounts[i]);
        }
    }
    if (result.empty()) {
        result = "S0";
    }
    return result;
}

// Convert to CCScript-style string.
String_t
game::CargoSpec::toCargoSpecString() const
{
    // ex ccexpr.pas:CargospecToString
    String_t result;
    afl::bits::SmallSet<Type> did;

    // Special case: if it has T=D=M, combine these
    if (m_amounts[Tritanium] != 0 && m_amounts[Tritanium] == m_amounts[Duranium] && m_amounts[Tritanium] == m_amounts[Molybdenum]) {
        result += afl::string::Format("%dTDM", m_amounts[Tritanium]);
        did += Tritanium;
        did += Molybdenum;
        did += Duranium;
    }

    // Regular processing
    for (size_t i = 0; i < LIMIT; ++i) {
        if (!did.contains(Type(i)) && m_amounts[i] != 0) {
            if (!result.empty()) {
                result += ' ';
            }
            result += afl::string::Format("%d%c", m_amounts[i], CARGO_TYPE_LETTERS[i]);
        }
    }
    return result;
}

// Convert to cost.
game::spec::Cost
game::CargoSpec::toCost() const
{
    game::spec::Cost rv;
    rv.set(rv.Tritanium,  get(Tritanium));
    rv.set(rv.Duranium,   get(Duranium));
    rv.set(rv.Molybdenum, get(Molybdenum));
    rv.set(rv.Supplies,   get(Supplies));
    rv.set(rv.Money,      get(Money));
    return rv;
}

// Parse cargo specification.
bool
game::CargoSpec::parse(const String_t& str, bool acceptMax)
{
    // ex GCargoSpec::parse
    // ex ccexpr.pas:ParseCargospec
    // ex pconfig.pas:ParseCost
    clear();

    /* Unlike PCC 1.x, we accept cargospecs and PHost format:
          Cargospec:  123TDM
          PHost:      T123 D123 M123 */
    String_t::size_type i = 0;
    while (1) {
        // skip whitespace
        while (i < str.size() && (str[i] == ' ' || str[i] == '\t')) {
            ++i;
        }
        if (i >= str.size()) {
            break;
        }

        if (str[i] == '-' || str[i] == '+' || (str[i] >= '0' && str[i] <= '9')) {
            // Cargospec
            int32_t amount;
            if (!eatNumber(amount, str, i)) {
                return false;
            }
            Type type;
            bool ok = false;
            while (i < str.size() && charToType(str[i], type)) {
                add(type, amount);
                ++i;
                ok = true;
            }
            if (!ok) {
                // Just a number and no type
                return false;
            }
        } else {
            // Must be PHost format
            Type type;
            if (!charToType(str[i], type)) {
                return false;
            }
            ++i;
            if (i >= str.size()) {
                // Syntax error
                return false;
            }
            if (acceptMax && eatMax(str, i)) {
                add(type, MAX_NUMBER);
            } else {
                int32_t amount;
                if (!eatNumber(amount, str, i)) {
                    return false;
                }
                add(type, amount);
            }
        }
    }
    return true;
}

// Set component value.
void
game::CargoSpec::set(Type type, int32_t n)
{
    m_amounts[type] = n;
}

// Get component value.
int32_t
game::CargoSpec::get(Type type) const
{
    return m_amounts[type];
}

// Add component.
void
game::CargoSpec::add(Type type, int32_t n)
{
    m_amounts[type] += n;
}

// Clear.
void
game::CargoSpec::clear()
{
    // ex GCargoSpec::clear
    for (size_t i = 0; i < LIMIT; ++i) {
        m_amounts[i] = 0;
    }
}

// Check whether this cargospec contains at least as much as required for \c other.
bool
game::CargoSpec::isEnoughFor(const CargoSpec& other) const
{
    // ex GCargoSpec::isEnoughFor
    // Subtract, and perform supply sale
    CargoSpec tmp(*this);
    tmp -= other;
    tmp.sellSuppliesIfNeeded();
    return tmp.isNonNegative();
}

// Check validity.
bool
game::CargoSpec::isNonNegative() const
{
    // ex GCargoSpec::isValid
    for (size_t i = 0; i < LIMIT; ++i) {
        if (m_amounts[i] < 0) {
            return false;
        }
    }
    return true;
}

// Check whether this CargoSpec is empty.
bool
game::CargoSpec::isZero() const
{
    // ex GCargoSpec::isEmpty
    for (size_t i = 0; i < LIMIT; ++i) {
        if (m_amounts[i] != 0) {
            return false;
        }
    }
    return true;
}

// Perform supply sale.
void
game::CargoSpec::sellSuppliesIfNeeded()
{
    // ex GCargoSpec::doSupplySale()
    // ex ccexpr.pas:ConvertSupplies
    if (m_amounts[Money] < 0 && m_amounts[Supplies] > 0) {
        if (m_amounts[Supplies] >= -m_amounts[Money]) {
            // Enough supplies to cover this money shortage
            m_amounts[Supplies] += m_amounts[Money];
            m_amounts[Money] = 0;
        } else {
            // Not enough supplies, so sell as much as possible
            m_amounts[Money] += m_amounts[Supplies];
            m_amounts[Supplies] = 0;
        }
    }
}

// In-place divide by integer.
bool
game::CargoSpec::divide(int32_t n)
{
    // ex GCargoSpec::divide
    if (n == 0) {
        return false;
    } else {
        for (size_t i = 0; i < LIMIT; ++i) {
            m_amounts[i] /= n;
        }
        return true;
    }
}

// Divide by CargoSpec.
bool
game::CargoSpec::divide(const CargoSpec& other, int32_t& result) const
{
    bool did = false;

    // Handle most
    for (size_t i = 0; i < LIMIT; ++i) {
        if (i != Money && other.m_amounts[i] != 0) {
            int32_t q = m_amounts[i] / other.m_amounts[i];
            if (!did || q < result) {
                did = true;
                result = q;
            }
        }
    }

    // Handle money and supply sale
    int32_t moneyCost = other.m_amounts[Money] + other.m_amounts[Supplies];
    if (moneyCost != 0) {
        int32_t q = (m_amounts[Money] + m_amounts[Supplies]) / moneyCost;
        if (!did || q < result) {
            did = true;
            result = q;
        }
    }

    return did;
}

// Convert character into cargo type.
bool
game::CargoSpec::charToType(char c, Type& type)
{
    if (const char* p = std::strchr(CARGO_TYPE_LETTERS, afl::string::charToUpper(c))) {
        type = Type(p - CARGO_TYPE_LETTERS);
        return true;
    }
    return false;
}
