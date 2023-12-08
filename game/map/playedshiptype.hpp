/**
  *  \file game/map/playedshiptype.hpp
  *  \brief Class game::map::PlayedShipType
  */
#ifndef C2NG_GAME_MAP_PLAYEDSHIPTYPE_HPP
#define C2NG_GAME_MAP_PLAYEDSHIPTYPE_HPP

#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "game/map/ship.hpp"

namespace game { namespace map {

    /** Played ships type.
        Contains all ships that can be played (ReadOnly or better). */
    class PlayedShipType : public ObjectVectorType<Ship> {
     public:
        /** Constructor.
            @param vec Ships */
        explicit PlayedShipType(ObjectVector<Ship>& vec);

        // ObjectVectorType:
        virtual bool isValid(const Ship& s) const;

        /** Count capital ships.
            @return number of capital ships
            @see Ship::hasWeapons */
        int countCapitalShips() const;
    };

} }

#endif
