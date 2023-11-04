/**
  *  \file server/play/torpedopacker.cpp
  *  \brief Class server::play::TorpedoPacker
  */

#include "server/play/torpedopacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/interface/torpedocontext.hpp"
#include "game/spec/shiplist.hpp"
#include "game/spec/torpedolauncher.hpp"

server::play::TorpedoPacker::TorpedoPacker(game::spec::ShipList& shipList, const game::Root& root, int firstSlot)
    : m_shipList(shipList), m_root(root), m_firstSlot(firstSlot)
{ }

server::Value_t*
server::play::TorpedoPacker::buildValue() const
{
    // ex ServerTorpWriter::write
    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    for (int i = m_firstSlot, n = m_shipList.launchers().size(); i <= n; ++i) {
        if (const game::spec::TorpedoLauncher* p = m_shipList.launchers().get(i)) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            game::interface::TorpedoContext tctx(false, i, m_shipList, m_root);
            game::interface::TorpedoContext lctx(true, i, m_shipList, m_root);

            // Torpedo costs
            afl::base::Ref<afl::data::Hash> torpCost(afl::data::Hash::create());
            addValue(*torpCost, tctx, "COST.D", "D");
            addValue(*torpCost, tctx, "COST.M", "M");
            addValue(*torpCost, tctx, "COST.MC", "MC");
            addValue(*torpCost, tctx, "COST.T", "T");
            addValueNew(*hv, new afl::data::HashValue(torpCost), "TORPCOST");

            // General stuff
            addValue(*hv, tctx, "DAMAGE", "DAMAGE");
            addValue(*hv, tctx, "KILL", "KILL");
            addValue(*hv, tctx, "NAME", "NAME");
            addValue(*hv, tctx, "TECH", "TECH");
            addValue(*hv, lctx, "MASS", "MASS");

            // Special case: KILL and DAMAGE are possibly doubled.
            // Write KILL1/DAMAGE1 values for convenience of JavaScript VCR.
            addValueNew(*hv, makeIntegerValue(p->getDamagePower()), "DAMAGE1");
            addValueNew(*hv, makeIntegerValue(p->getKillPower()),   "KILL1");

            // Launcher costs
            afl::base::Ref<afl::data::Hash> launCost(afl::data::Hash::create());
            addValue(*launCost, lctx, "COST.D", "D");
            addValue(*launCost, lctx, "COST.M", "M");
            addValue(*launCost, lctx, "COST.MC", "MC");
            addValue(*launCost, lctx, "COST.T", "T");
            addValueNew(*hv, new afl::data::HashValue(launCost), "TUBECOST");

            vv->pushBackNew(new afl::data::HashValue(hv));
        } else {
            vv->pushBackNew(0);
        }
    }
    return new afl::data::VectorValue(vv);
}

String_t
server::play::TorpedoPacker::getName() const
{
    return "torp";
}
