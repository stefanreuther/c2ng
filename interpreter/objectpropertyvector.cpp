/**
  *  \file interpreter/objectpropertyvector.cpp
  */

#include "interpreter/objectpropertyvector.hpp"


interpreter::ObjectPropertyVector::ObjectPropertyVector()
    : m_data()
{ }

interpreter::ObjectPropertyVector::~ObjectPropertyVector()
{ }

afl::data::Segment*
interpreter::ObjectPropertyVector::create(int id)
{
    if (id > 0) {
        // FIXME: give PtrVector a resize()?
        while (id > static_cast<int>(m_data.size())) {
            m_data.pushBackNew(0);
        }
        if (m_data[id-1] == 0) {
            m_data.replaceElementNew(id-1, new afl::data::Segment());
        }
        return m_data[id-1];
    } else {
        return 0;
    }
}


afl::data::Segment*
interpreter::ObjectPropertyVector::get(int id) const
{
    if (id > 0 && id <= static_cast<int>(m_data.size())) {
        return m_data[id-1];
    } else {
        return 0;
    }
}

afl::data::Value*
interpreter::ObjectPropertyVector::get(int id, afl::data::Segment::Index_t index) const
{
    if (afl::data::Segment* seg = get(id)) {
        return (*seg)[index];
    } else {
        return 0;
    }
}

void
interpreter::ObjectPropertyVector::clear()
{
    m_data.clear();
}
