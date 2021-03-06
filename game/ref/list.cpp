/**
  *  \file game/ref/list.cpp
  */

#include "game/ref/list.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/ref/sortpredicate.hpp"

namespace {
    class PredicateWrapper {
     public:
        PredicateWrapper(const game::ref::SortPredicate& pred)
            : m_predicate(pred)
            { }
        bool operator()(const game::Reference& a, const game::Reference& b)
            {
                // Compare using predicate
                int diff = m_predicate.compare(a, b);
                if (diff != 0) {
                    return diff < 0;
                }

                // Use type as tie-breaker
                if (a.getType() != b.getType()) {
                    return a.getType() < b.getType();
                }

                // If that still does not work, compare content.
                // Compare points and Ids separately.
                game::map::Point pa, pb;
                if (a.getPos(pa) && b.getPos(pb)) {
                    return pa.compare(pb) < 0;
                } else {
                    return a.getId() < b.getId();
                }
            }
     private:
        const game::ref::SortPredicate& m_predicate;
    };
}

game::ref::List::List()
    : m_content()
{ }

game::ref::List::~List()
{ }

void
game::ref::List::add(Reference ref)
{
    m_content.push_back(ref);
}

void
game::ref::List::add(Reference::Type type, const std::vector<Id_t> ids)
{
    // ex GObjectList::addNonObject, GObjectList::addObject
    for (size_t i = 0, n = ids.size(); i < n; ++i) {
        m_content.push_back(Reference(type, ids[i]));
    }
}

// /** Add all objects at a particular location. This is used for certain ship lists.
//     \param univ  universe
//     \param pt    location
//     \param flags ls_IncludePlanet: include planet in list;
//                  ls_IncludeForeignShips: also list foreign ships
//                  (default: only list ships we're playing);
//                  ls_SafeShipsOnly: include only safe ships (not guessed)
//     \param exclude_ship do not include this ship

//     \todo We need a means to say which player must safely see the ship */
void
game::ref::List::addObjectsAt(game::map::Universe& univ, game::map::Point pt, Options_t options, Id_t excludeShipId)
{
    // ex GObjectList::addObjectsAt

    // Canonical location
    pt = univ.config().getCanonicalLocation(pt);

    // Handle planet
    if (options.contains(IncludePlanet)) {
        if (Id_t pid = univ.findPlanetAt(pt)) {
            add(Reference(Reference::Planet, pid));
        }
    }

    // Handle ships
    // @change PCC2 checks for Object::Playable instead of ReadOnly
    game::map::AnyShipType type(univ);
    for (Id_t sid = type.findNextIndex(0); sid != 0; sid = type.findNextIndex(sid)) {
        game::map::Ship* pShip = univ.ships().get(sid);
        game::map::Point shipPos;
        if (sid != excludeShipId
            && pShip != 0
            && pShip->getPosition(shipPos)
            && shipPos == pt
            && (options.contains(IncludeForeignShips) || pShip->isPlayable(game::map::Object::ReadOnly))
            && (!options.contains(SafeShipsOnly) || pShip->isReliablyVisible(0)))
        {
            add(Reference(Reference::Ship, sid));
        }
    }
}

void
game::ref::List::clear()
{
    // ex GObjectList::clear
    m_content.clear();
}

game::Reference
game::ref::List::operator[](size_t pos) const
{
    return (pos < m_content.size()
            ? m_content[pos]
            : Reference());
}

void
game::ref::List::set(size_t pos, Reference ref)
{
    if (pos < m_content.size()) {
        m_content[pos] = ref;
    }
}

size_t
game::ref::List::size() const
{
    return m_content.size();
}

void
game::ref::List::sort(const SortPredicate& pred)
{
    // GObjectList::sort
    std::sort(m_content.begin(), m_content.end(), PredicateWrapper(pred));
}
