/**
  *  \file game/map/anyshiptype.hpp
  */
#ifndef C2NG_GAME_MAP_ANYSHIPTYPE_HPP
#define C2NG_GAME_MAP_ANYSHIPTYPE_HPP

#include "game/map/objectvectortype.hpp"
#include "game/map/ship.hpp"

namespace game { namespace map {

    class Ship;
    class Universe;

    /** Any ships type.
        Contains all ships. */
    class AnyShipType : public ObjectVectorType<Ship> {
     public:
        AnyShipType(Universe& univ);

        virtual bool isValid(const Ship& p) const;
    };

} }

#endif
