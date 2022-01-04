/**
  *  \file game/map/shipstorage.cpp
  */

#include "game/map/shipstorage.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/map/ship.hpp"
#include "game/map/shiputils.hpp"
#include "util/string.hpp"
#include "util/unicodechars.hpp"

namespace {
    const int32_t MAX_CARGO = game::MAX_NUMBER;
    const int32_t MAX_OVERLOAD = 20000;
}

game::map::ShipStorage::ShipStorage(Ship& sh, const game::spec::ShipList& shipList)
    : CargoContainer(),
      m_ship(sh),
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
game::map::ShipStorage::getName(afl::string::Translator& /*tx*/) const
{
    // ex GShipTransfer::getName
    return m_ship.getName();
}

String_t
game::map::ShipStorage::getInfo1(afl::string::Translator& tx) const
{
    // ex WMultiTransferUnitInfo::drawContent (part)
    String_t result;
    if (const game::spec::Hull* pHull = m_shipList.hulls().get(m_ship.getHull().orElse(0))) {
        result += pHull->getShortName(m_shipList.componentNamer());
    }

    int numBeams = m_ship.getNumBeams().orElse(0);
    if (numBeams > 0) {
        if (const game::spec::Beam* pBeam = m_shipList.beams().get(m_ship.getBeamType().orElse(0))) {
            util::addListItem(result, ", ", afl::string::Format("%d" UTF_TIMES "%s", numBeams, pBeam->getShortName(m_shipList.componentNamer())));
        }
    }

    int numLaunchers = m_ship.getNumLaunchers().orElse(0);
    if (numLaunchers > 0) {
        if (const game::spec::TorpedoLauncher* pLauncher = m_shipList.launchers().get(m_ship.getTorpedoType().orElse(0))) {
            util::addListItem(result, ", ", afl::string::Format("%d" UTF_TIMES "%s", numLaunchers, pLauncher->getShortName(m_shipList.componentNamer())));
        }
    }

    int numBays = m_ship.getNumBays().orElse(0);
    if (numBays > 0) {
        util::addListItem(result, ", ", afl::string::Format(tx("%d\xC3\x97" "Ftr"), numBays));
    }

    return result;
}

String_t
game::map::ShipStorage::getInfo2(afl::string::Translator& tx) const
{
    return afl::string::Format(tx("FCode: \"%s\", Damage: %d%%"), m_ship.getFriendlyCode().orElse(String_t()), m_ship.getDamage().orElse(0));
}

game::CargoContainer::Flags_t
game::map::ShipStorage::getFlags() const
{
    return Flags_t() + UnloadSource;
}

bool
game::map::ShipStorage::canHaveElement(Element::Type type) const
{
    // ex GShipTransfer::canHaveCargo, TShip.CanHave
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
    return getShipTransferMaxCargo(*this, type, m_ship, m_shipList);
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
