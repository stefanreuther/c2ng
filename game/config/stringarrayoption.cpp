/**
  *  \file game/config/stringarrayoption.cpp
  *  \brief Class game::config::StringArrayOption
  */

#include "game/config/stringarrayoption.hpp"

game::config::StringArrayOption::StringArrayOption(int minIndex, int numSlots)
    : m_minIndex(minIndex),
      m_data()
{
    // ConfigStringArrayOption::ConfigStringArrayOption
    m_data.resize(static_cast<size_t>(numSlots));
}

game::config::StringArrayOption::~StringArrayOption()
{ }

void
game::config::StringArrayOption::set(int index, String_t value)
{
    size_t x = static_cast<size_t>(index - m_minIndex);
    if (x < m_data.size() && m_data[x] != value) {
        m_data[x] = value;
        markChanged();
    }
}

String_t
game::config::StringArrayOption::operator()(int index) const
{
    // ConfigStringArrayOption::operator()(int index) const
    size_t x = static_cast<size_t>(index - m_minIndex);
    if (x < m_data.size()) {
        return m_data[x];
    } else {
        return String_t();
    }
}

int
game::config::StringArrayOption::getFirstIndex() const
{
    return m_minIndex;
}

int
game::config::StringArrayOption::getNumSlots() const
{
    return static_cast<int>(m_data.size());
}

// ConfigurationOption:
void
game::config::StringArrayOption::set(String_t value)
{
    // ConfigStringArrayOption::set(string_t value)
    size_t pos = 0;
    for (size_t i = 0; i < m_data.size(); ++i) {
        size_t next = value.find(',', pos);
        if (next == String_t::npos) {
            m_data[i] = afl::string::strTrim(value.substr(pos));
            pos = value.size();
        } else {
            m_data[i] = afl::string::strTrim(value.substr(pos, next - pos));
            pos = next+1;
        }
    }
    markChanged();
}

String_t
game::config::StringArrayOption::toString() const
{
    // ConfigStringArrayOption::toString() const
    String_t result;
    size_t limit = m_data.size();
    while (limit > 0 && m_data[limit-1].empty()) {
        --limit;
    }
    if (limit > 0) {
        result += m_data[0];
        for (size_t i = 1; i < limit; ++i) {
            result += ",";
            result += m_data[i];
        }
    }
    return result;
}
