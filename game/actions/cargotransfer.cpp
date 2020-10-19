/**
  *  \file game/actions/cargotransfer.cpp
  *  \brief Class game::actions::CargoTransfer
  */

#include <algorithm>
#include "game/actions/cargotransfer.hpp"
#include "game/exception.hpp"

namespace {
    int32_t doMove(int32_t amount,
                   game::Element::Type fromType,
                   game::CargoContainer& fromContainer,
                   game::Element::Type toType,
                   game::CargoContainer& toContainer,
                   bool partial)
    {
        // ex GCargoTransfer::moveFT [part]
        int32_t fromAmount = fromContainer.getEffectiveAmount(fromType) - fromContainer.getMinAmount(fromType);
        int32_t toSpace = toContainer.getMaxAmount(toType) - toContainer.getEffectiveAmount(toType);
        int32_t adjustedAmount = std::min(amount, std::min(fromAmount, toSpace));

        // Degenerate case: we're already overloaded
        if (adjustedAmount < 0) {
            return 0;
        }

        // Partial move?
        if (adjustedAmount != amount && !partial) {
            return 0;
        }

        // Do it
        fromContainer.change(fromType, -adjustedAmount);
        toContainer.change(toType, adjustedAmount);
        return adjustedAmount;
    }
}


// Constructor.
game::actions::CargoTransfer::CargoTransfer()
    : m_units(),
      m_overload(false)
{ }

// Destructor.
game::actions::CargoTransfer::~CargoTransfer()
{ }

// Add new participant.
void
game::actions::CargoTransfer::addNew(CargoContainer* container)
{
    // ex GCargoTransfer::addNewContainer
    if (container != 0) {
        m_units.pushBackNew(container);
        container->sig_change.add(&sig_change, &afl::base::Signal<void()>::raise);
        container->setOverload(m_overload);
    }
}

// Get participant by index.
game::CargoContainer*
game::actions::CargoTransfer::get(size_t index)
{
    // ex GCargoTransfer::operator[]
    return (index < m_units.size()
            ? m_units[index]
            : 0);
}

// Get number of participants.
size_t
game::actions::CargoTransfer::getNumContainers() const
{
    // ex GCargoTransfer::size
    return m_units.size();
}

// Set overload permission.
void
game::actions::CargoTransfer::setOverload(bool enable)
{
    m_overload = enable;
    for (size_t i = 0, n = m_units.size(); i < n; ++i) {
        m_units[i]->setOverload(enable);
    }    
}

// Check overload mode.
bool
game::actions::CargoTransfer::isOverload() const
{
    return m_overload;
}

// Move cargo.
int32_t
game::actions::CargoTransfer::move(Element::Type type, int32_t amount, size_t from, size_t to, bool partial, bool sellSupplies)
{
    // ex GCargoTransfer::moveFT, GCargoTransfer::move
    // Sort out trivial case
    if (amount == 0 || from == to) {
        return 0;
    }
    if (from >= m_units.size() || to >= m_units.size()) {
        return 0;
    }

    // Get containers
    CargoContainer& fromContainer = *m_units[from];
    CargoContainer& toContainer   = *m_units[to];

    // Get elements
    Element::Type fromType = type;
    Element::Type toType   = (sellSupplies && type == Element::Supplies && amount >= 0 && isSupplySaleAllowed()
                              ? Element::Money
                              : type);

    // Verify type
    if (!fromContainer.canHaveElement(fromType) || !toContainer.canHaveElement(toType)) {
        return 0;
    }

    // Move
    if (amount < 0) {
        return -doMove(-amount, toType, toContainer, fromType, fromContainer, partial);
    } else {
        return doMove(amount, fromType, fromContainer, toType, toContainer, partial);
    }
}

// Move cargo specified by a CargoSpec.
void
game::actions::CargoTransfer::move(CargoSpec& amount, const game::spec::ShipList& shipList, size_t from, size_t to, bool sellSupplies)
{
    // ex int/if/shipif.cc:doScriptTransfer [part]
    static const struct Map {
        CargoSpec::Type csType : 8;
        Element::Type eleType : 8;
    } map[] = {
        { CargoSpec::Neutronium, Element::Neutronium },
        { CargoSpec::Tritanium,  Element::Tritanium },
        { CargoSpec::Duranium,   Element::Duranium },
        { CargoSpec::Molybdenum, Element::Molybdenum },
        { CargoSpec::Fighters,   Element::Fighters },
        { CargoSpec::Colonists,  Element::Colonists },
        { CargoSpec::Supplies,   Element::Supplies },
        { CargoSpec::Money,      Element::Money },
    };

    // FIXME: this will fail if the transfer causes a temporary overload.
    // Some of those cases can be solved rather easily, some need more work:
    // - a Medium freighter (200 cargo) unloading 200 clans, uploading 200T.
    //   This will fail because we upload Tritanium first; could be solved by trying multiple orders.
    // - two Medium freighters, one with 150T, one with 150M, exchanging to 75T+75M on both.
    //   This requires multiple passes.
    //   The worst-case number of passes is the size of the cargo room if we have just one unit of free space.
    //   Be careful to not loop forever if there is no free space.
    // This affects c2web which currently (20200611) works around this by always enabling Overload.

    // Move normal stuff
    for (size_t i = 0; i < sizeof(map)/sizeof(map[0]); ++i) {
        amount.add(map[i].csType, -move(map[i].eleType, amount.get(map[i].csType), from, to, true, sellSupplies));
    }

    // Move weapons.
    // This is a hack, but it's the same one as used in PCC 1.x :-)
    // Because we can only move exact torpedo types, but we know that the user wants torpedoes, just try them all.
    // Only one of them will work (or none if the units are incompatible).
    for (int i = 1; i < shipList.launchers().size(); ++i) {
        amount.add(CargoSpec::Torpedoes, -move(Element::fromTorpedoType(i), amount.get(CargoSpec::Torpedoes), from, to, true, sellSupplies));
    }
}

