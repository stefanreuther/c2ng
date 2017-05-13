/**
  *  \file server/file/ca/internalreferencecounter.cpp
  *  \brief Class server::file::ca::InternalReferenceCounter
  */

#include "server/file/ca/internalreferencecounter.hpp"

server::file::ca::InternalReferenceCounter::InternalReferenceCounter()
    : m_data()
{ }

void
server::file::ca::InternalReferenceCounter::set(const ObjectId& id, int32_t value)
{
    m_data[id] = value;
}

bool
server::file::ca::InternalReferenceCounter::modify(const ObjectId& id, int32_t delta, int32_t& result)
{
    Map_t::iterator it = m_data.find(id);
    if (it != m_data.end()) {
        it->second += delta;
        result = it->second;

        // If this causes the object to become unreferenced, any further calls to modify this as if it were
        // the reference count of an existing object are bogus. Do some damage containment by removing it,
        // so further calls will actually treat this as a nonexistant object.
        if (result == 0) {
            m_data.erase(it);
        }
        return true;
    } else {
        return false;
    }
}
