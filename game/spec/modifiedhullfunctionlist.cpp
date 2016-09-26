/**
  *  \file game/spec/modifiedhullfunctionlist.cpp
  */

#include "game/spec/modifiedhullfunctionlist.hpp"

namespace {
    const int32_t MODIFIED_FUNCTION_BASE_ID = 99999;
}


game::spec::ModifiedHullFunctionList::ModifiedHullFunctionList()
    : m_modifiedFunctions()
{ }

game::spec::ModifiedHullFunctionList::~ModifiedHullFunctionList()
{ }

// Clear.
void
game::spec::ModifiedHullFunctionList::clear()
{
    m_modifiedFunctions.clear();
}

// Given a host-supplied device Id, return equivalent internal Id.
game::spec::ModifiedHullFunctionList::Function_t
game::spec::ModifiedHullFunctionList::getFunctionIdFromHostId(int hostFunctionId) const
{
    // ex GHullFunctionData::getIdFromHostId
    for (size_t i = 0, n = m_modifiedFunctions.size(); i < n; ++i) {
        if (hostFunctionId == m_modifiedFunctions[i]->getHostId()) {
            return Function_t(i + MODIFIED_FUNCTION_BASE_ID);
        }
    }
    return Function_t(hostFunctionId);
}

// Given a function definition, return equivalent internal Id.
game::spec::ModifiedHullFunctionList::Function_t
game::spec::ModifiedHullFunctionList::getFunctionIdFromDefinition(const HullFunction& def)
{
    // ex GHullFunctionData::getIdFromFunction
    if (def.getLevels() == ExperienceLevelSet_t::allUpTo(MAX_EXPERIENCE_LEVELS)) {
        // This is an unmodified function, hence its internal Id is the same as its basic function Id.
        return Function_t(def.getBasicFunctionId());
    } else {
        // This is a modified function. Check whether we know it already.
        for (size_t i = 0, n = m_modifiedFunctions.size(); i < n; ++i) {
            if (m_modifiedFunctions[i]->isSame(def)) {
                // We know it
                if (def.getHostId() >= 0 && m_modifiedFunctions[i]->getHostId() < 0) {
                    m_modifiedFunctions[i]->setHostId(def.getHostId());
                }
                return Function_t(i + MODIFIED_FUNCTION_BASE_ID);
            }
        }

        // When we're here, we do not know the function yet. Add it.
        m_modifiedFunctions.pushBackNew(new HullFunction(def));
        return Function_t(m_modifiedFunctions.size() - 1 + MODIFIED_FUNCTION_BASE_ID);
    }
}

bool
game::spec::ModifiedHullFunctionList::getFunctionDefinition(Function_t id, HullFunction& def) const
{
    // ex GHullFunctionData::getFunctionDefinition
    if (int32_t(id) >= MODIFIED_FUNCTION_BASE_ID && size_t(id - MODIFIED_FUNCTION_BASE_ID) < m_modifiedFunctions.size()) {
        // It's a modified function
        def = *m_modifiedFunctions[id - MODIFIED_FUNCTION_BASE_ID];
        return true;
    } else {
        // It's an unmodified function
        // FIXME: make validation stronger
        if (id >= 0) {
            def = HullFunction(id);
            return true;
        } else {
            def = HullFunction(-1);
            return false;
        }
    }
}
