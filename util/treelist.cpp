/**
  *  \file util/treelist.cpp
  *  \brief Class util::TreeList
  */

#include "util/treelist.hpp"

// Constructor.
util::TreeList::TreeList()
    : m_data()
{
    clear();
}

// Destructor. */
util::TreeList::~TreeList()
{ }

// Add a key/string pair.
size_t
util::TreeList::add(int32_t key, const String_t& s, size_t childOf)
{
    if (childOf < m_data.size()) {
        // Add new element
        size_t idx = m_data.size();
        m_data.push_back(Element(key, s));

        // Link it
        if (m_data[childOf].firstChild == nil) {
            m_data[childOf].firstChild = idx;
        } else {
            size_t i = m_data[childOf].firstChild;
            while (m_data[i].nextSibling != nil) {
                i = m_data[i].nextSibling;
            }
            m_data[i].nextSibling = idx;
        }
        return idx;
    } else {
        return 0;
    }
}

// Add a key/string pair, given a path.
size_t
util::TreeList::addPath(int32_t key, afl::base::Memory<const String_t> path, size_t childOf)
{
    while (const String_t* s = path.eat()) {
        size_t index = findChildByLabel(*s, childOf);
        if (index == nil) {
            index = add(0, *s, childOf);
        }
        childOf = index;
    }

    if (childOf < m_data.size()) {
        m_data[childOf].key = key;
    }
    return childOf;
}

// Swap content with another list.
void
util::TreeList::swap(TreeList& other)
{
    m_data.swap(other.m_data);
}

// Clear list. */
void
util::TreeList::clear()
{
    m_data.clear();
    m_data.push_back(Element(0, String_t())); // root node
}

// Check whether a node has children.
bool
util::TreeList::hasChildren(size_t index) const
{
    return getFirstChild(index) != nil;
}

// Get first child of a node.
size_t
util::TreeList::getFirstChild(size_t index) const
{
    return (index < m_data.size()
            ? m_data[index].firstChild
            : nil);
}

// Get next sibling of a node.
size_t
util::TreeList::getNextSibling(size_t index) const
{
    return (index < m_data.size()
            ? m_data[index].nextSibling
            : nil);
}

// Find child, given a label.
size_t
util::TreeList::findChildByLabel(const String_t& s, size_t parent) const
{
    for (size_t i = getFirstChild(parent); i != nil; i = getNextSibling(i)) {
        if (m_data[i].label == s) {
            return i;
        }
    }
    return nil;
}

// Get key/string pair, given an index.
bool
util::TreeList::get(size_t index, int32_t& key, String_t& s) const
{
    if (index < m_data.size()) {
        key = m_data[index].key;
        s = m_data[index].label;
        return true;
    } else {
        return false;
    }
}
