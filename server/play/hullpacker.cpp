/**
  *  \file server/play/hullpacker.cpp
  */

#include "server/play/hullpacker.hpp"
#include "afl/string/format.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/interface/hullcontext.hpp"
#include "game/actions/preconditions.hpp"
#include "game/spec/shiplist.hpp"

server::play::HullPacker::HullPacker(game::Session& session, int hullNr)
    : m_session(session),
      m_hullNr(hullNr)
{ }

server::Value_t*
server::play::HullPacker::buildValue() const
{
    // ex ServerHullWriter::write
    game::Root& r = game::actions::mustHaveRoot(m_session);
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);

    afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
    game::interface::HullContext ctx(m_hullNr, sl, r);

    // Cost
    afl::base::Ref<afl::data::Hash> cost(afl::data::Hash::create());
    addValue(*cost, ctx, "COST.D", "D");
    addValue(*cost, ctx, "COST.M", "M");
    addValue(*cost, ctx, "COST.MC", "MC");
    addValue(*cost, ctx, "COST.T", "T");
    addValueNew(*hv, new afl::data::HashValue(cost), "COST");

    // Remainder
    addValue(*hv, ctx, "BEAM.MAX", "BEAM.MAX");
    addValue(*hv, ctx, "CARGO.MAX", "CARGO.MAX");
    addValue(*hv, ctx, "CARGO.MAXFUEL", "CARGO.MAXFUEL");
    addValue(*hv, ctx, "CREW.NORMAL", "CREW.NORMAL");
    addValue(*hv, ctx, "ENGINE.COUNT", "ENGINE.COUNT");
    addValue(*hv, ctx, "FIGHTER.BAYS", "FIGHTER.BAYS");
    addValue(*hv, ctx, "IMAGE", "IMAGE");
    addValue(*hv, ctx, "MASS", "MASS");
    addValue(*hv, ctx, "NAME", "NAME");
    addValue(*hv, ctx, "TECH", "TECH");
    addValue(*hv, ctx, "TORP.LMAX", "TORP.LMAX");

    // Hull functions
    {
        game::spec::HullFunctionList list;
        sl.enumerateHullFunctions(list, m_hullNr, r.hostConfiguration(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS), game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS), true, true);
        addValueNew(*hv, packHullFunctionList(list), "FUNC");
    }

    return new afl::data::HashValue(hv);
}

String_t
server::play::HullPacker::getName() const
{
    return afl::string::Format("hull%d", m_hullNr);
}

server::Value_t*
server::play::packHullFunctionList(const game::spec::HullFunctionList& list)
{
    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    for (game::spec::HullFunctionList::Iterator_t it = list.begin(), end = list.end(); it != end; ++it) {
        afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
        Packer::addValueNew(*hv, makeIntegerValue(it->getBasicFunctionId()), "ID");
        Packer::addValueNew(*hv, makeIntegerValue(it->getPlayers().toInteger()), "PLAYERS");
        Packer::addValueNew(*hv, makeIntegerValue(it->getLevels().toInteger()), "LEVELS");

        switch (it->getKind()) { // decouple internal from external representation
         case game::spec::HullFunction::AssignedToShip:
            Packer::addValueNew(*hv, makeIntegerValue(0), "KIND");
            break;
         case game::spec::HullFunction::AssignedToHull:
            Packer::addValueNew(*hv, makeIntegerValue(1), "KIND");
            break;
         case game::spec::HullFunction::AssignedToRace:
            Packer::addValueNew(*hv, makeIntegerValue(2), "KIND");
            break;
        }
        vv->pushBackNew(new afl::data::HashValue(hv));
    }
    return new afl::data::VectorValue(vv);
}
