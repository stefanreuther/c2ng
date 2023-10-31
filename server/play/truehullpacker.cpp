/**
  *  \file server/play/truehullpacker.cpp
  *  \brief Class server::play::TruehullPacker
  */

#include "server/play/truehullpacker.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

server::play::TruehullPacker::TruehullPacker(const game::spec::ShipList& shipList, const game::Root& root, int firstSlot)
    : m_shipList(shipList), m_root(root), m_firstSlot(firstSlot)
{ }

server::Value_t*
server::play::TruehullPacker::buildValue() const
{
    // ex ServerTruehullWriter::write
    // @diff PCC2 emits "0" slots at the end of each race, we don't.
    // @diff PCC2 emits 11 races, this emits 31.
    const game::config::HostConfiguration& config = m_root.hostConfiguration();
    const game::spec::HullAssignmentList& list = m_shipList.hullAssignments();

    afl::base::Ref<afl::data::Vector> outer(afl::data::Vector::create());
    for (int pl = m_firstSlot; pl <= game::MAX_PLAYERS; ++pl) {
        if (pl <= 0) {
            outer->pushBackNew(0);
        } else {
            afl::base::Ref<afl::data::Vector> inner(afl::data::Vector::create());
            for (int idx = 1, n = list.getMaxIndex(config, pl); idx <= n; ++idx) {
                inner->pushBackInteger(list.getHullFromIndex(config, pl, idx));
            }
            outer->pushBackNew(new afl::data::VectorValue(inner));
        }
    }
    return new afl::data::VectorValue(outer);
}

String_t
server::play::TruehullPacker::getName() const
{
    return "truehull";
}
