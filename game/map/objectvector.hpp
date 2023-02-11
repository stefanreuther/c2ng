/**
  *  \file game/map/objectvector.hpp
  *  \brief Template class game::map::ObjectVector
  */
#ifndef C2NG_GAME_MAP_OBJECTVECTOR_HPP
#define C2NG_GAME_MAP_OBJECTVECTOR_HPP

#include "afl/container/ptrvector.hpp"
#include "game/types.hpp"

namespace game { namespace map {

    /** Vector of objects.
        The primary purpose of this class is to store classes derived from Object,
        but it can store everything that can be constructed from a single Id_t parameter.
        It provides creation and retrieval of objects using a 1-based index.

        @tparam T content type */
    template<typename T>
    class ObjectVector {
     public:
        /** Create object.
            If an object with that Id already exists, returns it; otherwise, creates one.
            The object is owned by ObjectVector.
            @param id Id (> 0)
            @return Object; null if Id is invalid (<= 0). */
        T* create(Id_t id);

        /** Get object.
            @param id Id
            @return object with the given Id; null if Id is invalid or was never created */
        T* get(Id_t id) const;

        /** Clear.
            Deletes all objects.
            @post size() == 0 */
        void clear();

        /** Get size.
            Returns highest possibly existing Id.
            For all values greater than that, get() will return 0.
            @return highest possible existing Id */
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
