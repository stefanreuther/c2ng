/**
  *  \file game/map/typedobjecttype.hpp
  *  \brief Template class game::map::TypedObjectType
  */
#ifndef C2NG_GAME_MAP_TYPEDOBJECTTYPE_HPP
#define C2NG_GAME_MAP_TYPEDOBJECTTYPE_HPP

#include "game/map/objecttype.hpp"

namespace game { namespace map {

    /** Typed ObjectType instance.
        Specializes/narrows the return type of getObjectByIndex() to an Object descendant.

        \tparam T contained object type, must be derived from game::map::Object */
    template<typename T>
    class TypedObjectType : public ObjectType {
     public:
        /** Get object, given an index.
            \param index Index
            \return Object or null */
        virtual T* getObjectByIndex(Id_t index) = 0;
    };

} }

#endif
