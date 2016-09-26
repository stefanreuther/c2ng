/**
  *  \file game/spec/componentvector.hpp
  */
#ifndef C2NG_GAME_SPEC_COMPONENTVECTOR_HPP
#define C2NG_GAME_SPEC_COMPONENTVECTOR_HPP

#include "afl/container/ptrvector.hpp"
#include "game/spec/basecomponentvector.hpp"

namespace game { namespace spec {

    template<typename T>
    class ComponentVector : public BaseComponentVector {
     public:
        T* create(int id);

        T* get(int id) const
            { return static_cast<T*>(BaseComponentVector::get(id)); }

        T* findNext(int id) const;
    };

    class Hull;
    class Beam;
    class Engine;
    class Torpedo;
    typedef ComponentVector<Hull> HullVector_t;
    typedef ComponentVector<Beam> BeamVector_t;
    typedef ComponentVector<Engine> EngineVector_t;
    typedef ComponentVector<Torpedo> TorpedoVector_t;

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
