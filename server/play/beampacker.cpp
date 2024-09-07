/**
  *  \file server/play/beampacker.cpp
  *  \brief Class server::play::BeamPacker
  */

#include "server/play/beampacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/interface/beamcontext.hpp"
#include "game/spec/shiplist.hpp"

server::play::BeamPacker::BeamPacker(game::spec::ShipList& shipList, const game::Root& root, int firstSlot)
    : m_shipList(shipList), m_root(root), m_firstSlot(firstSlot)
{ }

server::Value_t*
server::play::BeamPacker::buildValue() const
{
    // ex ServerBeamWriter::write
    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    for (int i = m_firstSlot, n = m_shipList.beams().size(); i <= n; ++i) {
        if (m_shipList.beams().get(i) != 0) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            game::interface::BeamContext ctx(i, m_shipList, m_root);

            // Cost
            afl::base::Ref<afl::data::Hash> cost(afl::data::Hash::create());
            addValue(*cost, ctx, "COST.D", "D");
            addValue(*cost, ctx, "COST.M", "M");
            addValue(*cost, ctx, "COST.MC", "MC");
            addValue(*cost, ctx, "COST.T", "T");
            addValueNew(*hv, new afl::data::HashValue(cost), "COST");

            // Remainder
            addValue(*hv, ctx, "DAMAGE", "DAMAGE");
            addValue(*hv, ctx, "KILL", "KILL");
            addValue(*hv, ctx, "MASS", "MASS");
            addValue(*hv, ctx, "NAME", "NAME");
            addValue(*hv, ctx, "NAME.SHORT", "NAME.SHORT");
            addValue(*hv, ctx, "TECH", "TECH");

            vv->pushBackNew(new afl::data::HashValue(hv));
        } else {
            vv->pushBackNew(0);
        }
    }
    return new afl::data::VectorValue(vv);
}

String_t
server::play::BeamPacker::getName() const
{
    return "beam";
}
