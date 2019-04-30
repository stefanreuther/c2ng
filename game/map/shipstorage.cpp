/**
  *  \file game/map/shipstorage.cpp
  */

#include "game/map/shipstorage.hpp"
#include "game/map/ship.hpp"
#include "game/actions/preconditions.hpp"

namespace {
    const int32_t MAX_CARGO = game::MAX_NUMBER;
    const int32_t MAX_OVERLOAD = 20000;
}

game::map::ShipStorage::ShipStorage(Ship& sh,
                                    InterpreterInterface& iface,
                                    const game::spec::ShipList& shipList)
    : CargoContainer(),
      m_ship(sh),
      m_interface(iface),
      m_shipList(shipList),
      m_changeConnection(sh.sig_change.add(&sig_change, &afl::base::Signal<void()>::raise))
{
    // ex GShipTransfer::GShipTransfer
    game::actions::mustBePlayed(sh);
}

game::map::ShipStorage::~ShipStorage()
{
    // ex GShipTransfer::~GShipTransfer
}

String_t
game::map::ShipStorage::getName(afl::string::Translator& tx) const
{
    // ex GShipTransfer::getName
    return m_ship.getName(PlainName, tx, m_interface);
}

game::CargoContainer::Flags_t
game::map::ShipStorage::getFlags() const
{
    return Flags_t() + UnloadSource;
}

bool
game::map::ShipStorage::canHaveElement(Element::Type type) const
{
    // ex GShipTransfer::canHaveCargo
    switch (type) {
     case Element::Neutronium:
     case Element::Tritanium:
     case Element::Duranium:
     case Element::Molybdenum:
     case Element::Colonists:
     case Element::Supplies:
     case Element::Money:
        return true;

     case Element::Fighters:
        return m_ship.getNumBays().orElse(0) > 0;

     default:
        const int numLaunchers = m_ship.getNumLaunchers().orElse(0);
        const int torpType = m_ship.getTorpedoType().orElse(0);
        return (numLaunchers > 0 && torpType > 0 && type == Element::fromTorpedoType(torpType));
    }
}

int32_t
game::map::ShipStorage::getMaxAmount(Element::Type type) const
{
    // ex GShipTransfer::getMaxCargo
    switch (type) {
     case Element::Neutronium:
        if (isOverload()) {
            return MAX_CARGO;
        } else if (const game::spec::Hull* pHull = getHull()) {
            return pHull->getMaxFuel();
        } else {
            return 0;
        }

     case Element::Money:
        return MAX_CARGO;

     default:
        const game::spec::Hull* pHull = getHull();
        int32_t available = (isOverload() ? MAX_OVERLOAD
                             : pHull != 0 ? pHull->getMaxCargo()
                             : 0);
        int32_t rest = available
            - getEffectiveAmount(Element::Tritanium) - getEffectiveAmount(Element::Duranium)
            - getEffectiveAmount(Element::Molybdenum) - getEffectiveAmount(Element::Supplies)
            - getEffectiveAmount(Element::Colonists);

        const int numLaunchers = m_ship.getNumLaunchers().orElse(0);
        const int torpType = m_ship.getTorpedoType().orElse(0);
        if (torpType > 0 && numLaunchers > 0) {
            rest -= getEffectiveAmount(Element::fromTorpedoType(torpType));
        }

        const int numBays = m_ship.getNumBays().orElse(0);
        if (numBays > 0) {
            rest -= getEffectiveAmount(Element::Fighters);
        }

        rest += getEffectiveAmount(type);

        // FIXME: protect against negative?
        return std::min(MAX_CARGO, rest);
    }
}

int32_t
game::map::ShipStorage::getMinAmount(Element::Type /*type*/) const
{
    // ex GShipTransfer::getDisplayOffset, sort-of
    return 0;
}

int32_t
game::map::ShipStorage::getAmount(Element::Type type) const
{
    // ex GShipTransfer::getCargoOnObject
    return m_ship.getCargo(type).orElse(0);
}

void
game::map::ShipStorage::commit()
{
    for (Element::Type i = Element::begin(), n = getTypeLimit(); i < n; ++i) {
        if (int32_t delta = getChange(i)) {
            m_ship.setCargo(i, IntegerProperty_t(m_ship.getCargo(i).orElse(0) + delta));
        }
    }
}

const game::spec::Hull*
game::map::ShipStorage::getHull() const
{
    return m_shipList.hulls().get(m_ship.getHull().orElse(-1));
}
