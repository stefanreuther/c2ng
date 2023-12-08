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

    /** All planets type.
        Contains all visible planets. */
    class AnyPlanetType : public ObjectVectorType<Planet> {
     public:
        /** Constructor.
            @param vec Planet vector */
        explicit AnyPlanetType(ObjectVector<Planet>& vec);

        // ObjectVectorType:
        virtual bool isValid(const Planet& p) const;
    };

} }

#endif
