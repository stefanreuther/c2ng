/**
  *  \file game/ref/userlist.cpp
  */

#include "game/ref/userlist.hpp"
#include "game/ref/list.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "game/map/universe.hpp"
#include "game/map/object.hpp"
#include "game/ref/sortpredicate.hpp"

game::ref::UserList::UserList()
    : m_items()
{ }

game::ref::UserList::~UserList()
{ }

void
game::ref::UserList::clear()
{
    m_items.clear();
}

void
game::ref::UserList::add(Type type, String_t name, Reference ref, bool marked, util::SkinColor::Color color)
{
    m_items.push_back(Item(type, name, ref, marked, color));
}

void
game::ref::UserList::add(const List& list, Session& session, const SortPredicate& divi, const SortPredicate& subdivi)
{
    // ex GObjectList::addDividers (sort-of)
    String_t thisClass;
    String_t thisSubclass;
    for (size_t i = 0, n = list.size(); i < n; ++i) {
        // Dividers
        const Reference r = list[i];
        const String_t newClass = divi.getClass(r);
        if (newClass != thisClass) {
            if (!newClass.empty()) {
                add(DividerItem, newClass, Reference(), false, util::SkinColor::Static);
            }
            thisClass = newClass;
            thisSubclass.clear();
        }
        const String_t newSubclass = subdivi.getClass(r);
        if (newSubclass != thisSubclass) {
            if (!newSubclass.empty()) {
                add(SubdividerItem, newSubclass, Reference(), false, util::SkinColor::Static);
            }
            thisSubclass = newSubclass;
        }

        // Actual item
        // - name
        String_t name;
        if (!session.getReferenceName(r, DetailedName, name)) {
            name = r.toString(session.translator());
        }

        // - marked, color
        bool marked = false;
        util::SkinColor::Color color = util::SkinColor::Static;
        if (const Game* g = session.getGame().get()) {
            if (const Turn* t = g->getViewpointTurn().get()) {
                if (const game::map::Object* p = t->universe().getObject(r)) {
                    marked = p->isMarked();

                    int owner = 0;
                    if (p->getOwner(owner) && owner != 0) {
                        switch (g->teamSettings().getPlayerRelation(owner)) {
                         case TeamSettings::ThisPlayer:    color = util::SkinColor::Green;  break;
                         case TeamSettings::AlliedPlayer:  color = util::SkinColor::Yellow; break;
                         case TeamSettings::EnemyPlayer:   color = util::SkinColor::Red;    break;
                        }
                    }
                }
            }
        }

        add(ReferenceItem, name, r, marked, color);
    }
}

void
game::ref::UserList::add(const UserList& list)
{
    for (size_t i = 0, n = list.m_items.size(); i < n; ++i) {
        m_items.push_back(list.m_items[i]);
    }
}

size_t
game::ref::UserList::size() const
{
    return m_items.size();
}

bool
game::ref::UserList::empty() const
{
    return m_items.empty();
}

bool
game::ref::UserList::find(Reference ref, size_t& pos) const
{
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (ref == m_items[i].reference) {
            pos = i;
            return true;
        }
    }
    return false;
}

const game::ref::UserList::Item*
game::ref::UserList::get(size_t index) const
{
    return (index < m_items.size() ? &m_items[index] : 0);
}

bool
game::ref::UserList::operator==(const UserList& other) const
{
    return m_items == other.m_items;
}

bool
game::ref::UserList::operator!=(const UserList& other) const
{
    return !operator==(other);
}