// Unload operation.
bool
game::actions::CargoTransfer::unload(bool sellSupplies)
{
    // ex GCargoTransfer::doUnload
    // @change: we allow N-to-1 unload

    // Figure out possible receiver
    bool haveReceiver = false;
    size_t receiverIndex = 0;
    for (size_t i = 0, n = m_units.size(); i < n; ++i) {
        if (m_units[i]->getFlags().contains(CargoContainer::UnloadTarget)) {
            if (haveReceiver) {
                // Ambiguous
                return false;
            }
            haveReceiver = true;
            receiverIndex = i;
        }
    }

    // Fail if we don't have a receiver
    if (!haveReceiver) {
        return false;
    }

    // Now, perform the transfer
    bool ok = false;
    for (size_t i = 0, n = m_units.size(); i < n; ++i) {
        if (i != receiverIndex && m_units[i]->getFlags().contains(CargoContainer::UnloadSource)) {
            CargoContainer& source = *m_units[i];
            move(Element::Tritanium,  source.getEffectiveAmount(Element::Tritanium),  i, receiverIndex, true, sellSupplies);
            move(Element::Duranium,   source.getEffectiveAmount(Element::Duranium),   i, receiverIndex, true, sellSupplies);
            move(Element::Molybdenum, source.getEffectiveAmount(Element::Molybdenum), i, receiverIndex, true, sellSupplies);
            move(Element::Colonists,  source.getEffectiveAmount(Element::Colonists),  i, receiverIndex, true, sellSupplies);
            move(Element::Supplies,   source.getEffectiveAmount(Element::Supplies),   i, receiverIndex, true, sellSupplies);
            move(Element::Money,      source.getEffectiveAmount(Element::Money),      i, receiverIndex, true, sellSupplies);
            ok = true;
        }
    }
    return ok;
}

// Check whether unload is allowed.
bool
game::actions::CargoTransfer::isUnloadAllowed() const
{
    size_t numSources = 0;
    size_t numTargets = 0;
    for (size_t i = 0, n = m_units.size(); i < n; ++i) {
        // A container having UnloadTarget + UnloadSource is treated as being just UnloadTarget by unload().
        // This is an if/else-if to replicate that behaviour.
        const CargoContainer::Flags_t flags = m_units[i]->getFlags();
        if (flags.contains(CargoContainer::UnloadTarget)) {
            ++numTargets;
        } else if (flags.contains(CargoContainer::UnloadSource)) {
            ++numSources;
        } else {
            // ignore
        }
    }
    return (numSources > 0) && (numTargets == 1);
}

// Check whether supply sale is allowed.
bool
game::actions::CargoTransfer::isSupplySaleAllowed() const
{
    for (size_t i = 0, n = m_units.size(); i < n; ++i) {
        if (m_units[i]->getFlags().contains(CargoContainer::SupplySale)) {
            return true;
        }
    }
    return false;
}

// Get permitted element types.
game::ElementTypes_t
game::actions::CargoTransfer::getElementTypes(const game::spec::ShipList& shipList) const
{
    // ex doCargoTransfer (part)
    // Check general availability
    ElementTypes_t allowedTypes;
    ElementTypes_t presentTypes;
    for (Element::Type type = Element::begin(), end = Element::end(shipList); type != end; ++type) {
        bool allowed = true;
        bool present = false;
        for (size_t i = 0, n = m_units.size(); i < n; ++i) {
            if (!m_units[i]->canHaveElement(type)) {
                allowed = false;
            }
            if (m_units[i]->getAmount(type) > m_units[i]->getMinAmount(type)) {
                present = true;
            }
        }
        if (allowed) {
            allowedTypes += type;
        }
        if (present) {
            presentTypes += type;
        }
    }

    // If we can sell supplies, pretend money is present
    if (presentTypes.contains(Element::Supplies) && !presentTypes.contains(Element::Money)) {
        if (isSupplySaleAllowed()) {
            presentTypes += Element::Money;
        }
    }

    return allowedTypes & presentTypes;
}

// Check validity of transaction.
bool
game::actions::CargoTransfer::isValid() const
{
    // ex GCargoTransfer::isValid
    for (size_t i = 0, n = m_units.size(); i < n; ++i) {
        if (!m_units[i]->isValid()) {
            return false;
        }
        if (m_units[i]->getFlags().contains(CargoContainer::Temporary) && !m_units[i]->isEmpty()) {
            return false;
        }
    }
    return true;
}

// Commit.
void
game::actions::CargoTransfer::commit()
{
    // ex GCargoTransfer::commit
    if (!isValid()) {
        throw Exception(Exception::ePerm);
    }
    for (size_t i = 0, n = m_units.size(); i < n; ++i) {
        m_units[i]->commit();
    }
}
