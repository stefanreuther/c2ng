/**
  *  \file server/play/enginepacker.cpp
  */

#include "server/play/enginepacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/interface/enginecontext.hpp"
#include "game/spec/shiplist.hpp"
#include "game/spec/engine.hpp"

server::play::EnginePacker::EnginePacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::EnginePacker::buildValue() const
{
    // ex ServerEngineWriter::write
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);

    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    for (int i = 0, n = sl.engines().size(); i <= n; ++i) {
        if (const game::spec::Engine* p = sl.engines().get(i)) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            game::interface::EngineContext ctx(i, sl);

            // Cost
            afl::base::Ref<afl::data::Hash> cost(afl::data::Hash::create());
            addValue(*cost, ctx, "COST.D", "D");
            addValue(*cost, ctx, "COST.M", "M");
            addValue(*cost, ctx, "COST.MC", "MC");
            addValue(*cost, ctx, "COST.T", "T");
            addValueNew(*hv, new afl::data::HashValue(cost), "COST");

            // Fuel
            afl::base::Ref<afl::data::Vector> ff(afl::data::Vector::create());
            for (int i = 0; i <= game::spec::Engine::MAX_WARP; ++i) {
                int32_t value;
                if (p->getFuelFactor(i, value)) {
                    ff->pushBackInteger(value);
                } else {
                    ff->pushBackNew(0);
                }
            }
            addValueNew(*hv, new afl::data::VectorValue(ff), "FUELFACTOR");

            // Remainder
            addValue(*hv, ctx, "NAME", "NAME");
            addValue(*hv, ctx, "SPEED$", "SPEED");
            addValue(*hv, ctx, "TECH", "TECH");
            vv->pushBackNew(new afl::data::HashValue(hv));
        } else {
            vv->pushBackNew(0);
        }
    }
    return new afl::data::VectorValue(vv);
}

String_t
server::play::EnginePacker::getName() const
{
    return "engine";
}
