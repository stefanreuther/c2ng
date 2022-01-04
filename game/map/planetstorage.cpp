/**
  *  \file game/map/planetstorage.cpp
  */

#include "game/map/planetstorage.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/limits.hpp"
#include "game/map/planet.hpp"

game::map::PlanetStorage::PlanetStorage(Planet& pl, const game::config::HostConfiguration& config)
    : CargoContainer(),
      m_planet(pl),
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
    return m_planet.getName(tx);
}

String_t
game::map::PlanetStorage::getInfo1(afl::string::Translator& tx) const
{
    if (m_planet.hasBase()) {
        return tx("Starbase");
    } else {
        return tx("Planet");
    }
}

String_t
game::map::PlanetStorage::getInfo2(afl::string::Translator& tx) const
{
    return afl::string::Format(tx("FCode: \"%s\""), m_planet.getFriendlyCode().orElse(String_t()));
}

game::CargoContainer::Flags_t
game::map::PlanetStorage::getFlags() const
{
    return Flags_t() + UnloadTarget + SupplySale;
}

bool
game::map::PlanetStorage::canHaveElement(Element::Type type) const
{
    // ex GPlanetTransfer::canHaveCargo, TPlanet.CanHave
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
    // ex GPlanetTransfer::getMaxCargo, TPlanet.Maximum
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
