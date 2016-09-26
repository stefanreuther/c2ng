/**
  *  \file game/map/objectreference.hpp
  */
#ifndef C2NG_GAME_MAP_OBJECTREFERENCE_HPP
#define C2NG_GAME_MAP_OBJECTREFERENCE_HPP

#include "game/types.hpp"

namespace game { namespace map {

    class Object;
    class ObjectType;
    class Universe;

    class ObjectReference {
     public:
        ObjectReference();
        ObjectReference(ObjectType& type, Id_t index);

        // Use default operator=, copy constructor

        bool isValid() const;
        ObjectType* getObjectType() const;
        Universe*   getUniverse() const;

        Id_t getObjectIndex() const;

        Object* get() const;

        bool operator==(const ObjectReference& o) const;
        bool operator!=(const ObjectReference& o) const;

     private:
        ObjectType* m_type;
        Id_t m_index;
    };

} }

#endif
