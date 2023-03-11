/**
  *  \file util/expressionlist.cpp
  *  \brief Class util::ExpressionList
  */

#include "util/expressionlist.hpp"

util::ExpressionList::ExpressionList()
    : m_items()
{
    // ex LRUList::LRUList
}

util::ExpressionList::~ExpressionList()
{ }

size_t
util::ExpressionList::size() const
{
    // ex LRUList::size
    return m_items.size();
}

bool
util::ExpressionList::empty() const
{
    // ex LRUList::empty
    return m_items.empty();
}

const util::ExpressionList::Item*
util::ExpressionList::get(size_t index) const
{
    // ex LRUList::operator[]
    if (index < m_items.size()) {
        return m_items[index];
    } else {
        return 0;
    }
}


bool
util::ExpressionList::findIndexForValue(const String_t& value, size_t& index) const
{
    // ex LRUList::findValue
    size_t i = 0;
    while (i < m_items.size()) {
        if (m_items[i]->value == value) {
            index = i;
            return true;
        }

        ++i;
    }
    return false;
}

void
util::ExpressionList::pushBackNew(Item* item)
{
    // ex LRUList::pushNew
    m_items.pushBackNew(item);
}

void
util::ExpressionList::pushFrontNew(Item* item, size_t limit)
{
    // ex LRUList::pushNewLimit
    // add an item and move it to the front
    m_items.pushBackNew(item);
    moveToFront(m_items.size()-1);

    // remove duplicates
    size_t in = 1, out = 1;
    while (in < m_items.size() && out < limit) {
        if (m_items[in]->value == item->value) {
            // skip
            ++in;
        } else {
            // keep
            if (in != out) {
                m_items.swapElements(in, out);
            }
            ++in, ++out;
        }
    }

    // drop unused items
    while (m_items.size() > out) {
        m_items.popBack();
    }
}

void
util::ExpressionList::moveToFront(size_t index)
{
    // ex LRUList::moveToFront
    if (index < m_items.size()) {
        while (index > 0) {
            m_items.swapElements(index, index-1);
            --index;
        }
    }
}

void
util::ExpressionList::clear()
{
    // ex LRUList::clear
    m_items.clear();
}
