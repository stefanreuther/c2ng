/**
  *  \file game/map/objectvectortype.hpp
  *  \brief Template class game::map::ObjectVectorType
  *
  *  This is the successor-in-spirit to GIndexedObjectType,
  *  although the job-split between base class and descendants is different.
  */
#ifndef C2NG_GAME_MAP_OBJECTVECTORTYPE_HPP
#define C2NG_GAME_MAP_OBJECTVECTORTYPE_HPP

#include "game/map/objecttype.hpp"
#include "game/map/objectvector.hpp"

namespace game { namespace map {

    template<typename T>
    class ObjectVectorType : public ObjectType {
     public:
        ObjectVectorType(ObjectVector<T>& vec);
        ~ObjectVectorType();

        virtual T* getObjectByIndex(Id_t index);
        virtual int getNextIndex(Id_t index) const;
        virtual int getPreviousIndex(Id_t index) const;

        virtual bool isValid(const T& obj) const = 0;

     private:
        ObjectVector<T>& m_vector;
    };

} }

template<typename T>
inline
game::map::ObjectVectorType<T>::ObjectVectorType(ObjectVector<T>& vec)
    : m_vector(vec)
{ }

template<typename T>
inline
game::map::ObjectVectorType<T>::~ObjectVectorType()
{ }

template<typename T>
T*
game::map::ObjectVectorType<T>::getObjectByIndex(Id_t index)
{
    if (T* p = m_vector.get(index)) {
        if (isValid(*p)) {
            return p;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

template<typename T>
game::Id_t
game::map::ObjectVectorType<T>::getNextIndex(Id_t index) const
{
    // ex GIndexedObjectType::getNextIndex
    return index < m_vector.size() ? index+1 : 0;
}

template<typename T>
game::Id_t
game::map::ObjectVectorType<T>::getPreviousIndex(Id_t index) const
{
    // ex GIndexedObjectType::getPreviousIndex
    return index > 0 ? index-1 : m_vector.size();
}

#endif
