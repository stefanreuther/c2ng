/**
  *  \file game/map/shiptransporter.cpp
  *
  *  FIXME: missing feature: jettison/undo jettison money, ammo. PCC1 does it, PCC2 doesn't.
  */

#include "game/map/shiptransporter.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "util/translation.hpp"
#include "afl/base/countof.hpp"
#include "game/limits.hpp"

game::map::ShipTransporter::ShipTransporter(Ship& sh,
                                            Ship::Transporter type,
                                            Id_t targetId,
                                            InterpreterInterface& iface,
                                            const Universe& univ,
                                            HostVersion hostVersion)
    : CargoContainer(),
      m_ship(sh),
      m_type(type),
      m_targetId(targetId),
      m_interface(iface),
      m_universe(univ),
      m_allowParallelTransfer(hostVersion.hasParallelShipTransfers()),
      m_changeConnection(sh.sig_change.add(&sig_change, &afl::base::Signal<void()>::raise))
{
    // ex GShipTransporterTransfer::GShipTransporterTransfer
    game::actions::mustBePlayed(sh);
}

String_t
game::map::ShipTransporter::getName(afl::string::Translator& tx) const
{
    // ex GShipTransporterTransfer::getName
    if (m_type == Ship::UnloadTransporter) {
        if (m_targetId == 0) {
            return _("Jettison");
        } else if (const Planet* p = m_universe.planets().get(m_targetId)) {
            return p->getName(Object::PlainName, tx, m_interface);
        } else {
            return afl::string::Format(_("Planet %d").c_str(), m_targetId);
        }
    } else {
        if (const Ship* s = m_universe.ships().get(m_targetId)) {
            return s->getName(Object::PlainName, tx, m_interface);
        } else {
            return afl::string::Format(_("Ship %d").c_str(), m_targetId);
        }
    }
}

game::CargoContainer::Flags_t
game::map::ShipTransporter::getFlags() const
{
    // ex GShipTransporterTransfer::GShipTransporterTransfer [part]
    if (m_type == Ship::UnloadTransporter) {
        return Flags_t() + UnloadTarget;
    } else {
        return Flags_t() + UnloadSource;
    }
}

bool
game::map::ShipTransporter::canHaveElement(Element::Type type) const
{
    // ex GShipTransporterTransfer::canHaveCargo
    switch (type) {
     case Element::Neutronium:
     case Element::Tritanium:
     case Element::Duranium:
     case Element::Molybdenum:
     case Element::Supplies:
     case Element::Colonists:
        return true;
     default:
        return false;
    }
}

int32_t
game::map::ShipTransporter::getMaxAmount(Element::Type /*type*/) const
{
    // ex GShipTransporterTransfer::getMaxCargo
    return MAX_NUMBER;
}

int32_t
game::map::ShipTransporter::getMinAmount(Element::Type /*type*/) const
{
    // ex GShipTransporterTransfer::getDisplayOffset [sort-of]
    return 0;
}

int32_t
game::map::ShipTransporter::getAmount(Element::Type type) const
{
    // ex GShipTransporterTransfer::getCargoOnObject
    return m_ship.getTransporterCargo(m_type, type).orElse(0);
}

void
game::map::ShipTransporter::commit()
{
    // ex GShipTransporterTransfer::changeCargoOnObject [sort-of]
    static const Element::Type elems[] = {
        Element::Neutronium,
        Element::Tritanium,
        Element::Duranium,
        Element::Molybdenum,
        Element::Supplies,
        Element::Colonists
    };

    // Apply changes.
    // If someone modified the transporter in parallel, this will add (and possibly redirect) the transport, but maintain balances.
    bool nonzero = false;
    for (size_t i = 0; i < countof(elems); ++i) {
        int32_t newValue = m_ship.getTransporterCargo(m_type, elems[i]).orElse(0) + getChange(elems[i]);
        m_ship.setTransporterCargo(m_type, elems[i], newValue);
        if (newValue != 0) {
            nonzero = true;
        }
    }

    if (nonzero) {
        // Nonzero transfer
        m_ship.setTransporterTargetId(m_type, m_targetId);

        // If someone else set up a parallel transfer on the other slot although our host does not allow that, cancel it.
        // This way, we're certain to not produce data the loader (turn writer) cannot handle.
        // This may overload the ship, but that is only a minor (yellow) error that is handled by host.
        if (!m_allowParallelTransfer) {
            Ship::Transporter other = (m_type == Ship::UnloadTransporter ? Ship::TransferTransporter : Ship::UnloadTransporter);
            if (m_ship.isTransporterActive(other)) {
                m_ship.cancelTransporter(other);
            }
        }
    } else {
        // Null transfer: set target to 0, canceling the transport
        m_ship.setTransporterTargetId(m_type, 0);
    }
}
