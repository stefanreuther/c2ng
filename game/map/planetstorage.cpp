/**
  *  \file game/map/planetstorage.cpp
  */

#include "game/map/planetstorage.hpp"
#include "game/map/planet.hpp"
#include "game/actions/preconditions.hpp"
#include "game/limits.hpp"

game::map::PlanetStorage::PlanetStorage(Planet& pl,
                                        InterpreterInterface& iface,
                                        const game::config::HostConfiguration& config)
    : CargoContainer(),
      m_planet(pl),
      m_interface(iface),
      m_hostConfiguration(config),
      m_changeConnection(pl.sig_change.add(&sig_change, &afl::base::Signal<void()>::raise))
{
    // ex GPlanetTransfer::GPlanetTransfer
    game::actions::mustBePlayed(pl);
}

game::map::PlanetStorage::~PlanetStorage()
{
    // ex GPlanetTransfer::~GPlanetTransfer
}

String_t
game::map::PlanetStorage::getName(afl::string::Translator& tx) const
{
    // ex GPlanetTransfer::getName
    // FIXME: PCC 1.x says "$NAME starbase" or "planet $NAME" here.
    // should we use same wording?
    return m_planet.getName(Planet::PlainName, tx, m_interface);
}

game::CargoContainer::Flags_t
game::map::PlanetStorage::getFlags() const
{
    return Flags_t() + UnloadTarget + SupplySale;
}

bool
game::map::PlanetStorage::canHaveElement(Element::Type type) const
{
    // ex GPlanetTransfer::canHaveCargo
    int torpType;
    if (type == Element::Fighters || Element::isTorpedoType(type, torpType)) {
        return m_planet.hasBase();
    } else {
        return true;
    }
}

int32_t
game::map::PlanetStorage::getMaxAmount(Element::Type type) const
{
    // ex GPlanetTransfer::getMaxCargo
    // FIXME: Host .31 torp safety
    /* the torp/ore limits are arbitrary, but should match Tim's TRN check. */
    int torpType, planetOwner;
    if (type == Element::Fighters) {
        if (m_planet.getOwner(planetOwner)) {
            return m_hostConfiguration[game::config::HostConfiguration::MaximumFightersOnBase](planetOwner);
        } else {
            return 0;
        }
    } else if (Element::isTorpedoType(type, torpType)) {
        return MAX_NUMBER;
    } else {
        return 1000000000;
    }
}

int32_t
game::map::PlanetStorage::getMinAmount(Element::Type /*type*/) const
{
    return 0;
}

int32_t
game::map::PlanetStorage::getAmount(Element::Type type) const
{
    // ex GPlanetTransfer::getCargoOnObject
    return m_planet.getCargo(type).orElse(0);
}

void
game::map::PlanetStorage::commit()
{
    for (Element::Type i = Element::begin(), n = getTypeLimit(); i < n; ++i) {
        if (int32_t delta = getChange(i)) {
            m_planet.setCargo(i, LongProperty_t(m_planet.getCargo(i).orElse(0) + delta));
        }
    }
}
