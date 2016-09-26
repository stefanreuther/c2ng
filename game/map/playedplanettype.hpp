/**
  *  \file game/map/playedplanettype.hpp
  *  \brief Class game::map::PlayedPlanetType
  */
#ifndef C2NG_GAME_MAP_PLAYEDPLANETTYPE_HPP
#define C2NG_GAME_MAP_PLAYEDPLANETTYPE_HPP

#include "game/map/objectvectortype.hpp"
#include "game/map/planet.hpp"

namespace game { namespace map {

    class Planet;
    class Universe;
    
    /** Played planets type.
        Contains all planets that can be played. */
    class PlayedPlanetType : public ObjectVectorType<Planet> {
     public:
        PlayedPlanetType(Universe& univ);

        virtual bool isValid(const Planet& p) const;
    };

} }

#endif
