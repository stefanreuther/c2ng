/**
  *  \file game/ref/fleetlist.cpp
  *  \brief Class game::ref::FleetList
  *
  *  TODO: This class is very similar to HistoryShipList. Can we merge them?
  */

#include "game/ref/fleetlist.hpp"
#include "game/map/fleettype.hpp"
#include "game/ref/sortpredicate.hpp"
#include "game/map/fleet.hpp"

namespace {
    /* Wrap a FleetList::SortPredicate for std::sort.
       Needs to be global in C++03. */
    class PredicateWrapper {
     public:
        PredicateWrapper(const game::ref::FleetList::SortPredicate& pred)
            : m_predicate(pred)
            { }
        bool operator()(const game::ref::FleetList::Item& a, const game::ref::FleetList::Item& b) const
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
        const game::ref::FleetList::SortPredicate& m_predicate;
    };
}

bool
game::ref::FleetList::Item::operator==(const Item& other) const
{
    return isAtReferenceLocation == other.isAtReferenceLocation
        && static_cast<const UserList::Item&>(*this) == static_cast<const UserList::Item&>(other);
}

bool
game::ref::FleetList::Item::operator!=(const Item& other) const
{
    return !operator==(other);
}

game::ref::FleetList::FleetList()
    : m_items()
{ }

game::ref::FleetList::~FleetList()
{ }

void
game::ref::FleetList::clear()
{
    m_items.clear();
}

void
game::ref::FleetList::add(const Item& item)
{
    m_items.push_back(item);
}

const game::ref::FleetList::Item*
game::ref::FleetList::get(size_t index) const
{
    // ex WFleetList::operator[](std::size_t index) (sort-of)
    return (index < m_items.size() ? &m_items[index] : 0);
}

size_t
game::ref::FleetList::size() const
{
    // ex WFleetList::size()
    return m_items.size();
}

bool
game::ref::FleetList::empty() const
{
    return m_items.empty();
}

bool
game::ref::FleetList::find(Reference ref, size_t& pos) const
{
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (ref == m_items[i].reference) {
            pos = i;
            return true;
        }
    }
    return false;
}

size_t
game::ref::FleetList::findInitialSelection() const
{
    // ex WFleetList::getInitialSelection
    size_t result = 0;
    bool hasFleet = false;
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (m_items[i].type == UserList::ReferenceItem) {
            if (m_items[i].isAtReferenceLocation) {
                // Found first marked "here"
                result = i;
                break;
            }
            if (!hasFleet) {
                // Found first fleet; remember as fall-back
                result = i;
                hasFleet = true;
            }
        }
    }
    return result;
}

void
game::ref::FleetList::sort(const SortPredicate& p)
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
game::ref::FleetList::sort(const game::ref::SortPredicate& p)
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

void
game::ref::FleetList::addAll(const game::map::Universe& univ, afl::base::Optional<game::map::Point> refLoc, Id_t except, bool includeAll, afl::string::Translator& tx)
{
    // ex WFleetList::listFleets, fleet.pas:FillFleetList
    game::map::FleetType& ty = const_cast<game::map::Universe&>(univ).fleets();
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        if (const game::map::Ship* sh = ty.getObjectByIndex(i)) {
            if (i != except) {
                game::map::Point pt;
                const bool isHere = refLoc.get() != 0 && sh->getPosition(pt) && pt == *refLoc.get();
                if (isHere || includeAll) {
                    add(Item(UserList::Item(UserList::ReferenceItem,
                                            game::map::Fleet::getTitle(*sh, tx),
                                            Reference(Reference::Ship, i),
                                            sh->isMarked(),
                                            sh->getPlayability(),
                                            util::SkinColor::Static),
                             isHere));
                }
            }
        }
    }
}

bool
game::ref::FleetList::operator==(const FleetList& other) const
{
    return m_items == other.m_items;
}

bool
game::ref::FleetList::operator!=(const FleetList& other) const
{
    return !operator==(other);
}
