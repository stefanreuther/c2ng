/**
  *  \file game/ref/fleetmemberlist.cpp
  *  \brief Class game::ref::FleetMemberList
  */

#include "game/ref/fleetmemberlist.hpp"
#include "game/map/ship.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/ref/sortpredicate.hpp"

using game::Reference;
using game::map::AnyShipType;
using game::map::Point;
using game::map::Ship;
using game::ref::FleetMemberList;
using game::ref::UserList;

namespace {
    /* Wrap a FleetList::SortPredicate for std::sort.
       Needs to be global in C++03. */
    class PredicateWrapper {
     public:
        PredicateWrapper(const FleetMemberList::SortPredicate& pred)
            : m_predicate(pred)
            { }
        bool operator()(const FleetMemberList::Item& a, const FleetMemberList::Item& b) const
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
        const FleetMemberList::SortPredicate& m_predicate;
    };

    FleetMemberList::Item makeItem(const Ship& sh, const FleetMemberList::Flags_t flags)
    {
        return FleetMemberList::Item(UserList::Item(UserList::ReferenceItem,
                                                    sh.getName(),
                                                    Reference(Reference::Ship, sh.getId()),
                                                    sh.isMarked(),
                                                    sh.getPlayability(),
                                                    util::SkinColor::Static),
                                     flags,
                                     sh.getFriendlyCode().orElse(""),
                                     sh.getPosition().orElse(Point()));
    }
}

bool
game::ref::FleetMemberList::Item::operator==(const Item& other) const
{
    return flags == other.flags
        && friendlyCode == other.friendlyCode
        && position == other.position
        && static_cast<const UserList::Item&>(*this) == static_cast<const UserList::Item&>(other);
}

bool
game::ref::FleetMemberList::Item::operator!=(const Item& other) const
{
    return !operator==(other);
}

game::ref::FleetMemberList::FleetMemberList()
    : m_items()
{ }

game::ref::FleetMemberList::~FleetMemberList()
{ }

void
game::ref::FleetMemberList::clear()
{
    m_items.clear();
}

void
game::ref::FleetMemberList::add(const Item& item)
{
    m_items.push_back(item);
}

const game::ref::FleetMemberList::Item*
game::ref::FleetMemberList::get(size_t index) const
{
    return (index < m_items.size() ? &m_items[index] : 0);
}

size_t
game::ref::FleetMemberList::size() const
{
    return m_items.size();
}

bool
game::ref::FleetMemberList::empty() const
{
    return m_items.empty();
}

afl::base::Optional<size_t>
game::ref::FleetMemberList::find(Reference ref) const
{
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (ref == m_items[i].reference) {
            return i;
        }
    }
    return afl::base::Nothing;
}

void
game::ref::FleetMemberList::sort(const SortPredicate& p)
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
            m_items.push_back(Item(UserList::Item(UserList::DividerItem, thisDivider, Reference(), false, game::map::Object::NotPlayable, util::SkinColor::Static), Flags_t(), String_t(), game::map::Point()));
        }
        currentDivider = thisDivider;
        m_items.push_back(copy[i]);
    }
}

void
game::ref::FleetMemberList::sort(const game::ref::SortPredicate& p)
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
game::ref::FleetMemberList::setFleet(const game::map::Universe& univ, Id_t fleetNumber)
{
    // ex WFleetMemberList::setFleet [part]
    m_items.clear();
    if (const Ship* leader = univ.ships().get(fleetNumber)) {
        // Add leader
        m_items.push_back(makeItem(*leader, Flags_t(Leader)));
        const Point leaderPos = leader->getPosition().orElse(Point());

        // Add members
        // We check AnyShipType for simplicity; fleets should only involve playable ships.
        const AnyShipType& ships(univ.allShips());
        for (Id_t i = ships.getNextIndex(0); i != 0; i = ships.getNextIndex(i)) {
            if (const Ship* mem = ships.getObjectByIndex(i)) {
                if (i != fleetNumber && mem->isPlayable(game::map::Object::ReadOnly) && mem->getFleetNumber() == fleetNumber) {
                    Flags_t flags;
                    Point memPos;
                    if (mem->getPosition().get(memPos) && memPos != leaderPos) {
                        flags += Away;
                    }
                    m_items.push_back(makeItem(*mem, flags));
                }
            }
        }

        // Process tow/towee
        for (size_t i = 0, n = m_items.size(); i < n; ++i) {
            if (const Ship* mem = ships.getObjectByIndex(m_items[i].reference.getId())) {
                if (mem->getMission().orElse(-1) == game::spec::Mission::msn_Tow) {
                    Id_t towId = mem->getMissionParameter(TowParameter).orElse(-1);
                    for (size_t j = 0; j < n; ++j) {
                        if (j != i && m_items[j].reference.getId() == towId) {
                            m_items[i].flags += Towing;
                            m_items[j].flags += Towed;
                            break;
                        }
                    }
                }
            }
        }
    }
}

bool
game::ref::FleetMemberList::operator==(const FleetMemberList& other) const
{
    return m_items == other.m_items;
}

bool
game::ref::FleetMemberList::operator!=(const FleetMemberList& other) const
{
    return !operator==(other);
}
