/**
  *  \file game/spec/basecomponentvector.cpp
  */

#include <memory>
#include "game/spec/basecomponentvector.hpp"

// Set new element.
void
game::spec::BaseComponentVector::setNew(int id, Component* p)
{
    std::auto_ptr<Component> save(p);

    // FIXME: give PtrVector a resize()?
    while (id > static_cast<int>(m_components.size())) {
        m_components.pushBackNew(0);
    }
    m_components.replaceElementNew(id-1, save.release());
}

game::spec::Component*
game::spec::BaseComponentVector::get(int id) const
{
    if (id > 0 && id <= static_cast<int>(m_components.size())) {
        return m_components[id-1];
    } else {
        return 0;
    }
}

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

bool
game::spec::BaseComponentVector::Names::getFirstKey(int& a) const
{
    // FIXME: use findNext etc.
    a = 0;
    return getNextKey(a);
}

bool
game::spec::BaseComponentVector::Names::getNextKey(int& a) const
{
    int n = m_vec.size();
    while (a < n) {
        ++a;
        if (m_vec.get(a)) {
            return true;
        }
    }
    return false;
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
