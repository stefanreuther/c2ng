/**
  *  \file game/spec/cost.cpp
  *  \brief Class game::spec::Cost
  */

#include <algorithm>
#include "game/spec/cost.hpp"
#include "game/cargospec.hpp"

// static string_t formatCost(GCost c)
// {
//     string_t result;
//     static const GCargoType order[] = { el_Money, el_Supplies, el_Tritanium, el_Duranium, el_Molybdenum };
//     const char* labels[] = { _("mc"), _("sup"), "T", "D", "M" };
//     for (size_t i = 0; i < countof(order); ++i) {
//         int32_t n = c.get(order[i]);
//         if (n != 0) {
//             if (!result.empty()) {
//                 result += ", ";
//             }
//             result += numToString(n);
//             result += " ";
//             result += labels[i];
//             for (size_t j = i+1; j < countof(order); ++j) {
//                 if (c.get(order[j]) == n) {
//                     c.set(order[j], 0);
//                     result += "/";
//                     result += labels[j];
//                 }
//             }
//         }
//     }
//     if (result.empty()) {
//         result = "-";
//     }
//     return result;
// }

// Constructor.
game::spec::Cost::Cost()
{
    // ex GCost::GCost
    clear();
}


// FIXME: delete
// /** Try to sell supplies to cover a money shortage. */
// void
// GCost::trySupplySale()
// {
//     if (mc < 0) {
//         sup += mc;
//         mc = 0;
//         if (sup < 0) {
//             // not enough supplies - leave it a money shortage
//             mc = sup;
//             sup = 0;
//         }
//     }
// }

// Add costs.
game::spec::Cost&
game::spec::Cost::operator+=(const Cost& other)
{
    // ex GCost::operator+=
    for (size_t i = 0; i < LIMIT; ++i) {
        m_amounts[i] += other.m_amounts[i];
    }
    return *this;
}

// Subtract.
game::spec::Cost&
game::spec::Cost::operator-=(const Cost& other)
{
    // ex GCost::operator-= (without implicit supply sale!)
    for (size_t i = 0; i < LIMIT; ++i) {
        m_amounts[i] -= other.m_amounts[i];
    }
    return *this;
}

// Multiply in-place.
game::spec::Cost&
game::spec::Cost::operator*=(int n)
{
    // ex GCost::operator*=
    for (size_t i = 0; i < LIMIT; ++i) {
        m_amounts[i] *= n;
    }
    return *this;
}

// Multiply.
game::spec::Cost
game::spec::Cost::operator*(int32_t n) const
{
    // ex GCost::operator*
    Cost result(*this);
    result *= n;
    return result;
}

// Compare for equality.
bool
game::spec::Cost::operator==(const Cost& other) const
{
    // ex GCost::operator==
    for (size_t i = 0; i < LIMIT; ++i) {
        if (m_amounts[i] != other.m_amounts[i]) {
            return false;
        }
    }
    return true;
}

// Compare for inequality.
bool
game::spec::Cost::operator!=(const Cost& other) const
{
    // ex GCost::operator!=
    return !operator==(other);
}

// Convert to PHost-style string.
String_t
game::spec::Cost::toPHostString() const
{
    return CargoSpec(*this).toPHostString();
}

// Convert to CCScript-style string.
String_t
game::spec::Cost::toCargoSpecString() const
{
    return CargoSpec(*this).toCargoSpecString();
}

// Limit amount of items to build.
int32_t
game::spec::Cost::getMaxAmount(const int32_t orderedAmount, const Cost& itemCost) const
{
    // ex GCost::limitAmount
    int32_t result;
    if (!isNonNegative() || !itemCost.isNonNegative() || orderedAmount < 0) {
        // This cost is not valid, or amount is negative.
        // Neither allows us to fulfil our postcondition, so return 0.
        result = 0;
    } else {
        // Regular path
        result = orderedAmount;
        if (itemCost.get(Tritanium) != 0) {
            result = std::min(result, get(Tritanium) / itemCost.get(Tritanium));
        }
        if (itemCost.get(Duranium) != 0) {
            result = std::min(result, get(Duranium) / itemCost.get(Duranium));
        }
        if (itemCost.get(Molybdenum) != 0) {
            result = std::min(result, get(Molybdenum) / itemCost.get(Molybdenum));
        }
        if (itemCost.get(Supplies) != 0) {
            result = std::min(result, get(Supplies) / itemCost.get(Supplies));
        }
        if (itemCost.get(Supplies) + itemCost.get(Money) != 0) {
            result = std::min(result, (get(Supplies) + get(Money)) / (itemCost.get(Supplies) + itemCost.get(Money)));
        }
    }
    return result;
}

// Set component.
void
game::spec::Cost::set(Type type, int32_t n)
{
    // ex GCost::set
    m_amounts[type] = n;
}

// Get component.
int32_t
game::spec::Cost::get(Type type) const
{
    // ex GCost::get
    return m_amounts[type];
}

// Add component.
void
game::spec::Cost::add(Type type, int32_t n)
{
    // ex GCost::add
    set(type, get(type) + n);
}

// Clear this cost.
void
game::spec::Cost::clear()
{
    for (size_t i = 0; i < LIMIT; ++i) {
        m_amounts[i] = 0;
    }
}

// Check whether this resource amount is large enough to buy an item.
bool
game::spec::Cost::isEnoughFor(const Cost& other) const
{
    // ex GCost::isEnoughFor
    return other.get(Tritanium) <= get(Tritanium)
        && other.get(Duranium) <= get(Duranium)
        && other.get(Molybdenum) <= get(Molybdenum)
        && other.get(Supplies) <= get(Supplies)
        && other.get(Supplies) + other.get(Money) <= get(Supplies) + get(Money);
}

bool
game::spec::Cost::isNonNegative() const
{
    // ex GCost::isValid()
    for (size_t i = 0; i < LIMIT; ++i) {
        if (m_amounts[i] < 0) {
            return false;
        }
    }
    return true;
}
    
// Check whether this cost is empty.
bool
game::spec::Cost::isZero() const
{
    // ex GCost::isEmpty()
    for (size_t i = 0; i < LIMIT; ++i) {
        if (m_amounts[i] != 0) {
            return false;
        }
    }
    return true;
}

// Parse a string into a Cost structure.
game::spec::Cost
game::spec::Cost::fromString(const String_t& value)
{
    // ex GCost::fromString
    return CargoSpec(value, true).toCost();
}
