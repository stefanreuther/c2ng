/**
  *  \file util/stringlist.cpp
  *  \brief Class util::StringList
  */

#include "util/stringlist.hpp"

namespace {
    bool sortFunction(const std::pair<int32_t,String_t>& a, const std::pair<int32_t,String_t>& b)
    {
        return a.second < b.second;
    }
}


util::StringList::StringList()
    : m_data()
{
    // ex StringList::StringList
}

util::StringList::~StringList()
{
    // ex StringList::~StringList
}

void
util::StringList::add(int32_t key, const String_t& s)
{
    // ex StringList::add
    m_data.push_back(std::make_pair(key, s));
}

void
util::StringList::sortAlphabetically()
{
    sort(m_data.begin(), m_data.end(), sortFunction);
}

void
util::StringList::swap(StringList& other)
{
    m_data.swap(other.m_data);
}

void
util::StringList::clear()
{
    m_data.clear();
}

// Random access
size_t
util::StringList::size() const
{
    // ex StringList::getCount
    return m_data.size();
}

bool
util::StringList::empty() const
{
    return m_data.empty();
}

bool
util::StringList::get(size_t index, int32_t& key, String_t& s) const
{
    // ex StringList::getItem, sort-of
    if (index < m_data.size()) {
        key = m_data[index].first;
        s = m_data[index].second;
        return true;
    } else {
        return false;
    }
}

bool
util::StringList::find(int32_t key, size_t& index) const
{
    // ex StringList::getItemByKey, StringList::getItemIndexByKey
    for (size_t i = 0, n = m_data.size(); i < n; ++i) {
        if (m_data[i].first == key) {
            index = i;
            return true;
        }
    }
    return false;
}
