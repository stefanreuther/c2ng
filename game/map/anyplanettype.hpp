/**
  *  \file game/map/anyplanettype.hpp
  *  \brief Class game::map::AnyPlanetType
  */
#ifndef C2NG_GAME_MAP_ANYPLANETTYPE_HPP
#define C2NG_GAME_MAP_ANYPLANETTYPE_HPP

#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "game/map/planet.hpp"

namespace game { namespace map {

    /** Played planets type.
        Contains all planets. */
    class AnyPlanetType : public ObjectVectorType<Planet> {
     public:
        AnyPlanetType(ObjectVector<Planet>& vec);

        virtual bool isValid(const Planet& p) const;
    };

} }

#endif
