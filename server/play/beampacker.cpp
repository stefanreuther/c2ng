/**
  *  \file server/play/beampacker.cpp
  */

#include "server/play/beampacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/interface/beamcontext.hpp"
#include "game/spec/shiplist.hpp"

server::play::BeamPacker::BeamPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::BeamPacker::buildValue() const
{
    // ex ServerBeamWriter::write
    game::Root& r = game::actions::mustHaveRoot(m_session);
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);

    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    for (int i = 0, n = sl.beams().size(); i <= n; ++i) {
        if (sl.beams().get(i) != 0) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            game::interface::BeamContext ctx(i, sl, r);

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
