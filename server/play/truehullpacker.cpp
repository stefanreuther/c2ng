/**
  *  \file server/play/truehullpacker.cpp
  */

#include "server/play/truehullpacker.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"

server::play::TruehullPacker::TruehullPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::TruehullPacker::buildValue() const
{
    // ex ServerTruehullWriter::write
    // @diff PCC2 emits "0" slots at the end of each race, we don't.
    // @diff PCC2 emits 11 races, this emits 31.
    const game::config::HostConfiguration& config = game::actions::mustHaveRoot(m_session).hostConfiguration();
    const game::spec::HullAssignmentList& list = game::actions::mustHaveShipList(m_session).hullAssignments();
    
    afl::base::Ref<afl::data::Vector> outer(afl::data::Vector::create());
    outer->pushBackNew(0);
    for (int pl = 1; pl <= game::MAX_PLAYERS; ++pl) {
        afl::base::Ref<afl::data::Vector> inner(afl::data::Vector::create());
        for (int idx = 1, n = list.getMaxIndex(config, pl); idx <= n; ++idx) {
            inner->pushBackInteger(list.getHullFromIndex(config, pl, idx));
        }
        outer->pushBackNew(new afl::data::VectorValue(inner));
    }
    return new afl::data::VectorValue(outer);
}

String_t
server::play::TruehullPacker::getName() const
{
    return "truehull";
}
