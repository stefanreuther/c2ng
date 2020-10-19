/**
  *  \file game/vcr/overview.cpp
  */

#include "game/vcr/overview.hpp"
#include "game/vcr/object.hpp"

namespace {
    bool SortGroups(const game::vcr::Overview::Item& a, const game::vcr::Overview::Item& b)
    {
        if (a.groupId == b.groupId) {
            return a.sequence < b.sequence;
        } else {
            return a.groupId < b.groupId;
        }
    }
}

game::vcr::Overview::Overview()
    : lines(),
      m_groupCounter(0)
{ }

void
game::vcr::Overview::clear()
{
    lines.clear();
    m_groupCounter = 0;
}

void
game::vcr::Overview::addBattle(Battle& b,
                               const game::config::HostConfiguration& config,
                               const game::spec::ShipList& shipList)
{
    // ex WCombatDiagram::init

    // Compute result
    b.prepareResult(config, shipList, Battle::NeedQuickOutcome);

    // Assimilate first object and obtain a group Id
    const Object* leftObject = b.getObject(0, false);
    if (leftObject == 0) {
        // Error: battle has no first object. Ignore.
        return;
    }
    
    std::vector<Item>::const_iterator lptr = findObject(*leftObject);
    Id_t groupId;
    if (lptr != lines.end()) {
        // already known, reuse group Id
        groupId = lptr->groupId;
    } else {
        // not known, allocate new group Id
        groupId = ++m_groupCounter;
        lines.push_back(Item(leftObject->isPlanet(), leftObject->getId(), groupId, lines.size(), leftObject->getName()));
    }

    // Assimilate other objects
    for (size_t side = 1, n = b.getNumObjects(); side < n; ++side) {
        if (const Object* rightObject = b.getObject(side, false)) {
            std::vector<Item>::const_iterator rptr = findObject(*rightObject);
            if (rptr != lines.end()) {
                // already known, rename into earlier group
                if (rptr->groupId < groupId) {
                    renameGroup(groupId, rptr->groupId);
                    groupId = rptr->groupId;
                } else {
                    renameGroup(rptr->groupId, groupId);
                }
            } else {
                // not known, put into this group */
                lines.push_back(Item(rightObject->isPlanet(), rightObject->getId(), groupId, lines.size(), rightObject->getName()));
            }
        }
    }
}

void
game::vcr::Overview::addDatabase(Database& db,
                                 const game::config::HostConfiguration& config,
                                 const game::spec::ShipList& shipList)
{
    for (size_t i = 0, n = db.getNumBattles(); i < n; ++i) {
        if (Battle* b = db.getBattle(i)) {
            addBattle(*b, config, shipList);
        }
    }
}

void
game::vcr::Overview::finish()
{
    // Sort
    std::sort(lines.begin(), lines.end(), SortGroups);
}

// /** Find an object in the diagram.
//     \param obj Object to find
//     \return iterator pointing to this object's line, end() if none */
std::vector<game::vcr::Overview::Item>::const_iterator
game::vcr::Overview::findObject(const Object& obj) const
{
    // ex WCombatDiagram::findObject
    std::vector<Item>::const_iterator i = lines.begin();
    while (i != lines.end() && (i->planet != obj.isPlanet() || i->id != obj.getId())) {
        ++i;
    }
    return i;
}

// /** Rename a group.
//     \param from Replace this group Id...
//     \param to ...by this one */
void
game::vcr::Overview::renameGroup(int from, int to)
{
    // ex WCombatDiagram::renameGroup
    for (std::vector<Item>::iterator i = lines.begin(); i != lines.end(); ++i) {
        if (i->groupId == from) {
            i->groupId = to;
        }
    }
}
