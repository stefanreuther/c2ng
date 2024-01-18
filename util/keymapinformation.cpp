/**
  *  \file util/keymapinformation.cpp
  *  \brief Class util::KeymapInformation
  */

#include "util/keymapinformation.hpp"

const util::KeymapInformation::Index_t util::KeymapInformation::nil;

util::KeymapInformation::KeymapInformation()
    : m_data()
{ }

util::KeymapInformation::~KeymapInformation()
{ }

void
util::KeymapInformation::clear()
{
    m_data.clear();
}

size_t
util::KeymapInformation::size() const
{
    return m_data.size();
}

void
util::KeymapInformation::add(size_t level, const String_t& name)
{
    m_data.push_back(std::make_pair(level, name));
}

bool
util::KeymapInformation::get(Index_t index, size_t& level, String_t& name) const
{
    if (index < m_data.size()) {
        level = m_data[index].first;
        name = m_data[index].second;
        return true;
    } else {
        return false;
    }
}

util::KeymapInformation::Index_t
util::KeymapInformation::find(const String_t& name) const
{
    for (Index_t i = 0, n = m_data.size(); i < n; ++i) {
        if (name == m_data[i].second) {
            return i;
        }
    }
    return nil;
}
