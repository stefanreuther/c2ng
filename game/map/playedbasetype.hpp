/**
  *  \file game/map/playedbasetype.hpp
  *  \brief Class game::map::PlayedBaseType
  */
#ifndef C2NG_GAME_MAP_PLAYEDBASETYPE_HPP
#define C2NG_GAME_MAP_PLAYEDBASETYPE_HPP

#include "game/map/objectvectortype.hpp"
#include "game/map/planet.hpp"

namespace game { namespace map {

    class Planet;
    class Universe;

    /** Played starbases type.
        Contains all playable starbases. */
    class PlayedBaseType : public ObjectVectorType<Planet> {
     public:
        PlayedBaseType(Universe& univ);

        virtual bool isValid(const Planet& p) const;
    };

} }

#endif
