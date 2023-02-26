/**
  *  \file game/spec/basecomponentvector.cpp
  *  \brief Class game::spec::BaseComponentVector
  */

#include <memory>
#include "game/spec/basecomponentvector.hpp"
#include "game/spec/component.hpp"

// Protected destructor.
game::spec::BaseComponentVector::~BaseComponentVector()
{ }

// Get a component by number.
game::spec::Component*
game::spec::BaseComponentVector::get(int id) const
{
    if (id > 0 && id <= static_cast<int>(m_components.size())) {
        return m_components[id-1];
    } else {
        return 0;
    }
}

// Clear.
void
game::spec::BaseComponentVector::clear()
{
    m_components.clear();
}

// Find next component, given an Id.
game::spec::Component*
game::spec::BaseComponentVector::findNext(int id) const
{
    int n = size();
    while (id < n) {
        ++id;
        if (Component* p = get(id)) {
            return p;
        }
    }
    return 0;
}

// Set new element.
void
game::spec::BaseComponentVector::setNew(int id, Component* p)
{
    std::auto_ptr<Component> save(p);
    if (id > 0) {
        size_t index = static_cast<size_t>(id - 1);
        if (index >= m_components.size()) {
            m_components.resize(index + 1);
        }
        m_components.replaceElementNew(index, save.release());
    }
}

bool
game::spec::BaseComponentVector::Names::getFirstKey(int& a) const
{
    a = 0;
    return getNextKey(a);
}

bool
game::spec::BaseComponentVector::Names::getNextKey(int& a) const
{
    if (const Component* p = m_vec.findNext(a)) {
        a = p->getId();
        return true;
    } else {
        return false;
    }
}

String_t
game::spec::BaseComponentVector::Names::get(int a) const
{
    if (Component* p = m_vec.get(a)) {
        return m_shortName
            ? p->getShortName(m_provider)
            : p->getName(m_provider);
    } else {
        return String_t();
    }
}
