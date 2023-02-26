/**
  *  \file game/spec/componentvector.hpp
  *  \brief Class game::spec::ComponentVector
  */
#ifndef C2NG_GAME_SPEC_COMPONENTVECTOR_HPP
#define C2NG_GAME_SPEC_COMPONENTVECTOR_HPP

#include "game/spec/basecomponentvector.hpp"

namespace game { namespace spec {

    /** Vector of typed Component objects.
        Implements a 1-based vector of Component objects of a given type.

        @tparam T Element type, derived from Component, must be constructible from an integer. */
    template<typename T>
    class ComponentVector : public BaseComponentVector {
     public:
        /** Create object.
            If the object already exists, returns it.
            Otherwise, creates a new one.
            @param id Id (> 0)
            @return object; null if Id is invalid */
        T* create(int id);

        /** Get object.
            @param id Id
            @return object; null if object was never created or Id is invalid */
        T* get(int id) const
            { return static_cast<T*>(BaseComponentVector::get(id)); }

        /** Find next component, given an Id.
            @param id Starting point. 0=get first component.
            @return Component with getId() > id; null if nonw. */
        T* findNext(int id) const;
    };

    class Hull;
    class Beam;
    class Engine;
    class TorpedoLauncher;
    typedef ComponentVector<Hull> HullVector_t;
    typedef ComponentVector<Beam> BeamVector_t;
    typedef ComponentVector<Engine> EngineVector_t;
    typedef ComponentVector<TorpedoLauncher> TorpedoVector_t;

} }


template<typename T>
T*
game::spec::ComponentVector<T>::create(int id)
{
    if (id > 0) {
        T* result = get(id);
        if (result == 0) {
            result = new T(id);
            setNew(id, result);
        }
        return result;
    } else {
        return 0;
    }
}

template<typename T>
T*
game::spec::ComponentVector<T>::findNext(int id) const
{
    return static_cast<T*>(BaseComponentVector::findNext(id));
}

#endif
