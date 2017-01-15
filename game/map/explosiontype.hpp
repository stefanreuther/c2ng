/**
  *  \file game/map/explosiontype.hpp
  */
#ifndef C2NG_GAME_MAP_EXPLOSIONTYPE_HPP
#define C2NG_GAME_MAP_EXPLOSIONTYPE_HPP

#include "game/map/objecttype.hpp"
#include "game/map/explosion.hpp"
#include "afl/container/ptrvector.hpp"

namespace game { namespace map {

    class Universe;

    class ExplosionType : public ObjectType {
     public:
        ExplosionType(Universe& univ);
        ~ExplosionType();

        void add(const Explosion& ex);

        // ObjectType:
        Explosion* getObjectByIndex(Id_t index);
        Universe* getUniverseByIndex(Id_t index);
        Id_t getNextIndex(Id_t index) const;
        Id_t getPreviousIndex(Id_t index) const;

     private:
        Universe& m_universe;

        afl::container::PtrVector<Explosion> m_explosions;
    };

} }

#endif
