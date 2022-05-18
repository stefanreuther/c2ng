/**
  *  \file game/map/beamupshiptransfer.cpp
  *
  *  PCC2 Comment:
  *
  *  Beam up multiple is implemented by using two custom transfer
  *  objects, completely unrelated to the normal cargo transfer
  *  partners. The planet half contains 10000 (minus existing command)
  *  of everything; the ship half contains 0 (plus existing command).
  *  Display is adjusted to actual content using display offsets, so
  *  users can (and should) easily overdraw.
  *
  *  In PCC 1.x, the BUM halves are a little closer related, making things
  *  a little more complex but sharing the "init from planet" part.
  *  PCC 1.x also has a "beam up+down" mode which is a cross of BUM
  *  and a regular cargo unload; I believe this feature to be very
  *  hard to understand, so I left it out here.
  */

#include "game/map/beamupshiptransfer.hpp"
#include "game/actions/preconditions.hpp"
#include "game/cargospec.hpp"
#include "game/map/fleetmember.hpp"
#include "game/map/shiputils.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"
#include "game/map/reverter.hpp"

namespace {
    using game::v3::Command;
    using game::v3::CommandContainer;
    using game::v3::CommandExtra;
}

game::map::BeamUpShipTransfer::BeamUpShipTransfer(Ship& sh,
                                                  const game::spec::ShipList& shipList,
                                                  Turn& turn,
                                                  const game::map::Configuration& mapConfig,
                                                  const game::config::HostConfiguration& config)
    : CargoContainer(),
      m_ship(sh),
      m_shipList(shipList),
      m_turn(turn),
      m_mapConfig(mapConfig),
      m_config(config),
      m_amount()
{
    // ex GShipBumTransfer::GShipBumTransfer
    game::actions::mustBePlayed(sh);
    parseBeamUpCommand(m_amount, turn, sh, +1);
}

game::map::BeamUpShipTransfer::~BeamUpShipTransfer()
{
    // ex GShipBumTransfer::~GShipBumTransfer
}

String_t
game::map::BeamUpShipTransfer::getName(afl::string::Translator& /*tx*/) const
{
    // ex GShipBumTransfer::getName
    return m_ship.getName();
}

String_t
game::map::BeamUpShipTransfer::getInfo1(afl::string::Translator& /*tx*/) const
{
    return String_t();
}

String_t
game::map::BeamUpShipTransfer::getInfo2(afl::string::Translator& /*tx*/) const
{
    return String_t();
}

game::CargoContainer::Flags_t
game::map::BeamUpShipTransfer::getFlags() const
{
    return Flags_t(UnloadSource);
}

bool
game::map::BeamUpShipTransfer::canHaveElement(Element::Type type) const
{
    // ex GShipBumTransfer::canHaveCargo
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
game::map::BeamUpShipTransfer::getMaxAmount(Element::Type type) const
{
    // ex GShipBumTransfer::getMaxCargo (totally changed)
    return getShipTransferMaxCargo(*this, type, m_ship, m_shipList);
}

int32_t
game::map::BeamUpShipTransfer::getMinAmount(Element::Type type) const
{
    return m_ship.getCargo(type).orElse(0);
}

int32_t
game::map::BeamUpShipTransfer::getAmount(Element::Type type) const
{
    return m_amount.get(type) + getMinAmount(type);
}

void
game::map::BeamUpShipTransfer::commit()
{
    // ex GShipBumTransfer::commit
    CargoSpec cs;
    cs.set(CargoSpec::Neutronium, m_amount.get(Element::Neutronium) + getChange(Element::Neutronium));
    cs.set(CargoSpec::Tritanium,  m_amount.get(Element::Tritanium)  + getChange(Element::Tritanium));
    cs.set(CargoSpec::Duranium,   m_amount.get(Element::Duranium)   + getChange(Element::Duranium));
    cs.set(CargoSpec::Molybdenum, m_amount.get(Element::Molybdenum) + getChange(Element::Molybdenum));
    cs.set(CargoSpec::Money,      m_amount.get(Element::Money)      + getChange(Element::Money));
    cs.set(CargoSpec::Supplies,   m_amount.get(Element::Supplies)   + getChange(Element::Supplies));
    cs.set(CargoSpec::Colonists,  m_amount.get(Element::Colonists)  + getChange(Element::Colonists));

    int shipOwner = 0;
    m_ship.getOwner(shipOwner);

    const int missionNumber = m_config[m_config.ExtMissionsStartAt]() + game::spec::Mission::pmsn_BeamUpMultiple;

    if (cs.isZero()) {
        if (CommandContainer* cc = CommandExtra::get(m_turn, shipOwner)) {
            cc->removeCommand(Command::BeamUp, m_ship.getId());
        }
        if (m_ship.getMission().orElse(0) == missionNumber) {
            // Reset to previous mission.
            // If the ship had a different mission at the beginning of the turn, and that was not "beam up multiple", use that.
            bool resetOK = false;
            if (Reverter* pRev = m_turn.universe().getReverter()) {
                int m, i, t;
                if (pRev->getPreviousShipMission(m_ship.getId(), m, i, t)) {
                    if (m != missionNumber) {
                        // No need to handle setMission() failure. If it fails, there's nothing we can do.
                        // However, if this is a blocked fleet member, it shouldn't have mission "beam up multiple" in the first place.
                        FleetMember(m_turn.universe(), m_ship, m_mapConfig).setMission(m, i, t, m_config, m_shipList);
                        resetOK = true;
                    }
                }
            }

            // If mission was not reset above, clear it to "none".
            if (!resetOK) {
                FleetMember(m_turn.universe(), m_ship, m_mapConfig).setMission(0, 0, 0, m_config, m_shipList);
            }
        }
    } else {
        CommandExtra::create(m_turn).create(shipOwner).addCommand(Command::BeamUp, m_ship.getId(), cs.toPHostString());
        FleetMember(m_turn.universe(), m_ship, m_mapConfig).setMission(missionNumber, 0, 0, m_config, m_shipList);
    }

    // PCC2 explicitly marked the ship dirty.
    // Our CommandExtra::onCommandChange does that automatically.
}

void
game::map::parseBeamUpCommand(util::Vector<int32_t,Element::Type>& out, Turn& turn, const Ship& ship, int factor)
{
    int shipOwner = 0;
    ship.getOwner(shipOwner);
    if (const CommandContainer* cc = CommandExtra::get(turn, shipOwner)) {
        if (const Command* cmd = cc->getCommand(Command::BeamUp, ship.getId())) {
            CargoSpec cs(cmd->getArg(), true);
            out.set(Element::Neutronium, factor * cs.get(CargoSpec::Neutronium));
            out.set(Element::Tritanium,  factor * cs.get(CargoSpec::Tritanium));
            out.set(Element::Duranium,   factor * cs.get(CargoSpec::Duranium));
            out.set(Element::Molybdenum, factor * cs.get(CargoSpec::Molybdenum));
            out.set(Element::Colonists,  factor * cs.get(CargoSpec::Colonists));
            out.set(Element::Supplies,   factor * cs.get(CargoSpec::Supplies));
            out.set(Element::Money,      factor * cs.get(CargoSpec::Money));
        }
    }
}
