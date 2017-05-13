/**
  *  \file interpreter/objectpropertyvector.cpp
  *  \brief Class interpreter::ObjectPropertyVector
  */

#include "interpreter/objectpropertyvector.hpp"

// Constructor.
interpreter::ObjectPropertyVector::ObjectPropertyVector()
    : m_data()
{ }

// Destructor.
interpreter::ObjectPropertyVector::~ObjectPropertyVector()
{ }

// Get or create a segment.
afl::data::Segment*
interpreter::ObjectPropertyVector::create(int id)
{
    if (id > 0) {
        size_t slot = static_cast<size_t>(id);
        if (slot > m_data.size()) {
            m_data.resize(slot);
        }
        --slot;
        if (m_data[slot] == 0) {
            m_data.replaceElementNew(slot, new afl::data::Segment());
        }
        return m_data[slot];
    } else {
        return 0;
    }
}

// Get segment.
afl::data::Segment*
interpreter::ObjectPropertyVector::get(int id) const
{
    if (id > 0 && id <= static_cast<int>(m_data.size())) {
        return m_data[id-1];
    } else {
        return 0;
    }
}

// Get value.
afl::data::Value*
interpreter::ObjectPropertyVector::get(int id, afl::data::Segment::Index_t index) const
{
    if (afl::data::Segment* seg = get(id)) {
        return (*seg)[index];
    } else {
        return 0;
    }
}

// Clear.
void
interpreter::ObjectPropertyVector::clear()
{
    m_data.clear();
}
