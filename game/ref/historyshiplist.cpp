/**
  *  \file game/ref/historyshiplist.cpp
  *  \brief Class game::ref::HistoryShipList
  */

#include <algorithm>
#include "game/ref/historyshiplist.hpp"
#include "game/ref/sortpredicate.hpp"

namespace {
    /* Wrap a HistoryShipList::SortPredicate for std::sort.
       Needs to be global in C++03. */
    class PredicateWrapper {
     public:
        PredicateWrapper(const game::ref::HistoryShipList::SortPredicate& pred)
            : m_predicate(pred)
            { }
        bool operator()(const game::ref::HistoryShipList::Item& a, const game::ref::HistoryShipList::Item& b) const
            {
                // Compare using predicate
                int diff = m_predicate.compare(a, b);
                if (diff != 0) {
                    return diff < 0;
                }

                // Use Id as tie-breaker. (This isn't intended to see anything other than ships.)
                return a.reference.getId() < b.reference.getId();
            }
     private:
        const game::ref::HistoryShipList::SortPredicate& m_predicate;
    };
}

bool
game::ref::HistoryShipList::Item::operator==(const Item& other) const
{
    return turnNumber == other.turnNumber
        && static_cast<const UserList::Item&>(*this) == static_cast<const UserList::Item&>(other);
}

bool
game::ref::HistoryShipList::Item::operator!=(const Item& other) const
{
    return !operator==(other);
}


/*
 *  HistoryShipList
 */

game::ref::HistoryShipList::HistoryShipList()
    : m_items(),
      m_turnNumber(0)
{ }

game::ref::HistoryShipList::~HistoryShipList()
{ }

void
game::ref::HistoryShipList::setReferenceTurn(int turnNumber)
{
    m_turnNumber = turnNumber;
}

int
game::ref::HistoryShipList::getReferenceTurn() const
{
    return m_turnNumber;
}

void
game::ref::HistoryShipList::clear()
{
    m_items.clear();
}

void
game::ref::HistoryShipList::add(const Item& item)
{
    m_items.push_back(item);
}

const game::ref::HistoryShipList::Item*
game::ref::HistoryShipList::get(size_t index) const
{
    return (index < m_items.size() ? &m_items[index] : 0);
}

size_t
game::ref::HistoryShipList::size() const
{
    return m_items.size();
}

bool
game::ref::HistoryShipList::empty() const
{
    return m_items.empty();
}

bool
game::ref::HistoryShipList::find(Reference ref, size_t& pos) const
{
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (ref == m_items[i].reference) {
            pos = i;
            return true;
        }
    }
    return false;
}

void
game::ref::HistoryShipList::sort(const SortPredicate& p)
{
    // Copy all references to temporary vector (i.e. remove dividers)
    std::vector<Item> copy;
    copy.reserve(m_items.size());
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (m_items[i].type == UserList::ReferenceItem) {
            copy.push_back(m_items[i]);
        }
    }

    // Sort
    std::sort(copy.begin(), copy.end(), PredicateWrapper(p));

    // Copy back, adding dividers
    String_t currentDivider;
    m_items.clear();
    for (size_t i = 0, n = copy.size(); i < n; ++i) {
        String_t thisDivider = p.getClass(copy[i]);
        if (thisDivider != currentDivider && !thisDivider.empty()) {
            m_items.push_back(Item(UserList::Item(UserList::DividerItem, thisDivider, Reference(), false, game::map::Object::NotPlayable, util::SkinColor::Static), 0));
        }
        currentDivider = thisDivider;
        m_items.push_back(copy[i]);
    }
}

void
game::ref::HistoryShipList::sort(const game::ref::SortPredicate& p)
{
    // Adaptor for game::ref::SortPredicate -> SortPredicate
    class Adaptor : public SortPredicate {
     public:
        Adaptor(const game::ref::SortPredicate& p)
            : m_predicate(p)
            { }
        virtual int compare(const Item& a, const Item& b) const
            { return m_predicate.compare(a.reference, b.reference); }
        virtual String_t getClass(const Item& a) const
            { return m_predicate.getClass(a.reference); }
     private:
        const game::ref::SortPredicate& m_predicate;
    };

    // Use our specific sorter
    sort(Adaptor(p));
}

bool
game::ref::HistoryShipList::operator==(const HistoryShipList& other) const
{
    return m_turnNumber == other.m_turnNumber
        && m_items == other.m_items;
}

bool
game::ref::HistoryShipList::operator!=(const HistoryShipList& other) const
{
    return !operator==(other);
}
