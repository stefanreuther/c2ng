/**
  *  \file server/play/planetpacker.cpp
  */

#include <stdexcept>
#include "server/play/planetpacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "server/errors.hpp"

server::play::PlanetPacker::PlanetPacker(game::Session& session, int planetNr)
    : m_session(session),
      m_planetNr(planetNr)
{ }

server::Value_t*
server::play::PlanetPacker::buildValue() const
{
    // ex ServerPlanetWriter::write
    game::Root& r = game::actions::mustHaveRoot(m_session);
    game::Game& g = game::actions::mustHaveGame(m_session);
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);
    const game::map::Planet* pPlanet = g.currentTurn().universe().planets().get(m_planetNr);
    if (pPlanet == 0) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
    game::interface::PlanetContext ctx(m_planetNr, m_session, r, g);
    addValue(*hv, ctx, "BASE.BUILDING", "BASE.BUILDING");
    addValue(*hv, ctx, "COLONISTS.HAPPY$", "COLONISTS.HAPPY");
    addValue(*hv, ctx, "COLONISTS.SUPPORTED", "COLONISTS.SUPPORTED");
    addValue(*hv, ctx, "COLONISTS.TAX", "COLONISTS.TAX");
    addValue(*hv, ctx, "COMMENT", "COMMENT");
    addValue(*hv, ctx, "DAMAGE", "DAMAGE");
    addValue(*hv, ctx, "DEFENSE", "DEFENSE");
    addValue(*hv, ctx, "DEFENSE.BASE", "DEFENSE.BASE");
    addValue(*hv, ctx, "DEFENSE.BASE.WANT", "DEFENSE.BASE.WANT");
    addValue(*hv, ctx, "DEFENSE.WANT", "DEFENSE.WANT");
    addValue(*hv, ctx, "DENSITY.D", "DENSITY.D");
    addValue(*hv, ctx, "DENSITY.M", "DENSITY.M");
    addValue(*hv, ctx, "DENSITY.N", "DENSITY.N");
    addValue(*hv, ctx, "DENSITY.T", "DENSITY.T");
    addValue(*hv, ctx, "FACTORIES", "FACTORIES");
    addValue(*hv, ctx, "FACTORIES.WANT", "FACTORIES.WANT");
    addValue(*hv, ctx, "FCODE", "FCODE");
    addValue(*hv, ctx, "FIGHTERS", "FIGHTERS");
    addValue(*hv, ctx, "GROUND.D", "GROUND.D");
    addValue(*hv, ctx, "GROUND.M", "GROUND.M");
    addValue(*hv, ctx, "GROUND.N", "GROUND.N");
    addValue(*hv, ctx, "GROUND.T", "GROUND.T");
    addValue(*hv, ctx, "INDUSTRY$", "INDUSTRY");
    addValue(*hv, ctx, "LEVEL", "LEVEL");
    addValue(*hv, ctx, "MINES", "MINES");
    addValue(*hv, ctx, "MINES.WANT", "MINES.WANT");
    addValue(*hv, ctx, "MISSION$", "MISSION");
    addValue(*hv, ctx, "NATIVES", "NATIVES");
    addValue(*hv, ctx, "NATIVES.GOV$", "NATIVES.GOV");
    addValue(*hv, ctx, "NATIVES.HAPPY$", "NATIVES.HAPPY");
    addValue(*hv, ctx, "NATIVES.RACE$", "NATIVES.RACE");
    addValue(*hv, ctx, "NATIVES.TAX", "NATIVES.TAX");
    addValue(*hv, ctx, "SHIPYARD.ACTION", "SHIPYARD.ACTION");
    addValue(*hv, ctx, "SHIPYARD.ID", "SHIPYARD.ID");
    addValue(*hv, ctx, "TECH.BEAM", "TECH.BEAM");
    addValue(*hv, ctx, "TECH.ENGINE", "TECH.ENGINE");
    addValue(*hv, ctx, "TECH.HULL", "TECH.HULL");
    addValue(*hv, ctx, "TECH.TORPEDO", "TECH.TORPEDO");
    addValue(*hv, ctx, "TEMP$", "TEMP");
    addValue(*hv, ctx, "TURN.COLONISTS", "TURN.COLONISTS");
    addValue(*hv, ctx, "TURN.MINERALS", "TURN.MINERALS");
    addValue(*hv, ctx, "TURN.MONEY", "TURN.MONEY");
    addValue(*hv, ctx, "TURN.NATIVES", "TURN.NATIVES");

    // Ground minerals
    afl::base::Ref<afl::data::Hash> ground(afl::data::Hash::create());
    addValue(*ground, ctx, "COLONISTS", "COLONISTS");
    addValue(*ground, ctx, "MINED.D", "D");
    addValue(*ground, ctx, "MINED.M", "M");
    addValue(*ground, ctx, "MINED.N", "N");
    addValue(*ground, ctx, "MINED.T", "T");
    addValue(*ground, ctx, "MONEY", "MC");
    addValue(*ground, ctx, "SUPPLIES", "SUPPLIES");
    addValueNew(*hv, new afl::data::HashValue(ground), "G");

    // Build order
    if (pPlanet->hasFullBaseData() && pPlanet->getBaseBuildOrderHullIndex().orElse(0) != 0) {
        afl::base::Ref<afl::data::Hash> build(afl::data::Hash::create());
        addValue(*build, ctx, "BUILD.BEAM$", "BEAM");
        addValue(*build, ctx, "BUILD.BEAM.COUNT", "BEAM.COUNT");
        addValue(*build, ctx, "BUILD.ENGINE$", "ENGINE");
        addValue(*build, ctx, "BUILD.HULL$", "HULL");
        addValue(*build, ctx, "BUILD.QPOS", "QPOS");
        addValue(*build, ctx, "BUILD.TORP$", "TORP");
        addValue(*build, ctx, "BUILD.TORP.COUNT", "TORP.COUNT");
        addValueNew(*hv, new afl::data::HashValue(build), "BUILD");
    }
    if (pPlanet->hasFullBaseData()) {
        // Ammo
        const int numTorpedoTypes = sl.launchers().size();
        {
            afl::base::Ref<afl::data::Vector> ammo(afl::data::Vector::create());
            ammo->pushBackInteger(0);
            for (int i = 1; i <= numTorpedoTypes; ++i) {
                ammo->pushBackInteger(pPlanet->getCargo(game::Element::fromTorpedoType(i)).orElse(0));
            }
            ammo->pushBackInteger(pPlanet->getCargo(game::Element::Fighters).orElse(0));
            addValueNew(*hv, new afl::data::VectorValue(ammo), "STORAGE.AMMO");
        }

        // Beams
        const int numBeamTypes = sl.beams().size();
        {
            afl::base::Ref<afl::data::Vector> beams(afl::data::Vector::create());
            beams->pushBackInteger(0);
            for (int i = 1; i <= numBeamTypes; ++i) {
                beams->pushBackInteger(pPlanet->getBaseStorage(game::BeamTech, i).orElse(0));
            }
            addValueNew(*hv, new afl::data::VectorValue(beams), "STORAGE.BEAMS");
        }

        // Engines
        const int numEngineTypes = sl.engines().size();
        {
            afl::base::Ref<afl::data::Vector> engines(afl::data::Vector::create());
            engines->pushBackInteger(0);
            for (int i = 1; i <= numEngineTypes; ++i) {
                engines->pushBackInteger(pPlanet->getBaseStorage(game::EngineTech, i).orElse(0));
            }
            addValueNew(*hv, new afl::data::VectorValue(engines), "STORAGE.ENGINES");
        }

        // Hulls
        // FIXME: can we implement this without knowing the owner?
        int owner = 0;
        pPlanet->getOwner(owner);
        const int numHullSlots = sl.hullAssignments().getMaxIndex(r.hostConfiguration(), owner);
        {
            afl::base::Ref<afl::data::Vector> hulls(afl::data::Vector::create());
            hulls->pushBackInteger(0);
            for (int i = 1; i <= numHullSlots; ++i) {
                hulls->pushBackInteger(pPlanet->getBaseStorage(game::HullTech, i).orElse(0));
            }
            addValueNew(*hv, new afl::data::VectorValue(hulls), "STORAGE.HULLS");
        }

        // Launchers
        {
            afl::base::Ref<afl::data::Vector> launchers(afl::data::Vector::create());
            launchers->pushBackInteger(0);
            for (int i = 1; i <= numTorpedoTypes; ++i) {
                launchers->pushBackInteger(pPlanet->getBaseStorage(game::TorpedoTech, i).orElse(0));
            }
            addValueNew(*hv, new afl::data::VectorValue(launchers), "STORAGE.LAUNCHERS");
        }
    }

    return new afl::data::HashValue(hv);
}

String_t
server::play::PlanetPacker::getName() const
{
    return afl::string::Format("planet%d", m_planetNr);
}
