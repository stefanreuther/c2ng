/**
  *  \file server/play/basichullfunctionpacker.cpp
  *  \brief Class server::play::BasicHullFunctionPacker
  */

#include "server/play/basichullfunctionpacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"

server::play::BasicHullFunctionPacker::BasicHullFunctionPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::BasicHullFunctionPacker::buildValue() const
{
    const game::spec::BasicHullFunctionList& list = game::actions::mustHaveShipList(m_session).basicHullFunctions();

    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    for (size_t i = 0, n = list.getNumFunctions(); i != n; ++i) {
        if (const game::spec::BasicHullFunction* hf = list.getFunctionByIndex(i)) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            addValueNew(*hv, makeStringValue(hf->getName()), "NAME");
            addValueNew(*hv, makeStringValue(hf->getCode()), "CODE");
            addValueNew(*hv, makeIntegerValue(hf->getId()), "ID");
            addValueNew(*hv, makeStringValue(hf->getDescription()), "INFO");
            addValueNew(*hv, makeStringValue(hf->getExplanation()), "INFO2");
            addValueNew(*hv, makeStringValue(hf->getPictureName()), "IMAGE");
            vv->pushBackNew(new afl::data::HashValue(hv));
        }
    }
    return new afl::data::VectorValue(vv);
}

String_t
server::play::BasicHullFunctionPacker::getName() const
{
    return "zab";
}
