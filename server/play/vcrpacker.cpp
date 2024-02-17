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
#include "game/turn.hpp"
#include "game/vcr/flak/battle.hpp"
#include "game/vcr/flak/object.hpp"
#include "interpreter/values.hpp"

namespace gi = game::interface;
using server::play::Packer;
using interpreter::makeIntegerValue;
using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;
using afl::base::Ref;

namespace {
    /* Pack fleets from a FLAK setup */
    afl::data::Value* packFleets(const game::vcr::flak::Setup& setup)
    {
        Ref<Vector> fleets(Vector::create());
        for (size_t i = 0, n = setup.getNumFleets(); i < n; ++i) {
            const game::vcr::flak::Setup::Fleet& in = setup.getFleetByIndex(i);
            Ref<Hash> out(Hash::create());

            // Scalar attributes
            Packer::addValueNew(*out, makeIntegerValue(in.player), "PLAYER");
            Packer::addValueNew(*out, makeIntegerValue(in.speed),  "SPEED");
            Packer::addValueNew(*out, makeIntegerValue(in.x), "X");
            Packer::addValueNew(*out, makeIntegerValue(in.y), "Y");
            Packer::addValueNew(*out, makeIntegerValue(static_cast<int>(in.firstShipIndex)), "FIRSTSHIP");
            Packer::addValueNew(*out, makeIntegerValue(static_cast<int>(in.numShips)), "NUMSHIPS");

            // Attack list
            Ref<Vector> attList(Vector::create());
            for (size_t i = 0; i < 2*in.numAttackListEntries; ++i) {
                attList->pushBackNew(makeIntegerValue(setup.getAttackList()[2*in.firstAttackListIndex + i]));
            }
            Packer::addValueNew(*out, new VectorValue(attList), "ATTLIST");

            // Finish
            fleets->pushBackNew(new HashValue(out));
        }
        return new VectorValue(fleets);
    }
}


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

    Ref<Vector> vv(Vector::create());
    afl::base::Ptr<game::vcr::Database> db = t.getBattles();
    if (db.get() != 0) {
        for (size_t i = 0, n = db->getNumBattles(); i < n; ++i) {
            Ref<Hash> hv(Hash::create());
            addValueNew(*hv, gi::getVcrProperty(i, gi::ivpMagic,     tx, r, db, sl), "MAGIC");
            addValueNew(*hv, gi::getVcrProperty(i, gi::ivpSeed,      tx, r, db, sl), "SEED");
            addValueNew(*hv, gi::getVcrProperty(i, gi::ivpFlags,     tx, r, db, sl), "CAPABILITIES");
            addValueNew(*hv, gi::getVcrProperty(i, gi::ivpAlgorithm, tx, r, db, sl), "ALGORITHM");
            addValueNew(*hv, gi::getVcrProperty(i, gi::ivpLocX,      tx, r, db, sl), "X");
            addValueNew(*hv, gi::getVcrProperty(i, gi::ivpLocY,      tx, r, db, sl), "Y");
            addValueNew(*hv, gi::getVcrProperty(i, gi::ivpAmbient,   tx, r, db, sl), "AMBIENT");

            Ref<Vector> units(Vector::create());
            game::vcr::Battle* battle = db->getBattle(i);
            if (battle != 0) {
                for (size_t side = 0, nsides = battle->getNumObjects(); side < nsides; ++side) {
                    Ref<Hash> u(Hash::create());

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
                    addValueNew(*u, gi::getVcrSideProperty(*battle, side, gi::ivsRole,            tx, sl, config, pl), "ROLE");

                    /* Additional properties for FLAK */
                    if (const game::vcr::flak::Object* obj = dynamic_cast<const game::vcr::flak::Object*>(battle->getObject(side, false))) {
                        addValueNew(*u, makeIntegerValue(obj->getMaxFightersLaunched()), "FLAKMAXFL");
                        addValueNew(*u, makeIntegerValue(obj->getRating()),              "FLAKRATING");
                        addValueNew(*u, makeIntegerValue(obj->getCompensation()),        "FLAKCOMPENSATION");
                        addValueNew(*u, makeIntegerValue(obj->getEndingStatus()),        "FLAKENDING");
                    }

                    units->pushBackNew(new HashValue(u));
                }
            }
            addValueNew(*hv, new VectorValue(units), "UNIT");

            /* Additional properties for FLAK */
            if (const game::vcr::flak::Battle* flakBattle = dynamic_cast<const game::vcr::flak::Battle*>(battle)) {
                const game::vcr::flak::Setup& setup = flakBattle->setup();
                addValueNew(*hv, packFleets(setup), "FLEET");
            }

            vv->pushBackNew(new HashValue(hv));
        }
    }
    return new VectorValue(vv);
}

String_t
server::play::VcrPacker::getName() const
{
    return "zvcr";
}
