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
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"

namespace {
    const int32_t MAX_CARGO = game::MAX_NUMBER;
    const int32_t MAX_OVERLOAD = 20000;

    using game::v3::Command;
    using game::v3::CommandContainer;
    using game::v3::CommandExtra;
}

game::map::BeamUpShipTransfer::BeamUpShipTransfer(Ship& sh,
                                                  InterpreterInterface& iface,
                                                  const game::spec::ShipList& shipList,
                                                  Turn& turn,
                                                  const game::config::HostConfiguration& config)
    : CargoContainer(),
      m_ship(sh),
      m_interface(iface),
      m_shipList(shipList),
      m_turn(turn),
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
game::map::BeamUpShipTransfer::getName(afl::string::Translator& tx) const
{
    // ex GShipBumTransfer::getName
    return m_ship.getName(PlainName, tx, m_interface);
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
    // FIXME: this function is copied from ShipStorage
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

    // GShip& ship = trn.getCurrentUniverse().getShip(sid);
    // const int bumMission = config.ExtMissionsStartAt() + GMission::pmsn_BeamUpMultiple;
    // GCommandContainer& cmds = trn.getCommands(ship.getOwner());
    int shipOwner = 0;
    m_ship.getOwner(shipOwner);

    int missionNumber = m_config[m_config.ExtMissionsStartAt]() + game::spec::Mission::pmsn_BeamUpMultiple;

    if (cs.isZero()) {
        if (CommandContainer* cc = CommandExtra::get(m_turn, shipOwner)) {
            cc->removeCommand(Command::phc_Beamup, m_ship.getId());
        }
        if (m_ship.getMission().orElse(0) == missionNumber) {
            // FIXME: implement "reset mission to previous value"
            //         /* Ship has mission "Beam up multiple". Try to set it away */
            //         const GShip& oldShip = trn.getPreviousUniverse().getShip(sid);
            //         if (oldShip.getShipKind() != GShip::CurrentShip
            //             || oldShip.getMission() == bumMission
            //             || !setFleetMission(trn.getCurrentUniverse(), sid,
            //                                 oldShip.getMission(),
            //                                 oldShip.getInterceptId(),
            //                                 oldShip.getTowId()))
            //         {
            //             /* oldShip is not a current ship (should not happen),
            //                it also has a "Beam up multiple" mission,
            //                or resetting failed (maybe because the ship started
            //                the turn with Intercept and is now member of a fleet),
            //                so clear it. */
            FleetMember(m_turn.universe(), m_ship).setMission(0, 0, 0, m_config, m_shipList);
            //         }
        }
    } else {
        CommandExtra::create(m_turn).create(shipOwner).addCommand(Command::phc_Beamup, m_ship.getId(), cs.toPHostString());
        FleetMember(m_turn.universe(), m_ship).setMission(missionNumber, 0, 0, m_config, m_shipList);
    }

    // PCC2 explicitly marked the ship dirty.
    // Our CommandExtra::onCommandChange does that automatically.
}

const game::spec::Hull*
game::map::BeamUpShipTransfer::getHull() const
{
    return m_shipList.hulls().get(m_ship.getHull().orElse(-1));
}



void
game::map::parseBeamUpCommand(util::Vector<int32_t,Element::Type>& out, Turn& turn, const Ship& ship, int factor)
{
    int shipOwner = 0;
    ship.getOwner(shipOwner);
    if (const CommandContainer* cc = CommandExtra::get(turn, shipOwner)) {
        if (const Command* cmd = cc->getCommand(Command::phc_Beamup, ship.getId())) {
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
