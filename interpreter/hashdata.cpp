/**
  *  \file interpreter/hashdata.cpp
  */

#include "interpreter/hashdata.hpp"

interpreter::HashData::HashData()
    : m_names(),
      m_content()
{ }

interpreter::HashData::~HashData()
{ }

// Name interface
// /** Set element by name. */
void
interpreter::HashData::setNew(const String_t& name, afl::data::Value* value)
{
    // ex IntHashData::setNew
    afl::data::NameMap::Index_t i;
    try {
        i = m_names.addMaybe(name);
    }
    catch (...) {
        delete value;
        throw;
    }
    m_content.setNew(i, value);
}

// /** Get element by name. */
afl::data::Value*
interpreter::HashData::get(const String_t& name) const
{
    // ex IntHashData::get
    return m_content[m_names.getIndexByName(name)];
}


// Index interface
// /** Set element by index.
//     \param index Index
//     \param value Value. Will get owned by IntHashData. */
void
interpreter::HashData::setNew(afl::data::NameMap::Index_t index, afl::data::Value* value)
{
    // ex IntHashData::setNew
    return m_content.setNew(index, value);
}

// /** Get element by index.
//     \param index Index
//     \return Element, owned by IntHashData */
afl::data::Value*
interpreter::HashData::get(afl::data::NameMap::Index_t index) const
{
    // ex IntHashData::get
    return m_content[index];
}

// /** Get name by index.
//     \param index Index */
String_t
interpreter::HashData::getName(afl::data::NameMap::Index_t index)
{
    // ex IntHashData::getName
    return m_names.getNameByIndex(index);
}

// /** Get number of elements in hash. */
afl::data::NameMap::Index_t
interpreter::HashData::getNumNames() const
{
    // ex IntHashData::getSize
    return m_names.getNumNames();
}
