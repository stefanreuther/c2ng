/**
  *  \file game/cargocontainer.cpp
  *  \brief Base class game::CargoContainer
  */

#include "game/cargocontainer.hpp"

// Constructor.
game::CargoContainer::CargoContainer()
    : m_delta(),
      m_overload(false)
{ }

// Destructor.
game::CargoContainer::~CargoContainer()
{ }

// Change amount of an element.
void
game::CargoContainer::change(Element::Type type, int32_t delta)
{
    if (delta != 0) {
        size_t index = static_cast<size_t>(type);
        if (m_delta.size() < index+1) {
            m_delta.resize(index+1);
        }
        m_delta[index] += delta;
        sig_change.raise();
    }
}

// Get current change.
int32_t
game::CargoContainer::getChange(Element::Type type) const
{
    size_t index = static_cast<size_t>(type);
    if (index < m_delta.size()) {
        return m_delta[index];
    } else {
        return 0;
    }
}

// Get effective amount.
int32_t
game::CargoContainer::getEffectiveAmount(Element::Type type) const
{
    return getAmount(type) + getChange(type);
}

// Clear everything.
void
game::CargoContainer::clear()
{
    m_delta.clear();
    sig_change.raise();
}

// Check validity.
bool
game::CargoContainer::isValid() const
{
    // We only check elements that have a nonzero delta.
    // This means that a "no-op" transaction from an invalid state is valid.
    // It also means that we can safely check only the content of the m_delta vector,
    // and don't get different behaviour if the transaction came about by being non-"no-op" temporarily.
    for (Element::Type t = Element::begin(); static_cast<size_t>(t) < m_delta.size(); ++t) {
        int32_t delta = getChange(t);
        if (delta != 0) {
            int32_t effectiveAmount = getAmount(t) + delta;
            if (effectiveAmount > getMaxAmount(t) || effectiveAmount < getMinAmount(t)) {
                return false;
            }
        }
    }
    return true;
}

// Check emptiness.
bool
game::CargoContainer::isEmpty() const
{
    for (size_t i = 0; i < m_delta.size(); ++i) {
        if (m_delta[i] != 0) {
            return false;
        }
    }
    return true;
}

// Set overload permission.
void
game::CargoContainer::setOverload(bool enable)
{
    // ex GCargoContainer::setFlags (sort-of)
    if (enable != m_overload) {
        m_overload = enable;
        sig_change.raise();
    }
}

// Check overload mode.
bool
game::CargoContainer::isOverload() const
{
    // ex GCargoContainer::getFlags (sort-of)
    return m_overload;
}

// Get upper limit for type.
game::Element::Type
game::CargoContainer::getTypeLimit() const
{
    return static_cast<Element::Type>(m_delta.size());
}
