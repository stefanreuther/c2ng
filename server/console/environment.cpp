/**
  *  \file server/console/environment.cpp
  *  \brief Class server::console::Environment
  */

#include <stdexcept>
#include "server/console/environment.hpp"
#include "afl/string/parse.hpp"

namespace {
    bool isPositional(const String_t& str, size_t& index)
    {
        if (!afl::string::strToInteger(str, index)) {
            return false;
        }
        if (index == 0) {
            return false;
        }
        --index;
        return true;
    }
}


server::console::Environment::Environment()
    : m_names(),
      m_values(),
      m_positionalParameters()
{ }

server::console::Environment::~Environment()
{ }

void
server::console::Environment::setNew(String_t name, ValuePtr_t value)
{
    pushNew(name, value);
}

server::console::Environment::ValuePtr_t
server::console::Environment::pushNew(String_t name, ValuePtr_t value)
{
    size_t paramIndex;
    if (name.empty() || isPositional(name, paramIndex)) {
        // @change c2console-classic allows setting positional parameters, c2console-ng doesn't.
        throw std::runtime_error("Invalid variable name");
    }

    afl::data::NameMap::Index_t index = (value.get() != 0
                                         ? m_names.addMaybe(name)
                                         : m_names.getIndexByName(name));
    ValuePtr_t result;
    if (index != m_names.nil) {
        result.reset(m_values.extractElement(index));
        m_values.setNew(index, value.release());
    }
    return result;
}

void
server::console::Environment::popNew(String_t name, ValuePtr_t value)
{
    pushNew(name, value);
}

server::console::Environment::SegmentPtr_t
server::console::Environment::pushPositionalParameters(afl::data::Segment& seg)
{
    SegmentPtr_t result(new afl::data::Segment());
    result->swap(m_positionalParameters);
    m_positionalParameters.swap(seg);
    return result;
}

void
server::console::Environment::popPositionalParameters(SegmentPtr_t ptr)
{
    if (ptr.get() != 0) {
        m_positionalParameters.swap(*ptr);
    } else {
        m_positionalParameters.clear();
    }
}

afl::data::Value*
server::console::Environment::get(String_t name)
{
    size_t n;
    if (isPositional(name, n)) {
        return m_positionalParameters[n];
    } else {
        return m_values[m_names.getIndexByName(name)];
    }
}

void
server::console::Environment::listContent(afl::data::Segment& result)
{
    // Positional
    for (size_t i = 0, n = m_positionalParameters.size(); i < n; ++i) {
        result.pushBackInteger(static_cast<int32_t>(i + 1));
        result.pushBack(m_positionalParameters[i]);
    }

    // Named
    for (size_t i = 0, n = m_names.getNumNames(); i < n; ++i) {
        if (afl::data::Value* p = m_values[i]) {
            result.pushBackString(m_names.getNameByIndex(i));
            result.pushBack(p);
        }
    }
}
