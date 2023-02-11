/**
  *  \file game/map/beamupplanettransfer.cpp
  *  \brief Class game::map::BeamUpPlanetTransfer
  */

#include "game/map/beamupplanettransfer.hpp"
#include "afl/string/format.hpp"
#include "game/map/beamupshiptransfer.hpp"
#include "game/map/planet.hpp"

game::map::BeamUpPlanetTransfer::BeamUpPlanetTransfer(Planet& pl,
                                                      const Ship& sh,
                                                      Turn& turn,
                                                      const game::config::HostConfiguration& config)
    : m_planet(pl),
      m_config(config),
      m_amount()
{
    // ex GPlanetBumTransfer::GPlanetBumTransfer
    /* No need to verify (mustBePlayed) the ship; we don't access it.
       It will be verified by the other half (BeamUpShipTransfer). */
    parseBeamUpCommand(m_amount, turn, sh, +1);
}

game::map::BeamUpPlanetTransfer::~BeamUpPlanetTransfer()
{ }

String_t
game::map::BeamUpPlanetTransfer::getName(afl::string::Translator& tx) const
{
    // ex GPlanetBumTransfer::getName
    return afl::string::Format(tx("Beam up from %s"), m_planet.getName(tx));
}

String_t
game::map::BeamUpPlanetTransfer::getInfo1(afl::string::Translator& /*tx*/) const
{
    return String_t();
}

String_t
game::map::BeamUpPlanetTransfer::getInfo2(afl::string::Translator& /*tx*/) const
{
    return String_t();
}

game::CargoContainer::Flags_t
game::map::BeamUpPlanetTransfer::getFlags() const
{
    return Flags_t(UnloadTarget);
}

bool
game::map::BeamUpPlanetTransfer::canHaveElement(Element::Type type) const
{
    // ex GPlanetBumTransfer::canHaveCargo
    // FIXME: duplicate of BeamUpShipTransfer::canHaveElement
    switch (type) {
     case Element::Neutronium:
     case Element::Tritanium:
     case Element::Duranium:
     case Element::Molybdenum:
     case Element::Money:
     case Element::Supplies:
        return true;
     case Element::Colonists:
        return m_config[m_config.AllowBeamUpClans]();
     default:
        return false;
    }
}

int32_t
game::map::BeamUpPlanetTransfer::getMaxAmount(Element::Type /*type*/) const
{
    // Report "infinity", so this is displayed as "(unl)" in cargo transfer.
    // Other side will make sure that we cannot actually beam down cargo.
    return 1000000000;
}

int32_t
game::map::BeamUpPlanetTransfer::getMinAmount(Element::Type type) const
{
    // Allow to take at most 10000 of each.
    return m_planet.getCargo(type).orElse(0) - 10000;
}

int32_t
game::map::BeamUpPlanetTransfer::getAmount(Element::Type type) const
{
    return m_planet.getCargo(type).orElse(0) - m_amount.get(type);
}

void
game::map::BeamUpPlanetTransfer::commit()
{ }
