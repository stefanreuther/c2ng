/**
  *  \file server/play/shippacker.cpp
  */

#include <stdexcept>
#include "server/play/shippacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"
#include "server/errors.hpp"
#include "server/play/hullpacker.hpp"

server::play::ShipPacker::ShipPacker(game::Session& session, int shipNr)
    : m_session(session),
      m_shipNr(shipNr)
{ }

server::Value_t*
server::play::ShipPacker::buildValue() const
{
    // ex ServerShipWriter::write
    // Preconditions
    game::Root& r = game::actions::mustHaveRoot(m_session);
    game::Game& g = game::actions::mustHaveGame(m_session);
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);
    const game::map::Ship* pShip = g.currentTurn().universe().ships().get(m_shipNr);
    if (pShip == 0) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    // Build result
    afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
    game::interface::ShipContext ctx(m_shipNr, m_session, r, g, g.currentTurn(), sl);

    addValue(*hv, ctx, "AUX$", "AUX");
    addValue(*hv, ctx, "AUX.AMMO", "AUX.AMMO");
    addValue(*hv, ctx, "AUX.COUNT", "AUX.COUNT");
    addValue(*hv, ctx, "BEAM$", "BEAM");
    addValue(*hv, ctx, "BEAM.COUNT", "BEAM.COUNT");
    addValue(*hv, ctx, "COMMENT", "COMMENT");
    addValue(*hv, ctx, "CREW", "CREW");
    addValue(*hv, ctx, "DAMAGE", "DAMAGE");
    addValue(*hv, ctx, "ENEMY$", "ENEMY");
    addValue(*hv, ctx, "ENGINE$", "ENGINE");
    addValue(*hv, ctx, "FCODE", "FCODE");
    addValue(*hv, ctx, "HEADING$", "HEADING");
    addValue(*hv, ctx, "HULL$", "HULL");
    addValue(*hv, ctx, "LEVEL", "LEVEL");
    addValue(*hv, ctx, "MISSION$", "MISSION");
    addValue(*hv, ctx, "MISSION.INTERCEPT", "MISSION.INTERCEPT");
    addValue(*hv, ctx, "MISSION.TOW", "MISSION.TOW");
    addValue(*hv, ctx, "MOVE.ETA", "MOVE.ETA");
    addValue(*hv, ctx, "MOVE.FUEL", "MOVE.FUEL");
    addValue(*hv, ctx, "OWNER.REAL", "OWNER.REAL");
    addValue(*hv, ctx, "SPEED$", "SPEED");
    addValue(*hv, ctx, "WAYPOINT.DX", "WAYPOINT.DX");
    addValue(*hv, ctx, "WAYPOINT.DY", "WAYPOINT.DY");

    // Cargo
    afl::base::Ref<afl::data::Hash> cargo(afl::data::Hash::create());
    addValue(*cargo, ctx, "CARGO.COLONISTS", "COLONISTS");
    addValue(*cargo, ctx, "CARGO.D", "D");
    addValue(*cargo, ctx, "CARGO.M", "M");
    addValue(*cargo, ctx, "CARGO.MONEY", "MC");
    addValue(*cargo, ctx, "CARGO.N", "N");
    addValue(*cargo, ctx, "CARGO.SUPPLIES", "SUPPLIES");
    addValue(*cargo, ctx, "CARGO.T", "T");
    addValueNew(*hv, new afl::data::HashValue(cargo), "CARGO");

    // Functions
    {
        game::spec::HullFunctionList list;
        pShip->enumerateShipFunctions(list, sl);
        addValueNew(*hv, packHullFunctionList(list), "FUNC");
    }

    // Transfer
    if (pShip->isTransporterActive(game::map::Ship::TransferTransporter)) {
        afl::base::Ref<afl::data::Hash> tx(afl::data::Hash::create());
        addValue(*tx, ctx, "TRANSFER.SHIP.COLONISTS", "COLONISTS");
        addValue(*tx, ctx, "TRANSFER.SHIP.D", "D");
        addValue(*tx, ctx, "TRANSFER.SHIP.ID", "ID");
        addValue(*tx, ctx, "TRANSFER.SHIP.M", "M");
        addValue(*tx, ctx, "TRANSFER.SHIP.N", "N");
        addValue(*tx, ctx, "TRANSFER.SHIP.SUPPLIES", "SUPPLIES");
        addValue(*tx, ctx, "TRANSFER.SHIP.T", "T");
        addValueNew(*hv, new afl::data::HashValue(tx), "TRANSFER");
    }

    // Unload
    if (pShip->isTransporterActive(game::map::Ship::UnloadTransporter)) {
        afl::base::Ref<afl::data::Hash> tx(afl::data::Hash::create());
        addValue(*tx, ctx, "TRANSFER.UNLOAD.COLONISTS", "COLONISTS");
        addValue(*tx, ctx, "TRANSFER.UNLOAD.D", "D");
        addValue(*tx, ctx, "TRANSFER.UNLOAD.ID", "ID");
        addValue(*tx, ctx, "TRANSFER.UNLOAD.M", "M");
        addValue(*tx, ctx, "TRANSFER.UNLOAD.N", "N");
        addValue(*tx, ctx, "TRANSFER.UNLOAD.SUPPLIES", "SUPPLIES");
        addValue(*tx, ctx, "TRANSFER.UNLOAD.T", "T");
        addValueNew(*hv, new afl::data::HashValue(tx), "UNLOAD");
    }
    return new afl::data::HashValue(hv);
}

String_t
server::play::ShipPacker::getName() const
{
    return afl::string::Format("ship%d", m_shipNr);
}
