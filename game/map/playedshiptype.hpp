/**
  *  \file game/map/playedshiptype.hpp
  */
#ifndef C2NG_GAME_MAP_PLAYEDSHIPTYPE_HPP
#define C2NG_GAME_MAP_PLAYEDSHIPTYPE_HPP

#include "game/map/objectvectortype.hpp"
#include "game/map/ship.hpp"

namespace game { namespace map {

    class Ship;
    class Universe;

    /** Played ships type.
        Contains all ships. */
    class PlayedShipType : public ObjectVectorType<Ship> {
     public:
        PlayedShipType(Universe& univ);

        virtual bool isValid(const Ship& p) const;
    };

} }

#endif
