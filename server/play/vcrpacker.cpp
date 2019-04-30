/**
  *  \file server/play/vcrpacker.cpp
  */

#include "server/play/vcrpacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/vcrproperty.hpp"
#include "game/interface/vcrsideproperty.hpp"
#include "game/playerlist.hpp"
#include "game/root.hpp"

namespace gi = game::interface;

server::play::VcrPacker::VcrPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::VcrPacker::buildValue() const
{
    // ex ServerVcrWriter::write
    game::Root& r = game::actions::mustHaveRoot(m_session);
    game::Game& g = game::actions::mustHaveGame(m_session);
    game::Turn& t = g.currentTurn();
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);
    afl::string::Translator& tx = m_session.translator();
    const game::config::HostConfiguration& config = r.hostConfiguration();
    const game::PlayerList& pl = r.playerList();

    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    afl::base::Ptr<game::vcr::Database> db = t.getBattles();
    if (db.get() != 0) {
        for (size_t i = 0, n = db->getNumBattles(); i < n; ++i) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            addValueNew(*hv, gi::getVcrProperty(i, gi::ivpMagic,     m_session, r, t, sl), "MAGIC");
            addValueNew(*hv, gi::getVcrProperty(i, gi::ivpSeed,      m_session, r, t, sl), "SEED");
            addValueNew(*hv, gi::getVcrProperty(i, gi::ivpFlags,     m_session, r, t, sl), "CAPABILITIES");
            addValueNew(*hv, gi::getVcrProperty(i, gi::ivpAlgorithm, m_session, r, t, sl), "ALGORITHM");

            afl::base::Ref<afl::data::Vector> units(afl::data::Vector::create());
            game::vcr::Battle* battle = db->getBattle(i);
            if (battle != 0) {
                for (size_t side = 0, nsides = battle->getNumObjects(); side < nsides; ++side) {
                    afl::base::Ref<afl::data::Hash> u(afl::data::Hash::create());

                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsBeamId,          tx, sl, config, pl), "BEAM");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsBeamCount,       tx, sl, config, pl), "BEAM.COUNT");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsCrewRaw,         tx, sl, config, pl), "CREW");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsDamage,          tx, sl, config, pl), "DAMAGE");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsFighterBays,     tx, sl, config, pl), "FIGHTER.BAYS");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsFighterCount,    tx, sl, config, pl), "FIGHTER.COUNT");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsHullId,          tx, sl, config, pl), "HULL");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsId,              tx, sl, config, pl), "ID");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsImage,           tx, sl, config, pl), "IMAGE");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsLevel,           tx, sl, config, pl), "LEVEL");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsMass,            tx, sl, config, pl), "MASS");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsName,            tx, sl, config, pl), "NAME");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsOwnerId,         tx, sl, config, pl), "OWNER");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsShield,          tx, sl, config, pl), "SHIELD");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsTorpId,          tx, sl, config, pl), "TORP");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsTorpCount,       tx, sl, config, pl), "TORP.COUNT");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsTorpLCount,      tx, sl, config, pl), "TORP.LCOUNT");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsIsPlanet,        tx, sl, config, pl), "ISPLANET");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsBeamKillRate,    tx, sl, config, pl), "CONFIG.BEAMKILLRATE");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsBeamChargeRate,  tx, sl, config, pl), "CONFIG.BEAMCHARGERATE");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsTorpMissRate,    tx, sl, config, pl), "CONFIG.TORPMISSRATE");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsTorpChargeRate,  tx, sl, config, pl), "CONFIG.TORPCHARGERATE");
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsCrewDefenseRate, tx, sl, config, pl), "CONFIG.CREWDEFENSERATE");

                    units->pushBackNew(new afl::data::HashValue(u));
                }
            }
            addValueNew(*hv, new afl::data::VectorValue(units), "UNIT");
            vv->pushBackNew(new afl::data::HashValue(hv));
        }
    }
    return new afl::data::VectorValue(vv);
}

String_t
server::play::VcrPacker::getName() const
{
    return "zvcr";
}
