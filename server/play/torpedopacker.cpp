/**
  *  \file server/play/torpedopacker.cpp
  *  \brief Class server::play::TorpedoPacker
  */

#include "server/play/torpedopacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/interface/torpedocontext.hpp"
#include "game/spec/shiplist.hpp"
#include "game/spec/torpedolauncher.hpp"

server::play::TorpedoPacker::TorpedoPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::TorpedoPacker::buildValue() const
{
    // ex ServerTorpWriter::write
    game::Root& r = game::actions::mustHaveRoot(m_session);
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);

    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    for (int i = 0, n = sl.launchers().size(); i <= n; ++i) {
        if (const game::spec::TorpedoLauncher* p = sl.launchers().get(i)) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            game::interface::TorpedoContext tctx(false, i, sl, r);
            game::interface::TorpedoContext lctx(true, i, sl, r);

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
            addValue(*hv, tctx, "MASS", "MASS");

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
