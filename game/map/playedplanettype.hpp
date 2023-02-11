/**
  *  \file game/map/playedplanettype.hpp
  *  \brief Class game::map::PlayedPlanetType
  */
#ifndef C2NG_GAME_MAP_PLAYEDPLANETTYPE_HPP
#define C2NG_GAME_MAP_PLAYEDPLANETTYPE_HPP

#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "game/map/planet.hpp"

namespace game { namespace map {

    /** Played planets type.
        Contains all planets that can be played (ReadOnly or better). */
    class PlayedPlanetType : public ObjectVectorType<Planet> {
     public:
        /** Constructor.
            @param vec Planet vector */
        explicit PlayedPlanetType(ObjectVector<Planet>& vec);

        // ObjectVectorType:
        virtual bool isValid(const Planet& p) const;
    };

} }

#endif
