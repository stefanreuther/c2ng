/**
  *  \file game/map/objectvector.hpp
  */
#ifndef C2NG_GAME_MAP_OBJECTVECTOR_HPP
#define C2NG_GAME_MAP_OBJECTVECTOR_HPP

#include "afl/container/ptrvector.hpp"
#include "game/types.hpp"

namespace game { namespace map {

    template<typename T>
    class ObjectVector {
     public:
        T* create(Id_t id);

        T* get(Id_t id) const;

        void clear();

        Id_t size() const;

     private:
        afl::container::PtrVector<T> m_components;
    };

} }



template<typename T>
T*
game::map::ObjectVector<T>::create(Id_t id)
{
    if (id > 0) {
        size_t requiredSize = static_cast<size_t>(id);
        if (m_components.size() < requiredSize) {
            m_components.resize(requiredSize);
        }
        if (m_components[id-1] == 0) {
            m_components.replaceElementNew(id-1, new T(id));
        }
        return m_components[id-1];
    } else {
        return 0;
    }
}

template<typename T>
T*
game::map::ObjectVector<T>::get(Id_t id) const
{
    if (id > 0 && id <= static_cast<Id_t>(m_components.size())) {
        return m_components[id-1];
    } else {
        return 0;
    }
}

template<typename T>
void
game::map::ObjectVector<T>::clear()
{
    m_components.clear();
}

template<typename T>
game::Id_t
game::map::ObjectVector<T>::size() const
{
    return Id_t(m_components.size());
}

#endif
